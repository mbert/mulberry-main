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


// Header for CPrefsSimple class

#ifndef __CPREFSSIMPLE__MULBERRY__
#define __CPREFSSIMPLE__MULBERRY__

#include "CPrefsPanel.h"

#include "CINETAccount.h"

#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsSimple = 5012;
const	PaneIDT		paneid_SimpleRealName = 'UNAM';
const	PaneIDT		paneid_SimpleEmailAddress = 'SUID';
const	PaneIDT		paneid_SimpleIMAPuid = 'IUID';
const	PaneIDT		paneid_SimpleIMAPip = 'IMIP';
const	PaneIDT		paneid_SimpleServerIMAP = 'CHIM';
const	PaneIDT		paneid_SimpleServerPOP3 = 'CHPO';
const	PaneIDT		paneid_SimpleServerLocal = 'CHLO';
const	PaneIDT		paneid_SimpleSMTPip = 'SMIP';
const	PaneIDT		paneid_SimpleSaveUser = 'SAVU';
const	PaneIDT		paneid_SimpleSavePswd = 'SAVP';
const	PaneIDT		paneid_SimpleLogonStartup = 'LOGO';
const	PaneIDT		paneid_SimpleLeaveOnServer = 'LEAV';
const	PaneIDT		paneid_SimpleCheckNever = 'CHKN';
const	PaneIDT		paneid_SimpleCheckEvery = 'CHKE';
const	PaneIDT		paneid_SimpleCheckInterval = 'REFR';
const	PaneIDT		paneid_SimpleNewMailAlert = 'ALRT';
const	PaneIDT		paneid_SimpleWDSeparatorTitle = 'SEPT';
const	PaneIDT		paneid_SimpleWDSeparator = 'SEPR';
const	PaneIDT		paneid_SimpleSetSignatureBtn = 'FOOT';
const	PaneIDT		paneid_SimpleDoCopyTo = 'COPY';
const	PaneIDT		paneid_SimpleCopyToMailbox = 'COPN';
const	PaneIDT		paneid_SimpleCopyToMailboxPopup = 'COPM';

// Resources
const	ResIDT		RidL_CPrefsSimpleBtns = 5012;

// Mesages
const	MessageT	msg_SimpleServerIMAP = 'CHIM';
const	MessageT	msg_SimpleServerPOP3 = 'CHPO';
const	MessageT	msg_SimpleServerLocal = 'CHLO';
const	MessageT	msg_SimpleCheckNever = 'CHKN';
const	MessageT	msg_SimpleCheckEvery = 'CHKE';
const	MessageT	msg_SimpleSetSignature = 'FOOT';
const	MessageT	msg_SimpleDoCopyTo = 'COPY';
const	MessageT	msg_SimpleCopyToMailboxPopup = 'COPM';

// Classes
class CTextFieldX;
class LBevelButton;
class CStaticText;
class LCheckBox;
class LCheckBoxGroupBox;
class LRadioButton;
class CMailboxPopup;

class	CPrefsSimple : public CPrefsPanel,
						public LListener
{
private:
	CTextFieldX*		mRealName;
	CTextFieldX*		mEmailAddress;
	CTextFieldX*		mIMAPuid;
	CTextFieldX*		mIMAPip;
	LRadioButton*		mServerIMAP;
	LRadioButton*		mServerPOP3;
	LRadioButton*		mServerLocal;
	CTextFieldX*		mSMTPip;
	LCheckBox*			mSaveUser;
	LCheckBox*			mSavePswd;
	LCheckBox*			mLogonStartup;
	LCheckBox*			mLeaveOnServer;
	LRadioButton*		mCheckNever;
	LRadioButton*		mCheckEvery;
	CTextFieldX*		mCheckInterval;
	LCheckBox*			mNewMailAlert;
	CStaticText*		mDirectorySeparatorTitle;
	CTextFieldX*		mDirectorySeparator;
	LBevelButton*		mSignatureBtn;
	LCheckBoxGroupBox*	mDoCopyTo;
	CTextFieldX*		mCopyToMailbox;
	CMailboxPopup*		mCopyToMailboxPopup;

	cdstring			mSignature;

public:
	enum { class_ID = 'Psmp' };

					CPrefsSimple();
					CPrefsSimple(LStream *inStream);
	virtual 		~CPrefsSimple();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

private:
			void	EditSignature(cdstring* text);			// Edit signature
			void	SetType(CINETAccount::EINETServerType type);
};

#endif
