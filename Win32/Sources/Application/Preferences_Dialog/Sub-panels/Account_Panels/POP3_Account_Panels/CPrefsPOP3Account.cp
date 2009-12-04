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


// CPrefsPOP3Account.cpp : implementation file
//

#include "CPrefsPOP3Account.h"

#include "CPrefsAccountAuth.h"
#include "CPrefsPOP3Options.h"
#include "CPrefsLocalOptions.h"
#include "CPrefsAccountExtras.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsPOP3Account dialog


CPrefsPOP3Account::CPrefsPOP3Account()
	: CPrefsAccountPanel(CPrefsPOP3Account::IDD, IDC_PREFS_ACCOUNT_POP3_TABS)
{
	//{{AFX_DATA_INIT(CPrefsPOP3Account)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


BEGIN_MESSAGE_MAP(CPrefsPOP3Account, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsPOP3Account)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_POP3_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsPOP3Account message handlers

void CPrefsPOP3Account::InitTabs()
{
	// Create tab panels
	CPrefsAccountAuth* auth = new CPrefsAccountAuth;
	mTabs.AddPanel(auth);
	CPrefsPOP3Options* options = new CPrefsPOP3Options;
	mTabs.AddPanel(options);
	CPrefsLocalOptions* local = new CPrefsLocalOptions;
	mTabs.AddPanel(local);
	CPrefsAccountExtras* extras = new CPrefsAccountExtras;
	mTabs.AddPanel(extras);
}
