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


// CPrefsAddressLDAP1.h : header file
//

#ifndef __CPREFSADDRESSLDAP1__MULBERRY__
#define __CPREFSADDRESSLDAP1__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAP1 dialog

class CPrefsAddressLDAP1 : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsAddressLDAP1)

// Construction
public:
	CPrefsAddressLDAP1();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAddressLDAP1)
	enum { IDD = IDD_PREFS_ACCOUNT_LDAP1 };
	cdstring	mLDAPRoot;
	cdstring	mLDAPName;
	cdstring	mLDAPEmail;
	cdstring	mLDAPNickName;
	cdstring	mLDAPAddr;
	cdstring	mLDAPCompany;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAddressLDAP1)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsAddressLDAP1)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
