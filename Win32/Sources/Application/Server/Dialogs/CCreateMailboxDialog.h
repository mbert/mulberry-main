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


// CCreateMailboxDialog.h : header file
//

#ifndef __CCREATEMAILBOXDIALOG__MULBERRY__
#define __CCREATEMAILBOXDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CINETAccount.h"
#include "CPopupButton.h"

#include "cdstring.h"

// Type
struct SCreateMailbox
{
	cdstring	new_name;
	cdstring	parent;
	cdstring	account;
	CINETAccount::EINETServerType	account_type;
	bool		use_wd;
	bool		subscribe;
	bool		directory;
};

/////////////////////////////////////////////////////////////////////////////
// CCreateMailboxDialog dialog

class CCreateMailboxDialog : public CHelpDialog
{
// Construction
public:
	CCreateMailboxDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(SCreateMailbox& details);

// Dialog Data
	//{{AFX_DATA(CCreateMailboxDialog)
	enum { IDD = IDD_CREATEMAILBOX };
	int			mDirectory;
	cdstring	mMailboxName;
	int			mFullPath;
	CButton		mCreateInHierarchyBtn;
	cdstring	mHierarchy;
	cdstring	mAccount;
	CStatic		mAccountCtrl;
	CPopupButton	mAccountPopup;
	int				mAccountValue;
	BOOL		mSubscribe;
	//}}AFX_DATA
	bool	mUseSubscribe;
	
	virtual void	SetDetails(SCreateMailbox* create);		// Set the dialogs info
	virtual void	GetDetails(SCreateMailbox* result);		// Get the dialogs return info

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateMailboxDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	SCreateMailbox* mInfo;

	// Generated message map functions
	//{{AFX_MSG(CCreateMailboxDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnAccountPopup(UINT nID);
	//}}AFX_MSG
	
	void InitAccountMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
