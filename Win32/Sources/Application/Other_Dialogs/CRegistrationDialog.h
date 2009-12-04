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


// CRegistrationDialog.h : header file
//

#ifndef __CREGISTRATIONDIALOG__MULBERRY__
#define __CREGISTRATIONDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CRegistrationDialog dialog

class CRegistrationDialog : public CHelpDialog
{
// Construction
public:
	static bool DoRegistration(bool initial);
	CRegistrationDialog(bool v3, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRegistrationDialog)
	enum { IDD1 = IDD_REGISTRATION1, IDD2 = IDD_REGISTRATION2  };
	CButton	mOKBtn;
	CEdit	mLicenseeNameCtrl;
	CEdit	mOrganisationCtrl;
	CEdit	mSerialNumberCtrl;
	CEdit	mRegKeyCtrl;
	CEdit	mRegKeyCtrl1;
	CEdit	mRegKeyCtrl2;
	CEdit	mRegKeyCtrl3;
	CEdit	mRegKeyCtrl4;
	cdstring	mLicenseeName;
	cdstring	mOrganisation;
	cdstring	mSerialNumber;
	cdstring	mRegKey;
	cdstring	mRegKey1;
	cdstring	mRegKey2;
	cdstring	mRegKey3;
	cdstring	mRegKey4;
	//}}AFX_DATA
	bool	mAllowDemo;
	bool	mV3;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRegistrationDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRegistrationDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnRunAsDemo();
	afx_msg void OnChangeEntry();
	afx_msg void OnChangeEntry1();
	afx_msg void OnChangeEntry2();
	afx_msg void OnChangeEntry3();
	afx_msg void OnChangeEntry4();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
