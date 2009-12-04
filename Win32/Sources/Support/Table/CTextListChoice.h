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


// CTextListChoice.h : header file
//

#ifndef __CTEXTLISTCHOICE__MULBERRY__
#define __CTEXTLISTCHOICE__MULBERRY__

#include "CHelpDialog.h"
#include "cdstring.h"
#include "templs.h"

/////////////////////////////////////////////////////////////////////////////
// CTextListChoice dialog

class CTextListChoice : public CHelpDialog
{
// Construction
public:
	CTextListChoice(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTextListChoice)
	enum { IDD = IDD_TEXTLISTCHOICE };
	CButton			mActionButton;
	CListBox		mTextListNoSel;
	CListBox		mTextListSingleSel;
	CListBox		mTextListMultiSel;
	CListBox*		mTextList;
	CEdit			mTextListEnterCtrl;
	CStatic			mTextListEnterDescriptionCtrl;

	cdstring		mButtonTitle;
	cdstring		mTextListDescription;
	cdstring		mTextListEnter;
	cdstring		mTextListEnterDescription;
	//}}AFX_DATA
	BOOL			mUseEntry;
	BOOL			mSingleSelection;
	BOOL			mNoSelection;
	BOOL			mSelectFirst;
	cdstrvect		mItems;
	ulvector		mSelection;

	static bool	PoseDialog(const char* title, const char* desc, const char* item, bool use_entry,
						bool single_selection, bool no_selection, bool select_first,
						cdstrvect& items, cdstring& text, ulvector& selection, const char* btn = NULL);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextListChoice)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTextListChoice)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeList();
	afx_msg void OnDblclickList();
	afx_msg void OnChangeEntry();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
