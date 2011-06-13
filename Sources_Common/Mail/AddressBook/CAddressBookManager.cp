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


// CAddressBookManager.h

// Class to handle all actions related to local & report address books,
// including opening/closing, nick-name resolution, searching etc

#include "CAddressBookManager.h"

#include "CAddressBookWindow.h"
#include "CAdbkManagerTable.h"
#include "CAdbkManagerView.h"
#include "CAdbkProtocol.h"
#include "CAddrLookupProtocol.h"
#include "CAddressDisplay.h"
#include "CAdminLock.h"
#include "CConnectionManager.h"
#include "CEditAddressDialog.h"
#include "CErrorHandler.h"
#include "CLDAPClient.h"
#include "CMessage.h"
#include "CMessageList.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextListChoice.h"
#include "CURL.h"
#include "C3PaneWindow.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CAddressDisplay.h"
#include "CStringResources.h"
#endif

#include <algorithm>
#include <memory>

CAddressBookManager* CAddressBookManager::sAddressBookManager = NULL;

CAddressBookManager::CAddressBookManager()
{
	mLocalProto = NULL;
	mOSProto = NULL;
	
	mProtos.set_delete_data(false);
	mProtoCount = 0;

	mAdbkNickName.set_delete_data(false);
	mAdbkSearch.set_delete_data(false);

	CAddressBookManager::sAddressBookManager = this;
	
	// Must inform any adbk manager windows that currently exist that
	// the adbk manager is now here so that protocol change broadcasts work
	UpdateWindows();
}

CAddressBookManager::~CAddressBookManager()
{
	// Remove all windows
	//CAddressBookWindow::CloseAllWindows();

	// Remove all server views now to prevent illegal updates to 'stale' windows
	{
		cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList>::lock _lock(CAdbkManagerView::sAdbkManagerViews);
		for(CAdbkManagerView::CAdbkManagerViewList::iterator iter = CAdbkManagerView::sAdbkManagerViews->begin(); iter != CAdbkManagerView::sAdbkManagerViews->end(); iter++)
			(*iter)->DoClose();
	}

	// Logoff each protocol (includes the local one)
	for(CAdbkProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		StopProtocol(*iter);
		RemoveProtocol(*iter);
	}

	// Remove existing
	mLookups.clear();

	// Clear out any persistent search results
	ClearSearch();
	CAddressBookManager::sAddressBookManager = NULL;
}

// Update all windows for changes
void CAddressBookManager::UpdateWindows()
{
	// Iterate over all adbk manager windows
	cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList>::lock _lock(CAdbkManagerView::sAdbkManagerViews);
	for(CAdbkManagerView::CAdbkManagerViewList::iterator iter = CAdbkManagerView::sAdbkManagerViews->begin(); iter != CAdbkManagerView::sAdbkManagerViews->end(); iter++)
		// Force visual update
		(*iter)->GetTable()->SetManager(this);
}

// Add a new adbk remote protocol
void CAddressBookManager::AddProtocol(CAdbkProtocol* proto)
{
	// Add item to root node
	mRoot.AddChild(proto->GetStoreRoot());

	// Add to protos list
	mProtos.push_back(proto);
	mProtoCount++;

	// Broadcast change to all after adding
	Broadcast_Message(eBroadcast_NewAdbkAccount, proto);
}

// Remove an adbk remote protocol
void CAddressBookManager::RemoveProtocol(CAdbkProtocol* proto)
{
	// Broadcast change to all before delete
	Broadcast_Message(eBroadcast_RemoveAdbkAccount, proto);

	// Remove node from parent
	proto->GetStoreRoot()->RemoveFromParent();

	// Delete protocol (will also delete the node)
	delete proto;
	proto = NULL;
}

// Add a new adbk remote protocol
CAdbkProtocol* CAddressBookManager::GetProtocol(const cdstring& name)
{
	for(CAdbkProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		if ((*iter)->GetAccountName() == name)
			return *iter;
	}

	return NULL;
}

// Clear all search results
void CAddressBookManager::ClearSearch()
{
	// Manually delete all items
	for(CAddressSearchResultList::iterator iter = mSearchResultList.begin();
		iter != mSearchResultList.end(); iter++)
	{
		// Delete address list then item itself
		delete (*iter)->second;
		delete *iter;
	}

	mSearchResultList.clear();
}

// Clear single search result
void CAddressBookManager::ClearSearchItem(CAddressSearchResult* item)
{
	CAddressSearchResultList::iterator found = std::find(mSearchResultList.begin(), mSearchResultList.end(), item);

	if (found != mSearchResultList.end())
	{
		delete item->second;
		delete item;
		mSearchResultList.erase(found);
	}
}

// Clear search result address
void CAddressBookManager::ClearSearchItemAddress(CAddressSearchResult* item, CAddress* addr)
{
	// Find result first
	CAddressSearchResultList::iterator found = std::find(mSearchResultList.begin(), mSearchResultList.end(), item);

	if (found != mSearchResultList.end())
	{
		// Now find address
		CAddressList::iterator found_addr = std::find(item->second->begin(), item->second->end(), addr);
		if (found_addr != item->second->end())
			item->second->erase(found_addr);
	}
}

// Sync with changed accounts
void CAddressBookManager::SyncAccounts()
{
	// Sync accounts for adbk and directory protocols
	SyncProtos();
	SyncLookups();

	// Make sure 3-pane is told to re-instate previously open address books
	if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->DoneInitAdbkAccounts();
}

// Sync with changed accounts
void CAddressBookManager::SyncProtos()
{
	// Only add local protocol if allowed by admin
	if (!CAdminLock::sAdminLock.mNoLocalAdbks)
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Carbon
		// Addressbook.Framework only available on OS X 10.2 and later
	    if (UEnvironment::GetOSVersion() >= 0x01020)
		{
			// First check if local exists
			if (mOSProto != NULL)
			{
				// Sync its account with prefs etc
				UpdateProtocol(mOSProto);

				// Resync account object
				mOSProto->SetAccount(&CPreferences::sPrefs->mOSAdbkAccount.Value());
			}
			else
			{
				// Create new local protocol
				mOSProto = new CAdbkProtocol(&CPreferences::sPrefs->mOSAdbkAccount.Value());
				mOSProto->SetDescriptor(CPreferences::sPrefs->mOSAdbkAccount.GetValue().GetName());
				AddProtocol(mOSProto);
			}
		}
#endif
#endif

		// First check if local exists
		if (mLocalProto != NULL)
		{
			// Sync its account with prefs etc
			UpdateProtocol(mLocalProto);

			// Resync account object
			mLocalProto->SetAccount(&CPreferences::sPrefs->mLocalAdbkAccount.Value());
		}
		else
		{
			// Create new local protocol
			mLocalProto = new CAdbkProtocol(&CPreferences::sPrefs->mLocalAdbkAccount.Value());
			mLocalProto->SetDescriptor(CPreferences::sPrefs->mLocalAdbkAccount.GetValue().GetName());
			AddProtocol(mLocalProto);
		}
	}

	// New smart alogorithm: compare protocol and protocol embedded in account

	// Sync remainder from prefs
	const CAddressAccountList& accts = CPreferences::sPrefs->mAddressAccounts.GetValue();

	// This maintains a list of protocols that need to be started
	CAdbkProtocolList started;
	started.set_delete_data(false);

	// First delete protocols not in accounts
	unsigned long proto_offset = 0;
	if (mOSProto != NULL)
		proto_offset++;
	if (mLocalProto != NULL)
		proto_offset++;
	for(CAdbkProtocolList::iterator iter1 = mProtos.begin() + proto_offset; iter1 != mProtos.end();)
	{
		bool found = false;

		// Look for match between existing protocol and new accounts
		for(CAddressAccountList::const_iterator iter2 = accts.begin(); !found && (iter2 != accts.end()); iter2++)
		{
			// Only if IMSP/ACAP/CardDAV
			if (((*iter2)->GetServerType() != CINETAccount::eIMSP) &&
				((*iter2)->GetServerType() != CINETAccount::eACAP) &&
				((*iter2)->GetServerType() != CINETAccount::eCardDAVAdbk))
				continue;

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
	for(CAddressAccountList::const_iterator iter2 = accts.begin(); iter2 != accts.end(); iter2++)
	{
		// Only if IMSP/ACAP/CardDAV
		if (((*iter2)->GetServerType() != CINETAccount::eIMSP) &&
			((*iter2)->GetServerType() != CINETAccount::eACAP) &&
			((*iter2)->GetServerType() != CINETAccount::eCardDAVAdbk))
			continue;

		bool found = false;

		// Look for protocol matching account
		for(CAdbkProtocolList::iterator iter1 = mProtos.begin() + proto_offset; !found && (iter1 != mProtos.end()); iter1++)
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
			CAdbkProtocol* aAdbkProtocol = new CAdbkProtocol(*iter2);
			try
			{
				aAdbkProtocol->SetDescriptor((*iter2)->GetServerIP());
				AddProtocol(aAdbkProtocol);

				// Always do logon - protocol will handle forced disconnect state
				if (aAdbkProtocol->IsOffline() || (*iter2)->GetLogonAtStart())
					started.push_back(aAdbkProtocol);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Do not allow to fail
			}
		}
	}

	// Do all protocol starts here once the entire list of accounts is consistent
	

	// Only start local protocols if allowed by admin
	if (mOSProto != NULL)
		StartProtocol(mOSProto);
	if (mLocalProto != NULL)
		StartProtocol(mLocalProto);

	for(CAdbkProtocolList::const_iterator iter = started.begin(); iter != started.end(); iter++)
		StartProtocol(*iter);
}

// Sync with changed accounts
void CAddressBookManager::SyncLookups()
{
	// Sync remainder from prefs
	const CAddressAccountList& accts = CPreferences::sPrefs->mAddressAccounts.GetValue();

	// Remove existing
	mLookups.clear();

	// Compare existing protocols with accounts
	for(CAddressAccountList::const_iterator iter1 = accts.begin(); iter1 != accts.end(); iter1++)
	{
		bool found = false;

		switch((*iter1)->GetServerType())
		{
		case CAddressAccount::eIMSP:
		case CAddressAccount::eACAP:
		case CAddressAccount::eCardDAVAdbk:
			// Never - these are ADBKProtocols
			break;

		case CAddressAccount::eLDAP:
			{
				// Create new client
				CLDAPClient* ldap_client = new CLDAPClient;
				
				// Give it account details
				ldap_client->SetAccount(*iter1);
				
				// Add to list
				mLookups.push_back(ldap_client);
				break;
			}
		case CAddressAccount::eWHOISPP:
		case CAddressAccount::eFinger:
		default:
			// Not supported at the moment
			break;
		}
	}	
}

// Start remote protocol
void CAddressBookManager::StartProtocol(CAdbkProtocol* proto, bool silent)
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
		// Open protocol and get adbks
		if (CMulberryApp::sApp->BeginINET(proto))
		{
			proto->Logon();
			proto->LoadList();
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
void CAddressBookManager::UpdateProtocol(CAdbkProtocol* proto)
{
	try
	{
		// Logoff protocol (clears adbks)
		proto->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Stop remote protocol
void CAddressBookManager::StopProtocol(CAdbkProtocol* proto)
{
	try
	{
		// Logoff protocol (clears adbks)
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
		// Iterate over all adbk manager windows
		cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList>::lock _lock(CAdbkManagerView::sAdbkManagerViews);
		for(CAdbkManagerView::CAdbkManagerViewList::iterator iter = CAdbkManagerView::sAdbkManagerViews->begin(); iter != CAdbkManagerView::sAdbkManagerViews->end(); iter++)
			(*iter)->GetTable()->RemoveChildren(temp, true);
	}
#endif
}

// Get index of protocol in list
long CAddressBookManager::GetProtocolIndex(const CAdbkProtocol* proto) const
{
	CAdbkProtocolList::const_iterator found = std::find(mProtos.begin(), mProtos.end(), proto);
	if (found != mProtos.end())
		return found - mProtos.begin();
	else
		return -1;
}

bool CAddressBookManager::FindProtocol(const CAdbkProtocol* proto, unsigned long& pos) const
{
	bool result = false;
	pos = 1;

	for(CAddressBookList::const_iterator iter = mRoot.GetChildren()->begin(); iter != mRoot.GetChildren()->end(); iter++, pos++)
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
void CAddressBookManager::MoveProtocol(long old_index, long new_index)
{
	// Cannot move local or OS
	if (((mLocalProto != NULL) || (mOSProto != NULL)) && (old_index == 0))
		return;
	else if ((mLocalProto != NULL) && (mOSProto != NULL) && (old_index == 1))
		return;
	
	// Determine index adjustment offset to remove local/web protos
	uint32_t proto_offset = 0;
	if (mLocalProto != NULL)
		proto_offset++;
	if (mOSProto != NULL)
		proto_offset++;

	// Cannot move above local or web
	if (new_index < proto_offset)
		new_index = proto_offset;

	CAdbkProtocol* proto = mProtos.at(old_index);

	// Broadcast change to all to remove it
	Broadcast_Message(eBroadcast_RemoveAdbkAccount, proto);

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
	CAddressAccountList& accts = CPreferences::sPrefs->mAddressAccounts.Value();
	CAddressAccount* acct = accts.at(old_index);
	accts.at(old_index) = NULL;
	accts.erase(accts.begin() + old_index);
	if (new_index < (long) mProtos.size() - 1 - proto_offset)
		accts.insert(accts.begin() + new_index, acct);
	else
		accts.push_back(acct);
	CPreferences::sPrefs->mAddressAccounts.SetDirty();

	// Add it back to visual display in new place
	Broadcast_Message(eBroadcast_InsertAdbkAccount, proto);
}

const CAddressBook* CAddressBookManager::GetNode(const cdstring& adbk) const
{
	const CAddressBook* result = NULL;
	for(CAdbkProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		result = (*iter)->GetNode(adbk);
		if (result != NULL)
			return result;
	}
	
	return NULL;
}

// Managing the store
CAddressBook* CAddressBookManager::NewAddressBook(CAdbkProtocol* proto, const cdstring& name, bool directory)
{
	if ((proto == NULL) || name.empty())
		return NULL;

	// Create account name
	cdstring acct_name = proto->GetAccountName();
	acct_name += cMailAccountSeparator;
	acct_name += name;

	// Determine the parent node for the new one
	CAddressBook* parent = proto->GetParentNode(acct_name);
	if (parent == NULL)
		return NULL;

	// Create a new node
	CAddressBook* node = new CAddressBook(proto, parent, !directory, directory, name);
	parent->AddChild(node, true);

	try
	{
		// Try to create it
		node->GetProtocol()->CreateAdbk(node);

		// Make sure protocol list is in sync after create
		node->GetProtocol()->ListChanged();

		// Write out an empty calendar with the appropriate name
		if (!directory)
		{
		}

		// Now insert into parent
		Broadcast_Message(eBroadcast_InsertNode, node);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Clean-up
		node->RemoveFromParent();
		delete node;
		
		CLOG_LOGRETHROW;
		throw;
	}

	SyncAddressBook(node, true);

	return node;
}

void CAddressBookManager::RenameAddressBook(CAddressBook* node, const cdstring& new_name)
{
	// Cache account name for preference change
	cdstring old_acct_name = node->GetAccountName();
	cdstring old_acct_url = node->GetURL();

	// Now rename the actual node
	node->GetProtocol()->RenameAdbk(node, new_name);
	
	// Change the node name locally
	node->NewName(new_name);
	cdstring new_acct_name = node->GetAccountName();
	cdstring new_acct_url = node->GetURL();

	// Rename in preferences
	CPreferences::sPrefs->RenameAddressBook(old_acct_name, new_acct_name);
	CPreferences::sPrefs->RenameAddressBookURL(old_acct_url, new_acct_url);

	// Check for actual calendar rather than directory
	if (!node->IsDirectory())
	{
#ifdef _TODO
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
			cal->EditName(node->GetDisplayShortName());
			
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
#endif
	}
	
	// Now adjust node position
	
	// First remove it from any listeners whilst it is still linked in to parent
	Broadcast_Message(eBroadcast_RemoveNode, node);
	
	// Now remove it from its parent and re-insert in proper location
	node->RemoveFromParent();
	
	CAddressBook* parent_node = node->GetProtocol()->GetParentNode(node->GetAccountName());
	if (parent_node)
	{
		parent_node->AddChild(node, true);
		Broadcast_Message(eBroadcast_InsertNode, node);
	}

	// Make sure protocol list is in sync after rename
	node->GetProtocol()->ListChanged();

}

void CAddressBookManager::DeleteAddressBook(CAddressBook* node)
{
	// Remove it from any listeners whilst it is still linked in to parent
	Broadcast_Message(eBroadcast_DeleteNode, node);

	// Now remove from parent's list
	node->RemoveFromParent();

	// Cache account name for preference change
	cdstring acct_name = node->GetAccountName();
	cdstring acct_url = node->GetURL();

	// Now remove the actual node (node is deleted after this)
	node->GetProtocol()->DeleteAdbk(node);
	
	// Make sure protocol list is in sync after rename
	node->GetProtocol()->ListChanged();

	SyncAddressBook(node, false);

	// Remove tha actual object
	delete node;

	// Remove from preferences
	CPreferences::sPrefs->DeleteAddressBook(acct_name);
	CPreferences::sPrefs->DeleteAddressBookURL(acct_url);
}

// Sync address book
void CAddressBookManager::SyncAddressBook(CAddressBook* adbk, bool add)
{
	if (add)
	{
		if (adbk->IsLookup())
		{
			// Only add if not already present
			CAddressBookList::iterator found = std::find(mAdbkNickName.begin(), mAdbkNickName.end(), adbk);
			if (found == mAdbkNickName.end())
				mAdbkNickName.push_back(adbk);
		}
		if (adbk->IsSearch())
		{
			// Only add if not already present
			CAddressBookList::iterator found = std::find(mAdbkSearch.begin(), mAdbkSearch.end(), adbk);
			if (found == mAdbkSearch.end())
				mAdbkSearch.push_back(adbk);
		}
	}
	else
	{
		// Remove from caches
		CAddressBookList::iterator found = std::find(mAdbkNickName.begin(), mAdbkNickName.end(), adbk);
		if (found != mAdbkNickName.end())
			mAdbkNickName.erase(found);
		found = std::find(mAdbkSearch.begin(), mAdbkSearch.end(), adbk);
		if (found != mAdbkSearch.end())
			mAdbkSearch.erase(found);
	}
}

void CAddressBookManager::GetCurrentAddressBookList(cdstrvect& adbks, bool url) const
{
	adbks.clear();

	// Look at each item trying to find a match
	for(CAddressBookList::const_iterator iter1 = mRoot.GetChildren()->begin();
		iter1 != mRoot.GetChildren()->end(); iter1++)
	{
		// Add empty string as protocol separator if more than one
		if (adbks.size() != 0)
			adbks.push_back(cdstring::null_str);

		// Look at each child
		if ((*iter1)->HasInferiors())
		{
			for(CAddressBookList::const_iterator iter2 = (*iter1)->GetChildren()->begin();
				iter2 != (*iter1)->GetChildren()->end(); iter2++)
			{
				// If its a real address book add it to the list
				if ((*iter2)->IsAdbk())
					adbks.push_back(url ? (*iter2)->GetURL() : (*iter2)->GetAccountName());
			}
		}
	}
}

#pragma mark ____________________________Searching

// Resolve nick-name
bool CAddressBookManager::ResolveNickName(const char* nick_name, CAddressList* list)
{
	bool found = false;

	// Look through all address books
	for(CAddressBookList::const_iterator iter = mAdbkNickName.begin(); iter != mAdbkNickName.end(); iter++)
	{
		CAddressBook* adbk = *iter;
		CAdbkAddress* addr;

		// Not if disconnected without cache
#ifdef _TODO
		if (adbk->IsLocalAdbk() && !adbk->IsCachedAdbk())
			continue;
#endif

		// Try to get nick-name from address book
		if (adbk->FindNickName(nick_name, addr))
		{
			// Add to list if found (must copy to prevent double-deleting of address)
			list->push_back(new CAdbkAddress(*addr));
			found = true;
			break;
		}
	}

	return found;
}

// Resolve nick-name
bool CAddressBookManager::ResolveGroupName(const char* grp_name, CAddressList* list, short level)
{
	bool found = false;

	// Look through all address books
	for(CAddressBookList::const_iterator iter = mAdbkNickName.begin(); iter != mAdbkNickName.end(); iter++)
	{
		CAddressBook* adbk = *iter;
		CGroup* grp;

		// Not if disconnected without cache
#ifdef _TODO
		if (adbk->IsLocalAdbk() && !adbk->IsCachedAdbk())
			continue;
#endif

		// Try to get group nick-name from address book
		if (adbk->FindGroupName(grp_name, grp))
		{
			for(unsigned long i = 0; i < grp->GetAddressList().size(); i++)
			{
				// Get the string and insert into list, resolving nick-names/groups
				const cdstring& aStr = grp->GetAddressList().at(i);
				if (!list->InsertTextAddress(aStr, level))
				{
					// Prevent recursion and stack explosion by turning of resolve
					CAddressDisplay::AllowResolve(false);
					CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::RecursionLimit");
					CAddressDisplay::AllowResolve(true);

					// Prevent further processing of nick-names to avoid alert more than once
					break;
				}
			}

			found = true;
			break;
		}
	}

	return found;
}

// Do search
void CAddressBookManager::SearchAddress(const cdstring& name, CAdbkAddress::EAddressMatch match, CAdbkAddress::EAddressField field, const cdstring& field_name)
{
	std::auto_ptr<CAddressList> addr_list(new CAddressList);

	// Get search results
	SearchAddress(name, match, field, *addr_list);

	// Add result to list
	cdstring title = field_name;
	title += " = ";
	title += name;
	CAddressSearchResult* item = new CAddressSearchResult(title, addr_list.release());
	mSearchResultList.push_back(item);
}

// Do search
void CAddressBookManager::SearchAddress(const cdstring& name, CAdbkAddress::EAddressMatch match, CAdbkAddress::EAddressField field, CAddressList& results)
{
	// Search
	CAdbkAddress::CAddressFields fields;
	fields.push_back(field);
	SearchAddress(name, match, fields, results);
}

// Do search
void CAddressBookManager::SearchAddress(const cdstring& name, CAdbkAddress::EAddressMatch match, const CAdbkAddress::CAddressFields& fields, CAddressList& results)
{
	// Look through all address books
	for(CAddressBookList::const_iterator iter = mAdbkSearch.begin(); iter != mAdbkSearch.end(); iter++)
	{
		// Not if disconnected without cache
#ifdef _TODO
		if ((*iter)->IsLocalAdbk() && !(*iter)->IsCachedAdbk())
			continue;
#endif

		(*iter)->SearchAddress(name, match, fields, results);
	}
}

// Do search (not IMSP/ACAP)
void CAddressBookManager::SearchAddress(EAddrLookup method, bool expanding, const cdstring& item, CAdbkAddress::EAddressMatch match, CAdbkAddress::EAddressField field)
{
	std::auto_ptr<CAddressList> addr_list(new CAddressList);

	// Get search results
	SearchAddress(method, expanding, item, match, field, *addr_list);

	// Add result to list
	cdstring title = item;
	CAddressSearchResult* result = new CAddressSearchResult(title, addr_list.release());
	mSearchResultList.push_back(result);
}

// Do search (not IMSP/ACAP)
void CAddressBookManager::SearchAddress(EAddrLookup method, bool expanding, const cdstring& item, CAdbkAddress::EAddressMatch match, CAdbkAddress::EAddressField field, CAddressList& results)
{
	// Look through all search servers
	for(CAddrLookupProtocolList::iterator iter = mLookups.begin(); iter != mLookups.end(); iter++)
	{
		bool do_it = false;

		// Match types
		switch(method)
		{
		case eMulberryLookup:
			// Never - done as adbk protocol
			break;
		case eLDAPLookup:
			// Must be connected and LDAP and marked for expansion/search
			do_it = CConnectionManager::sConnectionManager.IsConnected() &&
						((*iter)->GetAccount()->GetServerType() == CAddressAccount::eLDAP) &&
						(expanding ? (*iter)->GetAccount()->GetLDAPUseExpansion() : (*iter)->GetAccount()->GetLDAPSearch());
			
			// Also must check that LDAP attribute is specified
			if (do_it)
				do_it = !(*iter)->GetAccount()->GetLDAPMatch(field).empty();
			break;
		case eWhoisppLookup:
		case eFingerLookup:
		default:
			// Currently not supported
			break;
		}

		if (do_it)
			(*iter)->Lookup(item, match, field, results);
	}
}

// Do address expansion
void CAddressBookManager::ExpandAddresses(const cdstring& expand, cdstrvect& results)
{
	// Search for addresses in Mulberry address books
	CAddressList addr_list;
	CAdbkAddress::CAddressFields fields;
	if (CPreferences::sPrefs->mExpandFullName.GetValue())
	{
		fields.push_back(CAdbkAddress::eName);
	}
	if (CPreferences::sPrefs->mExpandNickName.GetValue())
	{
		fields.push_back(CAdbkAddress::eNickName);
	}
	if (CPreferences::sPrefs->mExpandEmail.GetValue())
	{
		fields.push_back(CAdbkAddress::eEmail);
	}
	SearchAddress(expand, CAdbkAddress::eMatchAnywhere, fields, addr_list);

	// Search for addresses in LDAP (only if required)
	if (!CPreferences::sPrefs->mSkipLDAP.GetValue() || (addr_list.size() == 0))
	{
		if (CPreferences::sPrefs->mExpandFullName.GetValue())
		{
			SearchAddress(eLDAPLookup, true, expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eName, addr_list);
		}
		if (CPreferences::sPrefs->mExpandNickName.GetValue())
		{
			SearchAddress(eLDAPLookup, true, expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eNickName, addr_list);
		}
		if (CPreferences::sPrefs->mExpandEmail.GetValue())
		{
			SearchAddress(eLDAPLookup, true, expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eEmail, addr_list);
		}
	}
	
	// Now make results unique
	addr_list.MakeUnique();
	
	// Add them to the cdstrvect
	addr_list.AddMailAddressToList(results, true);
}

// Do address expansion
void CAddressBookManager::ExpandCalendarAddresses(const cdstring& expand, cdstrvect& results)
{
	// Search for addresses in Mulberry address books
	CAddressList addr_list;
	if (CPreferences::sPrefs->mExpandFullName.GetValue())
	{
		SearchAddress(expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eName, addr_list);
	}
	if (CPreferences::sPrefs->mExpandNickName.GetValue())
	{
		SearchAddress(expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eNickName, addr_list);
	}
	if (CPreferences::sPrefs->mExpandEmail.GetValue())
	{
		SearchAddress(expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eEmail, addr_list);
	}
	
	// Search for addresses in LDAP (only if required)
	if (!CPreferences::sPrefs->mSkipLDAP.GetValue() || (addr_list.size() == 0))
	{
		if (CPreferences::sPrefs->mExpandFullName.GetValue())
		{
			SearchAddress(eLDAPLookup, true, expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eName, addr_list);
		}
		if (CPreferences::sPrefs->mExpandNickName.GetValue())
		{
			SearchAddress(eLDAPLookup, true, expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eNickName, addr_list);
		}
		if (CPreferences::sPrefs->mExpandEmail.GetValue())
		{
			SearchAddress(eLDAPLookup, true, expand, CAdbkAddress::eMatchAnywhere, CAdbkAddress::eEmail, addr_list);
		}
	}
	
	// Now make results unique
	addr_list.MakeUnique();
	
	// Add them to the cdstrvect
	addr_list.AddCalendarAddressToList(results, true);
}

// Do address capture
void CAddressBookManager::CaptureAddress(const cdstring& capture)
{
	// Parse addresses without resolution
	CAddressList parsed_addrs(capture.c_str(), capture.length(), 0, false);

	// Make unique via merge to a new list
	CAddressList addrs;
	addrs.MergeList(&parsed_addrs);
	
	// Now capture them all
	CaptureAddress(addrs);
}

// Do address capture
void CAddressBookManager::CaptureAddress(const CMessage& msg)
{
	// Get addresses to capture
	CAddressList addrs;
	ExtractCaptureAddress(msg, addrs);
	
	// Now capture them all
	CaptureAddress(addrs);
}

// Do address capture
void CAddressBookManager::CaptureAddress(const CMessageList& msgs)
{
	// Get addresses to capture
	CAddressList addrs;
	for(CMessageList::const_iterator iter = msgs.begin(); iter != msgs.end(); iter++)
	{
		ExtractCaptureAddress(**iter, addrs);
	}
	
	// Now capture them all
	CaptureAddress(addrs);
}

// Do address capture
void CAddressBookManager::CaptureAddress(CAddressList& addrs)
{
	try
	{
		// Look at each address and remove smarts or invalid
		for(CAddressList::iterator iter = addrs.begin(); iter != addrs.end() ; )
		{
			if (CPreferences::TestSmartAddress(**iter) || !(*iter)->IsValid())
			{
				iter = addrs.erase(iter);
				continue;
			}
			
			iter++;
		}

		// Must have some addresses to add
		if (addrs.empty())
			return;

		// Get default address book to capture to
		CAddressBook* adbk = NULL;
		
		// Get default address book chosen via prefs (uses accunt name not url)
		adbk = const_cast<CAddressBook*>(GetNode(CPreferences::sPrefs->mCaptureAddressBook.GetValue()));
		
		// If none set, use the first one in the first account
		if (adbk == NULL)
		{
			// Try the first account
			CAdbkProtocol* proto = NULL;
			if (mProtos.size())
				proto = mProtos.front();
			
			// Get the first address book from the proto
			if (proto)
			{
				// Check for valid address book list in protocol
				CAddressBook* adbk = proto->GetStoreRoot();
				if ((adbk != NULL) && adbk->HasInferiors())
				{
					// Get the first valid address book
					for(CAddressBookList::iterator iter = adbk->GetChildren()->begin(); iter != adbk->GetChildren()->end(); iter++)
					{
						if ((*iter)->IsAdbk())
						{
							adbk = *iter;
							break;
						}
					}
				}
			}
		}

		// If no default address book - warn the user
		if (adbk == NULL)
		{
			CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::MissingCaptureAdbk");
			return;
		}

		// Do user choice if more than one
		CAddressList add_addrs;
		if ((addrs.size() > 1) && CPreferences::sPrefs->mCaptureAllowChoice.GetValue())
		{
			cdstrvect txtaddrs;
			for(CAddressList::iterator iter = addrs.begin(); iter != addrs.end() ; iter++)
				txtaddrs.push_back((*iter)->GetFullAddress());

			// Display list to user
			ulvector selection;
			if (CTextListChoice::PoseDialog("Alerts::Letter::AddressDisplay_Title", "Alerts::Letter::AddressDisplay_Desc", NULL, false, false, false, true,
												txtaddrs, cdstring::null_str, selection, NULL))
			{
				for(ulvector::const_iterator iter = selection.begin(); iter != selection.end(); iter++)
					add_addrs.push_back(new CAdbkAddress(*addrs.at(*iter)));
			}
		}
		else
		{
			for(CAddressList::iterator iter = addrs.begin(); iter != addrs.end() ; iter++)
				add_addrs.push_back(new CAdbkAddress(**iter));
		}

		// Look at each address
		for(CAddressList::iterator iter = add_addrs.begin(); iter != add_addrs.end() ; iter++)
		{
			std::auto_ptr<CAdbkAddress> test(new CAdbkAddress(**iter));

			// Check to see whether it already exists
			cdstring email = test->GetMailAddress();
			
			// See if it exists
			CAddressList found;
			CAdbkAddress::CAddressFields fields;
			fields.push_back(CAdbkAddress::eEmail);
			adbk->SearchAddress(email, CAdbkAddress::eMatchExactly, fields, found);
			
			// If its empty then do capture
			if (found.empty())
			{
				// See if user wanted to edit it first
				if (CPreferences::sPrefs->mCaptureAllowEdit.GetValue())
				{
					// If edit is cancelled do not capture it
					if (!CEditAddressDialog::PoseDialog(test.get()))
						continue;
				}

				// If there is no name, use the email address as the name
				if (test->GetName().empty())
					test->SetName(email);
					
				// Now add to address book (which takes control of the address object) in sorted order
				adbk->AddAddress(test.release(), true);
			}
		}
		
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		// Flag error and ignore exception as capture should not cause the caller to fail
		CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::FailedCaptureAdbk");
	}
}

// Extract address captures
void CAddressBookManager::ExtractCaptureAddress(const CMessage& msg, CAddressList& addrs)
{
	// Make sure envelope exists
	if (!msg.GetEnvelope())
		return;

	// Get the From addresses
	if (CPreferences::sPrefs->mCaptureFrom.GetValue())
		addrs.MergeList(msg.GetEnvelope()->GetFrom());
	
	// Get the Cc addresses
	if (CPreferences::sPrefs->mCaptureCc.GetValue())
		addrs.MergeList(msg.GetEnvelope()->GetCC());
	
	// Get the Reply-to addresses
	if (CPreferences::sPrefs->mCaptureReplyTo.GetValue() && !msg.GetEnvelope()->GetReplyTo()->empty())
		addrs.MergeList(msg.GetEnvelope()->GetReplyTo());
	else
		addrs.MergeList(msg.GetEnvelope()->GetFrom());
	
	// Get the To addresses
	if (CPreferences::sPrefs->mCaptureTo.GetValue())
		addrs.MergeList(msg.GetEnvelope()->GetTo());
}

#pragma mark ____________________________Disconnected

void CAddressBookManager::GoOffline(bool force, bool sync, bool fast)
{
	// Do sync of auto sync address books if not being forced off
	if (!force && sync)
		DoOfflineSync(fast);

	// Go offline on each protocol
	for(CAdbkProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
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

void CAddressBookManager::DoOfflineSync(bool fast)
{
	// NB Must fail silently - no throws out of here
	
	try
	{
		// Go offline on each protocol
		for(CAdbkProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
		{
			// Convert allowed protos to disconnected state
			if ((*iter)->CanDisconnect())
				(*iter)->SynchroniseRemoteAll(fast);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CAddressBookManager::GoOnline(bool sync)
{
	// Go online on each protocol
	for(CAdbkProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
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

void CAddressBookManager::Suspend()
{
	// Nothing to do for now
}

void CAddressBookManager::Resume()
{
	// Nothing to do for now
}
