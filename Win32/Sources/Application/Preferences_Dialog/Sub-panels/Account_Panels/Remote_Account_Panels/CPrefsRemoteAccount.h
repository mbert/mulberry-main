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


// CPrefsRemoteAccount.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteAccount dialog

#ifndef __CPREFSREMOTEACCOUNT__MULBERRY__
#define __CPREFSREMOTEACCOUNT__MULBERRY__

#include "CPrefsAccountPanel.h"

class CPrefsRemoteAccount : public CPrefsAccountPanel
{
// Construction
public:
	CPrefsRemoteAccount();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsRemoteAccount)
	enum { IDD = IDD_PREFS_ACCOUNT_Remote };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsRemoteAccount)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsRemoteAccount)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	virtual void InitTabs();
};

#endif
