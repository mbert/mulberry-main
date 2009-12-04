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


// CCreateAdbkDialog.h : header file
//

#ifndef __CCREATEADBKDIALOG__MULBERRY__
#define __CCREATEADBKDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateAdbkDialog dialog

class CCreateAdbkDialog : public CHelpDialog
{
// Construction
public:
	CCreateAdbkDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CCreateAdbkDialog)
	enum { IDD = IDD_CREATEADBK };

	struct SCreateAdbk
	{
		cdstring	name;
		bool		personal;
		cdstring	account;
		bool		open_on_startup;
		bool		use_nicknames;
		bool		use_search;
	};

	cdstring		mAdbkName;
	cdstring		mAccount;
	CStatic			mAccountCtrl;
	CPopupButton	mAccountPopup;
	int				mAccountValue;
	BOOL			mPersonal;
	BOOL			mOpenOnStartup;
	BOOL			mUseNicknames;
	BOOL			mUseSearch;
	//}}AFX_DATA

	bool			mHasLocal;
	bool			mHasRemote;
	
	static bool PoseDialog(SCreateAdbk* details);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateAdbkDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	SetDetails(SCreateAdbk* create);		// Set the dialogs info
	void	GetDetails(SCreateAdbk* result);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CCreateAdbkDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnAccountPopup(UINT nID);
	//}}AFX_MSG
	
	void InitAccountMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
