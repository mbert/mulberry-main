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


// CMailAccountManager.h

#include "CMailAccountManager.h"

#include "CActionManager.h"
#include "CAdminLock.h"
#include "CConnectionManager.h"
#include "CDisconnectDialog.h"
#include "CFilterManager.h"
#include "CINETCommon.h"
#include "CMailAccount.h"
#include "CMailboxWindow.h"
#include "CMailCheckThread.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMulberryApp.h"
#include "CNamespaceDialog.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CPeriodicCheck.h"
#endif
#include "CPreferences.h"
#include "CProgress.h"
#include "CServerWindow.h"
#include "CSMTPWindow.h"
#include "CTaskQueue.h"
#include "CXStringResources.h"
#include "C3PaneWindow.h"
#include <algorithm>
#include <typeinfo>

#if __dest_os == __win32_os || __dest_os == __linux_os
#include "StValueChanger.h"
#endif

const long cSecsPerMin = 60;

CMailAccountManager* CMailAccountManager::sMailAccountManager = nil;

CMailAccountManager::CMailAccountManager()
	: mMRUCopyTo(true), mMRUAppendTo(true)
{
	mProtoCount = 0;
	mMainView = NULL;

	sMailAccountManager = this;

	// Reset timer and start in paused state
	Reset();
	mCheckPaused = false;
	mHaltCheck = false;

	mReusableProto = NULL;
	mReuseProto = false;

	// Create favourites
	InitFavourites();
	InitMRUs();

	// Start checking threads
	CMailCheckThread::BeginMailCheck();
}

CMailAccountManager::~CMailAccountManager()
{
	// Must stop any mail checks in progress
	CMailCheckThread::Pause(true);
	mHaltCheck = true;

	// End checking thread
	CMailCheckThread::EndMailCheck();

	// Remove all server views now to prevent illegal updates to 'stale' windows
	{
		cdmutexprotect<CServerView::CServerViewList>::lock _lock(CServerView::sServerViews);
		for(CServerView::CServerViewList::reverse_iterator riter = CServerView::sServerViews->rbegin();
				riter != CServerView::sServerViews->rend(); riter++)
			(*riter)->DoClose();
	}
	mMainView = NULL;

	// Logoff each protocol and remove it
	for(CMboxProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); )
	{
		StopProtocol(*iter);
		RemoveProtocol(*iter);

		// Must remove from list here as subsequent protos need mProtos to be consistent
		// when they logout
		iter = mProtos.erase(iter);
		mProtoCount--;
	}

	// Remove each favourite
	for(CFavourites::iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
		delete *iter;

	sMailAccountManager = NULL;
}

// Create primary server view
void CMailAccountManager::CreateMainView()
{
	try
	{
		// Look for 3-pane or 1-pane
		if (CPreferences::sPrefs->mUse3Pane.GetValue())
		{
			// Create the 3-pane view
			C3PaneWindow::Create3PaneWindow();
			mMainView = C3PaneWindow::s3PaneWindow->GetServerView();
			mMainView->SetManager();
			mMainView->ResetTable();
		}
		else
		{
			// Create the main server window
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			CServerWindow* window = (CServerWindow*) LWindow::CreateWindow(paneid_ServerWindow, CMulberryApp::sApp);
			{
				ThrowIfNil_(window);
			}
#else
			CServerWindow* window = CServerWindow::ManualCreate();
#endif

			// Make it the manager and force reset
			mMainView = window->GetServerView();
			mMainView->SetManager();
			mMainView->ResetTable();

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Must show
			window->Show();
#endif
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Add a new mail protocol
void CMailAccountManager::AddProtocol(CMboxProtocol* proto)
{
	// Add to protos list
	mProtos.push_back(proto);
	mProtoCount++;

	// Make sure main window exists
	if (!mMainView)
		CreateMainView();
	else
		Broadcast_Message(eBroadcast_NewMailAccount, proto);
}

// Changed a mail protocol
void CMailAccountManager::ChangedProtocolWDs(CMboxProtocol* proto)
{
	// Force window reset (via mbox list broadcast) if already logged on
	if (proto->IsLoggedOn())
	{
		proto->LoadList();

		// Reset those favourites that have associated CMbox flags
		EFavourite type = eFavouriteStart;
		const CFavouriteItemList& list = CPreferences::sPrefs->mFavourites.GetValue();
		for(CFavouriteItemList::const_iterator iter = list.begin(); iter != list.end(); iter++, type = static_cast<EFavourite>(type + 1))
		{
			if (IsFavouriteMboxFlag(type))
				proto->SyncFlag((*iter).GetItems(),
									static_cast<NMbox::EFlags>(GetFavouriteMboxFlag(type)));
		}
	}
}

// Remove a mail protocol
void CMailAccountManager::RemoveProtocol(CMboxProtocol* proto, bool full)
{
	// Remove single server windows - should try this via broadcast, but beware window object deleting itself
	CServerWindow* window = GetProtocolWindow(proto);
	if (window)
		FRAMEWORK_DELETE_WINDOW(window)

	// Broadcast change to all before delete
	Broadcast_Message(eBroadcast_RemoveMailAccount, proto);

	// May need to remove it totally
	if (full)
		proto->RemoveAccount();

	// Close it before delete as closing may cause logout actions on invalid object
	proto->Close();

	// Delete proto (caller must remove from list if requried)
	delete proto;
}

// Get protocol by name
CMboxProtocol* CMailAccountManager::GetProtocol(const cdstring& name)
{
	cdstring temp = name;
	char* p = ::strchr(temp.c_str_mod(), cMailAccountSeparator);
	if (p)
		*p = 0;

	for(CMboxProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		if ((*iter)->GetAccountName() == temp)
			return *iter;
	}

	return nil;
}

// Get protocol window
CServerWindow* CMailAccountManager::GetProtocolWindow(const CMboxProtocol* proto) const
{
	cdmutexprotect<CServerWindow::CServerWindowList>::lock _lock(CServerWindow::sServerWindows);
	for(CServerWindow::CServerWindowList::iterator iter = CServerWindow::sServerWindows->begin(); iter != CServerWindow::sServerWindows->end(); iter++)
	{
		// Found window so return
		if ((*iter)->GetServerView()->GetServer() == proto)
			return *iter;
	}

	return NULL;
}

// Sync with changed accounts
void CMailAccountManager::SyncAccounts(const CMailAccountList& accts)
{
	// Must stop any mail checks in progress
	CMailCheckThread::Pause(true);
	mHaltCheck = true;

	// Try to acquire run lock which is released only when mail check is no longer running
	// Use of mutex to allow messages to be pumped while waiting for mail check to finish
	cdmutex::lock_cdmutex _lock(CMailCheckThread::RunLock());

	// New smart alogorithm: compare protocol and protocol embedded in account

	bool changed = false;

	// Inform listeners of possible global change to accounts
	Broadcast_Message(eBroadcast_BeginMailAccountChange, nil);

	// This maintains a list of protocols that need to be started
	CMboxProtocolList started;

	// Match new set of accounts with existing protocols so that account information
	// is consistent during the removal/add stage.. Sometimes removing or adding my require
	// access to an account object in an existing protocol
	for(CMailAccountList::const_iterator iter2 = accts.begin(); iter2 != accts.end(); iter2++)
	{
		bool found = false;

		// Look for protocol matching account
		for(CMboxProtocolList::iterator iter1 = mProtos.begin(); !found && (iter1 != mProtos.end()); iter1++)
		{
			if ((*iter2)->GetProtocol() == *iter1)
			{
				found = true;

				// May need to redo server and/or authenticator
				if (((*iter1)->GetAccountUniqueness() != (*iter2)->GetUniqueness()) ||
					((*iter1)->GetAuthenticatorUniqueness() != (*iter2)->GetAuthenticator().GetUniqueness()))
				{
					StopProtocol(*iter1);

					// Force accumalted tasks to run as StopProtocol qill queue up mailbox view closes
					// that we must have complete before we go further
					while(CTaskQueue::sTaskQueue.DoNextWork()) {}

					// Force refresh of all mailbox lists
					(*iter1)->ClearList();
				}

				// Resync account object
				(*iter1)->SetAccount(*iter2);

				// Do login at start if required
				if ((*iter2)->GetLogonAtStart() && !(*iter1)->IsLoggedOn())
					started.push_back(*iter1);
			}
		}
	}

	// Now delete protocols not in accounts
	for(CMboxProtocolList::iterator iter1 = mProtos.begin(); iter1 != mProtos.end();)
	{
		bool found = false;

		// Look for match between existing protocol and new accounts
		for(CMailAccountList::const_iterator iter2 = accts.begin(); !found && (iter2 != accts.end()); iter2++)
		{
			if ((*iter2)->GetProtocol() == *iter1)
				found = true;
		}

		// Remove protocol if no matching account
		if (!found)
		{
			changed = true;

			StopProtocol(*iter1);

			// Force accumalted tasks to run as StopProtocol qill queue up mailbox view closes
			// that we must have complete before we go further
			while(CTaskQueue::sTaskQueue.DoNextWork()) {}

			RemoveProtocol(*iter1, true);

			// Remove from list
			iter1 = mProtos.erase(iter1);
			mProtoCount--;
			continue;
		}
		
		iter1++;
	}

	// Now add new protocols
	for(CMailAccountList::const_iterator iter2 = accts.begin(); iter2 != accts.end(); iter2++)
	{
		bool found = false;

		// Look for protocol matching account
		for(CMboxProtocolList::iterator iter1 = mProtos.begin(); !found && (iter1 != mProtos.end()); iter1++)
		{
			if ((*iter2)->GetProtocol() == *iter1)
				found = true;
		}

		// If not found add new protocol
		if (!found)
		{
			// Create a protocol for each account and add to list
			CMboxProtocol* aMboxProtocol = new CMboxProtocol(*iter2);
			aMboxProtocol->InitProtocol();
			try
			{
				AddProtocol(aMboxProtocol);
				changed = true;

				// Do login at start if required
				if ((*iter2)->GetLogonAtStart())
					started.push_back(aMboxProtocol);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Do not allow to fail
			}
		}
	}

	// Resort favourites if any change
	if (changed)
	{
		for(CFavourites::iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
		{
			if ((*iter)->IsSorted())
			{
				(*iter)->Sort();

				// Force visual update
				(*iter)->Broadcast_Message(CTreeNodeList::eBroadcast_ResetList, *iter);
			}
		}
	}

	// Inform listeners of end to global change to accounts
	Broadcast_Message(eBroadcast_EndMailAccountChange, nil);

	// Now try starting those proto that were opened
	for(CMboxProtocolList::const_iterator iter = started.begin(); iter != started.end(); iter++)
		StartProtocol(*iter);

	// Now try post-processing those proto that were opened
	for(CMboxProtocolList::reverse_iterator riter = started.rbegin(); riter != started.rend(); riter++)
		StartedProtocol(*riter);

	// Make sure 3-pane is told to re-instate previously open mailboxes
	if (C3PaneWindow::s3PaneWindow)
		C3PaneWindow::s3PaneWindow->DoneInitMailAccounts();

	// Unpause mail check after account changes
	CMailCheckThread::Pause(false);

	// Now fire off mail check if any started
	if (started.size())
		ForceMailCheck();
}

// Open window for protocol
void CMailAccountManager::OpenProtocol(CMboxProtocol* proto)
{
	// Look for existing window and select it
	CServerWindow* serverWindow = GetProtocolWindow(proto);
	if (serverWindow)
	{
		// Found so select it
		FRAMEWORK_WINDOW_TO_TOP(serverWindow)
		return;
	}

	// Create server window for proto
	try
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Create the mailbox list window
		CServerWindow* mail_list = (CServerWindow*) LWindow::CreateWindow(paneid_ServerWindow, CMulberryApp::sApp);
	{
		ThrowIfNil_(mail_list);
	}

		// Give it a server
		mail_list->GetServerView()->SetServer(proto);
		mail_list->Show();
#elif __dest_os == __win32_os
		CServerWindow* aServerWnd = CServerWindow::ManualCreate();

		// Give it a server
		aServerWnd->GetServerView()->SetServer(proto);
#elif __dest_os == __linux_os
		CServerWindow *aServerWnd =  CServerWindow::ManualCreate();
		aServerWnd->GetServerView()->SetServer(proto);
#endif
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Start remote protocol
void CMailAccountManager::StartProtocol(CMboxProtocol* proto, bool silent)
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
		// Begin IMAP
		if (CMulberryApp::sApp->BeginINET(proto))
		{
			// Now find the mailboxes
			proto->SetSorted(true);
			proto->LoadList();

			// Do any auto creates of copy/move to items
			if (CAdminLock::sAdminLock.mAutoCreateMailbox)
			{
				if (CPreferences::sPrefs->mIdentities.GetValue().front().UseCopyTo())
					proto->AutoCreate(CPreferences::sPrefs->mIdentities.GetValue().front().GetCopyTo(), CAdminLock::sAdminLock.mSubcribeAutoCreate);
				if (CPreferences::sPrefs->mDoMailboxClear.GetValue())
					proto->AutoCreate(CPreferences::sPrefs->clear_mailbox.GetValue(), CAdminLock::sAdminLock.mSubcribeAutoCreate);
			}
			
			// Do other auto-creates
			if (CAdminLock::sAdminLock.mAutoCreateMailboxes.size() != 0)
			{
				for(cdstrvect::const_iterator iter = CAdminLock::sAdminLock.mAutoCreateMailboxes.begin(); iter != CAdminLock::sAdminLock.mAutoCreateMailboxes.end(); iter++)
				{
					// The names from the admin tool do not include the account prefix - we add it here
					cdstring temp = proto->GetAccountName();
					temp += cMailAccountSeparator;
					temp += *iter;
					
					// Now auto create it
					proto->AutoCreate(temp, CAdminLock::sAdminLock.mSubcribeAutoCreate);
				}
			}
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

// Start remote protocol
void CMailAccountManager::StartedProtocol(CMboxProtocol* proto)
{
	// Only carry on if already logged on
	if (!proto->IsLoggedOn())
		return;

	try
	{
		// Now open after login mailboxes in reverse (if not 3-pane in use)
		bool reverse = (C3PaneWindow::s3PaneWindow == NULL);
		CMboxRefList::const_iterator iter1 = mFavourites.at(eFavouriteOpenLogin)->begin();
		CMboxRefList::const_iterator iter1_end = mFavourites.at(eFavouriteOpenLogin)->end();
		CMboxRefList::const_reverse_iterator riter1 = mFavourites.at(eFavouriteOpenLogin)->rbegin();
		CMboxRefList::const_reverse_iterator riter1_end = mFavourites.at(eFavouriteOpenLogin)->rend();

		while(reverse ? (riter1 != riter1_end) : (iter1 != iter1_end))
		{
			const CMboxRef* mbox_ref = static_cast<const CMboxRef*>(reverse ? *riter1 : *iter1);

			// Don't bother with directories
			if (mbox_ref->IsDirectory())
			{
				if (reverse)
					riter1++;
				else
					iter1++;
				continue;
			}

			// Resolve to mboxes - maybe wildcard so get list
			CMboxList match;
			if (mbox_ref->IsWildcard())
				mbox_ref->ResolveMbox(match, true, true);
			else
			{
				CMbox* mbox = mbox_ref->ResolveMbox(true);
				if (mbox)
					match.push_back(mbox);
			}

			// Check each mailbox in resolved list
			for(CMboxList::iterator iter = match.begin(); iter != match.end(); iter++)
			{
				CMbox* mbox = (CMbox*) *iter;

				// No related mbox
				if (mbox->GetProtocol() != proto)
					continue;

				// Open if not already on screen
				// If it is on screen we don't bring it to the front now
				if (!CMailboxView::FindView(mbox))
					CActionManager::OpenMailbox(mbox, true);

			}

			if (reverse)
				riter1++;
			else
				iter1++;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Stop remote protocol
void CMailAccountManager::StopProtocol(CMboxProtocol* proto)
{
	try
	{
		// Logoff protocol
		proto->Logoff();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Determine if any open protocols
bool CMailAccountManager::HasOpenProtocols() const
{
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		if ((*iter)->IsLoggedOn())
			return true;
	}

	return false;
}

// Move protocol
void CMailAccountManager::MoveProtocol(long old_index, long new_index)
{
	CMboxProtocol* proto = mProtos.at(old_index);

	// Broadcast change to all to remove it
	Broadcast_Message(eBroadcast_RemoveMailAccount, proto);

	// Adjust count
	if (old_index < new_index)
		new_index--;

	// Move protocol
	mProtos.erase(mProtos.begin() + old_index);
	if (new_index < (long) mProtos.size())
		mProtos.insert(mProtos.begin() + new_index, proto);
	else
		mProtos.push_back(proto);

	// Move mail account
	CMailAccountList& accts = CPreferences::sPrefs->mMailAccounts.Value();
	CMailAccount* acct = accts.at(old_index);
	accts.at(old_index) = NULL;
	accts.erase(accts.begin() + old_index);
	if (new_index < (long) mProtos.size() - 1)
		accts.insert(accts.begin() + new_index, acct);
	else
		accts.push_back(acct);
	CPreferences::sPrefs->mMailAccounts.SetDirty();

	// Add it back to visual display in new place
	Broadcast_Message(eBroadcast_NewMailAccount, proto);

}

void CMailAccountManager::InitNamespace(CMboxProtocol* proto, CMboxProtocol::SNamespace& names)
{
	boolvector names_used;
	names_used.insert(names_used.begin(), names.size(), false);

	// Add all current WDs
	cdstrvect user;
	const CDisplayItemList& wds = proto->GetMailAccount()->GetWDs();
	for(CDisplayItemList::const_iterator iter = wds.begin(); iter != wds.end(); iter++)
		user.push_back((*iter).GetName());

	// Add all NAMESPACES
	cdstrvect name_list;
	for(int i = CMboxProtocol::ePersonal; i <= CMboxProtocol::ePublic; i++)
	{
		for(cdstrpairvect::const_iterator iter = names.mItems[i].begin(); iter != names.mItems[i].end(); iter++)
			name_list.push_back((*iter).first);
	}

	// Remove ones that match namespace
	for(cdstrvect::iterator iter1 = user.begin(); iter1 != user.end();)
	{
		// Normalise
		cdstring temp = *iter1;
		if ((temp == cWILDCARD) || (temp == cWILDCARD_NODIR))
			temp = cdstring::null_str;
		
		unsigned long ctr = 0;
		bool erased = false;
		for(cdstrvect::const_iterator iter2 = name_list.begin(); iter2 != name_list.end(); iter2++, ctr++)
		{
			if (*iter2 == temp)
			{
				// Mark the NAMESPACE entry as already used
				names_used.at(ctr) = true;
				iter1 = user.erase(iter1);
				erased = true;
			}
		}
		
		if (!erased)
			iter1++;
	}

	// Do dialog
	bool do_auto = proto->GetMailAccount()->GetAutoNamespace();
	if (CNamespaceDialog::PoseDialog(names, names_used, user, do_auto))
	{
		proto->GetMailAccount()->SetAutoNamespace(do_auto);

		// Get names of chosen NAMESPACES
		name_list.clear();
		unsigned long ctr = 0;
		for(int i = CMboxProtocol::ePersonal; i <= CMboxProtocol::ePublic; i++)
		{	
			for(cdstrpairvect::const_iterator iter = names.mItems[i].begin(); iter != names.mItems[i].end(); iter++, ctr++)
			{
				if (names_used.at(ctr))
				{
					// Make sure empty values map to full hierarchy
					if ((*iter).first.empty())
						name_list.push_back(cWILDCARD_NODIR);
					else
						name_list.push_back((*iter).first);
				}
			}
		}

		// Add all original WDs
		cdstrvect original;
		for(CDisplayItemList::const_iterator iter = wds.begin(); iter != wds.end(); iter++)
			original.push_back((*iter).GetName());

		// Remove unwanted
		unsigned long pos = 1;
		for(cdstrvect::iterator iter = original.begin(); iter != original.end(); pos++)
		{
			// Check against user items
			cdstrvect::const_iterator found = std::find(user.begin(), user.end(), *iter);
			if (found != user.end())
			{
				iter++;
				continue;
			}
			
			// Check against NAMEPSACE items
			found = std::find(name_list.begin(), name_list.end(), *iter);
			if (found != name_list.end())
			{
				iter++;
				continue;
			}
				
			// Not found so remove it
			proto->RemoveWD(pos);
			pos--;
			iter = original.erase(iter);
		}
		
		// Now add user items
		for(cdstrvect::iterator iter = user.begin(); iter != user.end(); iter++)
		{
			// Check against current items
			cdstrvect::const_iterator found = std::find(original.begin(), original.end(), *iter);
			if (found == original.end())
				proto->AddWD(CDisplayItem(*iter), 0);
		}

		// Now add NAMESPACE items
		ctr = 0;
		for(int i = CMboxProtocol::ePersonal; i <= CMboxProtocol::ePublic; i++)
		{
			for(cdstrpairvect::const_iterator iter = names.mItems[i].begin(); iter != names.mItems[i].end(); iter++, ctr++)
			{
				if (names_used.at(ctr))
				{
					// Check against current items
					cdstrvect::const_iterator found = std::find(original.begin(), original.end(), (*iter).first);
					if (found == original.end())
						proto->AddWD(CDisplayItem((*iter).first.empty() ? cWILDCARD_NODIR : (*iter).first.c_str()), *(*iter).second.c_str());
				}
			}
		}
	}
}

#pragma mark ____________________________Mailbox Checking

// Called during idle
void CMailAccountManager::SpendTime()
{
	// Do standard check test
	if (!mCheckPaused && CheckTimer()
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		&& CMulberryApp::sMailCheckReset->UserPaused()
#endif
		)
		// Fire off thread
		CMailCheckThread::DoCheck();

	// Always check for opening of mailboxes
	CMailControl::NewMailOpen();

} // CMboxProtocol::SpendTime

// Reset check timer
void CMailAccountManager::Reset()
{
	// This is no longer needed for multiple alerts & threaded checking
#if 0
	// Current time
	time_t last_check = ::time(NULL);

	for(CMailNotificationList::iterator iter = CPreferences::sPrefs->mMailNotification.Value().begin();
			iter != CPreferences::sPrefs->mMailNotification.Value().end(); iter++)
	{
		(*iter).SetLastCheck(last_check);
		(*iter).SetCheckPending(false);
	}
#endif
}

// Check timer
bool CMailAccountManager::CheckTimer()
{
	// Used to indicate very first check
	static bool sOnceCheck = true;

	// Current time
	time_t last_check = ::time(NULL);

	bool result = false;
	for(CMailNotificationList::iterator iter = CPreferences::sPrefs->mMailNotification.Value().begin();
			iter != CPreferences::sPrefs->mMailNotification.Value().end(); iter++)
	{
		// Only if enabled
		if (!(*iter).IsEnabled())
			continue;

		// Check for once
		if ((*iter).GetCheckOnce())
		{
			if (sOnceCheck)
			{
				(*iter).SetCheckPending(true);
				(*iter).SetLastCheck(last_check);
				result = true;
			}
		}
		else if ((*iter).GetCheckInterval() &&
				 (::difftime(last_check, (*iter).GetLastCheck())/cSecsPerMin >= (*iter).GetCheckInterval()))
		{
			(*iter).SetCheckPending(true);
			(*iter).SetLastCheck(last_check);
			result = true;
		}
	}
	
	// Always reset flag
	sOnceCheck = false;

	return result;
}

// Force check of favourites
void CMailAccountManager::ForceMailCheck()
{
	// Current time
	time_t last_check = ::time(NULL);

	// Force pending check on every notifier that's enabled and not being checked once
	for(CMailNotificationList::iterator iter = CPreferences::sPrefs->mMailNotification.Value().begin();
			iter != CPreferences::sPrefs->mMailNotification.Value().end(); iter++)
	{
		// Only if enabled and not check once
		if (!(*iter).IsEnabled() || (*iter).GetCheckOnce())
			continue;

		(*iter).SetCheckPending(true);
		(*iter).SetLastCheck(last_check);
	}
	
	// Now do thread resume
	CMailCheckThread::DoCheck();
}

// Do mail check
bool CMailAccountManager::DoMailCheck(bool allow_alert, bool automatic)
{
	unsigned long total = 0;

	try
	{
		// Must pause to prevent re-entrancy
		Pause(true);
		mHaltCheck = false;

		// Do loop in case alert dialog takes a while
		bool silent = false;

		while(true)
		{
			// Iterate over each notification
			for(CMailNotificationList::iterator iter = CPreferences::sPrefs->mMailNotification.Value().begin();
					!mHaltCheck && (iter != CPreferences::sPrefs->mMailNotification.Value().end()); iter++)
			{
				// Must have check pending on this one
				if (!(*iter).GetCheckPending()) {
					continue;
				}
				// Find the favourite with matching name
				EFavourite type = GetFavouriteFromID((*iter).GetFavouriteID());

				// Only check if there are some mailboxes
				if (mFavourites.at(type)->empty())
				{
					// Reset timer
					(*iter).SetLastCheck(::time(NULL));
					(*iter).SetCheckPending(false);
					continue;
				}

				unsigned long check = RunMailCheck(type, *iter, silent);
				total += check;
				// Do alert here
				if (!mHaltCheck && check && !silent)
				{
					// Reset timer before dialog in case dialog takes a long time
					(*iter).SetLastCheck(::time(NULL));
					(*iter).SetCheckPending(false);

					CMailControl::NewMailAlert(*iter);
				}

				// Check whether another check is required
				if (!mHaltCheck && CheckTimer() && (*iter).GetCheckPending())
					RunMailCheck(type, *iter, true);

				// Reset timer
				(*iter).SetLastCheck(::time(NULL));
				(*iter).SetCheckPending(false);
			}

			// Check whether another check is required
			if (!mHaltCheck && !silent && CheckTimer())
				silent = true;
			else
				break;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}

	// Unpause (will reset)
	Pause(false);

	return total;

} // CMboxProtocol::DoMailCheck

// Do mail check
long CMailAccountManager::RunMailCheck(EFavourite type, const CMailNotification& notifier, bool silent)
{
	long newMail = 0;

	// Resolve to all possible mboxes
	CMboxList match;
	ResolveCabinetList(match, type);

	// Now convert CMbox to CMboxRefs as the mailbox object may get delete whilst this loop
	// is run so we need to dynamically resolve it
	CMboxRefList rmatch(true);
	for(CMboxList::iterator iter = match.begin(); !mHaltCheck && (iter != match.end()); iter++)
		rmatch.AddMbox(static_cast<CMbox*>(*iter));
	
	// Check each mailbox in resolved list
	for(CMboxRefList::iterator iter = rmatch.begin(); !mHaltCheck && (iter != rmatch.end()); iter++)
	{
		CMboxRef* mboxref = static_cast<CMboxRef*>(*iter);
		CMbox* mbox = mboxref->ResolveMbox(true);

		// No related mbox or not logged in
		if (!mbox ||
			!mbox->GetProtocol()->IsLoggedOn() ||
			(mbox->IsLocalMbox() && !mbox->IsCachedMbox()) ||		// Test for uncached disconnected mailbox
			mbox->IsDirectory())
			continue;

#if __dest_os == __mac_os || __dest_os == __mac_os_x
		// Mac OS not pre-emptive so yield during this loop
		// The Mail checking must have lower priority (i.e. yield more often)
		// On Win32 it runs pre-emptive but has a lower priority than the main (UI) thread

		::YieldToAnyThread();	// Perhaps this should yield to the main thread when we have more than two?
#endif

		// Check it
		unsigned long check_num = 0;
		try
		{
			check_num = mbox->Check();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
			
			// Do not allow one failure to prevent checks on other mailboxes
		}
		unsigned long new_num = check_num;
		
		// Check for new messages/all messages
		if (!notifier.GetCheckNew())
			new_num = mbox->GetNumberFound();

		// Now try filtering
		//if (open_mbox && check_num)
		//	CPreferences::sPrefs->GetFilterManager()->NewMailMailbox(open_mbox, check_num);

		// Now do alert if required
		if (new_num && !silent)
		{
			// Do not do alert - only update windows
			CMailControl::NewMailUpdate(mbox, new_num, notifier);
			newMail += new_num;
		}
	}

	return newMail;

}

#pragma mark ____________________________Finding Mailboxes

// Recursively search cache for mbox with requested name
CMbox* CMailAccountManager::FindMbox(const char* mbox_name) const
{
	// Search all entries
	if (!mbox_name) return nil;

	// Try primary list
	CMbox* found = nil;
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); !found && (iter != mProtos.end()); iter++)
		found = (*iter)->FindMbox(mbox_name);

	// Return whatever it got
	return found;
}

// Recursively search cache for mbox with requested account name
CMbox* CMailAccountManager::FindMboxAccount(const char* acct) const
{
	// Search all entries
	if (!acct) return nil;

	// Try primary list
	CMbox* found = nil;
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); !found && (iter != mProtos.end()); iter++)
		found = (*iter)->FindMboxAccount(acct);

	// Return whatever it got
	return found;
}

// Recursively search cache for mbox with requested url
CMbox* CMailAccountManager::FindMboxURL(const char* url) const
{
	// Search all entries
	if (!url) return nil;

	// Try primary list
	CMbox* found = nil;
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); !found && (iter != mProtos.end()); iter++)
		found = (*iter)->FindMboxURL(url);

	// Return whatever it got
	return found;
}

// Return matching mboxes
void CMailAccountManager::FindWildcard(const cdstring& name, CMboxList& mboxes, bool dynamic, bool no_dir) const
{
	const char* p = ::strchr(name.c_str(), cMailAccountSeparator);
	if (!p)
		return;

	// Get account name and bump past delimiter
	cdstring acct_name(name, 0, p - name.c_str());
	cdstring mbox_name(++p);

	// If wildcard account match all
	if (::strchr(acct_name.c_str(), '*'))
	{
		for(CMboxProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
		{
			if ((*iter)->GetAccountName().PatternMatch(acct_name))
				(*iter)->FindWildcard(mbox_name, mboxes, dynamic, no_dir);
		}
	}
	else
	{
		// Try to get matching protocol and do pattern match
		CMboxProtocol* proto = ((CMailAccountManager*) this)->GetProtocol(acct_name);
		if (proto)
			proto->FindWildcard(mbox_name, mboxes, dynamic, no_dir);
	}
}

// Get next mailbox with new messages
CMbox* CMailAccountManager::GetNewMailbox(const CMbox* mbox, bool reverse)
{
	// Get recent favourite list
	CMboxRefList* news = mFavourites.at(eFavouriteNew);

	// Only bother if something present
	if (!news->size())
		return nil;

	// Determine current position
	unsigned long index = 0;
	bool found = false;
	for(CMboxRefList::const_iterator iter = news->begin(); iter != news->end(); iter++, index++)
	{
		if (static_cast<CMboxRef&>(**iter) == *mbox)
		{
			found = true;
			break;
		}
	}

	// If not found determine nearest match
	if (!found)
	{
		// If only one item present then use it
		if (news->size() == 1)
			return static_cast<CMboxRef*>(news->front())->ResolveMbox(true);

		// Get visible index of current mailbox, then find nearest visible index of a new mailbox
		unsigned long vis_index = FetchIndexOf(mbox, true);
		index = 0;
		found = false;
		for(CMboxRefList::const_iterator iter = news->begin(); iter != news->end(); iter++, index++)
		{
			CMbox* mbox = static_cast<CMboxRef*>(*iter)->ResolveMbox(true);
			if (mbox)
			{
				unsigned long new_vis_index = FetchIndexOf(mbox, true);
				if (new_vis_index > vis_index)
				{
					// Back off
					if (index)
						index--;
					else
						index = news->size() - 1;
					found = true;
					break;
				}
			}
		}
	}

	// At this point we have the index of the current mailbox in the new list
	// or the index of the new mailbox previous to the current one (if the current one is not in the new list)

	// Do loop
	unsigned long next_index = index;

	// Do forward/backward search
	if (reverse)
	{
		// Bump down index
		if (next_index)
			next_index--;
		else
			next_index = news->size() - 1;
	}
	else
	{
		// Bump up index
		next_index++;
		if (next_index >= news->size())
			next_index = 0;
	}

		// Make sure its not a directory
	while((next_index != index) && news->at(next_index)->IsDirectory())
	{
		// Do forward/backward search
		if (reverse)
		{
			// Bump down index
			if (next_index)
				next_index--;
			else
				next_index = news->size() - 1;
		}
		else
		{
			// Bump up index
			next_index++;
			if (next_index >= news->size())
				next_index = 0;
		}
	}

	// If same index as before => no more recent
	if (next_index == index)
		return nil;
	else
		return static_cast<CMboxRef*>(news->at(next_index))->ResolveMbox(true);
}

// Get index of protocol in list
long CMailAccountManager::GetProtocolIndex(const CMboxProtocol* proto) const
{
	CMboxProtocolList::const_iterator found = std::find(mProtos.begin(), mProtos.end(), proto);
	if (found != mProtos.end())
		return found - mProtos.begin();
	else
		return -1;
}

// Find position of server
unsigned long CMailAccountManager::FetchIndexOf(const CMboxProtocol* proto, bool adjust_single) const
{
	unsigned long pos = 0;

	// Single proto may be at zero
	if (adjust_single && (mProtoCount == 1))
		return 0;
	bool multi = !adjust_single || (mProtoCount > 1);

	// Match protos
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		pos++;
		if (proto == *iter)
			return pos;

		// Bump pos by number in proto lists
		pos += (*iter)->CountHierarchy(!multi);
	}

	// Not found
	return 0;
}

unsigned long CMailAccountManager::FetchIndexOf(const CMboxRefList* favourite) const
{
	unsigned long pos = 0;
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
	{
		if ((*iter)->IsVisible())
		{
			pos++;
			if (favourite == *iter)
				return pos;

			// Bump pos by number in list
			pos += (*iter)->size();
		}
	}

	// Not found
	return 0;
}

unsigned long CMailAccountManager::FetchSiblingIndexOf(const CMboxRefList* favourite) const
{
	unsigned long pos = 0;
	unsigned long sibling = 0;
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
	{
		if ((*iter)->IsVisible())
		{
			pos++;
			if (favourite == *iter)
				return sibling;

			sibling = pos;

			// Bump pos by number in list
			pos += (*iter)->size();
		}
	}

	// Not found
	return sibling;
}

unsigned long CMailAccountManager::FetchServerOffset() const
{
	unsigned long pos = 0;
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
	{
		if ((*iter)->IsVisible())
		{
			pos++;

			// Bump pos by number in list
			pos += (*iter)->size();
		}
	}

	return pos;
}

// Find position of mailbox
unsigned long CMailAccountManager::FetchIndexOf(const CMbox* mbox, bool adjust_single) const
{
	if (!mProtoCount)
		return 0;

	unsigned long pos = 0;
	bool multi = !adjust_single || (mProtoCount > 1);
	CMboxProtocol* proto = mbox->GetProtocol();

	// Match protos
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		// Bump over server if multi
		if (multi)
			pos++;

		if (proto == *iter)
		{
			unsigned long index = (*iter)->FetchIndexOf(mbox, !multi);
			return (index ? pos + index : 0);
		}

		// Bump pos by number in proto lists
		pos += (*iter)->CountHierarchy(!multi);
	}

	// Not found
	return 0;
}

// Find position of mailbox
void CMailAccountManager::FetchIndexOf(const CMbox* mbox, ulvector& indices, unsigned long offset, bool adjust_single) const
{
	if (!mProtoCount)
		return;

	unsigned long pos = offset;
	bool multi = !adjust_single || (mProtoCount > 1);
	CMboxProtocol* proto = mbox->GetProtocol();

	// Match protos
	for(CMboxProtocolList::const_iterator iter1 = mProtos.begin(); iter1 != mProtos.end(); iter1++)
	{
		// Bump over server if multi
		if (multi)
			pos++;

		if (proto == *iter1)
		{
			// Now get all indices from protocol
			(*iter1)->FetchIndexOf(mbox, indices, pos, !multi);
			return;
		}

		// Bump pos by number in proto lists
		pos += (*iter1)->CountHierarchy(!multi);
	}

	// Not found
	return;
}

// Find position of mailbox
void CMailAccountManager::FetchFavouriteIndexOf(const CMbox* mbox, ulvector& indices) const
{
	unsigned long pos = 0;
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
	{
		if ((*iter)->IsVisible())
		{
			pos++;

			// Try to find match
			unsigned long index = (*iter)->ResolveIndexOf(mbox);
			if (index)
				indices.push_back(pos + index);

			// Bump pos by number in list
			pos += (*iter)->size();
		}
	}
}

#pragma mark ____________________________Favourites

void CMailAccountManager::InitFavourites()
{
	EFavourite type = eFavouriteStart;
	const CFavouriteItemList& list = CPreferences::sPrefs->mFavourites.GetValue();
	for(CFavouriteItemList::const_iterator iter = list.begin(); iter != list.end(); iter++, type = static_cast<EFavourite>(type + 1))
	{
		// Create new item with appropriate name
		if (type < eFavouriteOthers)
			mFavourites.push_back(new CMboxRefList(rsrc::GetIndexedString("UI::Server::CabinetItems", type).c_str()));
		else
			mFavourites.push_back(new CMboxRefList((*iter).GetName()));

		// Sync items that are stored in prefs
		if (IsFavouriteStored(type))
			mFavourites.at(type)->SyncList((*iter).GetItems());
		else
			mFavourites.at(type)->AdjustDirectories();

		mFavourites.at(type)->SetFlag(CTreeNodeList::eSorted, true);
		mFavourites.at(type)->SetFlag(CTreeNodeList::eHierarchic, (*iter).IsHierarchic());
		mFavourites.at(type)->SetFlag(CTreeNodeList::eVisible, (*iter).IsVisible());
		mFavourites.at(type)->SetFlag(CTreeNodeList::eExpanded, (*iter).IsExpanded());
	}
}

void CMailAccountManager::UpdateFavourites()
{
	bool was_dirty = CPreferences::sPrefs->mFavourites.IsDirty();

	// Manually remove any user defined favourites first
	for(CFavourites::iterator iter = mFavourites.begin() + eFavouriteOthers; iter != mFavourites.end(); )
	{
		RemoveFavourite(*iter, false);
	}

	EFavourite type = eFavouriteStart;
	const CFavouriteItemList& list = CPreferences::sPrefs->mFavourites.GetValue();
	for(CFavouriteItemList::const_iterator iter = list.begin(); iter != list.end(); iter++, type = static_cast<EFavourite>(type + 1))
	{
		if (IsFavouriteMboxFlag(type))
		{
			// These do change, sync with mailbox object and then broadcast

			// Get each protocol to sync its mailboxes
			mFavourites.at(type)->SyncList((*iter).GetItems());
			for(CMboxProtocolList::const_iterator iter2 = mProtos.begin(); iter2 != mProtos.end(); iter2++)
				(*iter2)->SyncFlag(CPreferences::sPrefs->mFavourites.GetValue().at(type).GetItems(), GetFavouriteMboxFlag(type));
			//Broadcast_Message(CTreeNodeList::eBroadcast_ResetList, mFavourites.at(type));
		}
		else if (IsFavouriteUserDefined(type))
		{
			mFavourites.push_back(new CMboxRefList((*iter).GetName()));
			mFavourites.at(type)->SyncList((*iter).GetItems());
			mFavourites.at(type)->SetFlag(CTreeNodeList::eSorted, true);
			mFavourites.at(type)->SetFlag(CTreeNodeList::eHierarchic, (*iter).IsHierarchic());
			mFavourites.at(type)->SetFlag(CTreeNodeList::eVisible, (*iter).IsVisible());
			mFavourites.at(type)->SetFlag(CTreeNodeList::eExpanded, (*iter).IsExpanded());

			// Broadcast addition
			Broadcast_Message(CMboxProtocol::eBroadcast_NewList, mFavourites.back());
		}
		else if (IsFavouriteStored(type))
		{
			// These do change and broadcast
			mFavourites.at(type)->SyncList((*iter).GetItems());
			//Broadcast_Message(CTreeNodeList::eBroadcast_ResetList, mFavourites.at(type));
		}
		else
		{
			// These do change and broadcast
			mFavourites.at(type)->AdjustDirectories();
			//Broadcast_Message(CTreeNodeList::eBroadcast_ResetList, mFavourites.at(type));
		}
	}
}

void CMailAccountManager::RenameFavourites(const cdstrpairvect& renames)
{
	for(cdstrpairvect::const_iterator iter1 = renames.begin(); iter1 != renames.end(); iter1++)
	{
		cdstring old_acct = (*iter1).first;
		cdstring new_acct = (*iter1).second;
		old_acct += cMailAccountSeparator;
		new_acct += cMailAccountSeparator;

		// Look at each favourite
		for(CFavourites::const_iterator iter2 = mFavourites.begin(); iter2 != mFavourites.end(); iter2++)
		{
			// Look at each item in the favourite
			for(CMboxRefList::const_iterator iter3 = (*iter2)->begin(); iter3 != (*iter2)->end(); iter3++)
			{
				// Check for old account
				if (::strncmp((*iter3)->GetName().c_str(), old_acct.c_str(), old_acct.length()) == 0)
				{
					cdstring name = &(*iter3)->GetName().c_str()[old_acct.length()];
					(*iter3)->SetName(new_acct + name);
				}
			}

			// Resort
			if ((*iter2)->IsSorted())
				(*iter2)->Sort();
		}
	}
}

// Get type for this favourite
CMailAccountManager::EFavourite CMailAccountManager::GetFavouriteType(const CMboxRefList* list) const
{
	EFavourite type = eFavouriteStart;
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++, type = static_cast<EFavourite>(type + 1))
		if (*iter == list)
			return type;

	return eFavouriteOthers;
}

// Get type for this favourite
CMailAccountManager::EFavourite CMailAccountManager::GetFavouriteFromID(const cdstring& id) const
{
	EFavourite type = eFavouriteStart;
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++, type = static_cast<EFavourite>(type + 1))
		if (GetFavouriteID(type) == id)
			return type;

	return eFavouriteAutoCheck;
}

// Get unique id for favourite
cdstring CMailAccountManager::GetFavouriteID(EFavourite type) const
{
	// Built-in favourites use '#' prefix, user favourites use '@' prefix
	if (type < eFavouriteOthers)
	{
		cdstring temp("#");
		temp += cdstring((unsigned long) type);
		return temp;
	}
	else
	{
		cdstring temp("@");
		temp += mFavourites.at(type)->GetName();
		return temp;
	}
}

bool CMailAccountManager::AddFavourite(const cdstring& name, unsigned long index, bool update_prefs)
{
	CMboxRefList* new_list = new CMboxRefList(name);
	new_list->SetFlag(CTreeNodeList::eSorted, true);
	new_list->SetFlag(CTreeNodeList::eHierarchic, true);
	cdstrpairvect dummy;
	new_list->SyncList(dummy);

	if (index == -1)
		mFavourites.push_back(new_list);
	else
		mFavourites.insert(mFavourites.begin() + index, new_list);

	// Broadcast addition
	Broadcast_Message(CMboxProtocol::eBroadcast_NewList, new_list);

	// Change preferences
	if (update_prefs)
	{
		CFavouriteItemList& list = CPreferences::sPrefs->mFavourites.Value();
		CFavouriteItem item(name);
		if (index == -1)
			list.push_back(item);
		else
			list.insert(list.begin() + index, item);

		// Make prefs as dirty
		CPreferences::sPrefs->mFavourites.SetDirty();
	}

	return true;
}

void CMailAccountManager::RenameFavourite(CMboxRefList* list, const cdstring& name, bool update_prefs)
{
	CFavourites::iterator found = std::find(mFavourites.begin(), mFavourites.end(), list);

	// Must exist
	if (found == mFavourites.end())
		return;

	// Check for valid rename
	unsigned long index = found - mFavourites.begin();

	// Rename item
	(*found)->SetName(name);
	
	// Rename in prefs
	if (update_prefs)
		CPreferences::sPrefs->RenameFavourite(index, name);
}

void CMailAccountManager::RemoveFavourite(CMboxRefList* list, bool update_prefs)
{
	CFavourites::iterator found = std::find(mFavourites.begin(), mFavourites.end(), list);

	// Must exist
	if (found == mFavourites.end())
		return;

	// Check for valid delete
	unsigned long index = found - mFavourites.begin();

	// Broadcast change
	Broadcast_Message(CMboxProtocol::eBroadcast_RemoveList, list);

	// Delete and remove it
	delete *found;
	mFavourites.erase(found);
	
	// Change prefs
	if (update_prefs)
		CPreferences::sPrefs->RemoveFavourite(index);
}

// Resolve a cabinet to actual mailboxes - including wildcards
void CMailAccountManager::ResolveCabinetList(CMboxList& list, EFavourite type) const
{
	// Accumulate all mailboxes from cabinet
	unsigned long index = 0;
	for(CMboxRefList::iterator iter = mFavourites.at(type)->begin();
			iter != mFavourites.at(type)->end(); iter++)
	{
		// Don't bother with directories
		if ((*iter)->IsDirectory())
			continue;

		// Check specifically for wildcard
		if ((*iter)->IsWildcard())
		{
			// Resolve to entire list
			static_cast<const CWildcardMboxRef*>(*iter)->ResolveMbox(list, true, true);
		}
		else
		{
			// Just get a single match but force it
			CMbox* found = static_cast<const CMboxRef*>(*iter)->ResolveMbox(true);
			if (found)
				list.push_back(found);
		}
	}

	// Must eliminate duplicates
	list.RemoveDuplicates();
}

#pragma mark ____________________________Favourite Tests

// Can this favourite be removed
bool CMailAccountManager::IsRemoveableFavouriteType(const CMboxRefList* list) const
{
	return (GetFavouriteType(list) >= eFavouriteOthers);
}

// Can items in this favourite be removed
bool CMailAccountManager::IsRemoveableItemFavouriteType(const CMboxRefList* list) const
{
	return (GetFavouriteType(list) != eFavouriteNew);
}

// Can favourite contain wildcard items
bool CMailAccountManager::IsWildcardFavouriteType(const CMboxRefList* list) const
{
	switch(sMailAccountManager->GetFavouriteType(list))
	{
	case eFavouriteNew:
	case eFavouriteSubscribed:
		return false;
	default:
		return true;
	}
}

bool CMailAccountManager::IsFavouriteMboxFlag(EFavourite type) const
{
	// Look for mail notification types
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++)
	{
		if (type == GetFavouriteFromID((*iter).GetFavouriteID()))
			return true;
	}

	// Look at other types
	switch(type)
	{
	case eFavouriteCopyTo:
	case eFavouriteAppendTo:
	case eFavouritePuntOnClose:
	case eFavouriteAutoSync:
		return true;
	default:
		return false;
	}
}

NMbox::EFlags CMailAccountManager::GetFavouriteMboxFlag(EFavourite type) const
{
	// Look for mail notification types
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++)
	{
		if (type == GetFavouriteFromID((*iter).GetFavouriteID()))
			return NMbox::eAutoCheck;
	}

	// Look at other types
	switch(type)
	{
	case eFavouriteCopyTo:
		return NMbox::eCopyTo;
	case eFavouriteAppendTo:
		return NMbox::eAppendTo;
	case eFavouritePuntOnClose:
		return NMbox::ePuntOnClose;
	case eFavouriteAutoSync:
		return NMbox::eAutoSync;
	default:
		return NMbox::eNone;
	}
}

bool CMailAccountManager::IsFavouriteStored(EFavourite type) const
{
	// Adjust preferences
	switch(type)
	{
	case eFavouriteAutoCheck:
	case eFavouriteOpenLogin:
	case eFavouriteCopyTo:
	case eFavouriteAppendTo:
	case eFavouritePuntOnClose:
	case eFavouriteAutoSync:
	default:
		// Always stored in prefs
		return true;

	case eFavouriteNew:
	case eFavouriteSubscribed:
		// Never stored in prefs
		return false;
	}
}

bool CMailAccountManager::IsFavouriteUserDefined(EFavourite type) const
{
	return (type >= eFavouriteOthers);
}

#pragma mark ____________________________Favourite Items

bool CMailAccountManager::AddFavouriteItem(CMboxRefList* list, CMbox* mbox)
{
	return ProcessFavouriteItem(list, mbox, true, true);
}

bool CMailAccountManager::AddFavouriteItem(CMboxRefList* list, CMboxRef* mboxref)
{
	return ProcessFavouriteItem(list, mboxref, true, true);
}

bool CMailAccountManager::IsFavouriteItem(EFavourite type, const CMbox* mbox) const
{
	return mFavourites.at(type)->ContainsMbox(mbox);
}

bool CMailAccountManager::GetFavouriteAutoCheck(const CMbox* mbox, EFavourite& type) const
{
	// Look for mail notification types
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++)
	{
		type = GetFavouriteFromID((*iter).GetFavouriteID());
		if (mFavourites.at(type)->ContainsMbox(mbox))
			return true;
	}

	return false;
}

bool CMailAccountManager::GetNotifierIndex(EFavourite type, unsigned long& index) const
{
	index = 0;

	// Look for mail notification types
	for(CMailNotificationList::const_iterator iter = CPreferences::sPrefs->mMailNotification.GetValue().begin();
			iter != CPreferences::sPrefs->mMailNotification.GetValue().end(); iter++, index++)
	{
		if (type == GetFavouriteFromID((*iter).GetFavouriteID()))
			return true;
	}

	return false;
	
}

void CMailAccountManager::RemoveFavouriteItem(CMboxRefList* list, CMbox* mbox)
{
	ProcessFavouriteItem(list, mbox, false, true);
}

void CMailAccountManager::RemoveFavouriteItem(CMboxRefList* list, CMboxRef* mboxref)
{
	ProcessFavouriteItem(list, mboxref, false, true);
}

void CMailAccountManager::RemoveAllFavouriteItems(CMboxRefList* list)
{
	// Determine type of list to drop into
	EFavourite type = GetFavouriteType(list);

	// Iterator over all mboxes
	while(list->size())
		ChangeFavouriteItem(list->back(), type, false);

	Broadcast_Message(CTreeNodeList::eBroadcast_ResetList, list);
}

bool CMailAccountManager::ProcessFavouriteItem(CMboxRefList* list, CMbox* mbox, bool add, bool act)
{
	// Determine type of list to drop into
	EFavourite type = GetFavouriteType(list);

	// Cannot add to recent list - maintained elsewhere
	if (type == eFavouriteNew)
		return false;

	// Cannot do directories
	if (mbox->IsDirectory())
		return false;

	// Match against all possible mailboxes in protocol
	CMboxList match;
	mbox->GetProtocol()->FindMbox(mbox->GetName(), match);

	// Iterator over all mboxes
	for(CMboxList::iterator iter = match.begin(); iter != match.end(); iter++)
	{
		// Change flags for favourites that have related CMbox flags
		if (IsFavouriteMboxFlag(type))
			static_cast<CMbox*>(*iter)->SetFlags(GetFavouriteMboxFlag(type), add);
	}

	// Do action if required
	if (act)
	{
		switch(type)
		{
		case eFavouriteSubscribed:
			// Subscribe/Unsubscribe - will add to/remove from list
			if (add)
				mbox->Subscribe();
			else
				mbox->Unsubscribe();
			break;
		default:
			// Just do standard change
			ChangeFavouriteItem(mbox, type, add);
			break;
		}

		// Broadcast change
		Broadcast_Message(eBroadcast_ChangedMbox, mbox);
	}

	return true;
}

bool CMailAccountManager::ProcessFavouriteItem(CMboxRefList* list, CMboxRef* mboxref, bool add, bool act)
{
	// Determine type of list to drop into
	EFavourite type = GetFavouriteType(list);

	// Cannot add to recent list - maintained elsewhere
	if (type == eFavouriteNew)
		return false;

	// Match against all possible mailboxes in protocol
	CMboxList match;
	mboxref->ResolveMbox(match, false);

	// Iterator over all mboxes
	for(CMboxList::iterator iter = match.begin(); iter != match.end(); iter++)
	{
		bool another_match = false;

		// Must check whether this mbox matches another entry when deleting
		if (!add)
		{
			for(CMboxRefList::const_iterator iter2 = list->begin(); iter2 != list->end(); iter2++)
			{
				// Do not match one being deleted
				if (*iter2 != mboxref)
				{
					// Try to match mailbox
					if (((typeid(**iter2) == typeid(CMboxRef)) && (*(CMboxRef*)*iter2 == *(CMbox*)*iter)) ||
						((typeid(**iter2) == typeid(CWildcardMboxRef)) && (*(CWildcardMboxRef*)*iter2 == *(CMbox*)*iter)))
					{
						another_match = true;
						break;
					}
				}
			}
		}

		// Remove flag only if no other match
		if (!another_match)
		{
			// Only set flags if not directory
			if ((*iter)->IsDirectory())
				continue;

			// Change flags for favourites that have related CMbox flags
			if (IsFavouriteMboxFlag(type))
				static_cast<CMbox*>(*iter)->SetFlags(GetFavouriteMboxFlag(type), add);
		}
	}

	// Do action if required
	if (act)
	{
		switch(type)
		{
		// Cannot add to recent list - maintained elsewhere
		case eFavouriteNew:
			return false;

		case eFavouriteSubscribed:
			// Subscribe/Unsubscribe - will add to list
			if (match.size())
			{
				if (add)
					static_cast<CMbox*>(match.front())->Subscribe();
				else
					static_cast<CMbox*>(match.front())->Unsubscribe();
				break;
			}
			// Fall through
		default:
			// Just do standard change
			ChangeFavouriteItem(mboxref, type, add);
			break;
		}

		// Broadcast change
		if (match.size())
			Broadcast_Message(eBroadcast_ChangedMbox, static_cast<CMbox*>(match.front()));
	}

	return true;
}

// Sync address book
void CMailAccountManager::ChangeFavouriteItem(CMboxNode* node, EFavourite type, bool add)
{
	if (add)
	{
		mFavourites.at(type)->AddMbox(node);

		// Adjust preferences after node insert
		SyncFavourite(node->GetAccountName(), node->GetDirDelim(), type, add);
	}
	else
	{
		// Adjust preferences before node delete
		SyncFavourite(node->GetAccountName(), node->GetDirDelim(), type, add);

		mFavourites.at(type)->RemoveMbox(node, true);
	}
}

void CMailAccountManager::RenameFavouriteItem(CMboxRefList* list, CMboxRef* mboxref, const cdstring& new_name)
{
	// Determine type of list to drop into
	EFavourite type = GetFavouriteType(list);

	CMboxRefList::iterator found = std::find(list->begin(), list->end(), mboxref);

	// Must exist
	if (found == list->end())
		return;

	// Must get old name before its changed
	cdstring old_name = mboxref->GetAccountName();

	// First removing cabinet setting from matching mailboxes
	ProcessFavouriteItem(list, mboxref, false, false);

	// Change name
	//mboxref->SetName(new_name);

	// Do visual notifcation
	list->NodeRenamed(mboxref, new_name);

	// Now add cabinet setting to new matching mailboxes
	ProcessFavouriteItem(list, mboxref, true, false);

	// Adjust preferences for stored favourites
	if (IsFavouriteStored(type))
	{
		// Get favourite item
		CFavouriteItemList& prefs_list = CPreferences::sPrefs->mFavourites.Value();
		CFavouriteItem& item = prefs_list.at(type);
		cdstrpair test = cdstrpair(old_name, mboxref->GetDirDelim());

		// Find match comparing only the mailbox names - NOT the dir delims
		cdstrpairvect::iterator found = item.GetItems().begin();
		while (found != item.GetItems().end() && !((*found).first == test.first))
			++found;

		// Change entry if found
		if (found != item.GetItems().end())
		{
			(*found).first = new_name;
			CPreferences::sPrefs->mFavourites.SetDirty();
		}
	}
}

void CMailAccountManager::SetHierarchic(CMboxList* list, bool hier)
{
	list->SetFlag(CTreeNodeList::eHierarchic, hier);
	unsigned long wd_index = list->GetHierarchyIndex();
	if (wd_index == 0)
	{
		if (list->GetProtocol()->GetMailAccount()->GetSubsHierarchic() != hier)
		{
			list->GetProtocol()->GetMailAccount()->SetSubsHierarchic(hier);

			// Make prefs dirty
			CPreferences::sPrefs->mMailAccounts.SetDirty();
		}
	}
	else
	{
		CDisplayItem* wd = &list->GetProtocol()->GetMailAccount()->GetWDs().at(wd_index - 1);
		if (wd->IsHierarchic() != hier)
		{
			wd->SetHierarchic(hier);

			// Make prefs dirty
			CPreferences::sPrefs->mMailAccounts.SetDirty();
		}
	}
}

void CMailAccountManager::SetFlag(CMboxRefList* list, CTreeNodeList::ETreeNodeListFlags flag, bool set)
{
	list->SetFlag(flag, set);
	EFavourite type = GetFavouriteType(list);

	// Get favourite item
	CFavouriteItemList& prefs_list = CPreferences::sPrefs->mFavourites.Value();
	CFavouriteItem& item = prefs_list.at(type);

	// Change state and mark dirty
	switch(flag)
	{
	case CTreeNodeList::eHierarchic:
		item.SetHierarchic(set);
		break;
	case CTreeNodeList::eExpanded:
		item.SetExpanded(set);
		break;
	case CTreeNodeList::eVisible:
		item.SetVisible(set);
		break;
	default:;
	}

	CPreferences::sPrefs->mFavourites.SetDirty();
}


#pragma mark ____________________________Sync Favourite

void CMailAccountManager::SyncFavourite(const char* acct_name, char dir, EFavourite type, bool add)
{
	cdstrpair test = cdstrpair(acct_name, dir);

	// Adjust preferences for stored favourites
	if (IsFavouriteStored(type))
	{
		// Get favourite item
		CFavouriteItemList& prefs_list = CPreferences::sPrefs->mFavourites.Value();
		CFavouriteItem& item = prefs_list.at(type);

		// Find match comparing only the mailbox names - NOT the dir delims
		cdstrpairvect::iterator found = item.GetItems().begin();
		while (found != item.GetItems().end() && !((*found).first == test.first))
			++found;

		// Change entry if found
		if (add && (found == item.GetItems().end()))
		{
			item.GetItems().push_back(test);
			CPreferences::sPrefs->mFavourites.SetDirty();
		}
		else if (!add && (found != item.GetItems().end()))
		{
			item.GetItems().erase(found);
			CPreferences::sPrefs->mFavourites.SetDirty();
		}
	}
}

// Rename in all favourites
void CMailAccountManager::RenamedMailbox(const cdstring& old_name, const cdstring& new_name)
{
	// Go through every cabinet and rename any references to this mailbox
	CMboxRef temp(old_name, 0);
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
		(*iter)->RenamedMbox(old_name, new_name);
}

// Delete from all favourites
void CMailAccountManager::RemovedMailbox(const char* mbox_name)
{
	// Go through every cabinet and delete any references to this mailbox (but not wildcards)
	CMboxRef temp(mbox_name, 0);
	for(CFavourites::const_iterator iter = mFavourites.begin(); iter != mFavourites.end(); iter++)
	{
		CMboxRef* found = (*iter)->FindMbox(&temp);
		if (found && !found->IsWildcard())
			RemoveFavouriteItem(*iter, found);
	}
}

#pragma mark ____________________________MRUs

// MRU mailboxes
void CMailAccountManager::InitMRUs()
{
	mMRUCopyTo.SetFlag(CTreeNodeList::eSorted, false);
	const cdstrvect& prefs_list1 = CPreferences::sPrefs->mMRUCopyTo.GetValue();
	for(cdstrvect::const_iterator iter = prefs_list1.begin(); iter != prefs_list1.end(); iter++)
		mMRUCopyTo.AddNode(new CMboxRef((*iter).c_str(), '.'));

	mMRUAppendTo.SetFlag(CTreeNodeList::eSorted, false);
	const cdstrvect& prefs_list2 = CPreferences::sPrefs->mMRUAppendTo.GetValue();
	for(cdstrvect::const_iterator iter = prefs_list2.begin(); iter != prefs_list2.end(); iter++)
		mMRUAppendTo.AddNode(new CMboxRef((*iter).c_str(), '.'));
}

void CMailAccountManager::RenameMRUs(const cdstrpairvect& renames)
{
	bool copy_to_change = false;
	bool append_to_change = false;

	for(cdstrpairvect::const_iterator iter1 = renames.begin(); iter1 != renames.end(); iter1++)
	{
		cdstring old_acct = (*iter1).first;
		cdstring new_acct = (*iter1).second;
		old_acct += cMailAccountSeparator;
		new_acct += cMailAccountSeparator;

		// Look at each copy to MRU
		for(CMboxRefList::const_iterator iter2 = mMRUCopyTo.begin(); iter2 != mMRUCopyTo.end(); iter2++)
		{
			// Check for old account
			if (::strncmp((*iter2)->GetName().c_str(), old_acct.c_str(), old_acct.length()) == 0)
			{
				cdstring name = &(*iter2)->GetName().c_str()[old_acct.length()];
				(*iter2)->SetName(new_acct + name);
				copy_to_change = true;
			}
		}

		// Look at each append to MRU
		for(CMboxRefList::const_iterator iter2 = mMRUAppendTo.begin(); iter2 != mMRUAppendTo.end(); iter2++)
		{
			// Check for old account
			if (::strncmp((*iter2)->GetName().c_str(), old_acct.c_str(), old_acct.length()) == 0)
			{
				cdstring name = &(*iter2)->GetName().c_str()[old_acct.length()];
				(*iter2)->SetName(new_acct + name);
				append_to_change = true;
			}
		}
	}

	// Broadcast any changes
	if (copy_to_change)
		Broadcast_Message(eBroadcast_MRUCopyToChange, nil);

	if (append_to_change)
		Broadcast_Message(eBroadcast_MRUAppendToChange, nil);
}

void CMailAccountManager::AddMRUCopyTo(CMboxNode* node)
{
	// Is it in favourites list?
	bool found = mFavourites.at(eFavouriteCopyTo)->ContainsMbox(node);

	if (!found)
	{
		// Find position of any original
		unsigned long index;
		if (mMRUCopyTo.FindPos(node, index))
		{
			// Delete the original
			cdstrvect& prefs_list = CPreferences::sPrefs->mMRUCopyTo.Value();
			mMRUCopyTo.RemoveMbox(mMRUCopyTo.at(index), true);
			prefs_list.erase(prefs_list.begin() + index);
		}

		// Try to add it
		if (mMRUCopyTo.AddMbox(node))
		{
			cdstrvect& prefs_list = CPreferences::sPrefs->mMRUCopyTo.Value();
			prefs_list.push_back(node->GetAccountName());

			if ((long) mMRUCopyTo.size() > CPreferences::sPrefs->mMRUMaximum.GetValue())
			{
				mMRUCopyTo.RemoveMbox(mMRUCopyTo.front(), true);
				prefs_list.erase(prefs_list.begin());
			}
		}

		CPreferences::sPrefs->mMRUCopyTo.SetDirty();

		Broadcast_Message(eBroadcast_MRUCopyToChange, nil);
	}
}

void CMailAccountManager::AddMRUAppendTo(CMboxNode* node)
{
	// Is it in favourites list?
	bool found = mFavourites.at(eFavouriteAppendTo)->ContainsMbox(node);

	if (!found)
	{
		// Find position of any original
		unsigned long index;
		if (mMRUAppendTo.FindPos(node, index))
		{
			// Delete the original
			cdstrvect& prefs_list = CPreferences::sPrefs->mMRUAppendTo.Value();
			mMRUAppendTo.RemoveMbox(mMRUAppendTo.at(index), true);
			prefs_list.erase(prefs_list.begin() + index);
		}

		// Try to add it
		if (mMRUAppendTo.AddMbox(node))
		{
			cdstrvect& prefs_list = CPreferences::sPrefs->mMRUAppendTo.Value();
			prefs_list.push_back(node->GetAccountName());

			if ((long) mMRUAppendTo.size() > CPreferences::sPrefs->mMRUMaximum.GetValue())
			{
				mMRUAppendTo.RemoveMbox(mMRUAppendTo.front(), true);
				prefs_list.erase(prefs_list.begin());
			}
		}

		CPreferences::sPrefs->mMRUAppendTo.SetDirty();

		Broadcast_Message(eBroadcast_MRUAppendToChange, nil);
	}
}

void CMailAccountManager::CleanMRUHistory(unsigned long max_history)
{
	// Clear out all MRUs
	mMRUCopyTo.DeleteAll();
	mMRUAppendTo.DeleteAll();

	// Reinitialise
	InitMRUs();

	// Trim copy tos
	bool changed = false;
	while(mMRUCopyTo.size() > max_history)
	{
		mMRUCopyTo.RemoveMbox(mMRUCopyTo.front(), true);
		CPreferences::sPrefs->mMRUCopyTo.Value().erase(CPreferences::sPrefs->mMRUCopyTo.Value().begin());
		changed = true;
	}

	// Broadcast any changes
	if (changed)
		CPreferences::sPrefs->mMRUCopyTo.SetDirty();
	Broadcast_Message(eBroadcast_MRUCopyToChange, nil);

	// Trim append tos
	changed = false;
	while(mMRUAppendTo.size() > max_history)
	{
		mMRUAppendTo.RemoveMbox(mMRUAppendTo.front(), true);
		CPreferences::sPrefs->mMRUAppendTo.Value().erase(CPreferences::sPrefs->mMRUAppendTo.Value().begin());
		changed = true;
	}

	// Broadcast any changes
	if (changed)
		CPreferences::sPrefs->mMRUAppendTo.SetDirty();
	Broadcast_Message(eBroadcast_MRUAppendToChange, nil);
}

#pragma mark ____________________________Disconnected

// Determine if any open protocols
bool CMailAccountManager::HasDisconnectedProtocols() const
{
	for(CMboxProtocolList::const_iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		if ((*iter)->CanDisconnect())
			return true;
	}

	return false;
}

void CMailAccountManager::GoOffline(bool force, bool sync, bool fast, bool partial, unsigned long size, bool sync_list, CProgress* progress)
{
	// Must stop any mail checks in progress
	CMailCheckThread::Pause(true);
	mHaltCheck = true;

	// Try to acquire run lock which is released only when mail check is no longer running
	// Use of mutex to allow messages to be pumped while waiting for mail check to finish
	cdmutex::lock_cdmutex _lock(CMailCheckThread::RunLock());

	// Do sync of auto sync mailboxes if not being forced off
	if (!force && sync)
		DoOfflineSync(fast, partial, size, progress);

	// Clear out new messages cabinet
	RemoveAllFavouriteItems(eFavouriteNew);

	// Save current set of open mailboxes (only if not update then disconnect)
	SaveOpenMailboxes();

	// Go offline on each protocol
	for(CMboxProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		// Convert allowed protos to disconnected state
		if ((*iter)->CanDisconnect())
			(*iter)->GoOffline(force, sync_list);
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
	Broadcast_Message(eBroadcast_RefreshAll);

	// Restore open mailboxes
	RestoreOpenMailboxes();

	// Unpause mail check after sync changes and do immediate check
	CMailCheckThread::Pause(false);
	ForceMailCheck();
}

void CMailAccountManager::DoOfflineSync(bool fast, bool partial, unsigned long size, CProgress* progress)
{
	// Only check if there are some mailboxes
	if (mFavourites.at(eFavouriteAutoSync)->empty())
		return;

	// Resolve to all possible mboxes
	CMboxList match;
	//ResolveCabinetList(match, eFavouriteAutoSync);

	const CMboxRefList&	targets = CDisconnectDialog::GetDisconnectDialog()->GetTargets();
	for(CMboxRefList::const_iterator iter = targets.begin(); iter != targets.end(); iter++)
	{
		CMbox* mbox = static_cast<CMboxRef*>(*iter)->ResolveMbox(true);
		match.push_back(mbox);
	}

	// Must fail silently
	unsigned long item = 0;
	try
	{
		StValueChanger<bool> change(mReuseProto, true);

		// Check each mailbox in resolved list
		for(CMboxList::iterator iter = match.begin(); iter != match.end(); iter++, item++)
		{
			// Bump up mailbox counters
			CDisconnectDialog::GetDisconnectDialog()->NextItem(item);
			
			// Only sync if the parent protocol is actually logged in
			CMbox* mbox = static_cast<CMbox*>(*iter);
			if (mbox->GetProtocol()->IsLoggedOn())
			{
				// Sync it and set indicator
				mbox->Synchronise(fast, partial, size);
				CDisconnectDialog::GetDisconnectDialog()->DoneItem(item, true);
			}
			else
				// Set failed indicator
				CDisconnectDialog::GetDisconnectDialog()->DoneItem(item, false);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Remove UI indicator
		CDisconnectDialog::GetDisconnectDialog()->DoneItem(item, false);
	}

	// Must clean up after last proto
	if (mReusableProto)
	{
		mReusableProto->Close();
		delete mReusableProto;
		mReusableProto = NULL;
	}
}

void CMailAccountManager::GoOnline(bool playback, bool update, bool do_pop3, CProgress* acct_progress, CProgress* item_progress)
{
	// Must stop any mail checks in progress
	CMailCheckThread::Pause(true);
	mHaltCheck = true;

	// Try to acquire run lock which is released only when mail check is no longer running
	// Use of mutex to allow messages to be pumped while waiting for mail check to finish
	cdmutex::lock_cdmutex _lock(CMailCheckThread::RunLock());

	// Clear out new messages cabinet
	RemoveAllFavouriteItems(eFavouriteNew);

	// Save current set of open mailboxes
	SaveOpenMailboxes();

	// Set progress details
	if (acct_progress)
		acct_progress->SetTotal(mProtos.size());

	// Go online on each protocol
	for(CMboxProtocolList::iterator iter = mProtos.begin(); iter != mProtos.end(); iter++)
	{
		// Always reset item progress
		if (item_progress)
			item_progress->SetCount(0);

		// Only do allowed accounts
		if ((*iter)->CanDisconnect())
		{
			// Check whether playback is required and account is logged in
			if (playback && (*iter)->IsLoggedOn() && !(*iter)->DoPlayback(item_progress))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			(*iter)->GoOnline(update);
		}
		
		// Special for POP3 and update
		else if (update && (*iter)->GetMailAccount()->IsPOP3())
		{
			// Only if specifically requested
			if (do_pop3)
			{
				// Must be started before check
				StartProtocol(*iter, false);
				(*iter)->GetINBOX()->Check();
			}
		}

		else if (!update && (*iter)->GetAccount()->GetLogonAtStart())
			// Allow start to throw
			StartProtocol(*iter, false);

		// Set progress details
		if (acct_progress)
			acct_progress->BumpCount();
	}

	Broadcast_Message(eBroadcast_RefreshAll);

	// Restore open mailboxes only if not update then disconnect
	if (!update)
		RestoreOpenMailboxes();

	// Unpause mail check after sync changes
	CMailCheckThread::Pause(false);

	// Check server if staying connected
	if (!update)
		ForceMailCheck();
}

void CMailAccountManager::SaveOpenMailboxes()
{
	// Do 3-pane first (only if we don't already have data)
	if (C3PaneWindow::s3PaneWindow && mSaveOpen3Pane.empty())
	{
		C3PaneWindow::s3PaneWindow->GetOpenItems(mSaveOpen3Pane);
		C3PaneWindow::s3PaneWindow->CloseOpenItems();
	}
	
	// Get open 1-pane windows
	mSaveOpen1Pane.clear();
	cdmutexprotect<CMailboxWindow::CMailboxWindowList>::lock _lock(CMailboxWindow::sMboxWindows);
	for(CMailboxWindow::CMailboxWindowList::iterator iter = CMailboxWindow::sMboxWindows->begin(); iter != CMailboxWindow::sMboxWindows->end(); iter++)
	{
		// Ignore SMTP queue
		if (*iter == CSMTPWindow::sSMTPWindow)
			continue;
		
		// Save the full name of the mailbox
		if ((*iter)->GetMailboxView()->GetMbox())
			mSaveOpen1Pane.push_back((*iter)->GetMailboxView()->GetMbox()->GetAccountName());
	}
}

void CMailAccountManager::RestoreOpenMailboxes()
{
	// Do 3-pane first
	if (C3PaneWindow::s3PaneWindow)
	{
		// Non-disconnectable mailboxes will not be re-opened as their accounts are
		// logged out at this point, and the preview only opens logged in mailboxes
		C3PaneWindow::s3PaneWindow->SetOpenItems(mSaveOpen3Pane);
		mSaveOpen3Pane.clear();
	}
	
	// Restore 1-pane window
	for(cdstrvect::const_iterator iter = mSaveOpen1Pane.begin(); iter != mSaveOpen1Pane.end(); iter++)
	{
		// Try to get real mailbox
		CMbox* mbox = FindMboxAccount(*iter);
		
		// Only restore if its account is set for disconnected operations
		if (mbox && mbox->GetProtocol()->CanDisconnect())
			CActionManager::OpenMailbox(mbox, false);
	}
	mSaveOpen3Pane.clear();
}

#pragma mark ____________________________Sleep

void CMailAccountManager::Suspend()
{
	// Must stop any mail checks in progress
	CMailCheckThread::Pause(true);
	mHaltCheck = true;

	// Try to acquire run lock which is released only when mail check is no longer running
	// Use of mutex to allow messages to be pumped while waiting for mail check to finish
	cdmutex::lock_cdmutex _lock(CMailCheckThread::RunLock());

}

void CMailAccountManager::Resume()
{
	// Unpause mail check after sync changes
	CMailCheckThread::Pause(false);
	
	// Checks will fire off as appropriate for intervals
}
