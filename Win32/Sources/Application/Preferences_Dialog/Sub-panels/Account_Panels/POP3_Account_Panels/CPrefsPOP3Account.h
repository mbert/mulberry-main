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


// CPrefsPOP3Account.h : header file
//

#ifndef __CPREFSPOP3ACCOUNT__MULBERRY__
#define __CPREFSPOP3ACCOUNT__MULBERRY__

#include "CPrefsAccountPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsPOP3Account dialog

class CPrefsPOP3Account : public CPrefsAccountPanel
{
// Construction
public:
	CPrefsPOP3Account();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsPOP3Account)
	enum { IDD = IDD_PREFS_ACCOUNT_POP3 };
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsPOP3Account)
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsPOP3Account)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	virtual void InitTabs();
};

#endif
