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

// CSpellChooseDictDialog.h : header file
//

#ifndef __CSPELLCHOOSEDICTSDIALOG__MULBERRY__
#define __CSPELLCHOOSEDICTSDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellChooseDictDialog dialog

class CSpellPlugin;

class CSpellChooseDictDialog : public CHelpDialog
{
// Construction
public:
	CSpellChooseDictDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CSpellPlugin* speller);

// Dialog Data
	//{{AFX_DATA(CSpellChooseDictDialog)
	enum { IDD = IDD_SPELL_CHOOSEDICT };
	//}}AFX_DATA
	CPopupButton	mDictionaries;
	CSpellPlugin*	mSpeller;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellChooseDictDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void	SetOptions(CSpellPlugin* speller);			// Set the speller options
	void	GetOptions(CSpellPlugin* speller);			// Get the speller options
			
	void	InitDictionaries();

	// Generated message map functions
	//{{AFX_MSG(CSpellChooseDictDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeDictionaryPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
