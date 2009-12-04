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


// CEditAddressDialog.h : header file
//

#ifndef __CEDITADDRESSDIALOG__MULBERRY__
#define __CEDITADDRESSDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CEditAddressDialog dialog

class CAdbkAddress;

class CEditAddressDialog : public CHelpDialog
{
// Construction
public:
	CEditAddressDialog(CWnd* pParent = NULL);   // standard constructor

	static	bool PoseDialog(CAdbkAddress* addr, bool allow_edit = true);

protected:
	virtual bool GetFields(CAdbkAddress* addr);
	virtual void SetFields(CAdbkAddress* addr, bool allow_edit = true);

// Dialog Data
	//{{AFX_DATA(CEditAddressDialog)
	enum { IDD = IDD_EDITADDRESS };
	cdstring	mNickName;
	cdstring	mFullName;
	cdstring	mEmail;
	cdstring	mCompany;
	cdstring	mPhoneWork;
	cdstring	mPhoneHome;
	cdstring	mFax;
	cdstring	mAddress;
	cdstring	mURL;
	cdstring	mNotes;
	//}}AFX_DATA
	bool mAllowEdit;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditAddressDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditAddressDialog)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
