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


// CAdbkProtocol

#include "CAdbkProtocol.h"

#include "CACAPClient.h"
#include "CActionManager.h"
#include "CAdbkClient.h"
#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "CConnectionManager.h"
#include "CIMSPClient.h"
#include "CLocalAdbkClient.h"
#include "CLocalVCardClient.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Carbon
#include "CMacOSXAdbkClient.h"
#endif
#endif
#include "CPasswordManager.h"
#include "CPreferences.h"

#include "CICalendarDateTime.h"
#include "CICalendarDuration.h"

#include "CCardDAVVCardClient.h"

#include "CVCardStoreXML.h"					// Share XML defintions with calendar store

#include "CVCardAddressBook.h"
#include "CVCardMapper.h"
#include "CVCardSync.h"
#include "CVCardVCard.h"

#include "cdfstream.h"

#include "XMLDocument.h"
#include "XMLNode.h"
#include "XMLObject.h"
#include "XMLSAXSimple.h"

#include <algorithm>
#include <memory>

#define USE_LOCAL_VCARDS

using namespace vcardstore;

// CAdbkProtocol: Handles quotas for all resources

// Constructor
CAdbkProtocol::CAdbkProtocol(CINETAccount* account)
	: CINETProtocol(account),
		mStoreRoot(this)
{
	mClient = NULL;
	mCacheClient = NULL;
	mCacheIsPrimary = false;
	mSyncingList = false;
	mListedFromCache = false;

	// Only IMSP/ACAP/CardDAV servers can disconnect
	switch(GetAccountType())
	{
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
	case CINETAccount::eCardDAVAdbk:
		SetFlags(eCanDisconnect, IsOfflineAllowed() && GetAddressAccount()->GetDisconnected());
		break;
	default:
		SetFlags(eCanDisconnect, false);
		SetACLDisabled(true);
		break;
	}

	// If not login at startup but it can disconnect, set to force disconnect mode
	mFlags.Set(eForceDisconnect, !GetAddressAccount()->GetLogonAtStart());

	switch(GetAccountType())
	{
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
		mDirDelim = os_dir_delim;
		break;
	case CINETAccount::eIMSP:
		mDirDelim = '.';
		break;
	case CINETAccount::eACAP:
	case CINETAccount::eCardDAVAdbk:
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
CAdbkProtocol::CAdbkProtocol(const CAdbkProtocol& copy, bool force_local, bool force_remote)
	: CINETProtocol(copy),
		mStoreRoot(this)
{
	// Init instance variables
	mClient = NULL;
	mCacheClient = NULL;
	mCacheIsPrimary = false;

	mRecorder = NULL; 				// Only original proto uses this - clones do not

	mDirDelim = copy.mDirDelim;

	// Copy client
	if (((GetAccountType() == CINETAccount::eIMSP) ||
		 (GetAccountType() == CINETAccount::eACAP) ||
		 (GetAccountType() == CINETAccount::eCardDAVAdbk))
		&& (force_local || force_remote))
	{
		if (force_remote)
		{
			SetFlags(eIsOffline, false);
			SetFlags(eDisconnected, false);
			switch(GetAccountType())
			{
			case CINETAccount::eIMSP:
				mClient = new CIMSPClient(NULL, this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalVCardClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eACAP:
				mClient = new CACAPClient(NULL, this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalVCardClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eCardDAVAdbk:
				mClient = new CCardDAVVCardClient(this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalVCardClient(this);
					mCacheIsPrimary = false;
				}
				break;
			default:;
			}
		}
		else
		{
			SetFlags(eIsOffline, true);
			SetFlags(eDisconnected, true);
			InitDisconnect();
			mClient = new CLocalVCardClient(this);
			static_cast<CLocalVCardClient*>(mClient)->SetRecorder(mRecorder);
		}
		CINETProtocol::mClient = mClient;
	}
	else
		CopyClient(copy);

}

// Default destructor
CAdbkProtocol::~CAdbkProtocol()
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

void CAdbkProtocol::SetAccount(CINETAccount* account)
{
	// Do inherited
	CINETProtocol::SetAccount(account);
	
	// Check disconnected state
	switch(GetAccountType())
	{
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
	case CINETAccount::eCardDAVAdbk:
		SetFlags(eCanDisconnect, IsOfflineAllowed() &&
									GetAddressAccount()->GetDisconnected());
		break;
	default:
		SetFlags(eCanDisconnect, false);
		break;
	}

	switch(GetAccountType())
	{
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
		mDirDelim = os_dir_delim;
		break;
	case CINETAccount::eIMSP:
		mDirDelim = '.';
		break;
	case CINETAccount::eACAP:
	case CINETAccount::eCardDAVAdbk:
		mDirDelim = '/';
		break;
	default:
		mDirDelim = 0;
		break;
	}

	// Look for change in account name
	if (mStoreRoot.GetName() != GetAccountName())
	{
		// Need to rename offline
		if (mCacheClient != NULL)
		{
			RenameOffline();
		}
		
		// Rename root
		mStoreRoot.SetName(GetAccountName());
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

void CAdbkProtocol::DirtyAccount()
{
	// Must override in derived classes
	switch(GetAccountType())
	{
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
	case CINETAccount::eCardDAVAdbk:
		CPreferences::sPrefs->mAddressAccounts.SetDirty();
		break;
	case CINETAccount::eOSAdbk:
		CPreferences::sPrefs->mOSAdbkAccount.SetDirty();
		break;
	default:
		break;
	}
}

void CAdbkProtocol::CreateClient()
{
	RemoveClient();

	// Check disconnected state
	switch(GetAccountType())
	{
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
	case CINETAccount::eCardDAVAdbk:
		if (CConnectionManager::sConnectionManager.IsConnected() && !IsForceDisconnect() || !CanDisconnect())
		{
			SetFlags(eIsOffline, false);
			SetFlags(eDisconnected, false);
			switch(GetAccountType())
			{
			case CINETAccount::eIMSP:
				mClient = new CIMSPClient(NULL, this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalVCardClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eACAP:
				mClient = new CACAPClient(NULL, this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalVCardClient(this);
					mCacheIsPrimary = false;
				}
				break;
			case CINETAccount::eCardDAVAdbk:
				mClient = new CCardDAVVCardClient(NULL, this);
				if (CanDisconnect())
				{
					InitOfflineCWD();
					mCacheClient = new CLocalVCardClient(this);
					mCacheIsPrimary = false;
				}
				break;
			default:;
			}
		}
		else
		{
			SetFlags(eIsOffline, true);
			SetFlags(eDisconnected, true);
			InitDisconnect();
			mClient = new CLocalVCardClient(this);
			static_cast<CLocalVCardClient*>(mClient)->SetRecorder(mRecorder);
		}
		break;
	case CINETAccount::eLocalAdbk:
		SetFlags(eIsOffline, true);
		SetFlags(eDisconnected, false);
		InitOffline();
#ifdef USE_LOCAL_VCARDS
		mClient = new CLocalVCardClient(this);
#else
		mClient = new CLocalAdbkClient(this);
#endif
		break;
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Carbon
	case CINETAccount::eOSAdbk:
		SetFlags(eIsOffline, true);
		SetFlags(eDisconnected, false);
		InitOffline();
		mClient = new CMacOSXAdbkClient(this);
		break;
#endif
#endif
	default:;
	}

	CINETProtocol::mClient = mClient;
}

void CAdbkProtocol::CopyClient(const CINETProtocol& copy)
{
	// NB This is called in the constructor and cannot rely on virtual functions
	// being called for this object!
	CAdbkClient* copy_it = static_cast<const CAdbkProtocol&>(copy).mClient;
	if (dynamic_cast<CIMSPClient*>(copy_it))
		mClient = new CIMSPClient(static_cast<const CIMSPClient&>(*copy_it), NULL, this);
	else if (dynamic_cast<CACAPClient*>(copy_it))
		mClient = new CACAPClient(static_cast<const CACAPClient&>(*copy_it), NULL, this);
	else if (dynamic_cast<CCardDAVVCardClient*>(copy_it))
		mClient = new CCardDAVVCardClient(static_cast<const CCardDAVVCardClient&>(*copy_it), this);
	else if (dynamic_cast<CLocalAdbkClient*>(copy_it))
		mClient = new CLocalAdbkClient(static_cast<const CLocalAdbkClient&>(*copy_it), this);
	else if (dynamic_cast<CLocalVCardClient*>(copy_it))
		mClient = new CLocalVCardClient(static_cast<const CLocalVCardClient&>(*copy_it), this);
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#if PP_Target_Carbon
	else if (dynamic_cast<CMacOSXAdbkClient*>(copy_it))
		mClient = new CMacOSXAdbkClient(static_cast<const CMacOSXAdbkClient&>(*copy_it), this);
#endif
#endif
	else
		mClient = NULL;

	CINETProtocol::mClient = mClient;
}

// Set into synchronising mode
void CAdbkProtocol::SetSynchronising()
{
	// Just remove recorder
	if (mRecorder)
	{
		// Remove from local client
#ifdef USE_LOCAL_VCARDS
		CLocalVCardClient* client = dynamic_cast<CLocalVCardClient*>(mClient);
#else
		CLocalAdbkClient* client = dynamic_cast<CLocalAdbkClient*>(mClient);
#endif
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

void CAdbkProtocol::RemoveClient()
{
	delete mClient;
	mClient = NULL;
	CINETProtocol::mClient = NULL;
	
	delete mCacheClient;
	mCacheClient = NULL;
	mCacheIsPrimary = false;
}

// Open connection to protocol server
void CAdbkProtocol::Open()
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

}

// Close connection to protocol server
void CAdbkProtocol::Close()
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
void CAdbkProtocol::Logon()
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
				SetCachedPswd(auth->GetUID(), auth->GetPswd());
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

// Logoff from protocol server
void CAdbkProtocol::Logoff()
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

const char* CAdbkProtocol::GetUserPrefix() const
{
	switch(GetAccountType())
	{
	case CINETAccount::eIMSP:
	case CINETAccount::eLocalAdbk:
	case CINETAccount::eOSAdbk:
	case CINETAccount::eCardDAVAdbk:
	default:
		return cdstring::null_str;
	case CINETAccount::eACAP:
		return "user/";
	}
}

#pragma mark ____________________________Adbks

// Load adbk list from server
void CAdbkProtocol::LoadList()
{
	// NB This protocol does not delete the list items when it logs out, so we
	// need to do that here before reloading

	// Tell listeners we are about to remove all calendar store nodes
	Broadcast_Message(eBroadcast_ClearList, this);

	// Remove calendar store nodes
	mStoreRoot.Clear();
	
	// Check whether connected or not or whether we always read from local cache
	if (IsDisconnected() || mCacheIsPrimary)
		ReadAddressBooks();
	else
	{
		// First try the disconnected cache - but only use if current
		if (mListedFromCache || ((mCacheClient == NULL) || !ReadAddressBooks(true)))
		{
			mClient->_ListAddressBooks(&mStoreRoot);

			// Always keep disconnected cache list in sync with server
			if (mCacheClient != NULL)
			{
				DumpAddressBooks();
			}
		}
		else
			mListedFromCache = true;

		// Look for default address book if remote
		if (!IsOffline())
		{
			bool has_default = false;

			// Get default name
			cdstring default_name;
			if (GetAccountType() == CINETAccount::eACAP)
				default_name += "user/";
			default_name += GetAccount()->GetAuthenticator().GetAuthenticator()->GetActualUID();

			// Scan abooks for default
			if (mStoreRoot.CountDescendants())
				for(CAddressBookList::const_iterator niter = mStoreRoot.GetChildren()->begin();
					!has_default && (niter != mStoreRoot.GetChildren()->end()); niter++)
				{
					if ((*niter)->IsAdbk() && ((*niter)->GetName() == default_name))
						has_default = true;
				}

			// Create default entry
			if (!has_default)
			{
				// Add adress book to list
				CAddressBook* adbk = NULL;
				
				// Policy:
				//   IMSP: create a dummy entry in default address book then delete it
				//   ACAP: just create the default address book
				//   CardDAV: look for one address book only and use that
				switch(GetAccountType())
				{
				case CINETAccount::eIMSP:
				{
					adbk = new CAddressBook(this, &mStoreRoot, true, false, default_name);
					mStoreRoot.AddChild(adbk);

					// Now force real creation of address book by storing fake address
					CAddressList list;
					list.push_back(new CAdbkAddress("a0001", default_name, default_name));
					AddAddress(adbk, &list);
					RemoveAddress(adbk, &list);
					break;
				}
				case CINETAccount::eACAP:
					adbk = new CAddressBook(this, &mStoreRoot, true, false, default_name);
					CreateAdbk(adbk);
					mStoreRoot.AddChild(adbk);
					break;
				case CINETAccount::eCardDAVAdbk:
					if (mStoreRoot.GetChildren() != NULL)
					{
						for(CAddressBookList::const_iterator niter = mStoreRoot.GetChildren()->begin();
							niter != mStoreRoot.GetChildren()->end(); niter++)
						{
							if ((*niter)->IsAdbk())
							{
								if (adbk)
								{
									adbk = NULL;
									break;
								}
								else
									adbk = *niter;
							}
						}
					}
					break;
				default:;
				}

				// Make sure address book is added to open on startup, nick-names and searching
				if (adbk)
				{
					adbk->SetFlags(CAddressBook::eOpenOnStart, true);
					adbk->SetFlags(CAddressBook::eLookup, true);
					adbk->SetFlags(CAddressBook::eSearch, true);

					// Change prefs list
					CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, true);
					CPreferences::sPrefs->ChangeAddressBookOpenOnStart(adbk, true);
					CPreferences::sPrefs->ChangeAddressBookLookup(adbk, true);
					CPreferences::sPrefs->ChangeAddressBookSearch(adbk, true);
				}
			}
		}
	}

	SyncList();
	Broadcast_Message(eBroadcast_RefreshList, this);
}


// Load calendar list from server
void CAdbkProtocol::LoadSubList(CAddressBook* adbk, bool deep)
{
	// When disconnected or cache is primary the list is always up to date
	if (IsDisconnected() || mCacheIsPrimary)
		return;
	
	// Tell listeners we are about to remove all child nodes
	Broadcast_Message(eBroadcast_ClearSubList, adbk);

	// Remove address book nodes
	adbk->Clear();
	
	// Now get new list
	mClient->_ListAddressBooks(adbk);

	// Always keep disconnected cache list in sync with server
	if (mCacheClient != NULL)
	{
		DumpAddressBooks();
	}

	SyncSubList(adbk);

	// Tell listeners we have added child nodes
	Broadcast_Message(eBroadcast_RefreshSubList, adbk);
}

// Sync adbks with prefs options
void CAdbkProtocol::SyncList()
{
	SyncSubList(&mStoreRoot);
}

// Sync adbks with prefs options
void CAdbkProtocol::SyncSubList(CAddressBook* adbk)
{
	// Synf if real address book
	if (adbk->IsAdbk())
	{
		// Get adbk URL
		cdstring adbk_url = adbk->GetURL();

		// Match in open at start list
		cdstrvect::const_iterator found = std::find(CPreferences::sPrefs->mAdbkOpenAtStartup.GetValue().begin(),
												 CPreferences::sPrefs->mAdbkOpenAtStartup.GetValue().end(),
												 adbk_url);

		if (found != CPreferences::sPrefs->mAdbkOpenAtStartup.GetValue().end())
		{
			adbk->SetFlags(CAddressBook::eOpenOnStart, true);

			// Open it if its not already open
			if (!adbk->IsOpen())
				CActionManager::OpenAddressBook(adbk, true);
		}

		// Match in nick name list
		found = std::find(CPreferences::sPrefs->mAdbkNickName.GetValue().begin(),
						CPreferences::sPrefs->mAdbkNickName.GetValue().end(), adbk_url);

		if (found != CPreferences::sPrefs->mAdbkNickName.GetValue().end())
			adbk->SetFlags(CAddressBook::eLookup, true);

		// Match in search list
		found = std::find(CPreferences::sPrefs->mAdbkSearch.GetValue().begin(),
						CPreferences::sPrefs->mAdbkSearch.GetValue().end(),  adbk_url);

		if (found != CPreferences::sPrefs->mAdbkSearch.GetValue().end())
			adbk->SetFlags(CAddressBook::eSearch, true);

		// Match in auto sync list
		found = std::find(CPreferences::sPrefs->mAdbkAutoSync.GetValue().begin(),
						CPreferences::sPrefs->mAdbkAutoSync.GetValue().end(),  adbk_url);

		if (found != CPreferences::sPrefs->mAdbkAutoSync.GetValue().end())
			adbk->SetFlags(CAddressBook::eAutoSync, true);

		// Force update of manager
		CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, true);
	}

	// Look at each child
	if (adbk->HasInferiors())
	{
		for(CAddressBookList::const_iterator iter = adbk->GetChildren()->begin(); iter != adbk->GetChildren()->end(); iter++)
		{
			SyncSubList(*iter);
		}
	}
}

// Refresh adbk list from server
void CAdbkProtocol::RefreshList()
{
	// Reload and sync
	LoadList();
}

void CAdbkProtocol::RefreshSubList(CAddressBook* adbk)
{
	// Reload and sync
	LoadSubList(adbk, false);
}

// List was changed in some way
void CAdbkProtocol::ListChanged()
{
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		DumpAddressBooks();
	}
}

CAddressBook* CAdbkProtocol::GetNode(const cdstring& adbk, bool parent) const
{
	// Break the name down into components
	cdstring acct;
	cdstring path;
	const char* start = adbk.c_str();
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

	return (path.empty() ? const_cast<CAddressBook*>(&mStoreRoot) : mStoreRoot.FindNode(path));
}

CAddressBook* CAdbkProtocol::GetParentNode(const cdstring& adbk) const
{
	return GetNode(adbk, true);
}

bool CAdbkProtocol::HasDisconnectedAdbks()
{
	if (CanDisconnect() || mCacheIsPrimary)
	{
		if (!mStoreRoot.HasInferiors())
			ReadAddressBooks();
		return mStoreRoot.HasInferiors();
	}
	else
		return false;
}

// Create a new adbk
void CAdbkProtocol::CreateAdbk(CAddressBook* adbk)
{
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_CreateAdbk(adbk);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		mCacheClient->_CreateAdbk(adbk);
		DumpAddressBooks();
	}
	
	// Always sync the cache state
	if (IsDisconnected())
		adbk->TestDisconnectCache();
}

// Touch mbox on server
void CAdbkProtocol::TouchAdbk(CAddressBook* adbk)
{
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_TouchAdbk(adbk);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchAdbk(adbk))
			DumpAddressBooks();
	}
	
	// Always sync the cache state
	if (IsDisconnected())
		adbk->TestDisconnectCache();
}

// Test mbox on server
bool CAdbkProtocol::TestAdbk(CAddressBook* adbk)
{
	// Do test action
	return (mCacheIsPrimary && (mCacheClient != NULL)) ? mCacheClient->_TestAdbk(adbk) : mClient->_TestAdbk(adbk);

}

// Delete adbk
void CAdbkProtocol::DeleteAdbk(CAddressBook* adbk)
{
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_DeleteAdbk(adbk);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TestAdbk(adbk))
			mCacheClient->_DeleteAdbk(adbk);
	}
}

void CAdbkProtocol::SizeAdbk(CAddressBook* adbk)
{
	// Don't do on server if cache is primary
	if (mCacheIsPrimary)
		mCacheClient->_SizeAdbk(adbk);
	else
		mClient->_SizeAdbk(adbk);
}

// Rename adbk
void CAdbkProtocol::RenameAdbk(CAddressBook* adbk, const char* adbk_new)
{
	// Rename it on the server
	// Don't do on server if cache is primary
	if (!mCacheIsPrimary)
		mClient->_RenameAdbk(adbk, adbk_new);

	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TestAdbk(adbk))
			mCacheClient->_RenameAdbk(adbk, adbk_new);
	}
}

// Open adbk on server
void CAdbkProtocol::OpenAdbk(CAddressBook* adbk)
{
	// Disconnected touch
	if (IsDisconnected())
		TouchAdbk(adbk);

	// See if offline or disconnected	
	if (IsOffline() || IsDisconnected())
	{
		// Just read full calendar from offline/disconnect store
		ReadFullAddressBook(adbk);
	}
	else
	{
		// Look for local cache first
		if ((mCacheClient != NULL) && !mCacheIsPrimary)
		{
			// Read in the calendar cache if it exists
			if (mCacheClient->_TestAdbk(adbk))
				mCacheClient->_ReadFullAddressBook(adbk);

			// Sync cache with server doing playback if needed
			SyncFromServer(adbk);
		}
		else
		{
			// Just read full calendar from server
			ReadFullAddressBook(adbk);
		}
	}
}

void CAdbkProtocol::SyncFromServer(CAddressBook* adbk)
{
	// Different based on full or component sync
	if (IsReadComponentAdbk())
		SyncComponentsFromServer(adbk);
	else
		SyncFullFromServer(adbk);
}

void CAdbkProtocol::SyncFullFromServer(CAddressBook* adbk)
{
	// Must have a loaded vcard address book
	if (adbk->GetVCardAdbk() == NULL)
		return;

	// We need to do this as a proper transaction with locking
	try
	{
		// We need to do this as a proper transaction with locking
		mClient->_LockAdbk(adbk);

		// Get client and server data state
		bool server_changed = false;
		bool cache_changed = false;

		// Special for empty local cache
		if (adbk->GetVCardAdbk()->GetETag().empty())
		{
			// Force server -> cache sync
			server_changed = true;
			cache_changed = false;
		}
		else
		{
			// Check to see whether server/cache state changed
			server_changed = mClient->_AdbkChanged(adbk);
			cache_changed = adbk->GetVCardAdbk()->NeedsSync();
		}
		
		// Now do appropriate sync (if neither have changed no need to do anything - just use the cache as-is)
		if (server_changed && !cache_changed)
		{
			// Server overwrites local cache
			adbk->ClearContents();
			mClient->_ReadFullAddressBook(adbk);
			
			// Write changes back to local cache
			if (mCacheClient->_TouchAdbk(adbk))
				DumpAddressBooks();
			mCacheClient->_WriteFullAddressBook(adbk);
		}
		else if (!server_changed && cache_changed)
		{
			// Local cache overwrites server
			mClient->_WriteFullAddressBook(adbk);
		}
		else if (server_changed && cache_changed)
		{
			// Get temp copy of server calendar
			CAddressBook temp(this, NULL);
			mClient->_ReadFullAddressBook(&temp);
			
			// Sync the two - after this 'adbk' will contain the sync'd data
			vCard::CVCardSync sync(*adbk->GetVCardAdbk(), *temp.GetVCardAdbk());
			sync.Sync();
			
			// Local overwrites server
			adbk->GetVCardAdbk()->ClearRecording();
			
			// Now write back to both server and cache
			mClient->_WriteFullAddressBook(adbk);
			if (mCacheClient->_TouchAdbk(adbk))
				DumpAddressBooks();
			mCacheClient->_WriteFullAddressBook(adbk);
		}

		// Always unlock
		mClient->_UnlockAdbk(adbk);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		// Always unlock
		mClient->_UnlockAdbk(adbk);

		CLOG_LOGRETHROW;
		throw;
	}
}

void CAdbkProtocol::SyncComponentsFromServer(CAddressBook* adbk)
{
	// We need to do this as a proper transaction with locking
	try
	{
		// Policy:
		//
		// 1. Get list of current items on server
		//
		// 2. Look at record DB and:
		//  2.1 Add new components to server and cache info
		//  2.2 Remove deleted components from server and cached server info if still present on server
		//  2.3 Cache info for changed items for later sync (or change them immediately if the server has not changed)
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
		vCard::CVCardAddressBook* vadbk = adbk->GetVCardAdbk();
		CCardDAVVCardClient* cardclient = static_cast<CCardDAVVCardClient*>(mClient);
		cdstrmap comps;
		bool server_changed = mClient->_AdbkChanged(adbk);
		if (server_changed)
			cardclient->_GetComponentInfo(adbk, *vadbk, comps);
		else
		{
			mCacheClient->_ReadFullAddressBook(adbk);
		}
		
		// Step 2
		cdstrmap cache_added;
		cdstrmap cache_changed;
		for(vCard::CVCardComponentRecordDB::const_iterator iter = vadbk->GetRecording().begin(); iter != vadbk->GetRecording().end(); iter++)
		{
			// Step 2.1
			if ((*iter).second.GetAction() == vCard::CVCardComponentRecord::eAdded)
			{
				// Add component to server
				const vCard::CVCardVCard* comp = vadbk->GetCardByKey((*iter).first);
				if (comp != NULL)
				{
					// Add component to server
					cardclient->_AddComponent(adbk, *vadbk, *comp);

					// Add to added cache
					cache_added.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				}
			}
			
			// Step 2.2
			else if ((*iter).second.GetAction() == vCard::CVCardComponentRecord::eRemoved)
			{
				// Is it still present on the server
				if (comps.count((*iter).second.GetRURL()) != 0)
				{
					// Remove component from server
					cardclient->_RemoveComponent(adbk, *vadbk, (*iter).second.GetRURL());
					
					// Remove from server component info
					comps.erase((*iter).second.GetRURL());
				}
			}

			// Step 2.3
			else if ((*iter).second.GetAction() == vCard::CVCardComponentRecord::eChanged)
			{
				if (server_changed)
					cache_changed.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				else
				{
					// Change it on the server
					const vCard::CVCardVCard* comp = vadbk->GetCardByKey((*iter).first);
					cardclient->_ChangeComponent(adbk, *vadbk, *comp);
					cache_changed.insert(cdstrmap::value_type((*iter).second.GetRURL(), (*iter).second.GetETag()));
				}
			}
		}
		
		
		// Now do sync
		if (server_changed)
		{
			// Get component info from cache
			cdstrvect component_keys;
			for(vCard::CVCardComponentDB::const_iterator iter = vadbk->GetVCards().begin(); iter != vadbk->GetVCards().end(); iter++)
			{
				component_keys.push_back((*iter).second->GetMapKey());
			}
			
			// Step 3
			cdstrset matching_components;
			for(cdstrvect::const_iterator iter = component_keys.begin(); iter != component_keys.end(); iter++)
			{
				vCard::CVCardVCard* cache_comp = vadbk->GetCardByKey(*iter);
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
						cardclient->_ChangeComponent(adbk, *vadbk, *cache_comp);
					}
					
					// Step 3.2.2
					else
					{
						// Do iCal SEQ etc comparison
						
						// First read in component from server into temp addressbook
						vCard::CVCardAddressBook tempvcard;
						vCard::CVCardVCard* server_comp = cardclient->_ReadComponent(adbk, tempvcard, server_rurl);
						if (server_comp != NULL)
						{
							int result = -1; //iCal::CICalendarSync::CompareComponentVersions(server_comp, cache_comp);
							
							if (result == 1)
							{
								// Cache is newer than server - cache overwrites to server
								cardclient->_ChangeComponent(adbk, *vadbk, *cache_comp);
							}
							else if (result == -1)
							{
								// Cache is older than server - server overwrites cache

								// Remove the cached component first
								vadbk->RemoveCardByKey(cache_comp->GetMapKey());
								cache_comp = NULL;
								
								// Copy component from server into local cache effectively replacing old one
								vCard::CVCardComponent* new_comp = server_comp->clone();
								new_comp->SetAddressBook(vadbk->GetRef());
								vadbk->AddCard(new_comp);
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
						vadbk->RemoveCardByKey(cache_comp->GetMapKey());
						cache_comp = NULL;
						
						// Read component from server into local cache effectively replacing old one
						cardclient->_ReadComponent(adbk, *vadbk, server_rurl);
					}
				}
				
				// Step 3.4
				else
				{
					// comp is deleted in this call
					vadbk->RemoveCardByKey(cache_comp->GetMapKey());
					cache_comp = NULL;
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
			if (rurls.size() != 0)
				cardclient->_ReadComponents(adbk, *vadbk, rurls);
		}

		// Clear out cache recording
		vadbk->ClearRecording();
		
		// Get the current server sync token
		cardclient->_UpdateSyncToken(adbk);

		// Now write back cache
		if (!mCacheClient->_TouchAdbk(adbk))
			DumpAddressBooks();
		mCacheClient->_WriteFullAddressBook(adbk);
	
		// Now map VCards into internal addresses
		vcardstore::MapFromVCards(adbk);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);

		CLOG_LOGRETHROW;
		throw;
	}
}

// Close existing address book
void CAdbkProtocol::CloseAdbk(CAddressBook* adbk)
{
}

void CAdbkProtocol::ReadFullAddressBook(CAddressBook* adbk)
{
	// Don't do on server if cache is primary
	if (mCacheIsPrimary)
		mCacheClient->_ReadFullAddressBook(adbk);
	else
		mClient->_ReadFullAddressBook(adbk);
}

void CAdbkProtocol::WriteFullAddressBook(CAddressBook* adbk)
{
	// Don't do on server if cache is primary
	if (mCacheIsPrimary)
		mCacheClient->_WriteFullAddressBook(adbk);
	else
	{
		mClient->_WriteFullAddressBook(adbk);

		// Always keep disconnected cache in sync with server
		if (mCacheClient != NULL)
		{
			if (mCacheClient->_TouchAdbk(adbk))
				DumpAddressBooks();
			mCacheClient->_WriteFullAddressBook(adbk);

			// Set sync time in node
			adbk->SyncNow();
		}
	}
}

bool CAdbkProtocol::DoWriteFull(CAddressBook* adbk)
{
	// Check for per-component protocol
	if (!IsWriteComponentAdbk() || IsDisconnected())
	{
		// Always fall back to writing entire address book
		WriteFullAddressBook(adbk);
		return true;
	}
	else
		return false;
}

void CAdbkProtocol::DidComponentWrite(CAddressBook* adbk)
{
	// Clear cache state after write
	if (adbk->IsOpen())
	{
		adbk->GetVCardAdbk()->ClearRecording();
		adbk->GetVCardAdbk()->SetDirty(false);
	}
	
	// Always keep disconnected cache in sync with server
	if (mCacheClient != NULL)
	{
		if (mCacheClient->_TouchAdbk(adbk))
			DumpAddressBooks();
		mCacheClient->_WriteFullAddressBook(adbk);

		// Set sync time in node
		adbk->SyncNow();
	}
}

// Add address - the address must already be stored in the address book
// and have a vCard representation if the address book is open
void CAdbkProtocol::AddAddress(CAddressBook* adbk, CAddressList* addrs)
{
	// Write to store - this will work even if the address book is not open
	mClient->_StoreAddress(adbk, addrs);
	if (adbk->IsOpen() && !IsDisconnected())
	{
		adbk->GetVCardAdbk()->ClearRecording();
		adbk->GetVCardAdbk()->SetDirty(false);
	}
	
	// Always keep disconnected cache in sync with server
	if (adbk->IsOpen() && (mCacheClient != NULL))
	{
		if (mCacheClient->_TouchAdbk(adbk))
			DumpAddressBooks();
		mCacheClient->_StoreAddress(adbk, addrs);

		// Set sync time in node
		adbk->SyncNow();
	}
}

// Add group - the address must already be stored in the address book
// and have a vCard representation if the address book is open
void CAdbkProtocol::AddGroup(CAddressBook* adbk, CGroupList* grps)
{
	// Write to store - this will work even if the address book is not open
	mClient->_StoreGroup(adbk, grps);
	if (adbk->IsOpen() && !IsDisconnected())
	{
		adbk->GetVCardAdbk()->ClearRecording();
		adbk->GetVCardAdbk()->SetDirty(false);
	}
	
	// Always keep disconnected cache in sync with server
	if (adbk->IsOpen() && (mCacheClient != NULL))
	{
		if (mCacheClient->_TouchAdbk(adbk))
			DumpAddressBooks();
		mCacheClient->_StoreGroup(adbk, grps);

		// Set sync time in node
		adbk->SyncNow();
	}
}

// Change address
void CAdbkProtocol::ChangeAddress(CAddressBook* adbk, CAddressList* addrs)
{
	// The address book MUST be open
	if (!adbk->IsOpen())
		return;

	// Check for per-component protocol
	if (DoWriteFull(adbk))
		return;
	
	mClient->_ChangeAddress(adbk, addrs);
	
	// Always keep disconnected cache in sync with server
	DidComponentWrite(adbk);
}

// Change group
void CAdbkProtocol::ChangeGroup(CAddressBook* adbk, CGroupList* grps)
{
	// The address book MUST be open
	if (!adbk->IsOpen())
		return;

	// Check for per-component protocol
	if (DoWriteFull(adbk))
		return;
	
	mClient->_ChangeGroup(adbk, grps);
	
	// Always keep disconnected cache in sync with server
	DidComponentWrite(adbk);
}

// Remove address
void CAdbkProtocol::RemoveAddress(CAddressBook* adbk, CAddressList* addrs)
{
	// The address book MUST be open
	if (!adbk->IsOpen())
		return;

	mClient->_DeleteAddress(adbk, addrs);
}

// Remove group
void CAdbkProtocol::RemoveGroup(CAddressBook* adbk, CGroupList* grps)
{
	// The address book MUST be open
	if (!adbk->IsOpen())
		return;

	mClient->_DeleteGroup(adbk, grps);
}

// Address/Group was removed
void CAdbkProtocol::RemovalOfAddress(CAddressBook* adbk)
{
	// The address book MUST be open
	if (!adbk->IsOpen())
		return;
	
	// Check for per-component protocol
	if (DoWriteFull(adbk))
		return;
	
	// Always keep disconnected cache in sync with server
	DidComponentWrite(adbk);
}

#pragma mark ____________________________Lookup

// Resolve address nick-name
void CAdbkProtocol::ResolveAddress(CAddressBook* adbk, const char* nick_name, CAdbkAddress*& addr)
{
	mClient->_ResolveAddress(adbk, nick_name, addr);
}

// Resolve group nick-name
void CAdbkProtocol::ResolveGroup(CAddressBook* adbk, const char* nick_name, CGroup*& grp)
{
	mClient->_ResolveGroup(adbk, nick_name, grp);
}

// Do search
void CAdbkProtocol::SearchAddress(CAddressBook* adbk,
									const cdstring& name,
									CAdbkAddress::EAddressMatch match,
									const CAdbkAddress::CAddressFields& fields,
									CAddressList& addr_list)
{
	mClient->_SearchAddress(adbk, name, match, fields, addr_list);
}

#pragma mark ____________________________ACLs

// Set new value on server
void CAdbkProtocol::SetACL(CAddressBook* adbk, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Adbk will take care of adding/replacing in list if successful
	mClient->_SetACL(adbk, acl);
}

// Delete value on server
void CAdbkProtocol::DeleteACL(CAddressBook* adbk, CACL* acl)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Adbk will take care of adding/replacing in list if successful
	mClient->_DeleteACL(adbk, acl);
}

// Get values on server
void CAdbkProtocol::GetACL(CAddressBook* adbk)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Adbk will take care of recovery
	mClient->_GetACL(adbk);
}

// List user's rights on server
void CAdbkProtocol::MyRights(CAddressBook* adbk)
{
	// Do nothing if not available
	if (!UsingACLs())
		return;

	// Send command to server
	// Adbk will take care of recovery
	mClient->_MyRights(adbk);
}

#pragma mark ____________________________Disconnected

void CAdbkProtocol::InitDisconnect()
{
	// Do inherited to setup directory
	CINETProtocol::InitDisconnect();

	// Now create recorder
	if (!mRecorder)
	{
		mRecorder = new CAdbkRecord;
		mRecorder->SetDescriptor(mOfflineCWD + "Record.mbr");
		mRecorder->SetFullRecord();
	}
	if (mRecorder)
		mRecorder->Open();
}

const cdstring& CAdbkProtocol::GetOfflineDefaultDirectory()
{
	static cdstring name("Address Books");
	return name;
}

void CAdbkProtocol::GoOffline()
{
	// Check that offline directory exists
	if (mOfflineCWD.empty())
		InitOffline();

	// Dump current addressbooks list to disk
	DumpAddressBooks();

	// Now do inherited connection switch
	CINETProtocol::GoOffline();
	
	// Check each node to get cached state
	mStoreRoot.TestDisconnectCache();
}

void CAdbkProtocol::GoOnline()
{
	// Always playback sync'd data
	DoPlayback();

	// Reset state of address books
	RecoverAddressBooks();

	// Now do inherited connection switch
	CINETProtocol::GoOnline();
}

const char* cAdbkListName = "adbklist.xml";

void CAdbkProtocol::DumpAddressBooks()
{
	cdstring list_name = mOfflineCWD + cAdbkListName;

	// Create output file
	cdofstream fout(list_name.c_str());
	if (fout.fail())
		return;

	// Create XML document object
	std::auto_ptr<xmllib::XMLDocument> doc(new xmllib::XMLDocument);
	
	// Root element is the preferences element
	doc->GetRoot()->SetName(cXMLElement_adbklist);
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

bool CAdbkProtocol::ReadAddressBooks(bool only_if_current)
{
	bool is_current = false;
	cdstring list_name = mOfflineCWD + cAdbkListName;
	
	// XML parse the data
	xmllib::XMLSAXSimple parser;
	parser.ParseFile(list_name);

	// See if we got any valid XML
	if (parser.Document())
	{
		// Check root node
		xmllib::XMLNode* root = parser.Document()->GetRoot();
		if (!root->CompareFullName(cXMLElement_adbklist))
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

void CAdbkProtocol::RecoverAddressBooks()
{
}

void CAdbkProtocol::SynchroniseRemoteAll(bool fast)
{
}

void CAdbkProtocol::SynchroniseRemote(CAddressBook* adbk, bool fast)
{
}

void CAdbkProtocol::SyncRemote(CAddressBook* remote, CAddressBook* local, bool fast)
{
}

void CAdbkProtocol::ClearDisconnect(CAddressBook* adbk)
{
}

bool CAdbkProtocol::DoPlayback()
{
	// Ignore if no defined recorder
	if (!mRecorder)
		return true;

	// Create remote clone
	std::auto_ptr<CAdbkProtocol> clone(new CAdbkProtocol(*this, false, true));
	clone->SetSynchronising();

	// Prevent further recording
	mRecorder->SetNoRecord();

	bool result = mRecorder->Playback(clone.get(), this);

	// Enable recording
	mRecorder->SetFullRecord();
	
	return result;
}
