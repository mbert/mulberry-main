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


// Header for CPreferencesDialog class

#ifndef __CPREFERENCESDIALOG__MULBERRY__
#define __CPREFERENCESDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CPreferences.h"

// Constants

// Panes
const	ClassIDT	class_PreferencesDialog ='Pref';
const	PaneIDT		paneid_PreferencesDialog = 5000;
const	PaneIDT		paneid_PrefsPanel = 'AREA';
const	PaneIDT		paneid_PrefsButtons = 'BTNS';
const	PaneIDT		paneid_PrefsAccountBtn = 'BTN1';
const	PaneIDT		paneid_PrefsAlertsBtn = 'BTN2';
const	PaneIDT		paneid_PrefsDisplayBtn = 'BTN3';
const 	PaneIDT		paneid_PrefsStyledBtn = 'BTN4';
const	PaneIDT		paneid_PrefsMailboxBtn = 'BTN5';
const	PaneIDT		paneid_PrefsMessageBtn = 'BTN6';
const	PaneIDT		paneid_PrefsLetterBtn = 'BTN7';
const 	PaneIDT		paneid_PrefsSecurityBtn = 'BTN8';
const	PaneIDT		paneid_PrefsIdentitiesBtn = 'BTN9';
const	PaneIDT		paneid_PrefsAddressBtn = 'BTNA';
const	PaneIDT		paneid_PrefsCalendarBtn = 'BTNB';
const	PaneIDT		paneid_PrefsAttachmentsBtn = 'BTNC';
const	PaneIDT		paneid_PrefsSpellingBtn = 'BTND';
const	PaneIDT		paneid_PrefsSpeechBtn = 'BTNE';
const	PaneIDT		paneid_PrefsSimpleBtn = 'SIMP';
const	PaneIDT		paneid_PrefsAdvancedBtn = 'ADVN';
const	PaneIDT		paneid_PrefsStorage = 'STOR';
const	PaneIDT		paneid_PrefsLocalCaption = 'LCAP';
const	PaneIDT		paneid_PrefsRemoteCaption = 'RCAP';
const	PaneIDT		paneid_PrefsLocalOnBtn = 'LOCL';
const	PaneIDT		paneid_PrefsRemoteOnBtn = 'REMT';
const	PaneIDT		paneid_PrefsOpenBtn = 'OPEN';
const	PaneIDT		paneid_PrefsSaveAsBtn = 'SAVA';
const	PaneIDT		paneid_PrefsSaveDefaultBtn = 'DEFA';
const	PaneIDT		paneid_PrefsCancelBtn = 'CANC';
const	PaneIDT		paneid_PrefsOKBtn = 'OKBT';

// Resources
const	ResIDT		RidL_CPrefsDialogBtns = 5000;

const	ResIDT		STRx_PrefsHelp = 5000;

// Mesages
const	MessageT	msg_SetAccount = 5001;
const	MessageT	msg_SetAlerts = 5002;
const	MessageT	msg_SetStyled = 5003;
const	MessageT	msg_SetDisplay = 5004;
const	MessageT	msg_SetMailbox = 5005;
const	MessageT	msg_SetMessage = 5006;
const	MessageT	msg_SetLetter = 5007;
const	MessageT	msg_SetSecurity = 5008;
const	MessageT	msg_SetIdentities = 5009;
const	MessageT	msg_SetAddress = 5010;
const	MessageT	msg_SetCalendar = 5011;
const	MessageT	msg_SetAttachments = 5012;
const	MessageT	msg_SetSpelling = 5013;
const	MessageT	msg_SetSpeech = 5014;

const	MessageT	msg_SimplePrefs = 'SIMP';
const	MessageT	msg_AdvancedPrefs = 'ADVN';
const	MessageT	msg_LocalPrefs = 'LOCL';
const	MessageT	msg_RemotePrefs = 'REMT';
const	MessageT	msg_OpenPrefs = 5050;
const	MessageT	msg_SavePrefsAs = 5051;
const	MessageT	msg_ShowHelpState = 5052;
const	MessageT	msg_SaveDefaultPrefs = 5055;

// Classes
class CMessage;
class CPreferencesFile;
class CStaticText;
class LCheckBox;
class LPushButton;
class LRadioButton;

class	CPreferencesDialog : public LDialogBox
{
private:
	CPreferences		mCopyPrefs;											// Modifiable copy of prefs
	CPreferencesFile*	mCurrentPrefsFile;									// Current prefs file created during dialog
	LView*				mPrefsPanel;										// The panel
	LView*				mCurrentPanel;										// Current panel view
	short				mCurrentPanelNum;									// Current prefs panel index
	LView*				mButtons;
	LView*				mStorage;
	CStaticText*		mLocalCaption;
	CStaticText*		mRemoteCaption;
	LRadioButton*		mSimpleBtn;											// Simple button
	LRadioButton*		mAdvancedBtn;										// Advanced button
	LRadioButton*		mLocalBtn;											// Save default button
	LRadioButton*		mRemoteBtn;											// Save default button
	LPushButton*		mSaveDefaultBtn;									// Save default button
	
	bool				mLoadedNewPrefs;									// New prefs file loaded
	bool				mWindowsReset;										// Windows were reset
	bool				mAccountNew;										// Mailbox account new
	bool				mAccountRename;										// Mailbox account rename
	cdstrpairvect		mRenames;											// List of account renames
	bool				mForceMenuListReset;								// Must force reset of menu lists on exit (OK or Cancel)

public:
	enum { class_ID = class_PreferencesDialog };

					CPreferencesDialog();
					CPreferencesDialog(LStream *inStream);
	virtual 		~CPreferencesDialog();

protected:
	virtual void	FinishCreateSelf(void);									// Do odds & ends

private:
	void	SetPrefsPanel(short panel);								// Set input panel
	void	SetWindowTitle(void);									// Set window title from file

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	CPreferences* GetNewPrefs(void) { return &mCopyPrefs; }
	bool	DoOK(void);												// Handle success OK
	void	DoCancel(void);											// Handle cancel
	void	UpdatePrefs();

	void	UpdateLastPanel();
	void	SetLastPanel();

	void	SetLoadedNewPrefs(bool loaded)
		{ mLoadedNewPrefs = loaded; }
	bool	GetLoadedNewPrefs() const
		{ return mLoadedNewPrefs; }

	void	SetAccountNew(bool acct_new)
		{ mAccountNew = acct_new; }
	bool	GetAccountNew(void) const
		{ return mAccountNew; }

	void	SetAccountRename(bool renamed)
		{ mAccountRename = renamed; }
	bool	GetAccountRename(void) const
		{ return mAccountRename; }
	cdstrpairvect& GetRenames(void)
		{ return mRenames; }

	void	SetForceWindowReset(bool force)
		{ mWindowsReset = force; }
	void	SetForceMenuListReset(bool force)
		{ mForceMenuListReset = force; }

	bool	IsLocal() const;

private:
	
	void	InitPreferencesDialog(void);							// Standard init

	void	MakeChanges(CPreferences* newPrefs);					// Make changes

	void	SetSimple(bool simple);									// Set to simple state

	bool	PrefsLocalOpenFile(void);								// Do open file
	bool	PrefsLocalSaveAs(void);									// Do save as on file
	void	PrefsLocalSaveDefault(void);							// Do save default file

	bool	PrefsRemoteOpenFile(void);								// Do open file
	bool	PrefsRemoteSaveAs(void);								// Do save as on file
	void	PrefsRemoteSaveDefault(void);							// Do save default file

};

#endif
