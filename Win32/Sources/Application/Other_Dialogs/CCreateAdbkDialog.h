/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CCmdEdit.h"
#include "CPopupButton.h"

#include "cdstring.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateAdbkDialog dialog

class CCreateAdbkDialog : public CHelpDialog
{
// Construction
public:
	struct SCreateAdbk
	{
		cdstring	name;
		cdstring	parent;
		cdstring	account;
		bool		directory;
		bool		use_wd;
		bool		open_on_startup;
		bool		use_nicknames;
		bool		use_search;
	};

	static bool PoseDialog(SCreateAdbk* details);

	CCreateAdbkDialog(CWnd* pParent = NULL);   // standard constructor

protected:
// Dialog Data
	//{{AFX_DATA(CCreateAdbkDialog)
	enum { IDD = IDD_CREATEADBK };
	CButton			mAddressBook;
	CButton			mDirectory;
	CCmdEdit		mAdbkName;
	CButton			mOpenOnStartup;
	CButton			mNicknames;
	CButton			mSearch;
	CButton			mFullPath;
	CButton			mUseDirectory;
	CStatic			mHierarchy;
	CStatic			mAccount;
	CPopupButton	mAccountPopup;
	//}}AFX_DATA

	SCreateAdbk*	mData;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCreateAdbkDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
	void	InitControls();
	void 	InitAccountMenu();

	void	SetDetails(SCreateAdbk* create);		// Set the dialogs info
	void	GetDetails(SCreateAdbk* result);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CCreateAdbkDialog)
	afx_msg void OnAddressBook();
	afx_msg void OnDirectory();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

#endif
