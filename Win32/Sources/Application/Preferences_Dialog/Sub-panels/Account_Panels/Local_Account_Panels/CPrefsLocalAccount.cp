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


// CPrefsLocalAccount.cpp : implementation file
//

#include "CPrefsLocalAccount.h"

#include "CPrefsLocalOptions.h"
#include "CPrefsAccountExtras.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLocalAccount dialog


CPrefsLocalAccount::CPrefsLocalAccount()
	: CPrefsAccountPanel(CPrefsLocalAccount::IDD, IDC_PREFS_ACCOUNT_Local_TABS)
{
	//{{AFX_DATA_INIT(CPrefsLocalAccount)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPrefsLocalAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsLocalAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_Local_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsLocalAccount message handlers

void CPrefsLocalAccount::InitTabs()
{
	// Create tab panels
	CPrefsLocalOptions* local = new CPrefsLocalOptions;
	mTabs.AddPanel(local);
	CPrefsAccountExtras* extras = new CPrefsAccountExtras;
	mTabs.AddPanel(extras);
}
