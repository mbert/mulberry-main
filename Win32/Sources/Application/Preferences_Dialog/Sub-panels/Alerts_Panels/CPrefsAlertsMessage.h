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


// CPrefsAlertsMessage.h : header file
//

#ifndef __CPREFSALERTSMESSAGE__MULBERRY__
#define __CPREFSALERTSMESSAGE__MULBERRY__

#include "CTabPanel.h"

#include "CSoundPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlertsMessage dialog

class CPreferences;

class CPrefsAlertsMessage : public CTabPanel
{
	DECLARE_DYNCREATE(CPrefsAlertsMessage)

// Construction
public:
	CPrefsAlertsMessage();

// Dialog Data
	//{{AFX_DATA(CPrefsAlertsMessage)
	enum { IDD = IDD_PREFS_ALERTS_Message };
	CPopupButton	mAlertStylePopup;
	int				mStyleValue;
	CSoundPopup		mNewMailSound;
	CButton			mNewMailSpeakCtrl;
	CPopupButton	mApplyToCabinet;
	
	CPreferences*	mCopyPrefs;
	
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAlertsMessage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAlertsMessage)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewStyle();
	afx_msg void OnRenameStyle();
	afx_msg void OnDeleteStyle();
	afx_msg void OnStyle(UINT nID);
	afx_msg void OnCheckNever();
	afx_msg void OnCheckOnce();
	afx_msg void OnCheckEvery();
	afx_msg void OnNewMailPlaySound();
	afx_msg void OnNewMailSound(UINT nID);
	afx_msg void OnNewMailSpeak();
	afx_msg void OnCabinet(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	virtual void	InitStylePopup();				// Set up styles menu
	virtual void	InitCabinetPopup();				// Set up cabinet menu

	virtual void	SetNotifaction();				// Set account details
	virtual void	UpdateNotifaction();			// Update current account

};

#endif
