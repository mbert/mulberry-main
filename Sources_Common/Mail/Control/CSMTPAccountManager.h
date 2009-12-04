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

#ifndef __CSMTPACCOUNTMANAGER__MULBERRY__
#define __CSMTPACCOUNTMANAGER__MULBERRY__

#include "CBroadcaster.h"

#include "CSMTPAccount.h"

#include "ptrvector.h"

class CSMTPSender;
typedef ptrvector<CSMTPSender> CSMTPSenderList;
class CSMTPSendThread;
typedef ptrvector<CSMTPSendThread> CSMTPThreads;
class CIdentity;
class CMbox;
class CMessage;
class CMailAccount;
class CMboxProtocol;

// Classes

class CSMTPAccountManager : public CBroadcaster
{
public:
	static CSMTPAccountManager* sSMTPAccountManager;

	CSMTPAccountManager();
	~CSMTPAccountManager();
	
	// Sending
	bool CanSendDisconnected() const;
	bool CanSendDisconnected(const CIdentity& identity) const;
	bool SendMessage(CMessage* msg, const CIdentity& identity, bool bouncing);

	// Connection
	bool GetConnected() const
		{ return mConnected; }
	void SetConnected(bool connect, bool wait_disconnect = true);
	void SuspendAll(bool allow_finish);

	// SMTP related
	CSMTPSenderList& GetSMTPSenders()							// Get list of senders
		{ return mSMTPSenders; }

	CSMTPSender* GetSMTPSender(const cdstring& acct_name);		// Find SMTP sender matching name
	void SyncAccount(const CSMTPAccount& acct);
	void SyncSMTPSenders(const CSMTPAccountList& accts);
	void ResumeSMTPSender(CSMTPSender* sender);					// Kick thread into life
	CMbox* GetOfflineMbox(CSMTPSender* sender, bool create = false) const;
	CMbox* FindMbox(const cdstring& mbox_name);

	bool ItemsHeld() const;
	bool PendingItems() const;
	bool TestHaltQueues();

	// Sleep controls
	void	Suspend();
	void	Resume();

private:
	CSMTPSenderList		mSMTPSenders;
	CSMTPThreads		mSMTPThreads;
	bool				mConnected;
	bool				mUseQueue;
	CMailAccount*		mAccount;
	CMboxProtocol*		mOffline;
	cdstring			mOfflineCWD;
	
	void OpenOffline();
	void CloseOffline();
	
	void AddOffline(CSMTPSender* sender);
	void RemoveOffline(CSMTPSender* sender);
	
	cdstring GetOfflineWD(const CSMTPAccount& acct) const;

	CSMTPSendThread* GetThread(const CSMTPSender* sender);
};

#endif
