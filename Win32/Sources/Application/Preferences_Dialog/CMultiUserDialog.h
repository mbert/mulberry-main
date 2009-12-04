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


// CMultiUserDialog.h : header file
//

#ifndef __CMULTIUSERDIALOG__MULBERRY__
#define __CMULTIUSERDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiUserDialog dialog

class CMultiUserDialog : public CHelpDialog
{
// Construction
public:
	CMultiUserDialog(bool real_name, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CMultiUserDialog)
	enum { IDD1 = IDD_MULTIUSER, IDD2 = IDD_MULTIUSERREALNAME };
	CButton	mOKBtn;
	CStatic	mPasswordCaptionCtrl;
	CEdit	mPasswordCtrl;
	CStatic	mUserIDCaptionCtrl;
	CEdit	mUserIDCtrl;
	CStatic	mSSLStateCaptionCtrl;
	CEdit	mSSLStateCtrl;
	CStatic	mAuthCtrl;
	cdstring	mServerTxt;
	CPopupButton mServerPopup;
	cdstring	mRealName;
	cdstring	mUserID;
	cdstring	mPassword;
	cdstring	mAuth;
	cdstring	mVersion;
	cdstring	mSerial;
	cdstring	mLicensedTo;
	//}}AFX_DATA
	bool		mUseUID;
	bool		mUsePswd;
	long		mServerIndex;
	cdstring	mMailServerName;
	cdstring	mSMTPServerName;
	cdstrvect	mMailServers;
	bool		mUseOther;

			void	UpdatePrefs(void);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMultiUserDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	bool mRealNameUse;

	// Generated message map functions
	//{{AFX_MSG(CMultiUserDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeEntry();
	afx_msg void OnServerPopup(UINT nID);
	afx_msg void OnServerOther();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
			void	InitServerField();
			void	InitServerMenu();

			void	UpdateConfigPrefs(void);
};

#endif
