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


// CBounceActionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CBounceActionDialog dialog

#ifndef __CBOUNCEACTIONDIALOG__MULBERRY__
#define __CBOUNCEACTIONDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CActionItem.h"
#include "CIdentityPopup.h"

class CBounceActionDialog : public CHelpDialog
{
// Construction
public:
	CBounceActionDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CActionItem::CActionBounce& details);

// Dialog Data
	//{{AFX_DATA(CBounceActionDialog)
	enum { IDD = IDD_BOUNCEACTION };
	cdstring		mTo;
	cdstring		mCC;
	cdstring		mBcc;
	int				mUseStandard;
	CIdentityPopup	mIdentityPopup;
	BOOL			mCreateDraft;
	//}}AFX_DATA

	cdstring	mCurrentIdentity;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CBounceActionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	SetDetails(CActionItem::CActionBounce& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionBounce& details);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CBounceActionDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnStandardIdentity();
	afx_msg void OnUseIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
