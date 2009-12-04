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


// CPreferencesDialog.h : header file
//

#ifndef __CPREFERENCESDIALOG__MULBERRY__
#define __CPREFERENCESDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog

class CPreferencesFile;
class CPrefsPanel;

class JXCardFile;
class JXIconTextButton;
class JXRadioGroup;
class JXSecondaryRadioGroup;
class JXStaticText;
class JXTextButton;
class JXTextRadioButton;

class CPreferencesDialog : public CDialogDirector
{
// Construction
public:
	static CPreferencesDialog* sPrefsDlog;

	CPreferencesDialog(JXDirector* supervisor);

	void ResetPrefs();
	void UpdatePrefs();

	void ForceUpdate()
		{ UpdateCopyPrefs(); }
	CPreferences* GetCopyPrefs()
		{ return &mCopyPrefs; }

	void	SetLoadedNewPrefs(bool loaded)
		{ mLoadedNewPrefs = loaded; }
	bool	GetLoadedNewPrefs() const
		{ return mLoadedNewPrefs; }

	void	SetAccountNew(bool acct_new)
		{ mAccountNew = acct_new; }
	bool	GetAccountNew() const
		{ return mAccountNew; }

	void	SetAccountRename(bool renamed)
		{ mAccountRename = renamed; }
	bool	GetAccountRename() const
		{ return mAccountRename; }
	cdstrpairvect& GetRenames()
		{ return mRenames; }

	void	SetForceWindowReset(bool force)
		{ mWindowsReset = force; }
	void	SetForceMenuListReset(bool force)
		{ mForceMenuListReset = force; }

	bool	IsLocal() const
		{ return mLocal; }

// Implementation
protected:

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual JBoolean OKToDeactivate();

	void AddPanel(CPrefsPanel* panel);

	bool OnOK();
	void OnCancel();

	void OnPrefsLocalBtn();
	void OnPrefsRemoteBtn();
	void OnPrefsOpenBtn();
	void OnPrefsSaveAsBtn();
	void OnPrefsSaveDefaultBtn();

	void	SetPrefsPanel(JIndex panel);						// Set input panel
	void	SetWindowTitle();									// Set window title from file
	void	MakeChanges(CPreferences* newPrefs);				// Make changes
	void	UpdateCopyPrefs();									// Update prefs from panel

	void	UpdateLastPanel();
	void	SetLastPanel();

	void	SetSimple(bool simple, bool init = false);

	bool	PrefsLocalOpen();
	bool	PrefsLocalSaveAs();
	void	PrefsLocalSaveDefault();

	bool	PrefsRemoteOpen();
	bool	PrefsRemoteSaveAs();
	void	PrefsRemoteSaveDefault();

private:
	CPreferences 		mCopyPrefs;
	CPreferencesFile*	mCurrentPrefsFile;									// Current prefs file created during dialog
	bool				mLoadedNewPrefs;										// Completely new set of prefs loaded
	bool				mWindowsReset;
	bool				mAccountNew;										// Mailbox account new
	bool				mAccountRename;										// Mailbox account rename
	cdstrpairvect		mRenames;											// List of account renames
	bool				mLocal;
	bool				mForceMenuListReset;								// Must force reset of menu lists on exit (OK or Cancel)

// begin JXLayout

    JXRadioGroup*          mPrefsButtonsGroup;
    JXIconTextButton*      mAccountsBtn;
    JXIconTextButton*      mAlertsBtn;
    JXIconTextButton*      mDisplayBtn;
    JXIconTextButton*      mFontsBtn;
    JXIconTextButton*      mMailboxBtn;
    JXIconTextButton*      mMessageBtn;
    JXIconTextButton*      mOutgoingBtn;
    JXIconTextButton*      mSecurityBtn;
    JXIconTextButton*      mIdentitiesBtn;
    JXIconTextButton*      mAddressesBtn;
    JXIconTextButton*      mCalendarBtn;
    JXIconTextButton*      mAttachmentBtn;
    JXIconTextButton*      mSpellingBtn;
    JXCardFile*            mCards;
    JXSecondaryRadioGroup* mPrefsGroup;
    JXTextRadioButton*     mPrefsSimpleBtn;
    JXTextRadioButton*     mPrefsAdvancedBtn;
    JXSecondaryRadioGroup* mStorageGroup;
    JXStaticText*          mPrefsLocalCaption;
    JXStaticText*          mPrefsRemoteCaption;
    JXTextRadioButton*     mPrefsLocalBtn;
    JXTextRadioButton*     mPrefsRemoteBtn;
    JXTextButton*          mPrefsOpenBtn;
    JXTextButton*          mPrefsSaveAsBtn;
    JXTextButton*          mPrefsSaveDefaultBtn;
    JXTextButton*          mOkBtn;
    JXTextButton*          mCancelBtn;

// end JXLayout
	JIndex				mCurrentCard;
	cdstrvect			mPanelNames;

};

#endif
