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


// CSearchOptionsDialog.h : header file
//

#ifndef __CSEARCHOPTIONSDIALOG__MULBERRY__
#define __CSEARCHOPTIONSDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CSearchOptionsDialog dialog

class CSearchOptionsDialog : public CHelpDialog
{
// Construction
public:
	CSearchOptionsDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSearchOptionsDialog)
	enum { IDD = IDD_SEARCHOPTIONS };
	BOOL	mMultiple;
	CButton	mMultipleCtrl;
	int		mMaximum;
	CEdit	mMaximumCtrl;
	BOOL	mLoadBalance;
	CButton	mLoadBalanceCtrl;
	BOOL	mOpenFirst;
	//}}AFX_DATA

	void SetItems();
	void GetItems();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSearchOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	void UpdateItems();


// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSearchOptionsDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnMultiple();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
