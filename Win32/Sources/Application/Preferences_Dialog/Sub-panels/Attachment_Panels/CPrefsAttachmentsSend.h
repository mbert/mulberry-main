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


// CPrefsAttachmentsSend.h : header file
//

#ifndef __CPREFSATTACHMENTSSEND__MULBERRY__
#define __CPREFSATTACHMENTSSEND__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachments dialog

class CPreferences;

class CPrefsAttachmentsSend : public CTabPanel
{
	DECLARE_DYNCREATE(CPrefsAttachmentsSend)

// Construction
public:
	CPrefsAttachmentsSend();
	~CPrefsAttachmentsSend();

// Dialog Data
	//{{AFX_DATA(CPrefsAttachmentsSend)
	enum { IDD = IDD_PREFS_ATTACHMENT_SEND };
	int			mDefault_mode;
	int			mDefault_Always;
	BOOL		mCheckDefaultMailClient;
	BOOL		mWarnMailtoFiles;
	BOOL		mCheckDefaultWebcalClient;
	BOOL		mWarnMissingAttachments;
	BOOL		mMissingAttachmentSubject;
	cdstring	mMissingAttachmentWords;

	CComboBox	mEncodingCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAttachmentsSend)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAttachmentsSend)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
