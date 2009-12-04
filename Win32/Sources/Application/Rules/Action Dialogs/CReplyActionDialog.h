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


// CReplyActionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CReplyActionDialog dialog

#ifndef __CREPLYACTIONDIALOG__MULBERRY__
#define __CREPLYACTIONDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CActionItem.h"
#include "CIdentityPopup.h"

class CReplyActionDialog : public CHelpDialog
{
// Construction
public:
	CReplyActionDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CActionItem::CActionReply& details);

// Dialog Data
	//{{AFX_DATA(CReplyActionDialog)
	enum { IDD = IDD_REPLYACTION };
	int				mReplyTo;
	BOOL			mQuote;
	int				mUseStandard;
	CIdentityPopup	mIdentityPopup;
	BOOL			mCreateDraft;
	//}}AFX_DATA

	cdstring	mText;
	cdstring	mCurrentIdentity;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CReplyActionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	SetDetails(CActionItem::CActionReply& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionReply& details);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CReplyActionDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnText();
	afx_msg void OnStandardIdentity();
	afx_msg void OnUseIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif