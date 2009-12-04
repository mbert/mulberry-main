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


// CPrefsAddressExpansion.h : header file
//

#ifndef __CPREFSADDRESSEXPANSION__MULBERRY__
#define __CPREFSADDRESSEXPANSION__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressExpansion dialog

class CPrefsAddressExpansion : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsAddressExpansion)

// Construction
public:
	CPrefsAddressExpansion();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAddressExpansion)
	enum { IDD = IDD_PREFS_ADDRESS_Expansion };
	BOOL	mExpandNoNicknames;
	BOOL	mExpandFailedNicknames;
	BOOL	mExpandFullName;
	BOOL	mExpandNickName;
	BOOL	mExpandEmail;
	BOOL	mSkipLDAP;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAddressExpansion)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsAddressExpansion)
	afx_msg void OnExpandNoNicknames();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
