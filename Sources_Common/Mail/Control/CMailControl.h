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


// CMailControl.h

/*
	Class to handle interaction between OS independent mail classes and OS/GUI dependent
	actions.
*/

#ifndef __CMAILCONTROL__MULBERRY__
#define __CMAILCONTROL__MULBERRY__

#include <time.h>
#include <deque>
#include <queue>
#include <vector>
#include "cdstring.h"
#include "templs.h"

#include "CINETAccount.h"

#include "CMboxRefList.h"

class CAdbkProtocol;
class CAddressList;
class CAdbkAddress;
class CAddressBook;
class CAttachment;
class CAuthenticator;
class CGroup;
class CGroupList;
class CINETProtocol;
class CMailNotification;
class CMboxProtocol;
class CMbox;
class CMessage;

typedef std::vector<CINETProtocol*> CINETProtocolList;

typedef std::vector<CMbox*> CSimpleMboxList;

class CBusyContext
{
public:
	CBusyContext() {mBusyTime = 0; mCancelOthers = false; }
	~CBusyContext() {}
	
	void Start(const cdstring& desc)
		{ mBusyTime = ::time(NULL); mDescriptor = desc; }

	time_t GetBusyTime() const
		{ return mBusyTime; }
		
	const cdstring& GetDescriptor() const
		{ return mDescriptor; }
	void SetDescriptor(const cdstring& desc)
		{ mDescriptor = desc; }
	
	bool CancelOthers() const
		{ return mCancelOthers; }
	void SetCancelOthers(bool others)
		{ mCancelOthers = others; }
		
private:
	 time_t		mBusyTime;
	 cdstring	mDescriptor;
	 bool		mCancelOthers;
};

class CMailControl
{
public:
	// Forced changes in server state
	static void		MboxServerReconnect(CMboxProtocol* server);
	static void		MboxServerDisconnect(CMboxProtocol* server);
	static bool		CleanUpServerDisconnect(CMboxProtocol* server, bool reconnect);
	static void		CleanUpMboxClose(CMbox* mbox);

	static void		AdbkServerReconnect(CAdbkProtocol* server);
	static void		AdbkServerDisconnect(CAdbkProtocol* server);

	static void		UserLogon();		// Force user logon

	// Unsolicted update of windows
	static void		MboxNewMessages(CMbox* mbox,		// New messages in mailbox
									unsigned long count);
	static void		MboxAbouttoUpdate(CMbox* mbox);		// Update about to happen
	static void		MboxUpdate(CMbox* mbox);			// Update only - no reset
	static void		MboxReset(CMbox* mbox);				// Reset message list
	static void		MboxRefresh(CMbox* mbox);			// Reset mailbox view

	static void		MessageChanged(CMessage* msg, bool replaced = false);			// Message state change
	static void		MessageRemoved(CMessage* msg);			// Message removed

	static void		AddressBookClosed(CAddressBook* adbk);							// Address book closed

	static void		AddressAdded(CAddressBook* adbk, CAddressList* addrs);			// Address added
	static void		AddressChanged(CAddressBook* adbk, CAddressList* addrs);		// Address state change
	static void		AddressRemoved(CAddressBook* adbk, CAddressList* addrs);		// Address removed

	static void		GroupAdded(CAddressBook* adbk, CGroupList* grps);				// Group added
	static void		GroupChanged(CAddressBook* adbk, CGroupList* grps);				// Group state change
	static void		GroupRemoved(CAddressBook* adbk, CGroupList* grps);				// Group removed

	// Dialogs required during mail/network processing
	static bool		PromptUser(CAuthenticator* auth,
								const CINETAccount* acct,
								bool secure,
								bool rmail, bool rsmtp, bool rprefs, bool radbk, bool rfilter, bool rhttp,
								bool first);
	static bool		DoUserIDPassword(CAuthenticatorUserPswd* auth,
										const CINETAccount* acct,
										bool secure,
										bool rmail, bool rsmtp, bool rprefs, bool radbk, bool rfilter, bool rhttp,
										bool save_uid, bool save_pswd, bool first);

	// New mail notification
	static void		NewMailUpdate(CMbox* mbox, long num_new, const CMailNotification& notifier);
	static void		NewMailAlert(const CMailNotification& notifier);
	static void		NewMailOpen();
	static void		NewMailAlertSpeakMbox(const CMbox* mbox, cdstring& speak);

	// Message/Attachment size alert
	static bool		CheckSizeWarning(const CMessage* msg, bool all = false);
	static bool		CheckSizeWarning(const CAttachment* attach);

	// Periodics
	static void			SpendTime(bool force_tickle, bool do_checks);
	static void			RegisterPeriodic(CINETProtocol* periodic, bool add);

	// Async alert processing
	static	void		PushAlert(cdstring& txt)
							{ sAlerts.push(txt); }
	static	bool		HasAlert()
							{ return !sAlerts.empty(); }
	static	void		PopAlert(cdstring& txt);

	// Status feedback

	static	bool		ResolveNickName(const char* nick_name,
											CAddressList* list);	// Resolve nick-name
	static	bool		ResolveGroupName(const char* grp_name,
											CAddressList* list,
											short level = 0);		// Resolve group-name

	// Busy processing
	static void		StartBusy();
	static void		StopBusy();
	static bool		IsBusy()
		{ return sBlockCount > 0; }
	static void		BlockBusy(bool block)
		{ sBlockBusy = block; }
	static void		AllowBusyKeys(bool keys)
		{ sAllowBusyKeys = keys; }

	static bool		BusyDialog()
		{ return sBusyDialog; }
	static bool		ProcessBusy(const CBusyContext* busy, bool allow_cancel = true);

#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static void		PreventYield(bool yield)
		{ sPreventYield = yield; }
	static bool		GetPreventYield()
		{ return sPreventYield; }
#endif

private:
	static bool					sBlockBusy;			// Prevent actions during busy call
	static bool					sAllowBusyKeys;		// Allow typing during busy operation
	static bool					sWasBlocked;		// Previously busy by someone else
	static unsigned long		sBlockCount;		// Counts nested block busy calls
	static unsigned long		sBusyCount;			// Busy dialog count
	static bool					sBusyDialog;		// Busy dialog on screen
	static bool					sCancelOthers;		// Force working threads to cancel
	static cdstrqueue			sAlerts;			// Async alert messages
	static CINETProtocolList	sPeriodics;			// Items for periodic checking
	static bool					sPeriodicsChanged; 	// Indicates change in sPeriodics list
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	static bool					sPreventYield;		// Prevent yielding in main thread - hack for non-re-entrant WinSock during blocking call (LDAP)
#endif
	
	// New mail alert items
	static long					sTotalNew;			// Accumalted number of new messages
	static CSimpleMboxList		sMboxToAlert;		// Accumulated mailboxes for alert
	static CMboxRefList			sMboxToOpen;		// List of mailboxes to open

	static bool BusyCancel(const CBusyContext* busy,			// Do busy processing looking for user cancel
								bool allow_cancel);
	static void	DisplayBusyDialog(const CBusyContext& busy);	// Display busy dialog to user
	static bool	HandleBusyDialog(const CBusyContext& busy);		// Handle busy dialog event returning if cancelled
	static void	RemoveBusyDialog();								// Remove the busy dialog

	// Never created - always static
		CMailControl() {};
		~CMailControl() {};
};

class StMailBusy
{
public:
	StMailBusy(CBusyContext* busy, const cdstring* desc)
		{ if (busy && desc) busy->Start(*desc); CMailControl::StartBusy(); }
	~StMailBusy()
		{ CMailControl::StopBusy(); }
};

#endif
