/*
    Copyright (c) 2007 Cyrus Daboo. All rights reserved.
    
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
	CCalendarProtocol.cpp

	Author:			
	Description:	<describe the CCalendarProtocol class here>
*/

#include "CCalendarProtocol.h"

#include "CCalDAVCalendarClient.h"
#include "CCalendarClient.h"
#include "CCalendarRecord.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CCalendarStoreXML.h"
#include "CConnectionManager.h"
#include "CLocalCalendarClient.h"
#include "CPasswordManager.h"
#include "CPreferences.h"
#include "CWebDAVCalendarClient.h"

#include "cdfstream.h"

#include "CICalendarSync.h"
#include "CICalendarDateTime.h"
#include "CICalendarDuration.h"

#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLObject.h"
#include "XMLSAXSimple.h"

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

#include <algorithm>
#include <memory>

#ifdef _MSC_VER
// suppress warning: 'this' : used in base member initializer list
// (base member just stashes the pointer for later use, so it's safe)
#pragma warning (disable : 4355)
#endif

using namespace calstore; 

CCalendarProtocol::CCalendarProtocol(CINETAccount* acct) :
	CINETProtocol(acct),
	mStoreRoot(this)
{
	mClient = NULL;
	mCacheClient = NULL;
	mCacheIsPrimary = false;
	mSyncingList = false;
	mListedFromCache = false;

	// Only WebDAV servers can disconnect
	switch(GetAccountType())
	{
	case CINETAccount::eWebDAVCalendar:
		SetFlags(eCanDisconnect, IsOfflineAllowed() && GetCalendarAccount()->GetDisconnected());
		break;
	case CINETAccount::eCalDAVCalendar:
		SetFlags(eCanDisconnect, IsOfflineAllowed() && GetCalendarAccount()->GetDisconnected());
		break;
	case CINETAccount::eHTTPCalendar:	// This is always in disconnected mode but will not be present if not allowed
		SetFlags(eCanDisconnect, true);
		break;
	default:
		SetFlags(eCanDisconnect, false);
		break;
	}

	// If not login at startup but it can disconnect, set to force disconnect mode
	mFlags.Set(eForceDisconnect, !GetCalendarAccount()->GetLogonAtStart());

	switch(GetAccountType())
	{
	case CINETAccount::eLocalCalendar:
		mDirDelim = os_dir_delim;
		break;
	case CINETAccount::eHTTPCalendar:
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
		mDirDelim = '/';
		break;
	default:
		mDirDelim = 0;
		break;
	}


	mRecorder = NULL;

	CreateClient();
}


// Copy constructor
CCalendarProtocol::CCalendarProtocol(const CCalendarProtocol& copy, bool force_local, bool force_remote) :
	CINETProtocol(copy),
	mStoreRoot(this)
{
	// Init instance variables
	mClient = NULL;
	mCacheClient = NULL;
	mCacheIsPrimary = false;

	mRecorder = NULL; 				// Only original proto uses this - clones do not

	mDirDelim = copy.mDirDelim;

	// Copy client
	if (((GetAccountType() == CINETAccount::eCalDAVCalendar) || (GetAccountType() == CINETAccount::eWebDAVCalendar) || (GetAccountType() == CINETAccount::eHTTPCalendar)) &&
		(force_local || force_remote))
	{
		if (force_remote)
		{
			SetFlags(eIsOffline, false);
			SetFlags(eDisconnected, false);
			switch(GetAccountType())
			{
			case CINETAccount::eWebDAVCalendar:
				mClient = new CWebDAVCalendarClient(this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalCalendarClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eCalDAVCalendar:
				mClient = new CCalDAVCalendarClient(this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalCalendarClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eHTTPCalendar:
				// Create WebDAV client with private account
				mClient = new CWebDAVCalendarClient(this);
				{
					CCalendarAccount* temp = new CCalendarAccount;
					temp->SetServerType(CCalendarAccount::eHTTPCalendar);
					temp->SetName(GetAccount()->GetName());
					mClient->SetPrivateAccount(temp);
				}
				InitOfflineCWD();
				mCacheClient = new CLocalCalendarClient(this);
				mCacheIsPrimary = true;
				break;
			default:;
			}
		}
		else
		{
			SetFlags(eIsOffline, true);
			SetFlags(eDisconnected, true);
			InitDisconnect();
			mClient = new CLocalCalendarClient(this);
			static_cast<CLocalCalendarClient*>(mClient)->SetRecorder(mRecorder);
		}
		CINETProtocol::mClient = mClient;
	}
	else
		CopyClient(copy);

}

CCalendarProtocol::~CCalendarProtocol()
{
	// Clear nodes first to ensure any active nodes write themselves out before the
	// client is deleted
	mStoreRoot.Clear();

	// Delete client
	RemoveClient();

	// Delete recorder
	delete mRecorder;
	mRecorder = NULL;
}

void CCalendarProtocol::SetAccount(CINETAccount* account)
{
	// Do inherited
	CINETProtocol::SetAccount(account);
	
	// Check disconnected state
	switch(GetAccountType())
	{
	case CINETAccount::eWebDAVCalendar:
		SetFlags(eCanDisconnect, IsOfflineAllowed() &&
									GetCalendarAccount()->GetDisconnected());
		break;
	case CINETAccount::eCalDAVCalendar:
		SetFlags(eCanDisconnect, IsOfflineAllowed() &&
									GetCalendarAccount()->GetDisconnected());
		break;
	case CINETAccount::eHTTPCalendar:
		SetFlags(eCanDisconnect, true);
		break;
	default:
		SetFlags(eCanDisconnect, false);
		break;
	}

	switch(GetAccountType())
	{
	case CINETAccount::eLocalCalendar:
		mDirDelim = os_dir_delim;
		break;
	case CINETAccount::eHTTPCalendar:
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
		mDirDelim = '/';
		break;
	default:
		mDirDelim = 0;
		break;
	}

	// Look for change in account name
	if (mStoreRoot.GetDisplayName() != GetAccountName())
	{
		// Need to rename offline
		if (mCacheClient != NULL)
		{
			RenameOffline();
		}
		
		// Rename root
		mStoreRoot.SetDisplayName(GetAccountName());
	}

	if (mCacheClient != NULL)
	{
		// Don't allow throw
		try
		{
			mCacheClient->Reset();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
	}

}

void CCalendarProtocol::DirtyAccount()
{
	// Must override in derived classes
	switch(GetAccountType())
	{
	case CINETAccount::eLocalCalendar:
		CPreferences::sPrefs->mLocalCalendarAccount.SetDirty();
		break;
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
		CPreferences::sPrefs->mCalendarAccounts.SetDirty();
		break;
	case CINETAccount::eHTTPCalendar:
		CPreferences::sPrefs->mWebCalendarAccount.SetDirty();
		break;
	default:
		break;
	}
}

void CCalendarProtocol::CreateClient()
{
	RemoveClient();

	// Check disconnected state
	switch(GetAccountType())
	{
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
	case CINETAccount::eHTTPCalendar:
		if ((CConnectionManager::sConnectionManager.IsConnected() && !IsForceDisconnect()) || !CanDisconnect())
		{
			SetFlags(eIsOffline, false);
			SetFlags(eDisconnected, false);
			switch(GetAccountType())
			{
			case CINETAccount::eWebDAVCalendar:
				mClient = new CWebDAVCalendarClient(this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalCalendarClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eCalDAVCalendar:
				mClient = new CCalDAVCalendarClient(this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalCalendarClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eHTTPCalendar:
				// Create WebDAV client with private account
				mClient = new CWebDAVCalendarClient(this);
				{
					CCalendarAccount* temp = new CCalendarAccount;
					temp->SetServerType(CCalendarAccount::eHTTPCalendar);
					temp->SetName(GetAccount()->GetName());
					mClient->SetPrivateAccount(temp);
				}
				InitOfflineCWD();
				mCacheClient = new CLocalCalendarClient(this);
				mCacheIsPrimary = true;
				break;
			default:;
			}
		}
		else
		{
			SetFlags(eIsOffline, true);
			SetFlags(eDisconnected, true);
			InitDisconnect();
			mClient = new CLocalCalendarClient(this);
			static_cast<CLocalCalendarClient*>(mClient)->SetRecorder(mRecorder);
		}
		break;
	case CINETAccount::eLocalCalendar:
		SetFlags(eIsOffline, true);
		SetFlags(eDisconnected, false);
		InitOffline();
		mClient = new CLocalCalendarClient(this);
		break;
	default:;
	}

	CINETProtocol::mClient = mClient;
}

void CCalendarProtocol::CopyClient(const CINETProtocol& copy)
{
	// NB This is called in the constructor and cannot rely on virtual functions
	// being called for this object!
	CCalendarClient* copy_it = static_cast<const CCalendarProtocol&>(copy).mClient;
	if (dynamic_cast<CWebDAVCalendarClient*>(copy_it))
		mClient = new CWebDAVCalendarClient(static_cast<const CWebDAVCalendarClient&>(*copy_it), this);
	else if (dynamic_cast<CCalDAVCalendarClient*>(copy_it))
		mClient = new CCalDAVCalendarClient(static_cast<const CCalDAVCalendarClient&>(*copy_it), this);
	else if (dynamic_cast<CLocalCalendarClient*>(copy_it))
		mClient = new CLocalCalendarClient(static_cast<const CLocalCalendarClient&>(*copy_it), this);
	else
		mClient = NULL;
	
	CINETProtocol::mClient = mClient;
}

// Set into synchronising mode
void CCalendarProtocol::SetSynchronising()
{
	// Just remove recorder
	if (mRecorder)
	{
		// Remove from local client
		CLocalCalendarClient* client = dynamic_cast<CLocalCalendarClient*>(mClient);
		if (client)
			client->SetRecorder(NULL);

		delete mRecorder;
		mRecorder = NULL;
	}
	
	// Remove cache client
	if (mCacheClient != NULL)
	{
		delete mCacheClient;
		mCacheClient = NULL;
		mCacheIsPrimary = false;
	}
}

void CCalendarProtocol::RemoveClient()
{
	delete mClient;
	mClient = NULL;
	CINETProtocol::mClient = NULL;
	
	delete mCacheClient;
	mCacheClient = NULL;
	mCacheIsPrimary = false;
}

// Open connection to protocol server
void CCalendarProtocol::Open()
{
	// Only bother if not already open
	if (IsOpenAllowed() && IsNotOpen())
	{
		// Get client to open
		SetErrorProcess(false);
		mClient->Open();
		if (mCacheClient != NULL)
			mCacheClient->Open();
		mMPState = eINETOpen;
	}

} // CCalendarProtocol::Open

// Close connection to protocol server
void CCalendarProtocol::Close()
{
	// Only bother if not already closed
	if (IsNotOpen())
		return;

	try
	{
		// Logoff if required
		if (IsLoggedOn())
			Logoff();

		// Get client to close
		mClient->Close();
		if (mCacheClient != NULL)
			mCacheClient->Close();

		mMPState = eINETNotOpen;
		SetErrorProcess(false);
		
		// Clean any free connections in cache
		CleanConnections();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Clean up and throw up
		mMPState = eINETNotOpen;
		SetErrorProcess(false);
		
		// Clean any free connections in cache
		CleanConnections();

		CLOG_LOGRETHROW;
		throw;
	}

}

// Logon to server
void CCalendarProtocol::Logon()
{
	// No need to block since if its not logged in there can be no other network
	// operation in progress. If it is logged in it won't issue a network call either.

	if (IsOpenAllowed() && !IsLoggedOn())
	{
		// Recovering after a failure should be on here as it can be turned off
		// at logoff
		SetNoRecovery(false);

		// Get client to logon
		mClient->Logon();
		if (mCacheClient != NULL)
			mCacheClient->Logon();

		// Recache user id & password after successful logon
		if (GetAccount()->GetAuthenticator().RequiresUserPswd())
		{
			CAuthenticatorUserPswd* auth = GetAccount()->GetAuthenticatorUserPswd();

			// Only bother if it contains something
			if (!auth->GetPswd().empty())
			{
				CCalendarProtocol::SetCachedPswd(auth->GetUID(), auth->GetPswd());
				CPasswordManager::GetManager()->AddPassword(GetAccount(), auth->GetPswd());
			}
		}

		// Make copy of current authenticator
		SetAuthenticatorUniqueness(GetAccount()->GetAuthenticator().GetUniqueness());

		// Add to list of periodic items
		CMailControl::RegisterPeriodic(this, true);

		mMPState = eINETLoggedOn;
		SetErrorProcess(false);

		// Broadcast change in state
		Broadcast_Message(eBroadcast_Logon, this);
	}

}

// Logoff server
void CCalendarProtocol::Logoff()
{
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	if (IsLoggedOn())
	{
		// Remove from list of periodic items
		CMailControl::RegisterPeriodic(this, false);

		// Do without errors appearing on screen as the user
		// is not really interested in failures during logoff
		bool old_error_alert = GetNoErrorAlert();
		SetNoErrorAlert(true);

		// No point in recovering after a failure
		SetNoRecovery(true);

		try
		{
			// Get client to logoff
			mClient->Logoff();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
		try
		{
			// Get client to logoff
			if (mCacheClient != NULL)
				mCacheClient->Logoff();
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
		SetNoErrorAlert(old_error_alert);

		// Set flag
		mMPState = eINETLoggedOff;
		SetErrorProcess(false);

		// Broadcast change in state
		Broadcast_Message(eBroadcast_Logoff, this);
		
		// Clean any free connections in cache
		CleanConnections();
	}

}

// Set the working directory prefix
void CCalendarProtocol::AddWD(const cdstring& wd)
{
	// Create new hierarchy
	CCalendarStoreNode* node = new CCalendarStoreNode(this, &mStoreRoot, true, false, false, wd);
	node->SetFlags(CCalendarStoreNode::eIsDisplayHierarchy);
	mStoreRoot.AddChild(node);
	GetCalendarAccount()->GetWDs().push_back(CDisplayItem(wd));
	
	// Broadcast addition
	Broadcast_Message(eBroadcast_RefreshList, this);
	
	// Make prefs as dirty
	CPreferences::sPrefs->mCalendarAccounts.SetDirty();
	
	// Now load it if logged in
	if (IsLoggedOn())
	{
		LoadList();
		SyncList();
	}
}

// Rename the working directory prefix
void CCalendarProtocol::RenameWD(CCalendarStoreNode& node, const cdstring& new_name)
{
	// Change name in list and force refresh
	for(CDisplayItemList::iterator iter = GetCalendarAccount()->GetWDs().begin(); iter != GetCalendarAccount()->GetWDs().end(); iter++)
	{
		if ((*iter).GetName() == node.GetName())
		{
			(*iter).SetName(new_name);
			break;
		}
	}
	node.SetName(new_name);
	
	// Make prefs as dirty
	CPreferences::sPrefs->mCalendarAccounts.SetDirty();
	
	// Now load it if logged in
	if (IsLoggedOn())
	{
		LoadList();
		SyncList();
	}
}

// Set the working directory prefix
void CCalendarProtocol::RemoveWD(CCalendarStoreNode& node)
{
	// Change name in list and force refresh
	for(CDisplayItemList::iterator iter = GetCalendarAccount()->GetWDs().begin(); iter != GetCalendarAccount()->GetWDs().end(); iter++)
	{
		if ((*iter).GetName() == node.GetName())
		{
			GetCalendarAccount()->GetWDs().erase(iter);
			break;
		}
	}
	
	// Make prefs as dirty
	CPreferences::sPrefs->mCalendarAccounts.SetDirty();
	
	// Now load it if logged in
	if (IsLoggedOn())
	{
		LoadList();
		SyncList();
	}
}

void CCalendarProtocol::LoadList()
{
	// NB This protocol does not delete the list items when it logs out, so we
	// need to do that here before reloading

	// Tell listeners we are about to remove all calendar store nodes
	Broadcast_Message(eBroadcast_ClearList, this);

	// Remove calendar store nodes
	mStoreRoot.Clear();
	
	// Check whether connected or not or whether we always read from local cache
	if (IsDisconnected() || mCacheIsPrimary)
		ReadCalendars();
	else
	{
		// First try the disconnected cache - but only use if current
		if (mListedFromCache || ((mCacheClient == NULL) || !ReadCalendars(true)))
		{
			// Add WDs
			for(CDisplayItemList::const_iterator iter = GetCalendarAccount()->GetWDs().begin(); iter != GetCalendarAccount()->GetWDs().end(); iter++)
			{
				CCalendarStoreNode* node = new CCalendarStoreNode(this, &mStoreRoot, true, false, false, (*iter).GetName());
				node->SetFlags(CCalendarStoreNode::eIsDisplayHierarchy);
				mStoreRoot.AddChild(node);
			}
			
			// Now get new list
			mClient->_ListCalendars(&mStoreRoot);

			// Always keep disconnected cache list in sync with server
			if (mCacheClient != NULL)
			{
				DumpCalendars();
			}
		}
		else
			mListedFromCache = true;
	}

	// Tell listeners we have added calendar store nodes
	Broadcast_Message(eBroadcast_RefreshList, this);
}

// Load calendar list from server
void CCalendarProtocol::LoadSubList(CCalendarStoreNode* node, bool deep)
{
	// When disconnected or cache is primary the list is always up to date
	if (IsDisconnected() || mCacheIsPrimary)
		return;
	
	// Tell listeners we are about to remove all child nodes
	Broadcast_Message(eBroadcast_ClearSubList, node);

	// Remove calendar store nodes
	node->Clear();
	
	// Now get new list
	mClient->_ListCalendars(node);

	// Always keep disconnected cache list in sync with server
	if (mCacheClient != NULL)
	{
		DumpCalendars();
	}

	// Tell listeners we have added child nodes
	Broadcast_Message(eBroadcast_RefreshSubList, node);
}

void CCalendarProtocol::SyncList()
{
	// Get calendar name
	cdstring calname = GetAccountName();
	calname += cMailAccountSeparator;

	// Find subscribed calendars for this account
	cdstrvect subs;
	for(cdstrset::const_iterator iter = CPreferences::sPrefs->mSubscribedCalendars.GetValue().begin(); iter != CPreferences::sPrefs->mSubscribedCalendars.GetValue().end(); iter++)
	{
		if ((*iter).compare_start(calname))
		{
			CCalendarStoreNode* node = SyncCalendarNode(*iter);

			// Sync this node if not a directory
			if ((node != NULL) && !node->IsDirectory())
			{
				// Subcribe but without changing prefs
				CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(node, true, true);
				Broadcast_Message(eBroadcast_RefreshNode, node);
			}
		}
	}
}

CCalendarStoreNode* CCalendarProtocol::SyncCalendarNode(const cdstring& node)
{
	// Break the name down into components
	cdstring acct;
	cdstring path;
	const char* start = node.c_str();
	const char* end = ::strchr(start, cMailAccountSeparator);
	if (end != NULL)
	{
		acct = cdstring(start, end - start);
		path = cdstring(end + 1);
	}
	else
		return NULL;
	
	// Now test account name
	if (acct != GetAccountName())
		return NULL;
	
	return mStoreRoot.FindNodeOrCreate(path);
}

void CCalendarProtocol::RefreshList()
{
	// Reload and sync
	LoadList();
	SyncList();
}

void CCalendarProtocol::RefreshSubList(CCalendarStoreNode* node)
{
	// Reload and sync
	LoadSubList(node, false);
	SyncList();
}

// List was changed in some way
void CCalendarProtocol::ListChanged()
{
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		DumpCalendars();
	}
}

CCalendarStoreNode* CCalendarProtocol::GetNode(const cdstring& cal, bool parent) const
{
	// Break the name down into components
	cdstring acct;
	cdstring path;
	const char* start = cal.c_str();
	const char* end = ::strchr(start, cMailAccountSeparator);
	if (end != NULL)
	{
		acct = cdstring(start, end - start);
		path = cdstring(end + 1);
	}
	else
		return NULL;

	// Now test account name
	if (acct != GetAccountName())
		return NULL;

	if (parent)
	{
		cdstrvect segments;
		path.split(cdstring(GetDirDelim()), segments);
		segments.pop_back();
		path.join(segments, cdstring(GetDirDelim()));
	}

	return ((path.empty() || path == mStoreRoot.GetName()) ? const_cast<CCalendarStoreNode*>(&mStoreRoot) : mStoreRoot.FindNode(path));
}

CCalendarStoreNode* CCalendarProtocol::GetNodeByRemoteURL(const cdstring& url) const
{
	// Scan down entire hierarchy
	return mStoreRoot.FindNodeByRemoteURL(url);
}

CCalendarStoreNode* CCalendarProtocol::GetParentNode(const cdstring& cal) const
{
	return GetNode(cal, true);
}

bool CCalendarProtocol::HasDisconnectedCalendars()
{
	if (CanDisconnect() || mCacheIsPrimary)
	{
		if (!mStoreRoot.HasInferiors())
			ReadCalendars();
		return mStoreRoot.HasInferiors();
	}
	else
		return false;
}

void CCalendarProtocol::CreateCalendar(const CCalendarStoreNode& node)
{
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_CreateCalendar(node);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		mCacheClient->_CreateCalendar(node);
		DumpCalendars();
	}
	
	// Always sync the cache state
	if (IsDisconnected())
		const_cast<CCalendarStoreNode&>(node).TestDisconnectCache();
}

void CCalendarProtocol::TouchCalendar(const CCalendarStoreNode& node)
{
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_TouchCalendar(node);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchCalendar(node))
			DumpCalendars();
	}
	
	// Always sync the cache state
	if (IsDisconnected())
		const_cast<CCalendarStoreNode&>(node).TestDisconnectCache();
}

bool CCalendarProtocol::TestCalendar(const CCalendarStoreNode& node)
{
	return (mCacheIsPrimary && (mCacheClient != NULL)) ? mCacheClient->_TestCalendar(node) : mClient->_TestCalendar(node);
}

void CCalendarProtocol::DeleteCalendar(const CCalendarStoreNode& node)
{
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_DeleteCalendar(node);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TestCalendar(node))
			mCacheClient->_DeleteCalendar(node);
	}
}

void CCalendarProtocol::RenameCalendar(const CCalendarStoreNode& node, const cdstring& node_new)
{
	// Rename it on the server
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_RenameCalendar(node, node_new);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TestCalendar(node))
			mCacheClient->_RenameCalendar(node, node_new);
	}
}

bool CCalendarProtocol::CheckCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	bool result = false;

	// See if offline or disconnected	
	if (IsOffline() || IsDisconnected())
	{
		// Nothing to do here. We will assume that local files are always
		// up to date.
	}
	else if (IsWebCalendar())
	{
		SubscribeFullCalendar(node, cal);
	}
	else
	{
		// Look for local cache first
		if ((mCacheClient != NULL) && !mCacheIsPrimary)
		{
			// Read in the calendar cache if it exists
			if (mCacheClient->_TestCalendar(node))
				mCacheClient->_ReadFullCalendar(node, cal);

			// Sync cache with server doing playback if needed
			result = SyncFromServer(node, cal);
		}
		else
		{
			// Just read full calendar from server
			result = mClient->_CalendarChanged(node, cal);
			if (result)
				ReadFullCalendar(node, cal);
		}
	}

	return result;
}

void CCalendarProtocol::SizeCalendar(CCalendarStoreNode& node)
{
	// Don't do on server if cache is primary
	if (mCacheIsPrimary)
		mCacheClient->_SizeCalendar(node);
	else
		mClient->_SizeCalendar(node);
}

void CCalendarProtocol::OpenCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// See if offline or disconnected	
	if (IsOffline() || IsDisconnected())
	{
		// Just read full calendar from offline/disconnect store
		ReadFullCalendar(node, cal);
	}
	else
	{
		// Look for local cache first
		if ((mCacheClient != NULL) && !mCacheIsPrimary)
		{
			// Read in the calendar cache if it exists
			if (mCacheClient->_TestCalendar(node))
				mCacheClient->_ReadFullCalendar(node, cal);

			// Sync cache with server doing playback if needed
			SyncFromServer(node, cal);
		}
		else
		{
			// Just read full calendar from server
			ReadFullCalendar(node, cal);
		}
	}
	
	// Make sure the calendar has some kind of name
	if (cal.GetName().empty())
	{
		cal.SetName(node.GetShortName());
	}
}

bool CCalendarProtocol::SyncFromServer(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Different based on full or component sync
    bool result = false;
	if (IsComponentCalendar())
		result = SyncComponentsFromServer(node, cal);
	else
		result = SyncFullFromServer(node, cal);
	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
    return result;
}

bool CCalendarProtocol::SyncFullFromServer(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// We need to do this as a proper transaction with locking
    bool result = false;
	try
	{
		// We need to do this as a proper transaction with locking
		mClient->_LockCalendar(node, cal);

		// Get client and server data state
		bool server_changed = false;
		bool cache_changed = false;

		// Special for empty local cache
		if (cal.GetETag().empty())
		{
			// Force server -> cache sync
			server_changed = true;
			cache_changed = false;
		}
		else
		{
			// Check to see whether server/cache state changed
			server_changed = mClient->_CalendarChanged(node, cal);
			cache_changed = cal.NeedsSync();
		}
		
		// Now do appropriate sync (if neither have changed no need to do anything - just use the cache as-is)
		if (server_changed && !cache_changed)
		{
			// Server overwrites local cache
			mClient->_ReadFullCalendar(node, cal);
			
			// Write changes back to local cache
			if (mCacheClient->_TouchCalendar(node))
				DumpCalendars();
			mCacheClient->_WriteFullCalendar(node, cal);
            result = true;
		}
		else if (!server_changed && cache_changed)
		{
			// Local cache overwrites server
			mClient->_WriteFullCalendar(node, cal);
            result = true;
		}
		else if (server_changed && cache_changed)
		{
			// Get temp copy of server calendar
			iCal::CICalendar temp;
			mClient->_ReadFullCalendar(node, temp);
			
			// Sync the two - after this 'cal' will contain the sync'd data
			iCal::CICalendarSync sync(cal, temp);
			sync.Sync();
			
			// Local overwrites server
			cal.ClearRecording();
			
			// Now write back to both server and cache
			mClient->_WriteFullCalendar(node, cal);
			if (mCacheClient->_TouchCalendar(node))
				DumpCalendars();
			mCacheClient->_WriteFullCalendar(node, cal);
            result = true;
		}

		// Always unlock
		mClient->_UnlockCalendar(node, cal);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Always unlock
		mClient->_UnlockCalendar(node, cal);

		CLOG_LOGRETHROW;
		throw;
	}
    
    return result;
}

bool CCalendarProtocol::SyncComponentsFromServer(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	if (! GetDidSyncTest())
	{
		mClient->_TestFastSync(node);
	}
	if (GetHasSync())
	{
		return SyncComponentsFromServerFast(node, cal);
	}
	else
	{
		return SyncComponentsFromServerSlow(node, cal);
	}
}

bool CCalendarProtocol::SyncComponentsFromServerFast(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// We need to do this as a proper transaction with locking
    bool result = false;
	try
	{
		// Policy:
		//
		// 1. Get list of changes on server
		//
		// 2. Look at record DB and:
		//  2.1 Add new components to server and cache info
		//  2.2 Remove deleted components from server and cached server info if still present on server
		//  2.3 Cache info for changed items for later sync (or change them immediately if the server has not changed),
		//      or remove cached items if removed from server
		//
		// 3. Scan list of local components
		//  3.1 If in server removed set, remove locally
		//  3.2 If in server changed set
		//   3.2.1 If server etag is unchanged then change component on server
		//   3.2.2 Otherwise determine whether server or cached component wins and use that
		//
		// 4. Copy remaining items in server set to local cache - they are new items
		//
		// We only need to do steps 3 & 4 if the data on the server is known to have changed.
		//
		
		// Now do it...
		
		// Step 1
		cdstrmap changed;
		cdstrset removed;
		cdstring synctoken = cal.GetSyncToken();
		mClient->_FastSync(node, cal, changed, removed, synctoken);
		
		if (synctoken.empty() && !cal.GetSyncToken().empty())
		{
            // Original token was invalid so invalidate the entire local cache and do a full sync
            cal.ClearSync();
			changed.clear();
			removed.clear();
			mClient->_FastSync(node, cal, changed, removed, synctoken);
		}
		
		if (synctoken.empty())
		{
			return false;
		}
		bool server_changed = synctoken != cal.GetSyncToken();
		bool changes_made = false;
		
		// Step 2
		cdstrmap both_changed;
		for(iCal::CICalendarComponentRecordDB::const_iterator iter = cal.GetRecording().begin(); iter != cal.GetRecording().end(); iter++)
		{
			// Step 2.1
			if ((*iter).second.GetAction() == iCal::CICalendarComponentRecord::eAdded)
			{
				// Add component to server
				const iCal::CICalendarComponent* comp = cal.GetComponentByKey((*iter).first);
				if (comp != NULL)
				{
					// Add component to server
					mClient->_AddComponent(node, cal, *comp);
					changes_made = true;
                    result = true;
				}
			}
			
			// Step 2.2
			else if ((*iter).second.GetAction() == iCal::CICalendarComponentRecord::eRemoved)
			{
				// Is it still present on the server
				if (removed.count((*iter).second.GetRURL()) == 0)
				{
					// Remove component from server
					mClient->_RemoveComponent(node, cal, (*iter).second.GetRURL());
					changes_made = true;
                    result = true;
					
					// Remove from server changed info
					if (changed.count((*iter).second.GetRURL()) != 0)
						changed.erase((*iter).second.GetRURL());
				}
			}
			
			// Step 2.3
			else if ((*iter).second.GetAction() == iCal::CICalendarComponentRecord::eChanged)
			{
				// Removed on server?
				if (removed.count((*iter).second.GetRURL()) != 0)
				{
					cal.RemoveComponentByKey((*iter).first);
                    result = true;
				}
				
				// Changed on server?
				else if (changed.count((*iter).second.GetRURL()) != 0)
				{
					both_changed.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				}

				// Changed locally only
				else
				{
					// Change it on the server
					const iCal::CICalendarComponent* comp = cal.GetComponentByKey((*iter).first);
					mClient->_ChangeComponent(node, cal, *comp);
					changes_made = true;
                    result = true;
				}
			}
		}
		
		
		// Now do sync
		if (server_changed)
		{
			// Get component info from cache
			iCal::CICalendarComponentDBList dbs;
			cal.GetAllDBs(dbs);
			cdstrvect component_keys;
			for(iCal::CICalendarComponentDBList::const_iterator iter1 = dbs.begin(); iter1 != dbs.end(); iter1++)
			{
				for(iCal::CICalendarComponentDB::const_iterator iter2 = (*iter1)->begin(); iter2 != (*iter1)->end(); iter2++)
				{
					component_keys.push_back((*iter2).second->GetMapKey());
				}
			}
			
			// Step 3
			cdstrvect rurls;
			for(cdstrvect::const_iterator iter = component_keys.begin(); iter != component_keys.end(); iter++)
			{
				iCal::CICalendarComponent* cache_comp = cal.GetComponentByKey(*iter);
				if (cache_comp == NULL)
					continue;
				
				// Get this components RURL
				cdstring cache_rurl = cache_comp->GetRURL();
				cdstring cache_etag = cache_comp->GetETag();
				
				// Step 3.1
				cdstrset::const_iterator found_removed = removed.find(cache_rurl);
				if (found_removed != removed.end())
				{
					// Remove locally
					cal.RemoveComponentByKey(cache_comp->GetMapKey());
                    result = true;
					continue;
				}
				
				// Get the server info
				cdstring server_rurl;
				cdstring server_etag;
				cdstrmap::const_iterator found = changed.find(cache_rurl);
				if (found != changed.end())
				{
					server_rurl = (*found).first;
					server_etag = (*found).second;
				}
				
				// Step 3.2
				if (!server_rurl.empty())
				{
					if (both_changed.count(server_rurl) != 0)
					{
						// Step 3.2.1
						if (cache_etag == server_etag)
						{
							// Write changed cache component to server
							mClient->_ChangeComponent(node, cal, *cache_comp);
							changes_made = true;
                            result = true;
						}
						
						// Step 3.2.2
						else
						{
							// Do iCal SEQ etc comparison
							
							// First read in component from server into temp calendar
							iCal::CICalendar tempcal;
							iCal::CICalendarComponent* server_comp = mClient->_ReadComponent(node, tempcal, server_rurl);
							if (server_comp != NULL)
							{
								int compare_result = iCal::CICalendarSync::CompareComponentVersions(server_comp, cache_comp);
								
								if (compare_result == 1)
								{
									// Cache is newer than server - cache overwrites to server
									mClient->_ChangeComponent(node, cal, *cache_comp);
									changes_made = true;
                                    result = true;
								}
								else if (compare_result == -1)
								{
									// Cache is older than server - server overwrites cache
									
									// Remove the cached component first
									cal.RemoveComponentByKey(cache_comp->GetMapKey());
									cache_comp = NULL;
									
									// Copy component from server into local cache effectively replacing old one
									iCal::CICalendarComponent* new_comp = server_comp->clone();
									new_comp->SetCalendar(cal.GetRef());
									cal.AddComponent(new_comp);
                                    result = true;
								}
							}
						}
					}
					else if (cache_etag != server_etag)
					{
						rurls.push_back(cache_rurl);
					}
					changed.erase(server_rurl);
				}
			}
			
			// Read components from server into local cache as its a new one on the server
			for(cdstrmap::const_iterator iter = changed.begin(); iter != changed.end(); iter++)
			{
				rurls.push_back((*iter).first);
			}

            while(rurls.size() != 0)
            {
                // Limit to at most 50 resources
                cdstrvect rurls_batched;
                while(rurls.size() != 0 and rurls_batched.size() < 50)
                {
                    rurls_batched.push_back(rurls.back());
                    rurls.pop_back();
                }
            
				mClient->_ReadComponents(node, cal, rurls_batched);
            }
		}
		
		// Clear out cache recording
		cal.ClearRecording();
		
		// Get the current server sync token if changes were made or it was different
		if (server_changed || changes_made)
		{
			mClient->_UpdateSyncToken(node, cal);
		}
		cal.SetSyncToken(synctoken);
		
		// Now write back cache
		if (!mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
		
		CLOG_LOGRETHROW;
		throw;
	}
    
    return result;
}

bool CCalendarProtocol::SyncComponentsFromServerSlow(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// We need to do this as a proper transaction with locking
    bool result = false;
	try
	{
		// Policy:
		//
		// 1. Get list of current items on server
		//
		// 2. Look at record DB and:
		//  2.1 Add new components to server and cache info
		//  2.2 Remove deleted components from server and cached server info if still present on server
		//  2.3 Cache info for changed items for later sync (or change them immediately if the server has not changed
		//
		// 3. Scan list of local components
		//  3.1 If in added set, ignore it
		//  3.2 If in changed set
		//   3.2.1 If server etag is unchanged then change component on server
		//   3.2.2 Otherwise determine whether server or cached component wins and use that
		//  3.3 If on server
		//   3.3.1 If server etag has changed copy server comnponent to cache
		//  3.4 Remove from local cache - its been removed from the server
		//  3.5 Remove from server set
		//
		// 4. Copy remaining items in server set to local cache - they are new items
		//
		// We only need to do steps 3 & 4 if the data on the server is known to have changed.
		//
		
		// Now do it...

		// Step 1
		cdstrmap comps;
		bool server_changed = mClient->_CalendarChanged(node, cal);
		bool changes_made = false;
		if (server_changed)
			mClient->_GetComponentInfo(node, cal, comps);
		else
		{
			mCacheClient->_ReadFullCalendar(node, cal);
		}
		
		// Step 2
		cdstrmap cache_added;
		cdstrmap cache_changed;
		for(iCal::CICalendarComponentRecordDB::const_iterator iter = cal.GetRecording().begin(); iter != cal.GetRecording().end(); iter++)
		{
			// Step 2.1
			if ((*iter).second.GetAction() == iCal::CICalendarComponentRecord::eAdded)
			{
				// Add component to server
				const iCal::CICalendarComponent* comp = cal.GetComponentByKey((*iter).first);
				if (comp != NULL)
				{
					// Add component to server
					mClient->_AddComponent(node, cal, *comp);
					changes_made = true;
                    result = true;

					// Add to added cache
					cache_added.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				}
			}
			
			// Step 2.2
			else if ((*iter).second.GetAction() == iCal::CICalendarComponentRecord::eRemoved)
			{
				// Is it still present on the server
				if (comps.count((*iter).second.GetRURL()) != 0)
				{
					// Remove component from server
					mClient->_RemoveComponent(node, cal, (*iter).second.GetRURL());
					changes_made = true;
                    result = true;
					
					// Remove from server component info
					comps.erase((*iter).second.GetRURL());
				}
			}

			// Step 2.3
			else if ((*iter).second.GetAction() == iCal::CICalendarComponentRecord::eChanged)
			{
				if (server_changed)
					cache_changed.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				else
				{
					// Change it on the server
					const iCal::CICalendarComponent* comp = cal.GetComponentByKey((*iter).first);
					mClient->_ChangeComponent(node, cal, *comp);
					changes_made = true;
                    result = true;
					cache_changed.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				}
			}
		}
		
		
		// Now do sync
		if (server_changed)
		{
			// Get component info from cache
			iCal::CICalendarComponentDBList dbs;
			cal.GetAllDBs(dbs);
			cdstrvect component_keys;
			for(iCal::CICalendarComponentDBList::const_iterator iter1 = dbs.begin(); iter1 != dbs.end(); iter1++)
			{
				for(iCal::CICalendarComponentDB::const_iterator iter2 = (*iter1)->begin(); iter2 != (*iter1)->end(); iter2++)
				{
					component_keys.push_back((*iter2).second->GetMapKey());
				}
			}
			
			// Step 3
			cdstrset matching_components;
			for(cdstrvect::const_iterator iter = component_keys.begin(); iter != component_keys.end(); iter++)
			{
				iCal::CICalendarComponent* cache_comp = cal.GetComponentByKey(*iter);
				if (cache_comp == NULL)
					continue;

				// Get this components RURL
				cdstring cache_rurl = cache_comp->GetRURL();
				cdstring cache_etag = cache_comp->GetETag();
				
				// Get the server info
				cdstring server_rurl;
				cdstring server_etag;
				cdstrmap::const_iterator found = comps.find(cache_rurl);
				if (found != comps.end())
				{
					server_rurl = (*found).first;
					server_etag = (*found).second;
				}

				// Step 3.1
				if (cache_added.count(cache_rurl) != 0)
					continue;
				
				// Step 3.2
				else if (cache_changed.count(cache_rurl) != 0)
				{
					// Step 3.2.1
					if (cache_etag == server_etag)
					{
						// Write changed cache component to server
						mClient->_ChangeComponent(node, cal, *cache_comp);
                        result = true;
						changes_made = true;
					}
					
					// Step 3.2.2
					else
					{
						// Do iCal SEQ etc comparison
						
						// First read in component from server into temp calendar
						iCal::CICalendar tempcal;
						iCal::CICalendarComponent* server_comp = mClient->_ReadComponent(node, tempcal, server_rurl);
						if (server_comp != NULL)
						{
							int compare_result = iCal::CICalendarSync::CompareComponentVersions(server_comp, cache_comp);
							
							if (compare_result == 1)
							{
								// Cache is newer than server - cache overwrites to server
								mClient->_ChangeComponent(node, cal, *cache_comp);
                                result = true;
								changes_made = true;
							}
							else if (compare_result == -1)
							{
								// Cache is older than server - server overwrites cache

								// Remove the cached component first
								cal.RemoveComponentByKey(cache_comp->GetMapKey());
								cache_comp = NULL;
								
								// Copy component from server into local cache effectively replacing old one
								iCal::CICalendarComponent* new_comp = server_comp->clone();
								new_comp->SetCalendar(cal.GetRef());
								cal.AddComponent(new_comp);
                                result = true;
							}
						}
					}
				}
				
				// Step 3.3
				else if (!server_rurl.empty())
				{
					// Step 3.3.1
					if (cache_etag != server_etag)
					{
						// Copy from server to cache overwriting cache value
						
						// Remove the cached component first
						cal.RemoveComponentByKey(cache_comp->GetMapKey());
						cache_comp = NULL;
						
						// Read component from server into local cache effectively replacing old one
						mClient->_ReadComponent(node, cal, server_rurl);
                        result = true;
					}
				}
				
				// Step 3.4
				else
				{
					// comp is deleted in this call
					cal.RemoveComponentByKey(cache_comp->GetMapKey());
					cache_comp = NULL;
                    result = true;
				}
				
				// Step 3.5
				if (!server_rurl.empty())
					matching_components.insert(server_rurl);
			}
			
			// Step 4
			cdstrvect rurls;
			for(cdstrmap::const_iterator iter = comps.begin(); iter != comps.end(); iter++)
			{
				cdstrset::const_iterator found = matching_components.find((*iter).first);
				if (found == matching_components.end())
					rurls.push_back((*iter).first);
			}

			// Read components from server into local cache as its a new one on the server
            while(rurls.size() != 0)
            {
                // Limit to at most 50 resources
                cdstrvect rurls_batched;
                while(rurls.size() != 0 and rurls_batched.size() < 50)
                {
                    rurls_batched.push_back(rurls.back());
                    rurls.pop_back();
                }
                
                mClient->_ReadComponents(node, cal, rurls_batched);
            }
		}

		// Clear out cache recording
		cal.ClearRecording();
		
		// Get the current server sync token if changes were made or it was differemt
		if (server_changed || changes_made)
			mClient->_UpdateSyncToken(node, cal);

		// Now write back cache
		if (!mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		CLOG_LOGRETHROW;
		throw;
	}
    
    return result;
}

void CCalendarProtocol::CloseCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	
}

// Overwrite calendar at node with new calendar data
void CCalendarProtocol::CopyCalendar(const CCalendarStoreNode& node, iCal::CICalendar& newcal)
{
	// NB This assumes that node does not have an open calendar associated with it

	// Different based on full or component sync
	if (IsComponentCalendar())
	{
		// Delete existing contents
		RemoveAllComponents(node, newcal);
		
		// Write out new contents
		WriteFullCalendar(node, newcal);
	}
	else
		// Overwrite existing item
		WriteFullCalendar(node, newcal);
}

void CCalendarProtocol::CopyCalendarContents(const CCalendarStoreNode& node, iCal::CICalendar& newcal)
{
	// NB This assumes that node does not have an open calendar associated with it

	// Different based on full or component sync
	if (IsComponentCalendar())
	{
		// Write out new contents
		WriteFullCalendar(node, newcal);
	}
	else
	{
		// Read existing items into calendar
		mClient->_ReadFullCalendar(node, newcal);

		// Write everything back including new items
		WriteFullCalendar(node, newcal);
	}
}

void CCalendarProtocol::ReadFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal, bool if_changed)
{
	// Don't do on server if cache is primary
	if (mCacheIsPrimary)
		mCacheClient->_ReadFullCalendar(node, cal, if_changed);
	else
	{
		mClient->_ReadFullCalendar(node, cal, if_changed);
		if (IsComponentCalendar())
			mClient->_UpdateSyncToken(node, cal);
	}

	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
}

void CCalendarProtocol::WriteFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Don't do on server if cache is primary
	if (mCacheIsPrimary)
		mCacheClient->_WriteFullCalendar(node, cal);
	else
	{
		mClient->_WriteFullCalendar(node, cal);
		cal.SetDirty(false);

		// Always keep disconnected cache in sync with server
		if (mCacheClient != NULL)
		{
			if (mCacheClient->_TouchCalendar(node))
				DumpCalendars();
			mCacheClient->_WriteFullCalendar(node, cal);

			// Set sync time in node
			node.SyncNow();
		}
	}
}

void CCalendarProtocol::SubscribeFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Always read from the main server
	bool if_changed = !cal.GetETag().empty();
	if (! if_changed)
		cal.Clear();
	mClient->_ReadFullCalendar(node, cal, if_changed);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		// Server always overwrites cache
		if (mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);

		// Set sync time in node
		node.SyncNow();
	}
}

void CCalendarProtocol::PublishFullCalendar(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Always write to the main server
	mClient->_WriteFullCalendar(node, cal);

	// Set sync time in node
	node.SyncNow();
	
	// Node state changed
	DumpCalendars();
}

void CCalendarProtocol::AddComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Check for per-component protocol
	if (!IsComponentCalendar() || IsDisconnected())
	{
		// Always fall back to writing entire calendar
		WriteFullCalendar(node, cal);
		Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
		return;
	}
	
	// Write event to server
	mClient->_AddComponent(node, cal, component);
	cal.ClearRecording();
	cal.SetDirty(false);
	
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);

		// Set sync time in node
		node.SyncNow();
	}

	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
}

void CCalendarProtocol::ChangeComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Check for per-component protocol
	if (!IsComponentCalendar() || IsDisconnected())
	{
		// Always fall back to writing entire calendar
		WriteFullCalendar(node, cal);
		Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
		return;
	}
	
	// Write event to server
	mClient->_ChangeComponent(node, cal, component);
	cal.ClearRecording();
	cal.SetDirty(false);
	
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);

		// Set sync time in node
		node.SyncNow();
	}

	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
}

void CCalendarProtocol::RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const iCal::CICalendarComponent& component)
{
	// Check for per-component protocol
	if (!IsComponentCalendar() || IsDisconnected())
	{
		// Always fall back to writing entire calendar
		WriteFullCalendar(node, cal);
		Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
		return;
	}
	
	// Write event to server
	mClient->_RemoveComponent(node, cal, component);
	cal.ClearRecording();
	cal.SetDirty(false);
	
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);

		// Set sync time in node
		node.SyncNow();
	}

	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
}

void CCalendarProtocol::RemoveComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl)
{
	// Check for per-component protocol
	if (!IsComponentCalendar() || IsDisconnected())
	{
		// Always fall back to writing entire calendar
		WriteFullCalendar(node, cal);
		Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
		return;
	}
	
	// Write event to server
	mClient->_RemoveComponent(node, cal, rurl);
	cal.ClearRecording();
	cal.SetDirty(false);
	
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchCalendar(node))
			DumpCalendars();
		mCacheClient->_WriteFullCalendar(node, cal);

		// Set sync time in node
		node.SyncNow();
	}

	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
}

// Only called for component based calendar stores
void CCalendarProtocol::RemoveAllComponents(const CCalendarStoreNode& node, iCal::CICalendar& cal)
{
	// Get component info and delete each one
	cdstrmap comps;
	mClient->_GetComponentInfo(node, cal, comps);

	for(cdstrmap::const_iterator iter = comps.begin(); iter != comps.end(); iter++)
	{
		mClient->_RemoveComponent(node, cal, (*iter).first);
	}
	cal.ClearRecording();
	cal.SetDirty(false);
	
	// Always keep disconnected cache in sync with server
	if ((mCacheClient != NULL) && mCacheClient->_TestCalendar(node))
	{
		mCacheClient->_DeleteCalendar(node);
		
		// Node state changed
		DumpCalendars();
	}

	Broadcast_Message(eBroadcast_RefreshNode, (void*)&node);
}

void CCalendarProtocol::ReadComponent(const CCalendarStoreNode& node, iCal::CICalendar& cal, const cdstring& rurl)
{
	// Read event from server
	mClient->_ReadComponent(node, cal, rurl);
}

#pragma mark ____________________________ACLs

// Set new value on server
void CCalendarProtocol::SetACL(CCalendarStoreNode& node, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of adding/replacing in list if successful
	mClient->_SetACL(node, acl);
}

// Delete value on server
void CCalendarProtocol::DeleteACL(CCalendarStoreNode& node, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of adding/replacing in list if successful
	mClient->_DeleteACL(node, acl);
}

// Get values on server
void CCalendarProtocol::GetACL(CCalendarStoreNode& node)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of recovery
	mClient->_GetACL(node);
}

// List rights on server
void CCalendarProtocol::ListRights(CCalendarStoreNode& node, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of recovery
	mClient->_ListRights(node, acl);
}

// List user's rights on server
void CCalendarProtocol::MyRights(CCalendarStoreNode& node)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of recovery
	mClient->_MyRights(node);
}

void CCalendarProtocol::GetScheduleInboxOutbox(cdstring& inboxURI, cdstring& outboxURI)
{
	// Do nothing if not available
	if (!GetHasScheduling())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of recovery
	mClient->_GetScheduleInboxOutbox(mStoreRoot, inboxURI, outboxURI);
}

void CCalendarProtocol::Schedule(const cdstring& outboxURI,
								 const cdstring& originator,
								 const cdstrvect& recipients,
								 const iCal::CICalendar& cal,
								 iCal::CITIPScheduleResultsList& results)
{
	// Do nothing if not available
	if (!GetHasScheduling())
		return;

	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);

	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;

	// Send command to server
	// Calendar will take care of recovery
	mClient->_Schedule(outboxURI, originator, recipients, cal, results);
}

void CCalendarProtocol::GetFreeBusyCalendars(cdstrvect& calendars)
{
	// Do nothing if not available
	if (!GetHasScheduling())
		return;
	
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);
	
	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;
	
	// Send command to server
	mClient->_GetFreeBusyCalendars(calendars);
}

void CCalendarProtocol::SetFreeBusyCalendars(const cdstrvect& calendars)
{
	// Do nothing if not available
	if (!GetHasScheduling())
		return;
	
	// Must block
	cdmutex::lock_cdmutex _lock(_mutex);
	
	// Verify its still active because mutex wait could have resulted in closure
	if (!IsLoggedOn())
		return;
	
	// Send command to server
	mClient->_SetFreeBusyCalendars(calendars);
}

#pragma mark ____________________________Disconnected

void CCalendarProtocol::InitDisconnect()
{
	// Do inherited to setup directory
	CINETProtocol::InitDisconnect();

	// Do not do this for HTTP accounts as we do not do playback with those
	switch(GetAccountType())
	{
	case CINETAccount::eWebDAVCalendar:
	case CINETAccount::eCalDAVCalendar:
		// Now create recorder
		if (!mRecorder)
		{
			mRecorder = new CCalendarRecord;
			mRecorder->SetDescriptor(mOfflineCWD + "Record.mbr");
			mRecorder->SetFullRecord();
		}
		if (mRecorder)
			mRecorder->Open();
		break;
	default:;
	}
}

const cdstring& CCalendarProtocol::GetOfflineDefaultDirectory()
{
	static cdstring name1("Calendars");
	static cdstring name2("Web Calendars");
	return (GetAccountType() == CINETAccount::eHTTPCalendar) ? name2 : name1;
}

void CCalendarProtocol::GoOffline()
{
	// Check that offline directory exists
	if (mOfflineCWD.empty())
		InitOffline();

	// Dump current calendars list to disk
	DumpCalendars();

	// Now do inherited connection switch
	CINETProtocol::GoOffline();
	
	// Check each node to get cached state
	mStoreRoot.TestDisconnectCache();
}

void CCalendarProtocol::GoOnline()
{
	// Always playback sync'd data
	DoPlayback();

	// Reset state of calendars
	RecoverCalendars();

	// Now do inherited connection switch
	CINETProtocol::GoOnline();
}

void CCalendarProtocol::RecoverCalendars()
{
}

void CCalendarProtocol::SynchroniseDisconnect(bool fast)
{
}

void CCalendarProtocol::SynchroniseRemote(CCalendarStoreNode& node, bool fast)
{
}

void CCalendarProtocol::ClearDisconnect(CCalendarStoreNode& node)
{
}

bool CCalendarProtocol::DoPlayback()
{
	// Ignore if no defined recorder
	if (!mRecorder)
		return true;

	// Create remote clone
	std::auto_ptr<CCalendarProtocol> clone(new CCalendarProtocol(*this, false, true));
	clone->SetSynchronising();

	// Prevent further recording
	mRecorder->SetNoRecord();

	bool result = mRecorder->Playback(clone.get(), this);

	// Enable recording
	mRecorder->SetFullRecord();
	
	return result;
}

const char* cCalendarListName = "calendarlist.xml";

void CCalendarProtocol::DumpCalendars()
{
	cdstring list_name = mOfflineCWD + cCalendarListName;

	// Create output file
	cdofstream fout(list_name.c_str());
	if (fout.fail())
		return;

	// Create XML document object
	std::auto_ptr<xmllib::XMLDocument> doc(new xmllib::XMLDocument);
	
	// Root element is the preferences element
	doc->GetRoot()->SetName(cXMLElement_calendarlist);
	xmllib::XMLObject::WriteAttribute(doc->GetRoot(), cXMLAttribute_version, (uint32_t)1);
	
	// Store the current date
	iCal::CICalendarDateTime dt = iCal::CICalendarDateTime::GetNowUTC();
	dt.SetDateOnly(true);
	xmllib::XMLObject::WriteAttribute(doc->GetRoot(), cXMLAttribute_datestamp, dt.GetText());
	
	// Now add each node (and child nodes) to XML doc
	mStoreRoot.WriteXML(doc.get(), doc->GetRoot(), true);
	
	// Write to stream
	doc->Generate(fout);
	
}

bool CCalendarProtocol::ReadCalendars(bool only_if_current)
{
	bool is_current = false;
	cdstring list_name = mOfflineCWD + cCalendarListName;
	
	// XML parse the data
	xmllib::XMLSAXSimple parser;
	parser.ParseFile(list_name);

	// See if we got any valid XML
	if (parser.Document())
	{
		// Check root node
		xmllib::XMLNode* root = parser.Document()->GetRoot();
		if (!root->CompareFullName(cXMLElement_calendarlist))
			return is_current;
		
		// Get the datestamp
		cdstring dtstamp;
		if (xmllib::XMLObject::ReadAttribute(root, cXMLAttribute_datestamp, dtstamp))
		{
			iCal::CICalendarDateTime dtnow = iCal::CICalendarDateTime::GetNowUTC();
			dtnow.SetDateOnly(true);

			iCal::CICalendarDateTime dt;
			dt.Parse(dtstamp);
			
			iCal::CICalendarDuration diff = dtnow - dt;
			is_current = (diff.GetTotalSeconds() <= 48 * 60 * 60);
		}

		if (!only_if_current || is_current)
		{
			// Now have store root read in all children
			mStoreRoot.ReadXML(root, true);

			// Always cache the disconnected state
			if (IsDisconnected() || mCacheIsPrimary)
				mStoreRoot.TestDisconnectCache();
		}
	}
	
	return is_current;
}
