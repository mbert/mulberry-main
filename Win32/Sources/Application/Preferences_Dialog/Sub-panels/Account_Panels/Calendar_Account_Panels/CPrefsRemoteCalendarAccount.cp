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


// Source for CPrefsRemoteCalendarAccount class

#include "CPrefsRemoteCalendarAccount.h"

#include "CPreferences.h"
#include "CPrefsAccountAuth.h"
#include "CPrefsAccountExtras.h"
#include "CPrefsAccountRemoteCalendar.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSIEVEAccount dialog


CPrefsRemoteCalendarAccount::CPrefsRemoteCalendarAccount()
	: CPrefsAccountPanel(CPrefsRemoteCalendarAccount::IDD, IDC_PREFS_ACCOUNT_REMOTECAL_TABS)
{
	//{{AFX_DATA_INIT(CPrefsRemoteCalendarAccount)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPrefsRemoteCalendarAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsRemoteCalendarAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_REMOTECAL_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteCalendarAccount message handlers

void CPrefsRemoteCalendarAccount::InitTabs()
{
	// Create tab panels
	mTabs.AddPanel(new CPrefsAccountAuth);
	mTabs.AddPanel(new CPrefsAccountRemoteCalendar);
	mTabs.AddPanel(new CPrefsAccountExtras);
}
