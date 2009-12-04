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


// Header for CPrefsAlertsMessage class

#ifndef __CPREFSALERTSMESSAGE__MULBERRY__
#define __CPREFSALERTSMESSAGE__MULBERRY__

#include "CPrefsTabSubPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsAlertsMessage = 5080;
const	PaneIDT		paneid_AlertStylePopup = 'STYL';
const	PaneIDT		paneid_CheckNever = 'CHKN';
const	PaneIDT		paneid_CheckOnce = 'ONCE';
const	PaneIDT		paneid_CheckEvery = 'CHKE';
const	PaneIDT		paneid_CheckInterval = 'REFR';
const	PaneIDT		paneid_CheckNew = 'MNEW';
const	PaneIDT		paneid_CheckAll = 'MALL';
const	PaneIDT		paneid_NewMailAlert = 'ALT1';
const	PaneIDT		paneid_NewMailBackAlert = 'BALT';
const	PaneIDT		paneid_NewMailFlashIcon = 'FLSH';
const	PaneIDT		paneid_NewMailPlaySound = 'PLY1';
const	PaneIDT		paneid_NewMailSpeak = 'SPK1';
const	PaneIDT		paneid_NewMailSpeakText = 'SPT1';
const	PaneIDT		paneid_NewMailSound = 'SND1';
const	PaneIDT		paneid_NewMailOpen = 'OPEN';
const	PaneIDT		paneid_ApplyToCabinet = 'CABS';

// Mesages
const	MessageT	msg_AlertStylePopup = 'STYL';
enum
{
	eAlertStylePopup_New = 1,
	eAlertStylePopup_Rename,
	eAlertStylePopup_Delete,
	eAlertStylePopup_Off,
	eAlertStylePopup
};
const	MessageT	msg_CheckNever = 'CHKN';
const	MessageT	msg_CheckOnce = 'ONCE';
const	MessageT	msg_CheckEvery = 'CHKE';
const	MessageT	msg_NewMailPlaySound = 'PLY1';
const	MessageT	msg_NewMailSound = 'SND1';
const	MessageT	msg_NewMailSpeak = 'SPK1';

// Resources
const	ResIDT		RidL_CPrefsAlertsMessageBtns = 5080;

// Classes
class LCheckBox;
class LPopupButton;
class LRadioButton;
class CPreferences;
class CSoundPopup;
class CTextFieldX;

class	CPrefsAlertsMessage : public CPrefsTabSubPanel,
									public LListener
{
private:
	LPopupButton*		mAlertStylePopup;
	int					mStyleValue;
	LRadioButton*		mCheckNever;
	LRadioButton*		mCheckOnce;
	LRadioButton*		mCheckEvery;
	CTextFieldX*		mCheckInterval;
	LRadioButton*		mCheckNew;
	LRadioButton*		mCheckAll;
	LCheckBox*			mNewMailAlert;
	LCheckBox*			mNewMailBackAlert;
	LCheckBox*			mNewMailFlashIcon;
	LCheckBox*			mNewMailPlaySound;
	CSoundPopup*		mNewMailSound;
	LCheckBox*			mNewMailSpeak;
	CTextFieldX*		mNewMailSpeakText;
	LCheckBox*			mNewMailOpen;
	LPopupButton*		mApplyToCabinet;
	
	CPreferences*		mCopyPrefs;

public:
	enum { class_ID = 'Rmsg' };

					CPrefsAlertsMessage();
					CPrefsAlertsMessage(LStream *inStream);
	virtual 		~CPrefsAlertsMessage();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(bool IC_on);				// Toggle display of IC
	virtual void	SetData(void* data);			// Set data
	virtual void	UpdateData(void* data);			// Force update of data

private:
	virtual void	InitStyleMenu(void);			// Set up styles menu
	virtual void	InitCabinetMenu(void);			// Set up cabinet menu
	virtual void	DoNewStyle(void);				// Add new account
	virtual void	DoRenameStyle(void);			// Rename account
	virtual void	DoDeleteStyle(void);			// Delete accoount
	virtual void	SetNotifaction(void);			// Set account details
	virtual void	UpdateNotifaction(void);		// Update current account
};

#endif
