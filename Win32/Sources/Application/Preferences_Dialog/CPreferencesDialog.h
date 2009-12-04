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

#include "CHelpPropertySheet.h"
#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPreferencesDialog dialog

class CPreferences;
class CPreferencesFile;

class CPrefsSimple;
class CPrefsAccounts;
class CPrefsAlerts;
class CPrefsDisplay;
class CPrefsFormatting;
class CPrefsMailbox;
class CPrefsMessage;
class CPrefsLetter;
class CPrefsSecurity;
class CPrefsIdentity;
class CPrefsAddress;
class CPrefsCalendar;
class CPrefsAttachments;
class CPrefsSpelling;

class CPreferencesDialog : public CHelpPropertySheet
{
	DECLARE_DYNAMIC(CPreferencesDialog)

// Construction
public:
	CPreferencesDialog(CWnd* pParent = NULL);   // standard constructor
	~CPreferencesDialog();

	void InitPrefs(void);
	void ResetPrefs(void);
	void UpdatePrefs();

	void ForceUpdate(void)
		{ UpdateCopyPrefs(); }
	CPreferences* GetCopyPrefs(void)
		{ return &mCopyPrefs; }

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

	bool	IsLocal() const
		{ return mLocal; }

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPreferencesDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnPrefsSimpleBtn();
	afx_msg void OnPrefsAdvancedBtn();
	afx_msg void OnPrefsLocalBtn();
	afx_msg void OnPrefsRemoteBtn();
	afx_msg void OnPrefsOpenBtn();
	afx_msg void OnPrefsSaveAsBtn();
	afx_msg void OnPrefsSaveDefaultBtn();
	//}}AFX_MSG

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
	CButton mPrefsSimpleBtn;
	CButton mPrefsAdvancedBtn;
	CButton mStorageArea;
	CStatic mPrefsLocalCaption;
	CStatic mPrefsRemoteCaption;
	CButton mPrefsLocalBtn;
	CButton mPrefsRemoteBtn;
	CButton mPrefsOpenBtn;
	CButton mPrefsSaveAsBtn;
	CButton mPrefsSaveDefaultBtn;

	CPrefsSimple* mPrefsSimple;
	CPrefsAccounts* mPrefsAccounts;
	CPrefsAlerts* mPrefsAlerts;
	CPrefsDisplay* mPrefsDisplay;
	CPrefsFormatting* mPrefsFormatting;
	CPrefsMailbox* mPrefsMailbox;
	CPrefsMessage* mPrefsMessage;
	CPrefsLetter* mPrefsLetter;
	CPrefsSecurity* mPrefsSecurity;
	CPrefsIdentity* mPrefsIdentity;
	CPrefsAddress* mPrefsAddress;
	CPrefsCalendar* mPrefsCalendar;
	CPrefsAttachments* mPrefsAttachments;
	CPrefsSpelling* mPrefsSpelling;

	void	SetWindowTitle(void);									// Set window title from file

	void	MakeChanges(CPreferences* newPrefs);					// Make changes
	void	UpdateCopyPrefs(void);									// Update prefs from panel

	void	UpdateLastPanel();
	void	SetLastPanel();

	void	SetSimple(bool simple, bool init = false);

	bool	PrefsLocalOpen();
	bool	PrefsLocalSaveAs();
	void	PrefsLocalSaveDefault();

	bool	PrefsRemoteOpen();
	bool	PrefsRemoteSaveAs();
	void	PrefsRemoteSaveDefault();

	DECLARE_MESSAGE_MAP()
};

#endif
