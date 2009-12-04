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

#include "CSoundPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSecurity dialog

class CPreferences;

class CPrefsSecurity : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsSecurity)

	enum
	{
		eUsePGP = IDM_PREFERREDPLUGIN_PGP,
		eUseGPG,
		eUseSMIME
	};

// Construction
public:
	CPrefsSecurity();
	~CPrefsSecurity();

// Dialog Data
	//{{AFX_DATA(CPrefsSecurity)
	enum { IDD = IDD_PREFS_SECURITY };
	CPopupButton	mPreferredPlugin;
	BOOL			mUseMIMESecurity;
	BOOL			mEncryptToSelf;
	BOOL			mCachePassphrase;
	BOOL			mAutoVerify;
	BOOL			mAutoDecrypt;
	BOOL			mWarnUnencryptedSend;
	BOOL			mUseErrorAlerts;
	BOOL			mVerifyOKAlert;
	BOOL			mVerifyOKPlaySound;
	CButton			mVerifyOKPlaySoundCtrl;
	CSoundPopup		mVerifyOKSound;
	cdstring		mVerifyOKSoundStr;
	BOOL			mVerifyOKSpeak;
	CButton			mVerifyOKSpeakCtrl;
	cdstring		mVerifyOKSpeakText;
	CEdit			mVerifyOKSpeakTextCtrl;
	//}}AFX_DATA
	UINT			mPreferredPluginValue;

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsSecurity)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsSecurity)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnPreferredPlugin(UINT nID);
	afx_msg void OnVerifyOKPlaySound();
	afx_msg void OnVerifyOKSound(UINT nID);
	afx_msg void OnVerifyOKSpeak();
	afx_msg void OnCertificates();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
