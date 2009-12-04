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

// CSpellAddDialog.h : header file
//

#ifndef __CSPELLADDDIALOG__MULBERRY__
#define __CSPELLADDDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellAddDialog dialog

class CSpellPlugin;

class CSpellAddDialog : public CHelpDialog
{
// Construction
public:
	CSpellAddDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CSpellPlugin* speller, const char* add_word);

// Dialog Data
	//{{AFX_DATA(CSpellAddDialog)
	enum { IDD = IDD_SPELL_ADD };
	cdstring	mSuggestion[19];
	CEdit		mSuggestionCtrl[19];
	BOOL		mCheck[19];
	CButton		mCheckCtrl[19];
	BOOL		mCapitalise;
	CButton		mCapitaliseCtrl;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellAddDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	SetDetails(CSpellPlugin* speller, const char* word);			// Set the speller
			void	DoAdd(CSpellPlugin* speller);									// Do dictionary add

	// Generated message map functions
	//{{AFX_MSG(CSpellAddDialog)
	afx_msg void OnSpellAddChk(UINT nID);
	afx_msg void OnSpellAddCaps();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
