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


// Header for CEditIdentityDSN class

#ifndef __CEDITIDENTITYDSN__MULBERRY__
#define __CEDITIDENTITYDSN__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CEditIdentityDSN dialog

class CEditIdentityDSN : public CTabPanel
{
	DECLARE_DYNAMIC(CEditIdentityDSN)

// Construction
public:
	CEditIdentityDSN();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CEditIdentityDSN)
	enum { IDD = IDD_IDENTITY_OUTGOING_DSN };
	BOOL	mActive;
	BOOL	mUseDSN;
	BOOL	mSuccess;
	BOOL	mFailure;
	BOOL	mDelay;
	int		mFull;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CEditIdentityDSN)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CEditIdentityDSN)
	virtual BOOL OnInitDialog();
	afx_msg void OnIdentityDSNActive();
	//}}AFX_MSG
	
	void EnablePanel(bool enable);
	
	DECLARE_MESSAGE_MAP()
};

#endif
