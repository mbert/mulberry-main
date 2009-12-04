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


// Header for CPrefsAccountAuth class

#ifndef __CPREFSACCOUNTAUTH__MULBERRY__
#define __CPREFSACCOUNTAUTH__MULBERRY__

#include "CPrefsTabSubPanel.h"

#include "CAuthenticator.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountAuth = 5020;
const	PaneIDT		paneid_AccountAuthPopup = 'AUTH';
const	PaneIDT		paneid_AccountAuthPanel = 'AUTP';
const	PaneIDT		paneid_AccountTLSGroup = 'TLSG';
const	PaneIDT		paneid_AccountTLSPopup = 'TLS ';
const	PaneIDT		paneid_AccountUseTLSClientCert = 'UCER';
const	PaneIDT		paneid_AccountTLSClientCert = 'CERT';

// Mesages
const	MessageT	msg_MailAccountAuth = 'AUTH';
enum
{
	eMailAccountAuthPopup_PlainText = 1,
	eMailAccountAuthPopup_CramMD5,
	eMailAccountAuthPopup_DigestMD5,
	eMailAccountAuthPopup_Kerberos,
	eMailAccountAuthPopup_Separator,
	eMailAccountAuthPopup_Anonymous
};
const	MessageT	msg_AccountTLSPopup = 'TLS ';
const	MessageT	msg_AccountUseTLSClientCert = 'UCER';

enum
{
	eTLSPopup_NoTLS = 1,
	eTLSPopup_SSL,
	eTLSPopup_TLS
};

// Resources
const	ResIDT		RidL_CPrefsAccountAuthBtns = 5020;

// Classes
class CINETAccount;
class CPrefsAuthPanel;
class LCheckBox;
class LPopupButton;

class	CPrefsAccountAuth : public CPrefsTabSubPanel,
							public LListener
{
private:
	LPopupButton*		mAuthPopup;
	LView*				mAuthSubPanel;
	CPrefsAuthPanel*	mCurrentPanel;										// Current panel view
	ResIDT				mCurrentPanelNum;									// Current prefs panel index
	cdstring			mAuthType;
	LPopupButton*		mTLSPopup;
	LCheckBox*			mUseTLSClientCert;
	LPopupButton*		mTLSClientCert;

	cdstrvect			mCertSubjects;
	cdstrvect			mCertFingerprints;

public:
	enum { class_ID = 'Aath' };

					CPrefsAccountAuth();
					CPrefsAccountAuth(LStream *inStream);
	virtual 		~CPrefsAccountAuth();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(bool IC_on);	// Toggle display of IC
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

protected:
	virtual void	SetAuthPanel(const cdstring& auth_type);		// Set auth panel
			void	BuildAuthPopup(CINETAccount* account);
			void	InitTLSItems(CINETAccount* account);
			void	BuildCertPopup();
			void	TLSItemsState();
};

#endif
