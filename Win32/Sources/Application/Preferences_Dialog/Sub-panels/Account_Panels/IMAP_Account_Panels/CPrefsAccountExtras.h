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


// CPrefsAccountExtras.h : header file
//

#ifndef __CPREFSACCOUNTEXTRAS__MULBERRY__
#define __CPREFSACCOUNTEXTRAS__MULBERRY__

#include "CTabPanel.h"

#include "CIdentityPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountExtras dialog

class CPrefsAccountExtras : public CTabPanel
{
// Construction
public:
	CPrefsAccountExtras();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAccountExtras)
	enum { IDD = IDD_PREFS_ACCOUNT_Mail_Extras };
	BOOL			mTieIdentity;
	CButton			mTieIdentityCtrl;
	CIdentityPopup	mIdentityPopup;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAccountExtras)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAccountExtras)
	virtual BOOL OnInitDialog();
	afx_msg void OnTieIdentity();
	afx_msg void OnIdentityPopup(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
