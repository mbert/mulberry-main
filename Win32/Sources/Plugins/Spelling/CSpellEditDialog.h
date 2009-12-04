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

// CSpellEditDialog.h : header file
//

#ifndef __CSPELLEDITDIALOG__MULBERRY__
#define __CSPELLEDITDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CDictionaryPageList.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellEditDialog dialog

class	CSpellPlugin;

class CSpellEditDialog : public CHelpDialog
{
// Construction
public:
	CSpellEditDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CSpellPlugin* speller);

// Dialog Data
	//{{AFX_DATA(CSpellEditDialog)
	enum { IDD = IDD_SPELL_EDIT };
	CStatic		mPageCount;
	CStatic		mWordBottom;
	CStatic		mWordTop;
	CEdit		mWord;
	CButton		mRemove;
	CButton		mFind;
	CButton		mAdd;
	CDictionaryPageList	mList;
	CStatic		mDictName;
	//}}AFX_DATA

	CSpellPlugin*		mSpeller;

			void	SetSpeller(CSpellPlugin* speller)			// Set the speller
				{ mSpeller = speller; }

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellEditDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellEditDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnSpellEditAdd();
	afx_msg void OnSpellEditFind();
	afx_msg void OnSpellEditRemove();
	afx_msg void OnSelcancelSpellEditList();
	afx_msg void OnSelchangeSpellEditList();
	afx_msg void OnChangeSpellEditWord();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
