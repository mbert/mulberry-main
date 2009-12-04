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


// CPrefsRemoteAccount.cpp : implementation file
//

#include "CPrefsRemoteAccount.h"

#include "CPrefsAccountAuth.h"
#include "CPrefsRemoteOptions.h"
#include "CPrefsRemoteSets.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteAccount dialog


CPrefsRemoteAccount::CPrefsRemoteAccount()
	: CPrefsAccountPanel(CPrefsRemoteAccount::IDD, IDC_PREFS_ACCOUNT_REMOTE_TABS)
{
	//{{AFX_DATA_INIT(CPrefsRemoteAccount)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPrefsRemoteAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsRemoteAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_REMOTE_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteAccount message handlers

void CPrefsRemoteAccount::InitTabs()
{
	// Create tab panels
	CPrefsAccountAuth* auth = new CPrefsAccountAuth;
	mTabs.AddPanel(auth);
	CPrefsRemoteOptions* remote_options = new CPrefsRemoteOptions;
	mTabs.AddPanel(remote_options);
	CPrefsRemoteSets* remote_sets = new CPrefsRemoteSets;
	mTabs.AddPanel(remote_sets);
}
