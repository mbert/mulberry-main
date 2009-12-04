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


// CCreateAccountDialog.h : header file
//

#ifndef __CCREATEACCOUNTDIALOG__MULBERRY__
#define __CCREATEACCOUNTDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateAccountDialog dialog

class cdstring;

class CCreateAccountDialog : public CHelpDialog
{
// Construction
public:
	CCreateAccountDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateAccountDialog)
	enum { IDD = IDD_NEWACCOUNT };
	CPopupButton mType;
	cdstring mText;
	//}}AFX_DATA
	CBitmap	mMaiboxBMP;
	CBitmap mSMTPBMP;
	CBitmap mRemoteBMP;
	CBitmap mAdbkBMP;
	CBitmap mAddrSearchBMP;
	CBitmap mSIEVEBMP;

	virtual	void	GetDetails(cdstring& name, short& type);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateAccountDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CCreateAccountDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnNewAccountType(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	void InitTypeMenu(void);
};

#endif
