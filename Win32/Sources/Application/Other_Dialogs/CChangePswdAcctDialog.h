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


// CChangePswdAcctDialog.h : header file
//

#ifndef __CCHANGEPSWDACCTDIALOG__MULBERRY__
#define __CCHANGEPSWDACCTDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CChangePswdAcctDialog dialog

class CINETAccount;
class CPswdChangePlugin;

class CChangePswdAcctDialog : public CHelpDialog
{
// Construction
public:
	CChangePswdAcctDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CChangePswdAcctDialog)
	enum { IDD = IDD_CHANGEPSWDACCT };
	CPopupButton	mAccountPopup;
	CPopupButton	mPluginPopup;
	//}}AFX_DATA
	CBitmap	mMaiboxBMP;
	CBitmap mSMTPBMP;
	CBitmap mRemoteBMP;
	CBitmap mAdbkBMP;
	CBitmap mAddrSearchBMP;
	cdstring mPlugin;

	void	GetDetails(CINETAccount*& acct, CPswdChangePlugin*& plugin); 					// Get details from dialog

	static void AcctPasswordChange();
	static bool PromptAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin);
	static bool DoAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChangePswdAcctDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	void InitAccountMenu();
	void InitPluginsMenu();

	// Generated message map functions
	//{{AFX_MSG(CChangePswdAcctDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnChangePswdAcctAcctPopup(UINT nID);
	afx_msg void OnChanegPswdAcctPluginPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif