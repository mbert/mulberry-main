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


// CChangePswdDialog.h : header file
//

#ifndef __CCHANGEPSWDDIALOG__MULBERRY__
#define __CCHANGEPSWDDIALOG__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CChangePswdDialog dialog

class cdstring;
class CINETAccount;

class CChangePswdDialog : public CHelpDialog
{
// Construction
public:
	CChangePswdDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangePswdDialog)
	enum { IDD = IDD_CHANGEPASSWORD };
	int mIconID;
	CStatic		mIcon;
	cdstring	mDescription;
	cdstring	mServerName;
	cdstring	mUserName;
	cdstring	mOldPassword;
	cdstring	mNew1Password;
	cdstring	mNew2Password;
	CStatic		mCapsLock;
	//}}AFX_DATA
	bool		mCaps;
	UINT		mTimerID;

	static bool		PromptPasswordChange(const CINETAccount* acct,
											cdstring& old_pass,
											cdstring& new_pass);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangePswdDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CChangePswdDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
