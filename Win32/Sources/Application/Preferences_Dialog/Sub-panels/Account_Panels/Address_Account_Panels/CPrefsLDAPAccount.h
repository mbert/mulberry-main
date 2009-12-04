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


// CPrefsLDAPAccount.h : header file
//

#ifndef __CPREFSLDAPACCOUNT__MULBERRY__
#define __CPREFSLDAPACCOUNT__MULBERRY__

#include "CPrefsAccountPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLDAPAccount dialog

class CPrefsLDAPAccount : public CPrefsAccountPanel
{
// Construction
public:
	CPrefsLDAPAccount();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsLDAPAccount)
	enum { IDD = IDD_PREFS_ACCOUNT_LDAP };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsLDAPAccount)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsLDAPAccount)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	virtual void InitTabs();
};

#endif
