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


// CPrefsSecurity.h : header file
//

#ifndef __CPREFSSECURITY__MULBERRY__
#define __CPREFSSECURITY__MULBERRY__

#include "CPrefsPanel.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSecurity dialog

class CTextInputField;
class JXTextButton;
class JXTextCheckbox;

class CPrefsSecurity : public CPrefsPanel
{
// Construction
public:
	CPrefsSecurity(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
	enum
	{
		eUsePGP = 1,
		eUseGPG,
		eUseSMIME
	};

// begin JXLayout1

    HPopupMenu*      mPreferredPlugin;
    JXTextCheckbox*  mUseMIMESecurity;
    JXTextCheckbox*  mEncryptToSelf;
    JXTextCheckbox*  mCachePassphrase;
    JXTextCheckbox*  mAutoVerify;
    JXTextCheckbox*  mAutoDecrypt;
    JXTextCheckbox*  mWarnUnencryptedSend;
    JXTextCheckbox*  mUseErrorAlerts;
    JXTextCheckbox*  mSigAlert;
    JXTextCheckbox*  mSigPlaySound;
    CTextInputField* mSigSound;
    JXTextButton*    mCertificatesBtn;

// end JXLayout1

	virtual void Receive(JBroadcaster* sender, const Message& message);

	void OnCertificates();
};

#endif
