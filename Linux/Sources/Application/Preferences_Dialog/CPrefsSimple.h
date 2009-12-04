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


// CPrefsSimple.h : header file
//

#ifndef __CPREFSSIMPLE__MULBERRY__
#define __CPREFSSIMPLE__MULBERRY__

#include "CPrefsPanel.h"

#include "CINETAccount.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSimple dialog

class JXSecondaryRadioGroup;
class JXRadioGroup;
class JXStaticText;
class JXTextCheckbox;
class JXImageButton;
class CMailboxPopup;
class CTextInputField;
class JXIntegerInput;
class CMailboxPopupButton;
template <class T> class CInputField;

class CPrefsSimple : public CPrefsPanel
{

// Construction
public:
	CPrefsSimple(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
// begin JXLayout1

    CTextInputField*             mRealName;
    CTextInputField*             mEmailAddress;
    CTextInputField*             mIMAPuid;
    CTextInputField*             mIMAPip;
    JXRadioGroup*                mServerTypeGroup;
    CTextInputField*             mSMTPip;
    JXSecondaryRadioGroup*       mCheckMailGroup;
    CInputField<JXIntegerInput>* mCheckInterval;
    JXTextCheckbox*              mNewMailAlert;
    JXTextCheckbox*              mSaveUser;
    JXTextCheckbox*              mSavePswd;
    JXTextCheckbox*              mLogonStartup;
    JXTextCheckbox*              mLeaveOnServer;
    JXStaticText*                mDirectorySeparatorTitle;
    CTextInputField*             mDirectorySeparator;
    JXImageButton*               mSignatureBtn;
    JXTextCheckbox*              mDoCopyTo;
    CTextInputField*             mCopyToMailbox;
    CMailboxPopupButton*         mCopyToMailboxPopup;

// end JXLayout1
	cdstring mSignature;

	virtual void Receive(JBroadcaster* sender, const Message& message);

	void SetType(CINETAccount::EINETServerType type);
	void OnSignatureBtn();
	void OnCopyTo();
	void OnChangeCopyTo(JIndex nID);				// Change move to menu
};

#endif
