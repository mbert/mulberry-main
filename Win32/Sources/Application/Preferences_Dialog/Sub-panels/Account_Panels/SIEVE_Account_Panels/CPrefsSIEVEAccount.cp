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


// CPrefsSIEVEAccount.cpp : implementation file
//

#include "CPrefsSIEVEAccount.h"

#include "CPreferences.h"
#include "CPrefsAccountAuth.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSIEVEAccount dialog


CPrefsSIEVEAccount::CPrefsSIEVEAccount()
	: CPrefsAccountPanel(CPrefsSIEVEAccount::IDD, IDC_PREFS_ACCOUNT_SIEVE_TABS)
{
	//{{AFX_DATA_INIT(CPrefsSIEVEAccount)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPrefsSIEVEAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsSIEVEAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_SIEVE_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSIEVEAccount message handlers

void CPrefsSIEVEAccount::InitTabs()
{
	// Create tab panels
	mTabs.AddPanel(new CPrefsAccountAuth);
}
