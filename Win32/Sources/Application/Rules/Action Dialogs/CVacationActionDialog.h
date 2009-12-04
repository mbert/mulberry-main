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


// CVacationActionDialog.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CVacationActionDialog dialog

#ifndef __CVACATIONACTIONDIALOG__MULBERRY__
#define __CVACATIONACTIONDIALOG__MULBERRY__

#include "CActionItem.h"

class CVacationActionDialog : public CDialog
{
// Construction
public:
	CVacationActionDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CActionItem::CActionVacation& details);

// Dialog Data
	//{{AFX_DATA(CVacationActionDialog)
	enum { IDD = IDD_VACATIONACTION };
	int				mDays;
	cdstring		mSubject;
	cdstring		mAddresses;
	CEdit			mAddressList;
	//}}AFX_DATA

	cdstring			mText;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CVacationActionDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void	SetDetails(CActionItem::CActionVacation& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionVacation& details);		// Get the dialogs return info

	// Generated message map functions
	//{{AFX_MSG(CVacationActionDialog)
	afx_msg void OnText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
