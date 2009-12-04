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


// CForwardActionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CForwardActionDialog dialog

#ifndef __CFORWARDACTIONDIALOG__MULBERRY__
#define __CFORWARDACTIONDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CActionItem.h"
#include "CIdentityPopup.h"

class CForwardActionDialog : public CHelpDialog
{
// Construction
public:
	CForwardActionDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CActionItem::CActionForward& details);

// Dialog Data
	//{{AFX_DATA(CForwardActionDialog)
	enum { IDD = IDD_FORWARDACTION };
	cdstring			mTo;
	cdstring			mCC;
	cdstring			mBcc;
	BOOL			mQuote;
	BOOL			mAttach;
	int				mUseStandard;
	CIdentityPopup	mIdentityPopup;
	BOOL			mCreateDraft;
	//}}AFX_DATA

	cdstring	mText;
	cdstring	mCurrentIdentity;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CForwardActionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	SetDetails(CActionItem::CActionForward& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionForward& details);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CForwardActionDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnText();
	afx_msg void OnStandardIdentity();
	afx_msg void OnUseIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif