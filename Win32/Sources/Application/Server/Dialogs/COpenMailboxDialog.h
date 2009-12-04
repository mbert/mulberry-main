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


// COpenMailboxDialog.h : header file
//

#ifndef __COPENMAILBOXDIALOG__MULBERRY__
#define __COPENMAILBOXDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// COpenMailboxDialog dialog

class CMbox;

class COpenMailboxDialog : public CHelpDialog
{
// Construction
public:
	COpenMailboxDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CMbox*& mbox);

// Dialog Data
	//{{AFX_DATA(COpenMailboxDialog)
	enum { IDD = IDD_OPENMAILBOX };
	CPopupButton	mAccountPopup;
	int				mAccountValue;
	cdstring		mMailboxName;
	//}}AFX_DATA

	virtual CMbox*	GetSelectedMbox(void);					// Set the dialogs return info

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COpenMailboxDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CMbox*			mBrowsed;

	// Generated message map functions
	//{{AFX_MSG(COpenMailboxDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnOpenMailboxAccount(UINT nID);
	afx_msg void OnOpenMailboxBrowse();
	//}}AFX_MSG
	
	void InitAccountMenu(void);

	DECLARE_MESSAGE_MAP()
};

#endif
