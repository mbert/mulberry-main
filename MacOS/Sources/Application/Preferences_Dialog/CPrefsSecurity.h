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


// Header for CPrefsSecurity class

#ifndef __CPREFSSECURITY__MULBERRY__
#define __CPREFSSECURITY__MULBERRY__

#include "CPrefsPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsSecurity = 5013;
const	PaneIDT		paneid_PreferredPlugin = 'PLUG';
const	PaneIDT		paneid_UseMIMESecurity = 'MIME';
const	PaneIDT		paneid_EncryptToSelf = 'SELF';
const	PaneIDT		paneid_CachePassphrase = 'CACH';
const	PaneIDT		paneid_AutoVerify = 'AVER';
const	PaneIDT		paneid_AutoDecrypt = 'ADEC';
const	PaneIDT		paneid_WarnUnencryptedSend = 'USND';
const	PaneIDT		paneid_UseErrorAlerts = 'NOTI';
const	PaneIDT		paneid_VerifyOKAlert = 'ALRT';
const	PaneIDT		paneid_VerifyOKPlaySound = 'PLAY';
const	PaneIDT		paneid_VerifyOKSound = 'SOND';
const	PaneIDT		paneid_VerifyOKSpeak = 'SPEK';
const	PaneIDT		paneid_VerifyOKSpeakText = 'SPKT';

// Resources
const	ResIDT		RidL_CPrefsSecurityBtns = 5013;

// Mesages
const	MessageT	msg_VerifyOKPlaySound = 'PLAY';
const	MessageT	msg_VerifyOKSound = 'SOND';
const	MessageT	msg_VerifyOKSpeak = 'SPEK';
const	MessageT	msg_SecurityCertificates = 'CERT';

// Classes
class LCheckBox;
class LPopupButton;
class CSoundPopup;
class CTextFieldX;

class	CPrefsSecurity : public CPrefsPanel,
									public LListener
{
private:
	enum
	{
		eUsePGP = 1,
		eUseGPG,
		eUseSMIME
	};

	LPopupButton*		mPreferredPlugin;
	LCheckBox*			mUseMIMESecurity;
	LCheckBox*			mEncryptToSelf;
	LCheckBox*			mCachePassphrase;
	LCheckBox*			mAutoVerify;
	LCheckBox*			mAutoDecrypt;
	LCheckBox*			mWarnUnencryptedSend;
	LCheckBox*			mUseErrorAlerts;
	LCheckBox*			mVerifyOKAlert;
	LCheckBox*			mVerifyOKPlaySound;
	CSoundPopup*		mVerifyOKSound;
	LCheckBox*			mVerifyOKSpeak;
	CTextFieldX*		mVerifyOKSpeakText;

public:
	enum { class_ID = 'Psec' };

					CPrefsSecurity();
					CPrefsSecurity(LStream *inStream);
	virtual 		~CPrefsSecurity();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs
};

#endif
