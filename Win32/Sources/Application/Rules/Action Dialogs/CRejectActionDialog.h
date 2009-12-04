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


// CRejectActionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRejectActionDialog dialog

#ifndef __CREJECTACTIONDIALOG__MULBERRY__
#define __CREJECTACTIONDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CActionItem.h"
#include "CIdentityPopup.h"

class CRejectActionDialog : public CHelpDialog
{
// Construction
public:
	CRejectActionDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CActionItem::CActionReject& details);

// Dialog Data
	//{{AFX_DATA(CRejectActionDialog)
	enum { IDD = IDD_REJECTACTION };
	int				mReturn;
	int				mUseStandard;
	CIdentityPopup	mIdentityPopup;
	BOOL			mCreateDraft;
	//}}AFX_DATA

	cdstring	mCurrentIdentity;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRejectActionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	SetDetails(CActionItem::CActionReject& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionReject& details);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CRejectActionDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnStandardIdentity();
	afx_msg void OnUseIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif