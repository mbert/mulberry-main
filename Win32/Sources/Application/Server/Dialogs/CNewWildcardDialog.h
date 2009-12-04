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


// CNewWildcardDialog.h : header file
//

#ifndef __CNEWWILDCARDDIALOG__MULBERRY__
#define __CNEWWILDCARDDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CNewWildcardDialog dialog

class cdstring;

class CNewWildcardDialog : public CHelpDialog
{
// Construction
public:
	CNewWildcardDialog(CWnd* pParent = NULL);   // standard constructor

	static bool		PoseDialog(cdstring& name);

// Dialog Data
	//{{AFX_DATA(CNewWildcardDialog)
	enum { IDD = IDD_CABINETENTRY };
	CPopupButton	mAccountPopup;
	CPopupButton	mCriteriaPopup;
	cdstring		mText;
	//}}AFX_DATA

	virtual	void	GetDetails(cdstring& name);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CNewWildcardDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CNewWildcardDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewWildcardAccount(UINT nID);
	afx_msg void OnNewWildcardCriteria(UINT nID);
	//}}AFX_MSG
	
	void InitAccountMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
