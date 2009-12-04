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


// CNewSearchDialog.h : header file
//

#ifndef __CNEWSEARCHDIALOG__MULBERRY__
#define __CNEWSEARCHDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CNewSearchDialog dialog

class cdstring;

class CNewSearchDialog : public CHelpDialog
{
// Construction
public:
	CNewSearchDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(bool single, unsigned long& index, cdstring& wd);

// Dialog Data
	//{{AFX_DATA(CNewSearchDialog)
	enum { IDD = IDD_NEWSEARCHHIER };
	CPopupButton	mAccountPopup;
	CPopupButton	mCriteriaPopup;
	CEdit			mTextCtrl;
	cdstring		mText;
	int				mFullHierarchy;
	CButton			mFullHierarchyCtrl;
	CButton			mTopHierarchyCtrl;
	//}}AFX_DATA
	long mStartIndex;

	virtual	void	SetDetails(bool single, long index);
	virtual	void	GetDetails(cdstring& name, unsigned long& index);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewSearchDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mSingle;

	// Generated message map functions
	//{{AFX_MSG(CNewSearchDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewSearchAccount(UINT nID);
	afx_msg void OnNewSearchCriteria(UINT nID);
	//}}AFX_MSG
	
	void InitAccountMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
