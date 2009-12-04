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


// CPrefsLetterGeneral.h : header file
//

#ifndef __CPREFSLETTERGENERAL__MULBERRY__
#define __CPREFSLETTERGENERAL__MULBERRY__

#include "CTabPanel.h"
#include "CMailboxPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLetterGeneral dialog

class CPreferences;

class CPrefsLetterGeneral : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsLetterGeneral)

// Construction
public:
	CPrefsLetterGeneral();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsLetterGeneral)
	enum { IDD = IDD_PREFS_LETTER_GENERAL };
	cdstring		mMailDomain;
	int				spaces_per_tab;
	BOOL			mTabSpace;
	int				wrap_length;
	cdstring		mReplyQuote;
	cdstring		mForwardQuote;
	cdstring		mForwardSubject;
	BOOL			mSeparateBCC;
	int				mSaveDraft;
	cdstring		mSaveToMailboxName;
	CEdit			mSaveToMailboxNameCtrl;
	CMailboxPopup	mSaveToMailboxPopup;
	BOOL			mAutoSaveDrafts;
	int				mAutoSaveDraftsInterval;
	//}}AFX_DATA

	cdstring mBCCCaption;

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsLetterGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPreferences* mCopyPrefs;

	// Generated message map functions
	//{{AFX_MSG(CPrefsLetterGeneral)
	virtual BOOL OnInitDialog();
	afx_msg	void OnReplyCaption();
	afx_msg void OnForwardCaption();
	afx_msg void OnPrintCaption();
	afx_msg void OnSeparateBCC();
	afx_msg void OnBCCCaption();
	afx_msg void OnSaveDraftEnable();
	afx_msg void OnSaveDraftDisable();
	afx_msg void OnChangeSaveTo(UINT nID);				// Change save to menu
	afx_msg void OnAutoSaveDrafts();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
