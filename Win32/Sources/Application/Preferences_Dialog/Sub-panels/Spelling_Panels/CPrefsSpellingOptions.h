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


// CPrefsSpellingOptions.h : header file
//

#ifndef __CPrefsSpellingOptions__MULBERRY__
#define __CPrefsSpellingOptions__MULBERRY__

#include "CTabPanel.h"

#include "CColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpellingOptions dialog

class CPrefsSpellingOptions : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsSpellingOptions)

// Construction
public:
	CPrefsSpellingOptions();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsSpellingOptions)
	enum { IDD = IDD_PREFS_SPELLING_OPTIONS };
	BOOL	mCaseSensitive;
	BOOL	mIgnoreCapitalised;
	BOOL	mIgnoreAllCaps;
	BOOL	mIgnoreWordsNumbers;
	BOOL	mIgnoreMixedCase;
	BOOL	mIgnoreDomainNames;
	BOOL	mReportDoubledWords;
	BOOL	mAutoCorrect;
	BOOL	mAutoPositionDialog;
	BOOL	mSpellOnSend;
	BOOL	mSpellAsYouType;
	BOOL	mSpellColourBackground;
	CColorPickerButton	mBackgroundColourCtrl;
	//}}AFX_DATA
	COLORREF		mBackgroundColour;

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsSpellingOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsSpellingOptions)
	virtual BOOL OnInitDialog();
	afx_msg void OnSpellColourBackground();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
