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


// CPrefsAlertsAttachment.h : header file
//

#ifndef __CPREFSALERTSATTACHMENT__MULBERRY__
#define __CPREFSALERTSATTACHMENT__MULBERRY__

#include "CTabPanel.h"

#include "CSoundPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlertsAttachment dialog

class CPrefsAlertsAttachment : public CTabPanel
{
	DECLARE_DYNCREATE(CPrefsAlertsAttachment)

// Construction
public:
	CPrefsAlertsAttachment();

// Dialog Data
	//{{AFX_DATA(CPrefsAlertsAttachment)
	enum { IDD = IDD_PREFS_ALERTS_Attachment };
	BOOL			mAttachmentAlert;
	BOOL			mAttachmentPlaySound;
	CButton			mAttachmentPlaySoundCtrl;
	CSoundPopup		mAttachmentSound;
	cdstring		mAttachmentSoundStr;
	BOOL			mAttachmentSpeak;
	CButton			mAttachmentSpeakCtrl;
	cdstring		mAttachmentSpeakText;
	CEdit			mAttachmentSpeakTextCtrl;
	
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAlertsAttachment)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAlertsAttachment)
	virtual BOOL OnInitDialog();
	afx_msg void OnAttachmentPlaySound();
	afx_msg void OnAttachmentSound(UINT nID);
	afx_msg void OnAttachmentSpeak();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
