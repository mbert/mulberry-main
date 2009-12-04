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


// CSMTPAccountManager.h

#include "CSMTPAccountManager.h"

#include "CAdminLock.h"
#include "CConnectionManager.h"
#include "CIdentity.h"
#include "CLocalCommon.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMessage.h"
//#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CSMTPSender.h"
#include "CSMTPSendThread.h"
#include "CSMTPWindow.h"

#include <algorithm>
#include <unistd.h>

CSMTPAccountManager* CSMTPAccountManager::sSMTPAccountManager = NULL;

CSMTPAccountManager::CSMTPAccountManager()
{
	sSMTPAccountManager = this;
	
	// Set connected state based on connection state and last value of SMTP queue enable
	mConnected = CConnectionManager::sConnectionManager.IsConnected() &&
					CPreferences::sPrefs->mSMTPQueueEnabled.GetValue();
	
	// Reset queue enable
	CPreferences::sPrefs->mSMTPQueueEnabled.SetValue(mConnected);
	mUseQueue = !CAdminLock::sAdminLock.mNoSMTPQueues;

	mAccount = NULL;
	mOffline = NULL;

	// Only if allowed
	if (mUseQueue)
	{
		// Warning this may throw if an error occurs while trying
		// to startup the local proto e.g. locked volume. This exception
		// must not propogate out of here!
		try
		{
			OpenOffline();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Force queues off
			CloseOffline();
			
			mUseQueue = false;
		}
	}
}

CSMTPAccountManager::~CSMTPAccountManager()
{
	// Must shut down all threads
	SuspendAll(false);
	
	// Now terminate offline account
	CloseOffline();
	sSMTPAccountManager = NULL;
}

bool CSMTPAccountManager::CanSendDisconnected() const
{
	// Check lock
	if (!mUseQueue)
		return false;

	// Look at each sender
	for(CSMTPSenderList::const_iterator iter = mSMTPSenders.begin(); iter != mSMTPSenders.end(); iter++)
		if ((*iter)->GetUseQueue())
			return true;
	
	return false;
}

bool CSMTPAccountManager::CanSendDisconnected(const CIdentity& identity) const
{
	// Check lock
	if (!mUseQueue)
		return false;

	// Get matching sender
	cdstring acct_name = identity.GetSMTPAccount(true);
	CSMTPSender* smtp = const_cast<CSMTPAccountManager*>(this)->GetSMTPSender(acct_name);
	if (!smtp)
		return false;

	return smtp->GetUseQueue();
}

bool CSMTPAccountManager::SendMessage(CMessage* msg, const CIdentity& id, bool bouncing)
{
	bool held = false;

	// Get matching sender
	cdstring acct_name = id.GetSMTPAccount(true);
	CSMTPSender* smtp = GetSMTPSender(acct_name);
	if (!smtp)
	{
		CLOG_LOGTHROW(CGeneralException, -1);
		throw CGeneralException(-1);
	}

	// Look for mailbox
	CMbox* mbox = GetOfflineMbox(smtp);

	// Do offline store
	if (mbox)
	{
		// Must append with draft flag set, but remove draft after
		SBitFlags& msg_flags = msg->GetFlags();
		msg_flags.Set(NMessage::eDraft, true);

		// Check whether to hold new messages being added to the queue
		if (static_cast<CSMTPAccount*>(smtp->GetAccount())->GetHoldMessages())
			msg_flags.Set(NMessage::eMDNSent, true);

		// Append but don't do files and don't do MRUs
		unsigned long new_uid = 0;
		mbox->AppendMessage(msg, new_uid, false, false);

		// Now reset flags
		msg_flags.Set(NMessage::eDraft | NMessage::eMDNSent, false);

		// Reset any open window
		CMailboxWindow* aWindow = CMailboxWindow::FindWindow(mbox);
		if (aWindow)
			aWindow->GetMailboxView()->ResetTable();

		// if connected then resume its thread
		if (mConnected)
			GetThread(smtp)->Resume();
		else
			held = ItemsHeld();
	}
	else
		smtp->SMTPSend(msg, false);
		
	return held;
}

void CSMTPAccountManager::SetConnected(bool connected, bool wait_disconnect)
{
	// Only if changing
	if (connected == mConnected)
		return;

	if (connected)
	{
		// Resume all threads
		for(CSMTPThreads::iterator iter = mSMTPThreads.begin(); iter != mSMTPThreads.end(); iter++)
		{
			if (*iter)
				(*iter)->Resume();
		}
	}
	else
		// Suspend all threads
		SuspendAll(wait_disconnect);

	mConnected = connected;
	
	// Reset state preference
	CPreferences::sPrefs->mSMTPQueueEnabled.SetValue(mConnected);

	// Refresh any queue window
	if (CSMTPWindow::sSMTPWindow)
		CSMTPWindow::sSMTPWindow->ResetConnection();
}

void CSMTPAccountManager::SuspendAll(bool allow_finish)
{
	// Suspend all threads
	for(CSMTPThreads::iterator iter = mSMTPThreads.begin(); iter != mSMTPThreads.end(); iter++)
	{
		// Might not be threaded
		if (!*iter)
			continue;

		// Force pause of SMTP batch processing if not waiting
		if (!allow_finish)
			(*iter)->Suspend();

		// NB The mutex will throw if it times on on aquire. However, we MUST NOT allow the
		// SMTP suspension to timeout or throw up from here as it may prevent proper disconnect
		// handling - the SMTP queue is left active in disconnected mode
		
		bool suspend_complete = false;
		while(!suspend_complete)
		{
			try
			{
				// Try to acquire run lock which is released only when SMTP is no longer running
				// Use of mutex to allow messages to be pumped while waiting for SMTP to finish
				cdmutex::lock_cdmutex _lock((*iter)->GetRunLock());
				
				// Now make sure its suspended
				(*iter)->Suspend();
				
				suspend_complete = true;
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);
			}
		}
	}
}

// Find SMTP sender matching server ip - create if no exist
CSMTPSender* CSMTPAccountManager::GetSMTPSender(const cdstring& acct_name)
{
	// Look for match first
	for(CSMTPSenderList::iterator iter = mSMTPSenders.begin(); iter != mSMTPSenders.end(); iter++)
		if ((*iter)->GetAccountName() == acct_name)
			return *iter;
		
	// Always try and return something that is valid
	return (mSMTPSenders.size() != 0) ? mSMTPSenders.front() : NULL;
}

void CSMTPAccountManager::SyncAccount(const CSMTPAccount& acct)
{
	if (!mAccount)
	{
		// Create and initialise the account
		mAccount = new CMailAccount;
		mAccount->SetServerType(CINETAccount::eLocal);

		// Create a search hierarchy to list all mailboxes
		CDisplayItemList wds;
		wds.push_back(CDisplayItem("*"));
		mAccount->SetWDs(wds);
	}

	// Get appropriate local path for files
	cdstring wd = GetOfflineWD(acct);
	mAccount->GetCWD().SetName(wd);
	mOfflineCWD = wd;
}

void CSMTPAccountManager::SyncSMTPSenders(const CSMTPAccountList& accts)
{
	// Brute force method in lieu of a smart algorithm:
	// Just delete old set and add new set

	// Remove all existing senders first (must suspend them)
	SuspendAll(false);
	while(mSMTPSenders.size())
		RemoveOffline(mSMTPSenders.front());

	// Check for change to offline mailbox path if queues in use
	if (mUseQueue && (mOfflineCWD != GetOfflineWD(*accts.front())))
	{
		// Close out existing protocol
		CloseOffline();
		
		// Now open it again - this will get the new path loaded
		OpenOffline();
	}

	// Look for existing protocols and sync or add new protocols
	for(CSMTPAccountList::const_iterator iter = accts.begin(); iter != accts.end(); iter++)
		AddOffline(new CSMTPSender(*iter));
}

// Kick thread into life
void CSMTPAccountManager::ResumeSMTPSender(CSMTPSender* sender)
{
	// Only if connected
	if (mConnected)
	{
		CSMTPSendThread* thread = GetThread(sender);
		if (thread)
			thread->Resume();
	}
}

// Check to see if queues are disabled but connection is active
bool CSMTPAccountManager::ItemsHeld() const
{
	// Only if network connection present and SMTP queues are disabled
	return (CConnectionManager::sConnectionManager.IsConnected() && !GetConnected());
}

// Check to see if queues are enabled and there are some held drafts
bool CSMTPAccountManager::PendingItems() const
{
	// Check all threads
	for(CSMTPSenderList::const_iterator iter = mSMTPSenders.begin(); iter != mSMTPSenders.end(); iter++)
	{
		CMbox* mbox = GetOfflineMbox(*iter);

		// Might not be threaded
		if (!mbox)
			continue;

		bool found = false;
		try
		{
			// Ping mailbox for messages
			mbox->Check();
			
			// If none then skip it
			if (!mbox->GetNumberMessages())
				continue;
		
			// Open using EXAMINE to keep it read-only - i.e. no flag changes
			// Also open but do not use 'full open' state so that message lists are not created
			mbox->Open(NULL, false, true, false);
			
			// Search for undeleted hold messages
			ulvector dummy;
			mbox->MatchMessageFlags(NMessage::eHold, NMessage::eDeleted, dummy, false);
			found = dummy.size();
			
			// Always close it
			mbox->Close();
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);
		}
		
		// If any found then return immediately
		if (found)
			return true;
	}
	
	return false;
}

// Check to see if queues have anything to process and suspend if not
bool CSMTPAccountManager::TestHaltQueues()
{
	// Only bother if connected
	if (!GetConnected())
		return true;

	// Suspend all threads
	for(CSMTPThreads::iterator iter = mSMTPThreads.begin(); iter != mSMTPThreads.end(); iter++)
	{
		// Might not be threaded
		if (!*iter)
			continue;

		// Try to acquire run lock which is released only when SMTP is no longer running
		// Use of mutex to allow messages to be pumped while waiting for SMTP to finish
		cdmutex::try_cdmutex _lock((*iter)->GetRunLock());
		
		// If not locked (i.e. thread still running) must exit
		if (!_lock.GotLock())
			return false;

		// Now make sure its suspended
		(*iter)->Suspend();
	}
	
	return true;
}

cdstring CSMTPAccountManager::GetOfflineWD(const CSMTPAccount& acct) const
{
	// Check lock
	if (!mUseQueue)
		return cdstring::null_str;

	cdstring account_wd = acct.GetCWD().GetName();
	cdstring wd;

	try
	{
		// Look for user specified SMTP account directory
		if (account_wd.empty())
		{
			// Must ensure directory structure exists
			wd = CConnectionManager::sConnectionManager.GetCWD();
			::chkdir(wd);

			// Create first level of default
			wd += "Mailboxes";
			wd += os_dir_delim;
			::chkdir(wd);

			// Create first level of default
			wd += "SMTPQueues";
			wd += os_dir_delim;
			::chkdir(wd);
			
			account_wd = wd;
		}
		else
		{
			// Determine absolute or relative path
			cdstring wd;
			if (IsRelativePath(account_wd))
			{
				// Prefix with CWD
				wd = CConnectionManager::sConnectionManager.GetCWD();
				::chkdir(wd);

		#if __dest_os == __mac_os || __dest_os == __mac_os_x
				wd += &account_wd.c_str()[1];
		#elif __dest_os == __win32_os
				wd += &account_wd.c_str()[1];
		#else
				wd += account_wd;
		#endif
			}
			else
				wd = account_wd;

			::chkdir(wd);
			
			account_wd = wd;
		}
	}
	catch(CGeneralException& gex)
	{
		CLOG_LOGCATCH(CGeneralException& ex);
			
		::report_file_error(wd, gex.GetErrorCode());

		account_wd = cdstring::null_str;
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
						
		account_wd = cdstring::null_str;
	}

	return account_wd;
}

void CSMTPAccountManager::OpenOffline()
{
	// Check lock
	if (!mUseQueue)
		return;

	// Init account
	SyncAccount(*CPreferences::sPrefs->mSMTPAccounts.GetValue().front());

	// Init protocol
	mOffline = new CMboxProtocol(mAccount);
	mOffline->InitProtocol();
	mOffline->Open();
	mOffline->Logon();
	mOffline->LoadList();

	mUseQueue = true;
}

void CSMTPAccountManager::CloseOffline()
{
	// May be NULL if admin locked
	if (!mOffline)
		return;

	// Close account
	mOffline->Logoff();

	delete mOffline;
	mOffline = NULL;
	delete mAccount;
	mAccount = NULL;

	mUseQueue = false;
}

void CSMTPAccountManager::AddOffline(CSMTPSender* sender)
{
	// Add it to the list
	mSMTPSenders.push_back(sender);
	
	// Create thread only if queuing enabled
	if (mUseQueue &&
		static_cast<CSMTPAccount*>(sender->GetAccount())->GetUseQueue())
	{
		CSMTPSendThread* thread = NULL;
		bool added = false;
		
		// Don't let this fail
		try
		{
			// Set use of queue
			sender->SetUseQueue(true);

			// Make sure mailbox exists - will create if missing
			GetOfflineMbox(sender, true);

			thread = new CSMTPSendThread(sender);
			mSMTPThreads.push_back(thread);
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Clean up
			delete thread;

			// Force this sender to operate without a queue
			sender->SetUseQueue(false);
			mSMTPThreads.push_back(NULL);
		}
	}
	else
	{
		// Don't use queue
		sender->SetUseQueue(false);
		mSMTPThreads.push_back(NULL);
	}
}

void CSMTPAccountManager::RemoveOffline(CSMTPSender* sender)
{
	// Find it in the list
	CSMTPSenderList::iterator found = std::find(mSMTPSenders.begin(), mSMTPSenders.end(), sender);
	if (found != mSMTPSenders.end())
	{
		// Shut down its thread
		CSMTPThreads::iterator thread = mSMTPThreads.begin() + (found - mSMTPSenders.begin());
		if (*thread)
			(*thread)->Suspend();
		mSMTPThreads.erase(thread);
		mSMTPSenders.erase(found);
	}
}

CSMTPSendThread* CSMTPAccountManager::GetThread(const CSMTPSender* sender)
{
	CSMTPSenderList::iterator found = std::find(mSMTPSenders.begin(), mSMTPSenders.end(), sender);
	if (found != mSMTPSenders.end())
		return *(mSMTPThreads.begin() + (found - mSMTPSenders.begin()));
	else
		return NULL;
}

CMbox* CSMTPAccountManager::GetOfflineMbox(CSMTPSender* sender, bool create) const
{
	// See if queue is globally enabled and enabled in sender's account
	if (mUseQueue && sender->GetUseQueue())
	{
		// return mailbox if already present
		if (sender->GetQueueMbox())
			return sender->GetQueueMbox();

		// Make sure a mailbox exists
		cdstring mbox_name = sender->GetAccountName();

		// Look for matching mailbox
		CMbox* mbox = mOffline->FindMbox(mbox_name);
		if (!mbox && create)
		{
			// Create it
			mbox = new CMbox(mOffline, mbox_name, os_dir_delim, mOffline->GetHierarchies().front(), false);
			
			// Add mbox to protocol list and create it
			mbox = mbox->AddMbox();
			if (!mbox)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}
			mbox->Create();
		}

		// Must never appear in cabinets
		mbox->SetFlags(NMbox::eNoCabinet);
		
		// Now give mbox to async SMTP sender
		sender->SetQueueMbox(mbox);

		return mbox;
	}
	else
		return NULL;
}

CMbox* CSMTPAccountManager::FindMbox(const cdstring& mbox_name)
{
	return mOffline ? mOffline->FindMbox(mbox_name) : NULL;
}

#pragma mark ____________________________Sleep

void CSMTPAccountManager::Suspend()
{
	// Must stop any sending in progress when connected
	if (mConnected)
		SuspendAll(false);
}

void CSMTPAccountManager::Resume()
{
	// Resume all threads if in connected state
	if (mConnected)
	{
		for(CSMTPThreads::iterator iter = mSMTPThreads.begin(); iter != mSMTPThreads.end(); iter++)
		{
			if (*iter)
				(*iter)->Resume();
		}
	}
}
