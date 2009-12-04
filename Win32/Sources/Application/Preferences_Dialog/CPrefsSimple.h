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
#include "CIconButton.h"
#include "CINETAccount.h"
#include "CMailboxPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSimple dialog

class CPreferences;

class CPrefsSimple : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsSimple)

// Construction
public:
	CPrefsSimple();
	~CPrefsSimple();

// Dialog Data
	//{{AFX_DATA(CPrefsSimple)
	enum { IDD = IDD_PREFS_SIMPLE };
	cdstring	mRealName;
	cdstring	mEmailAddress;
	CEdit		mEmailAddressCtrl;
	cdstring	mIMAPuid;
	CEdit		mIMAPuidCtrl;
	cdstring	mIMAPip;
	CEdit		mIMAPipCtrl;
	int			mServerType;
	cdstring	mSMTPip;
	CEdit		mSMTPipCtrl;
	BOOL		mSaveUser;
	CButton		mSaveUserCtrl;
	BOOL		mSavePswd;
	CButton		mSavePswdCtrl;
	BOOL		mLogonStartup;
	CButton		mLogonStartupCtrl;
	BOOL		mLeaveOnServer;
	CButton		mLeaveOnServerCtrl;
	int			mDoCheck;
	UINT		mCheckInterval;
	CEdit		mCheckIntervalCtrl;
	BOOL		mNewMailAlert;
	CStatic		mDirectorySeparatorTitle;
	cdstring	mDirectorySeparator;
	CEdit		mDirectorySeparatorCtrl;
	CIconButton	mSignatureBtnCtrl;
	BOOL		mDoCopyTo;
	CButton		mDoCopyToCtrl;
	cdstring	mCopyToMailbox;
	CEdit		mCopyToMailboxCtrl;
	CMailboxPopup	mCopyToMailboxPopup;
	bool		mDoesUIDPswd;

	cdstring	mSignature;
	//}}AFX_DATA

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsSimple)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsSimple)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnServerIMAP();
	afx_msg void OnServerPOP3();
	afx_msg void OnServerLocal();
	afx_msg void OnCheckEvery();
	afx_msg void OnCheckNever();
	afx_msg void OnSignatureBtn();
	afx_msg void OnCopyTo();
	afx_msg void OnChangeCopyTo(UINT nID);				// Change move to menu
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

			void SetType(CINETAccount::EINETServerType type);
};

#endif
