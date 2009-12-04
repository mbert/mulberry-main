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


// CPrefsMailAccount.cpp : implementation file
//

#include "CPrefsMailAccount.h"

#include "CPrefsAccountAuth.h"
#include "CPrefsAccountExtras.h"
#include "CPrefsMailIMAP.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailAccount dialog


CPrefsMailAccount::CPrefsMailAccount()
	: CPrefsAccountPanel(CPrefsMailAccount::IDD, IDC_PREFS_ACCOUNT_IMAP_TABS)
{
	//{{AFX_DATA_INIT(CPrefsMailAccount)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(CPrefsMailAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsMailAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_IMAP_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailAccount message handlers

void CPrefsMailAccount::InitTabs()
{
	// Create tab panels
	CPrefsAccountAuth* auth = new CPrefsAccountAuth;
	mTabs.AddPanel(auth);
	CPrefsMailIMAP* mail_imap = new CPrefsMailIMAP;
	mTabs.AddPanel(mail_imap);
	CPrefsAccountExtras* extras = new CPrefsAccountExtras;
	mTabs.AddPanel(extras);
}
