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

// CSpellOptionsDialog.h : header file
//

#ifndef __CSPELLOPTIONSDIALOG__MULBERRY__
#define __CSPELLOPTIONSDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CColorButton.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellOptionsDialog dialog

class CSpellPlugin;

class CSpellOptionsDialog : public CHelpDialog
{
// Construction
public:
	CSpellOptionsDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CSpellPlugin* speller);

// Dialog Data
	//{{AFX_DATA(CSpellOptionsDialog)
	enum { IDD = IDD_SPELL_OPTIONS };
	BOOL	mIgnoreCapitalised;
	BOOL	mIgnoreAllCaps;
	BOOL	mIgnoreWordsNumbers;
	BOOL	mIgnoreMixedCase;
	BOOL	mIgnoreDomainNames;
	BOOL	mReportDoubledWords;
	BOOL	mCaseSensitive;
	BOOL	mPhoneticSuggestions;
	BOOL	mTypoSuggestions;
	BOOL	mSuggestSplit;
	BOOL	mAutoCorrect;
	BOOL	mAutoPositionDialog;
	BOOL	mSpellOnSend;
	BOOL	mSpellAsYouType;
	BOOL	mSpellColourBackground;
	CColorPickerButton	mBackgroundColourCtrl;
	int		mSuggestFast;
	//}}AFX_DATA
	COLORREF		mBackgroundColour;
	CPopupButton	mDictionaries;
	CSpellPlugin*	mSpeller;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void	SetOptions(CSpellPlugin* speller);			// Set the speller options
	void	GetOptions(CSpellPlugin* speller);			// Get the speller options
			
	void	InitDictionaries();

	// Generated message map functions
	//{{AFX_MSG(CSpellOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDictionaryPopup(UINT nID);
	afx_msg void OnSpellColourBackground();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
