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


// CPrefsLetterOptions.h : header file
//

#ifndef __CPREFSLETTEROPTIONS__MULBERRY__
#define __CPREFSLETTEROPTIONS__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterOptions dialog

class CPreferences;

class CPrefsLetterOptions : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsLetterOptions)

// Construction
public:
	CPrefsLetterOptions();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsLetterOptions)
	enum { IDD = IDD_PREFS_LETTER_OPTIONS };
	BOOL	mAutoInsertSignature;
	BOOL	mSigEmptyLine;
	BOOL	mSigDashes;
	BOOL	mNoSubjectWarn;
	BOOL	mDisplayAttachments;
	BOOL	mAppendDraft;
	BOOL	inbox_append;
	BOOL	mSmartURLPaste;
	BOOL	mComposeWrap;
	BOOL	mDeleteOriginalDraft;
	BOOL	mTemplateDrafts;
	BOOL	mReplyNoSignature;
	BOOL	mWarnReplySubject;
	BOOL	mShowCC;
	BOOL	mShowBCC;
	BOOL	mAlwaysUnicode;
	BOOL	mDisplayIdentityFrom;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsLetterOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPreferences* mCopyPrefs;

	// Generated message map functions
	//{{AFX_MSG(CPrefsLetterOptions)
	afx_msg void OnDeleteOriginalDraft();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
