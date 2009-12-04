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


// CPrefsAddressLDAP2.h : header file
//

#ifndef __CPREFSADDRESSLDAP2__MULBERRY__
#define __CPREFSADDRESSLDAP2__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAP2 dialog

class CPrefsAddressLDAP2 : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsAddressLDAP2)

// Construction
public:
	CPrefsAddressLDAP2();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAddressLDAP2)
	enum { IDD = IDD_PREFS_ACCOUNT_LDAP2 };
	cdstring	mLDAPWork;
	cdstring	mLDAPHome;
	cdstring	mLDAPFax;
	cdstring	mLDAPURLs;
	cdstring	mLDAPNotes;
	CEdit	mLDAPNotesCtrl;
	BOOL	mLDAPPut;
	CButton	mLDAPPutBtn;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAddressLDAP2)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsAddressLDAP2)
	afx_msg void OnLDAPPut();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
