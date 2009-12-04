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


// Source for CPrefsAccountRemoteCalendar class

#include "CPrefsAccountRemoteCalendar.h"

#include "CAdminLock.h"
#include "CCalendarAccount.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountRemoteCalendar dialog

IMPLEMENT_DYNAMIC(CPrefsAccountRemoteCalendar, CTabPanel)

CPrefsAccountRemoteCalendar::CPrefsAccountRemoteCalendar()
	: CTabPanel(CPrefsAccountRemoteCalendar::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAccountRemoteCalendar)
	mLogonAtStartup = FALSE;
	mAllowDisconnected = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAccountRemoteCalendar::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAccountRemoteCalendar)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_REMOTECAL_LOGINATSTART, mLogonAtStartup);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_REMOTECAL_ALLOWDISCONNECT, mAllowDisconnected);
	DDX_UTF8Text(pDX, IDC_PREFS_ACCOUNT_REMOTECAL_PATH, mBaseRURL);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAccountRemoteCalendar, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAccountRemoteCalendar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountRemoteCalendar message handlers

// Set data
void CPrefsAccountRemoteCalendar::SetContent(void* data)
{
	CCalendarAccount* account = (CCalendarAccount*) data;

	// Copy info
	mLogonAtStartup = account->GetLogonAtStart();
	mAllowDisconnected = account->GetDisconnected();
	mBaseRURL = account->GetBaseRURL();
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		GetDlgItem(IDC_PREFS_ACCOUNT_REMOTECAL_ALLOWDISCONNECT)->EnableWindow(false);
}

// Force update of data
bool CPrefsAccountRemoteCalendar::UpdateContent(void* data)
{
	CCalendarAccount* account = (CCalendarAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup);
	account->SetDisconnected(mAllowDisconnected);
	account->SetBaseRURL(mBaseRURL);
	
	return true;
}
