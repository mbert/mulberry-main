/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/* 
	CCalendarStoreManager.cpp

	Author:			
	Description:	<describe the CCalendarStoreManager class here>
*/

#include "CCalendarStoreManager.h"

#include "CAdminLock.h"
#include "CCalendarStoreTable.h"
#include "CCalendarStoreView.h"
#include "CCalendarStoreWebcal.h"
#include "CCalendarView.h"
#include "CConnectionManager.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CTextListChoice.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"

#include "cdmutexprotect.h"

#include <algorithm>
#include <memory>

using namespace calstore; 

CCalendarStoreManager* CCalendarStoreManager::sCalendarStoreManager = NULL;

CCalendarStoreManager::CCalendarStoreManager()
{
	mLocalProto = NULL;
	mWebProto = NULL;
	sCalendarStoreManager = this;
	mProtos.set_delete_data(false);
	mProtoCount = 0;
	
	// Must inform any calendar store manager windows that currently exist that
	// the calendar store manager is now here so that protocol change broadcasts work
	UpdateWindows();
}

CCalendarStoreManager::~CCalendarStoreManager()
{
	// Remove all server views now to prevent illegal updates to 'stale' windows
	{
		cdmutexprotect<CCalendarStoreView::CCalendarStoreViewList>::lock _lock(CCalendarStoreView::sCalendarStoreViews);
		for(CCalendarStoreView::CCalendarStoreViewList::iterator iter = CCalendarStoreView::sCalendarStoreViews->begin(); iter != CCalendarStoreView::sCalendarStoreViews->end(); iter++)
			(*iter)->DoClose();
	}

	// Logoff each protocol (includes the local one)
	for(CCalendarProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		StopProtocol(*iter);
		RemoveProtocol(*iter);
	}

	sCalendarStoreManager = NULL;
}

// Update all windows for changes
void CCalendarStoreManager::UpdateWindows()
{
	// Iterate over all Calendar manager windows
	cdmutexprotect<CCalendarStoreView::CCalendarStoreViewList>::lock _lock1(CCalendarStoreView::sCalendarStoreViews);
	for(CCalendarStoreView::CCalendarStoreViewList::iterator iter = CCalendarStoreView::sCalendarStoreViews->begin(); iter != CCalendarStoreView::sCalendarStoreViews->end(); iter++)
		// Force visual update
		(*iter)->GetTable()->SetManager(this);

	// Iterate over all Calendar views
	cdmutexprotect<CCalendarView::CCalendarViewList>::lock _lock2(CCalendarView::sCalendarViews);
	for(CCalendarView::CCalendarViewList::iterator iter = CCalendarView::sCalendarViews->begin(); iter != CCalendarView::sCalendarViews->end(); iter++)
		// Force visual update
		Add_Listener(*iter);
}

// Add a new Calendar remote protocol
void CCalendarStoreManager::AddProtocol(CCalendarProtocol* proto)
{
	// Add item to root node
	mRoot.AddChild(proto->GetStoreRoot());

	// Add to protos list
	mProtos.push_back(proto);
	mProtoCount++;

	// Broadcast change to all after adding
	Broadcast_Message(eBroadcast_NewCalendarAccount, proto);
}

// Remove an Calendar remote protocol
void CCalendarStoreManager::RemoveProtocol(CCalendarProtocol* proto)
{
	// Broadcast change to all before delete
	Broadcast_Message(eBroadcast_RemoveCalendarAccount, proto);

	// Remove node from parent
	proto->GetStoreRoot()->RemoveFromParent();

	// Delete protocol (will also delete the node)
	delete proto;
	proto = NULL;
}

// Get protocol
calstore::CCalendarProtocol* CCalendarStoreManager::GetProtocol(const cdstring& name)
{
	for(CCalendarProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		if ((*iter)->GetAccountName() == name)
			return *iter;
	}

	return NULL;
}

// Sync with changed accounts
void CCalendarStoreManager::SyncAccounts()
{
	// Only add local protocol if allowed by admin
	if (!CAdminLock::sAdminLock.mNoLocalCalendars)
	{
		// First check if local exists
		if (mLocalProto != NULL)
		{
			// Sync its account with prefs etc
			UpdateProtocol(mLocalProto);

			// Resync account object
			mLocalProto->SetAccount(&CPreferences::sPrefs->mLocalCalendarAccount.Value());
		}
		else
		{
			// Create new local protocol
			mLocalProto = new CCalendarProtocol(&CPreferences::sPrefs->mLocalCalendarAccount.Value());
			mLocalProto->SetDescriptor(CPreferences::sPrefs->mLocalCalendarAccount.GetValue().GetName());
			AddProtocol(mLocalProto);
		}
	}

	// Only add web protocol if allowed by admin and login at startup is on
	if (!CAdminLock::sAdminLock.mNoLocalCalendars)
	{
		// First check if local exists
		if (mWebProto != NULL)
		{
			// Sync its account with prefs etc
			UpdateProtocol(mWebProto);

			// Resync account object
			mWebProto->SetAccount(&CPreferences::sPrefs->mWebCalendarAccount.Value());
		}
		else
		{
			// Create new local protocol
			mWebProto = new CCalendarProtocol(&CPreferences::sPrefs->mWebCalendarAccount.Value());
			mWebProto->SetDescriptor(CPreferences::sPrefs->mWebCalendarAccount.GetValue().GetName());
			
			// Check whether any items exist
			if (mWebProto->HasDisconnectedCalendars())
				// Add protocol only if items exist
				AddProtocol(mWebProto);
			else
			{
				// Don't use webcal if no items exist
				delete mWebProto;
				mWebProto = NULL;
			}
		}
	}

	// Sync remainder from prefs
	const CCalendarAccountList& accts = CPreferences::sPrefs->mCalendarAccounts.GetValue();

	// New smart alogorithm: compare protocol and protocol embedded in account

	// This maintains a list of protocols that need to be started
	CCalendarProtocolList started;
	started.set_delete_data(false);

	// First delete protocols not in accounts
	unsigned long proto_offset = 0;
	if (mLocalProto != NULL)
		proto_offset++;
	if (mWebProto != NULL)
		proto_offset++;
	for(CCalendarProtocolList::iterator iter1 = mProtos.begin() + proto_offset; iter1 != mProtos.end();)
	{
		bool found = false;

		// Look for match between existing protocol and new accounts
		for(CCalendarAccountList::const_iterator iter2 = accts.begin(); !found && (iter2 != accts.end()); iter2++)
		{
			if ((*iter2)->GetProtocol() == *iter1)
				found = true;
		}

		// Remove protocol if no matching account
		if (!found)
		{
			StopProtocol(*iter1);
			RemoveProtocol(*iter1);

			// Remove from list
			iter1 = mProtos.erase(iter1);
			mProtoCount--;
			continue;
		}
		
		iter1++;
	}

	// Now resync accounts with existing protocols and add new protocols
	for(CCalendarAccountList::const_iterator iter2 = accts.begin(); iter2 != accts.end(); iter2++)
	{
		bool found = false;

		// Look for protocol matching account
		for(CCalendarProtocolList::iterator iter1 = mProtos.begin() + proto_offset; !found && (iter1 != mProtos.end()); iter1++)
		{
			if ((*iter2)->GetProtocol() == *iter1)
			{
				found = true;

				// May need to redo server and/or authenticator
				if (((*iter1)->GetAccountUniqueness() != (*iter2)->GetUniqueness()) ||
					((*iter1)->GetAuthenticatorUniqueness() != (*iter2)->GetAuthenticator().GetUniqueness()))
					UpdateProtocol(*iter1);

				// Resync account object
				(*iter1)->SetAccount(*iter2);

				// Always do logon - protocol will handle forced disconnect state
				if (!(*iter1)->IsLoggedOn() && ((*iter1)->IsOffline() || (*iter2)->GetLogonAtStart()))
					started.push_back(*iter1);
			}
		}

		// If not found add new protocol
		if (!found)
		{
			// Create a protocol for each account and add to list
			CCalendarProtocol* aCalendarProtocol = new CCalendarProtocol(*iter2);
			if (aCalendarProtocol != NULL)
			{
				try
				{
					aCalendarProtocol->SetDescriptor((*iter2)->GetServerIP());
					AddProtocol(aCalendarProtocol);

					// Always do logon - protocol will handle forced disconnect state
					if (aCalendarProtocol->IsOffline() || (*iter2)->GetLogonAtStart())
						started.push_back(aCalendarProtocol);
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Do not allow to fail
				}
			}
		}
	}

	// Do all protocol starts here once the entire list of accounts is consistent
	

	// Only start local protocols if allowed by admin
	if (mLocalProto != NULL)
		StartProtocol(mLocalProto);
	if (mWebProto != NULL)
		StartProtocol(mWebProto);

	for(CCalendarProtocolList::const_iterator iter = started.begin(); iter != started.end(); iter++)
		StartProtocol(*iter);

	// Make sure 3-pane is told to re-instate previously open calendars
	if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->DoneInitCalendarAccounts();
}

// Start remote protocol
void CCalendarStoreManager::StartProtocol(CCalendarProtocol* proto, bool silent)
{
	// Only do if not already logged on
	if (proto->IsLoggedOn())
		return;

	// Check for a non-disconnectable proto when disconnected
	if (!CConnectionManager::sConnectionManager.IsConnected() &&
		!proto->IsOffline() && !proto->CanDisconnect())
		return;

	try
	{
		// Open protocol and get Calendars
		if (CMulberryApp::sApp->BeginINET(proto))
		{
			proto->Logon();
			proto->LoadList();
			proto->SyncList();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		if (!silent)
		{
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

// Update remote protocol
void CCalendarStoreManager::UpdateProtocol(CCalendarProtocol* proto)
{
	try
	{
		// Logoff protocol (clears Calendars)
		proto->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Stop remote protocol
void CCalendarStoreManager::StopProtocol(CCalendarProtocol* proto)
{
	try
	{
		// Logoff protocol (clears Calendars)
		proto->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	// Always force table to logged off state
	int pos = 0;

#ifdef _TODO
	if (FindProtocol(proto, pos))
	{
		TableIndexT temp = pos;
		// Iterate over all Calendar manager windows
		cdmutexprotect<CCalendarStoreView::CCalendarStoreViewList>::lock _lock(CCalendarStoreView::sCalendarStoreViews);
		for(CCalendarStoreView::CCalendarStoreViewList::iterator iter = CCalendarStoreView::sCalendarStoreViews->begin(); iter != CCalendarStorerView::sCalendarStoreViews->end(); iter++)
			(*iter)->GetTable()->RemoveChildren(temp, true);
	}
#endif
}

// Get index of protocol in list
long CCalendarStoreManager::GetProtocolIndex(const CCalendarProtocol* proto) const
{
	CCalendarProtocolList::const_iterator found = std::find(mProtos.begin(), mProtos.end(), proto);
	if (found != mProtos.end())
		return found - mProtos.begin();
	else
		return -1;
}

bool CCalendarStoreManager::FindProtocol(const CCalendarProtocol* proto, unsigned long& pos) const
{
	bool result = false;
	pos = 1;

	for(CCalendarStoreNodeList::const_iterator iter = mRoot.GetChildren()->begin(); iter != mRoot.GetChildren()->end(); iter++, pos++)
	{
		if ((*iter)->GetProtocol() == proto)
		{
			result = true;
			break;
		}
		pos += (*iter)->CountDescendants();
	}

	return result;
}

// Move protocol
void CCalendarStoreManager::MoveProtocol(long old_index, long new_index)
{
	// Cannot move local or web
	if (((mLocalProto != NULL) || (mWebProto != NULL)) && (old_index == 0))
		return;
	else if ((mLocalProto != NULL) && (mWebProto != NULL) && (old_index == 1))
		return;
	
	// Determine index adjustment offset to remove local/web protos
	uint32_t proto_offset = 0;
	if (mLocalProto != NULL)
		proto_offset++;
	if (mWebProto != NULL)
		proto_offset++;

	// Cannot move above local or web
	if (new_index < proto_offset)
		new_index = proto_offset;

	CCalendarProtocol* proto = mProtos.at(old_index);

	// Broadcast change to all to remove it
	Broadcast_Message(eBroadcast_RemoveCalendarAccount, proto);

	// Remove node from parent
	proto->GetStoreRoot()->RemoveFromParent();

	// Adjust count
	if (old_index < new_index)
		new_index--;

	// Move protocol
	mProtos.erase(mProtos.begin() + old_index);
	if (new_index < (long) mProtos.size())
	{
		// Add item to root node
		mRoot.InsertChild(proto->GetStoreRoot(), new_index);

		// Add to protos list
		mProtos.insert(mProtos.begin() + new_index, proto);
	}
	else
	{
		// Add item to root node
		mRoot.AddChild(proto->GetStoreRoot());

		// Add to protos list
		mProtos.push_back(proto);
	}

	// Adjust indices to remove local/web
	old_index -= proto_offset;
	new_index -= proto_offset;

	// Move mail account
	CCalendarAccountList& accts = CPreferences::sPrefs->mCalendarAccounts.Value();
	CCalendarAccount* acct = accts.at(old_index);
	accts.at(old_index) = NULL;
	accts.erase(accts.begin() + old_index);
	if (new_index < (long) mProtos.size() - 1 - proto_offset)
		accts.insert(accts.begin() + new_index, acct);
	else
		accts.push_back(acct);
	CPreferences::sPrefs->mCalendarAccounts.SetDirty();

	// Add it back to visual display in new place
	Broadcast_Message(eBroadcast_InsertCalendarAccount, proto);
}

uint32_t cCalendarColours[] =
{
	0x0066B866,		// Green
	0x006666B8,		// Blue
	0x00CC6666,		// Red
	0x00B8B866,		// Yellow
	0x0066B8B8,		// Cyan
	0x00B866B8		// Magenta
};

uint32_t CCalendarStoreManager::GetColourFromIndex(uint32_t index) const
{
	if (index < 6)
		return cCalendarColours[index];
	else
	{
		// Use different saturations
		uint32_t saturated = cCalendarColours[index % 6];
		
		// Desaturate
		uint32_t level = index / 6;
		if (level > 13)
			level = 13;
		level |= (level << 8);
		
		if ((saturated & 0x000000FF) == 0)
			saturated |= level;
		level = level << 8;
		if ((saturated & 0x0000FF00) == 0)
			saturated |= level;
		level = level << 8;
		if ((saturated & 0x00FF0000) == 0)
			saturated |= level;
		
		return saturated;
	}
}

const CCalendarStoreNode* CCalendarStoreManager::GetNode(const iCal::CICalendar* cal) const
{
	return mRoot.FindNode(cal);
}

const CCalendarStoreNode* CCalendarStoreManager::GetNode(const cdstring& cal) const
{
	const CCalendarStoreNode* result = NULL;
	for(CCalendarProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		result = (*iter)->GetNode(cal);
		if (result != NULL)
			return result;
	}
	
	return NULL;
}

const CCalendarStoreNode* CCalendarStoreManager::GetNodeByRemoteURL(const cdstring& url) const
{
	// Double-check admin lock
	if (CAdminLock::sAdminLock.mNoLocalCalendars || (mWebProto == NULL))
		return NULL;
	else
		return mWebProto->GetNodeByRemoteURL(url);
}

int32_t	CCalendarStoreManager::GetActiveIndex(const iCal::CICalendar* cal) const
{
	iCal::CICalendarList::const_iterator found = std::find(mActiveCalendars.begin(), mActiveCalendars.end(), cal);
	if (found != mActiveCalendars.end())
		return found - mActiveCalendars.begin();
	else
		return -1;
}

uint32_t CCalendarStoreManager::GetCalendarColour(const iCal::CICalendar* cal) const
{
	const CCalendarStoreNode* node = GetNode(cal);
 	if (node == NULL)
		return 0;
	else
		return GetCalendarColour(node);
}

uint32_t CCalendarStoreManager::GetCalendarColour(const CCalendarStoreNode* node) const
{
	// Look it up in the preferences
	cdstring acctname = node->GetAccountName();
	cdstruint32map::const_iterator found = CPreferences::sPrefs->mCalendarColours.GetValue().find(acctname);
	if (found != CPreferences::sPrefs->mCalendarColours.GetValue().end())
		return (*found).second;
	
	// Must assign new colour
	uint32_t current_size = CPreferences::sPrefs->mCalendarColours.GetValue().size();
	return GetColourFromIndex(current_size + 1);
}

// Managing the store
CCalendarStoreNode* CCalendarStoreManager::NewCalendar(CCalendarProtocol* proto, const cdstring& name, bool directory, const cdstring& remote_url)
{
	if ((proto == NULL) || name.empty())
		return NULL;

	// Create account name
	cdstring acct_name = proto->GetAccountName();
	acct_name += cMailAccountSeparator;
	acct_name += name;

	// Determine the parent node for the new one
	CCalendarStoreNode* parent = proto->GetParentNode(acct_name);
	if (parent == NULL)
		return NULL;

	// Create a new node
	CCalendarStoreNode* node = new CCalendarStoreNode(proto, parent, directory, false, false, name);
	if (!remote_url.empty())
		node->SetRemoteURL(remote_url);
	parent->AddChild(node, true);

	try
	{
		// Try to create it
		node->GetProtocol()->CreateCalendar(*node);

		// Make sure protocol list is in sync after create
		node->GetProtocol()->ListChanged();

		// Write out an empty calendar with the appropriate name
		if (!directory)
		{
			std::auto_ptr<iCal::CICalendar> cal(new iCal::CICalendar);
			
			// Give it to the node - this will activate the node
			node->SetCalendar(cal.get());
			
			// Change the name in the calendar
			cal->EditName(node->GetShortName());
			
			// Now write the calendar data
			node->GetProtocol()->WriteFullCalendar(*node, *cal);
			
			// Clear out calendar
			node->SetCalendar(NULL);
		}

		// Now insert into parent
		Broadcast_Message(eBroadcast_InsertNode, node);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Clean-up
		node->SetCalendar(NULL);
		node->RemoveFromParent();
		delete node;
		
		CLOG_LOGRETHROW;
		throw;
	}

	return node;
}

void CCalendarStoreManager::RenameCalendar(CCalendarStoreNode* node, const cdstring& new_name)
{
	// Cache account name for preference change
	cdstring old_acct_name = node->GetAccountName();

	// Now rename the actual node
	node->GetProtocol()->RenameCalendar(*node, new_name);
	
	// Change the node name locally
	node->NewName(new_name);
	cdstring new_acct_name = node->GetAccountName();

	// Rename in preferences
	CPreferences::sPrefs->RenameCalendar(old_acct_name, new_acct_name);

	// Check for actual calendar rather than directory
	if (!node->IsDirectory())
	{
		// If the calendar is open rename the calendar object and do an immediate write
		iCal::CICalendar* cal = node->GetCalendar();
		std::auto_ptr<iCal::CICalendar> opened;
		try
		{
			if (cal == NULL)
			{
				opened.reset(new iCal::CICalendar);
				cal = opened.get();
				
				// Give it to the node - this will activate the node
				node->SetCalendar(cal);
				
				// Now read the calendar data
				node->GetProtocol()->ReadFullCalendar(*node, *cal);
			}
			
			// Change the name in the calendar
			cal->EditName(node->GetShortName());
			
			// Now write the calendar data
			node->GetProtocol()->WriteFullCalendar(*node, *cal);
			
			// Clean-up
			if (opened.get() != NULL)
			{
				node->SetCalendar(NULL);
			}
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
			
			// Clean-up
			if (opened.get() != NULL)
			{
				node->SetCalendar(NULL);
			}
			
			CLOG_LOGRETHROW;
			throw;
		}
	}
	
	// Now adjust node position
	
	// First remove it from any listeners whilst it is still linked in to parent
	Broadcast_Message(eBroadcast_RemoveNode, node);
	
	// Now remove it from its parent and re-insert in proper location
	node->RemoveFromParent();
	
	CCalendarStoreNode* parent_node = node->GetProtocol()->GetParentNode(node->GetAccountName());
	if (parent_node)
	{
		parent_node->AddChild(node, true);
		Broadcast_Message(eBroadcast_InsertNode, node);
	}

	// Make sure protocol list is in sync after rename
	node->GetProtocol()->ListChanged();

}

void CCalendarStoreManager::DeleteCalendar(CCalendarStoreNode* node)
{
	// Remove from subscriptions
	if (node->IsSubscribed())
		SubscribeNode(node, false, false);

	// Deactivate the node first if it is active
	if (node->IsActive())
		DeactivateNode(node);

	// Remove it from any listeners whilst it is still linked in to parent
	Broadcast_Message(eBroadcast_DeleteNode, node);

	// Now remove from parent's list
	node->RemoveFromParent();

	// Cache account name for preference change
	cdstring acct_name = node->GetAccountName();

	// Now remove the actual node (node is deleted after this)
	node->GetProtocol()->DeleteCalendar(*node);
	
	// Make sure protocol list is in sync after rename
	node->GetProtocol()->ListChanged();

	// Remove tha ctual object
	delete node;

	// Remove from preferences
	CPreferences::sPrefs->DeleteCalendar(acct_name);
}

void CCalendarStoreManager::SaveAllCalendars()
{
	// Save all active calendars
	for(iCal::CICalendarList::iterator iter = mActiveCalendars.begin(); iter != mActiveCalendars.end(); iter++)
	{
		const CCalendarStoreNode* node = GetNode(*iter);

		// Must write out the calendar but only if dirty
		if ((node != NULL) && ((*iter)->IsDirty()))
			node->GetProtocol()->WriteFullCalendar(*node, **iter);
	}
}

void CCalendarStoreManager::NewWebCalendar(const cdstring& url)
{
	// Double-check admin lock
	if (CAdminLock::sAdminLock.mNoLocalCalendars)
		return;

	// Check whether web proto exists
	CreateWebAccount();
	
	// Name is last part of url
	cdstring name;
	const char* p = NULL;
	if ((p = ::strrchr(url.c_str(), '/')) != NULL)
		name = ++p;
	else
		name = url;
	
	// Create a new node in the web account
	CCalendarStoreNode* node = NewCalendar(mWebProto, name, false, url);
	if (node == NULL)
		return;
	
	// Subscribed webcal's always start off read-only
	if (node->GetWebcal())
		node->GetWebcal()->SetReadOnly(true);
	
	// Now force it to download from server
	calstore::CCalendarStoreManager::StNodeActivate _activate(node);
	if (node->GetCalendar() != NULL)
		node->GetProtocol()->SubscribeFullCalendar(*node, *node->GetCalendar());
}

// Create the web proto
void CCalendarStoreManager::CreateWebAccount()
{
	// Check if already present
	if (mWebProto != NULL)
		return;
	
	// Make sure login at startup is on so that the account is recreated on next restart
	if (!CPreferences::sPrefs->mWebCalendarAccount.GetValue().GetLogonAtStart())
	{
		CPreferences::sPrefs->mWebCalendarAccount.Value().SetLoginAtStart(true);
		CPreferences::sPrefs->mWebCalendarAccount.SetDirty();
	}
		
	// Create new local protocol
	mWebProto = new CCalendarProtocol(&CPreferences::sPrefs->mWebCalendarAccount.Value());
	mWebProto->SetDescriptor(CPreferences::sPrefs->mWebCalendarAccount.GetValue().GetName());
	
	// Add it after the local proto if that exists
	uint32_t proto_offset = 0;
	if (mLocalProto)
		proto_offset++;

	// Add item to root node
	mRoot.InsertChild(mWebProto->GetStoreRoot(), proto_offset);

	// Add to protos list
	mProtos.insert(mProtos.begin() + proto_offset, mWebProto);
	mProtoCount++;

	// Add it back to visual display in new place
	Broadcast_Message(eBroadcast_InsertCalendarAccount, mWebProto);

	// Start it	
	StartProtocol(mWebProto);
}

// Managing nodes
void CCalendarStoreManager::SubscribeNode(CCalendarStoreNode* node, bool subs, bool no_prefs_update)
{
	// Only if changing
	if (node->IsSubscribed() ^ subs)
	{
		node->SetSubscribed(subs);
		if (node->IsSubscribed())
		{
			ActivateNode(node);
			
			// Add to subscribed list
			mSubscribedCalendars.push_back(node->GetCalendar());
		}
		else
		{
			// Remove from subscribed list
			mSubscribedCalendars.erase(std::remove(mSubscribedCalendars.begin(), mSubscribedCalendars.end(), node->GetCalendar()), mSubscribedCalendars.end());
			
			DeactivateNode(node);
		}
		
		cdstring acctname = node->GetAccountName();

		// Update preferences
		if (!no_prefs_update)
		{
			if (subs)
			{
				CPreferences::sPrefs->mSubscribedCalendars.Value().insert(acctname);
				CPreferences::sPrefs->mSubscribedCalendars.SetDirty();
			}
			else
			{
				CPreferences::sPrefs->mSubscribedCalendars.Value().erase(acctname);
				CPreferences::sPrefs->mSubscribedCalendars.SetDirty();
			}
		}
		
		// Make sure colour is assigned in prefs
		cdstruint32map::const_iterator found = CPreferences::sPrefs->mCalendarColours.GetValue().find(acctname);
		if (found == CPreferences::sPrefs->mCalendarColours.GetValue().end())
		{
			uint32_t colour = GetCalendarColour(node);
			CPreferences::sPrefs->mCalendarColours.Value().insert(cdstruint32map::value_type(acctname, colour));
			CPreferences::sPrefs->mCalendarColours.SetDirty();
		}

		// Broadcast change
		Broadcast_Message(node->IsSubscribed() ? eBroadcast_SubscribeNode : eBroadcast_UnsubscribeNode, node);
	}
}

void CCalendarStoreManager::ChangeNodeColour(CCalendarStoreNode* node, uint32_t colour)
{
	cdstring acctname = node->GetAccountName();

	// Look for node in prefs
	cdstruint32map::iterator found = CPreferences::sPrefs->mCalendarColours.Value().find(acctname);
	if (found != CPreferences::sPrefs->mCalendarColours.Value().end())
	{
		// If different change the preference value
		if ((*found).second != colour)
		{
			(*found).second = colour;
			CPreferences::sPrefs->mCalendarColours.SetDirty();
		}
	}
	else
	{
		// Add new preference value
		CPreferences::sPrefs->mCalendarColours.Value().insert(cdstruint32map::value_type(acctname, colour));
		CPreferences::sPrefs->mCalendarColours.SetDirty();
	}

	// Broadcast change
	Broadcast_Message(eBroadcast_ChangedNode, node);
}

void CCalendarStoreManager::ActivateNode(CCalendarStoreNode* node)
{
	// Only if not already active
	if (!node->Activate())
		return;
	
	// Create calendar
	iCal::CICalendar* cal = new iCal::CICalendar;
	mActiveCalendars.push_back(cal);
	if (!node->IsInbox())
		mReceivableCalendars.push_back(cal);
	
	// Give it to the node - this will activate the node
	node->SetCalendar(cal);
	
	// Now read the calendar data
	node->GetProtocol()->OpenCalendar(*node, *cal);
	
	if (CPreferences::sPrefs->mAutomaticEDST.GetValue())
	{
		cdstrvect tzids;
		if (!cal->ValidEDST(tzids))
		{
			// Do nothing
			cdstring temp(os_endl2);
			temp += "Calendar : ";
			temp += node->GetDisplayShortName();
			temp += os_endl2;
			for(cdstrvect::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
			{
				if (temp.length())
					temp += os_endl;
				temp += *iter;
			}
			bool dontshow = false;
			short answer = CErrorHandler::PutCautionAlertRsrcStr(true, "ErrorDialog::Text::iCalendarEDSTUpgrade", temp, NULL, &dontshow);
			if (dontshow)
				CPreferences::sPrefs->mAutomaticEDST.SetValue(false);
			if (answer== CErrorHandler::Ok)
			{
				cal->UpgradeEDST();
				node->GetProtocol()->WriteFullCalendar(*node, *cal);
			}
		}
	}
}

void CCalendarStoreManager::DeactivateNode(CCalendarStoreNode* node)
{
	// Only if active
	if (!node->Deactivate())
		return;
	
	// Get the calendar for this node
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(node->GetCalendarRef());
	if (cal != NULL)
	{
		// Must write out the calendar if changed
		if (cal->IsDirty())
			node->GetProtocol()->WriteFullCalendar(*node, *cal);
		
		// Remove from active list
		mActiveCalendars.erase(std::remove(mActiveCalendars.begin(), mActiveCalendars.end(), cal), mActiveCalendars.end());
		mReceivableCalendars.erase(std::remove(mReceivableCalendars.begin(), mReceivableCalendars.end(), cal), mReceivableCalendars.end());
		
		// As a safety check also remove from subscribed list
		mSubscribedCalendars.erase(std::remove(mSubscribedCalendars.begin(), mSubscribedCalendars.end(), cal), mSubscribedCalendars.end());
		
		// Delete it
		delete cal;
		cal = NULL;
	}
	
	// Deactivate the node
	node->SetCalendar(NULL);
}

iCal::CICalendar* CCalendarStoreManager::PickCalendar(const iCal::CICalendarComponent* comp) const
{
	// If one always return that one
	if (mReceivableCalendars.size() == 1)
		return mReceivableCalendars.front();

	iCal::CICalendar* result = NULL;

	// Get list of active calendars
	cdstrvect cals;
	uint32_t cal_pos = 1;
	for(iCal::CICalendarList::const_iterator iter = mReceivableCalendars.begin(); iter != mReceivableCalendars.end(); iter++,  cal_pos++)
	{
		const CCalendarStoreNode* node = GetNode(*iter);
		cdstring name = node ? node->GetAccountName(HasMultipleProtocols()) : (*iter)->GetName();
		if (name.empty())
		{
			name = rsrc::GetString("::NumberedCalendar");
			name.Substitute((long)cal_pos);
		}
		cals.push_back(name);
	}

	// Do the dialog
	ulvector selected;
	if (CTextListChoice::PoseDialog("CITIPProcessor::ChooseCalendarTitle", "CITIPProcessor::ChooseCalendar", NULL, false, true, false, false, cals, cdstring::null_str, selected))
	{
		result = mReceivableCalendars.at(selected.front());
	}
	
	// Just use the first one for now
	return result;
}

// Get identity associated with mailbox
const CIdentity* CCalendarStoreManager::GetTiedIdentity(const iCal::CICalendar* cal) const
{
	// Get node associated with calendar
	const CCalendarStoreNode* node = GetNode(cal);
	if (node == NULL)
		return NULL;

	// Try identity for this one first
	const CIdentity* result = CPreferences::sPrefs->mTiedCalendars.GetValue().GetTiedCalIdentity(node);
	
	// Try inherited ones next
	if (!result && CPreferences::sPrefs->mTiedMboxInherit.GetValue())
	{
		// Get account name and dir char
		cdstring acctname = node->GetAccountName();
		
		char delim = node->GetProtocol()->GetDirDelim();
		
		// Chop at dir delim character
		if (delim)
		{
			// Loop up the hierarchy looking for a tied identity
			while(!result && (::strrchr(acctname.c_str(), delim) != NULL))
			{
				*::strrchr(acctname.c_str_mod(), delim) = 0;
				result = CPreferences::sPrefs->mTiedCalendars.GetValue().GetTiedCalIdentity(acctname);
			}
		}
	}
	
	// Try account one at the end
	if (result == NULL)
	{
		result = &node->GetProtocol()->GetCalendarAccount()->GetAccountIdentity();
	}
	
	return result;
}

#pragma mark ____________________________Disconnected

void CCalendarStoreManager::GoOffline(bool force, bool sync, bool fast)
{
	// Do sync if not being forced off
	if (!force && sync)
		DoOfflineSync(fast);

	// Go offline on each protocol
	for(CCalendarProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		// Convert allowed protos to disconnected state
		if ((*iter)->CanDisconnect())
			(*iter)->GoOffline();
		else if (!(*iter)->IsOffline())
		{
			// This must fail silently
			try
			{
				// Other network protos must be logged out
				(*iter)->Close();
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);
			}
		}
	}

	// Force visual update
	UpdateWindows();
}

void CCalendarStoreManager::DoOfflineSync(bool fast)
{
	// NB Must fail silently - no throws out of here
	
	try
	{
		// Go offline on each protocol
		for(CCalendarProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
		{
			// Convert allowed protos to disconnected state
			if ((*iter)->CanDisconnect())
				(*iter)->SynchroniseDisconnect(fast);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CCalendarStoreManager::GoOnline(bool sync)
{
	// Go online on each protocol
	for(CCalendarProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		// Only do allowed accounts
		if ((*iter)->CanDisconnect())
		{
			if (sync && (*iter)->IsLoggedOn())
				(*iter)->DoPlayback();
			(*iter)->GoOnline();
		}
		else if ((*iter)->GetAccount()->GetLogonAtStart())
			// Allow start to throw
			StartProtocol(*iter, false);
	}

	// Force visual update
	UpdateWindows();
}

#pragma mark ____________________________Sleep

void CCalendarStoreManager::Suspend()
{
	// Nothing to do for now
}

void CCalendarStoreManager::Resume()
{
	// Nothing to do for now
}
