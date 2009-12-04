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


// Header for CPrefsAccount class

#ifndef __CPREFSACCOUNT__MULBERRY__
#define __CPREFSACCOUNT__MULBERRY__

#include "CPrefsPanel.h"

#include "CAuthenticator.h"
#include "CINETAccount.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsAccount = 5001;
const	PaneIDT		paneid_AccountPopup = 'ACCT';
const	PaneIDT		paneid_AccountType = 'TYPE';
const	PaneIDT		paneid_AccountServerIP = 'SERV';
const	PaneIDT		paneid_AccountPanel = 'PANL';
const	PaneIDT		paneid_AccountMatchUIDPswd = 'LINK';

// Mesages
const	MessageT	msg_MailAccountPopup = 'ACCT';
enum
{
	eMailAccountPopup_New = 1,
	eMailAccountPopup_Rename,
	eMailAccountPopup_Delete,
	eMailAccountPopup_Off,
	eMailAccountPopup
};

const	MessageT	msg_MailAccountType = 'TYPE';
enum
{
	eMailAccountTypePopup_IMAP = 1
};

const	MessageT	msg_AccountTabs = 'TABS';

// Resources
const	ResIDT		RidL_CPrefsAccountBtns = 5001;

const ResIDT		menu_AccountIMAPMailbox = 1;
const ResIDT		menu_AccountPOPMailbox = 2;
const ResIDT		menu_AccountLocalMailbox = 3;
const ResIDT		menu_AccountSMTPMailbox = 5;
const ResIDT		menu_AccountWebDAVOptions = 7;
const ResIDT		menu_AccountIMSPOptions = 8;
const ResIDT		menu_AccountACAPOptions = 9;
const ResIDT		menu_AccountCardDAVAdbk = 11;
const ResIDT		menu_AccountIMSPAdbk = 12;
const ResIDT		menu_AccountACAPAdbk = 13;
const ResIDT		menu_AccountLDAPSearch = 15;
const ResIDT		menu_AccountManageSIEVE = 17;
const ResIDT		menu_AccountCalDAVCalendar = 19;
const ResIDT		menu_AccountWebDAVCalendar = 20;

const ResIDT		cicn_MailAccount = 4;
const ResIDT		cicn_SMTPAccount = 5;
const ResIDT		cicn_RemoteAccount = 6;
const ResIDT		cicn_AdbkAccount = 7;
const ResIDT		cicn_AddrSearchAccount = 8;
const ResIDT		cicn_ManageSIEVEAccount = 9;
const ResIDT		cicn_CalendarAccount = 10;

// Classes
class CPrefsAccountPanel;
class CTextFieldX;
class CStaticText;
class LCheckBox;
class LPopupButton;

class	CPrefsAccount : public CPrefsPanel,
						public LListener
{
private:
	LPopupButton*		mAccountPopup;
	int					mAccountValue;
	bool				mIsSMTP;
	CStaticText*		mServerType;
	CTextFieldX*		mServerIP;
	LView*				mPanel;
	CPrefsAccountPanel*	mCurrentPanel;
	short				mCurrentPanelNum;
	LCheckBox*			mMatchUIDPswd;

public:
	enum { class_ID = 'Pacc' };

					CPrefsAccount();
					CPrefsAccount(LStream *inStream);
	virtual 		~CPrefsAccount();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

private:
	virtual void	InitAccountMenu(void);							// Set up account menu
	virtual void	DoNewAccount(void);								// Add new account
	virtual void	DoRenameAccount(void);							// Rename account
	virtual void	DoDeleteAccount(void);							// Delete accoount
	virtual void	UpdateItems(bool enable);						// Update item entry
	virtual void	SetAccount(const CINETAccount* account);		// Set account details
	virtual void	UpdateAccount(void);							// Update current account
	
	virtual void	GetCurrentAccount(CINETAccount*& acct, 
										long& index,
										CINETAccountList*& list);	// Get current account details

	virtual void	SetPanel(const CINETAccount* account);			// Set panel
};

#endif
