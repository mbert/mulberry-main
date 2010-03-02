/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CPrefsAddressIMSP.cpp : implementation file
//

#include "CPrefsAddressIMSP.h"

#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressIMSP dialog

IMPLEMENT_DYNAMIC(CPrefsAddressIMSP, CTabPanel)

CPrefsAddressIMSP::CPrefsAddressIMSP()
	: CTabPanel(CPrefsAddressIMSP::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddressIMSP)
	mLogonAtStartup = FALSE;
	mAllowDisconnected = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAddressIMSP::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddressIMSP)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_ADBK_LOGINATSTART, mLogonAtStartup);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_ADBK_ALLOWDISCONNECT, mAllowDisconnected);
	DDX_UTF8Text(pDX, IDC_PREFS_ACCOUNT_ADBK_PATH, mBaseRURL);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddressIMSP, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAddressIMSP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressIMSP message handlers

// Set data
void CPrefsAddressIMSP::SetContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info
	mLogonAtStartup = account->GetLogonAtStart();
	mAllowDisconnected = account->GetDisconnected();

	if (account->GetServerType() == CINETAccount::eCardDAVAdbk)
		mBaseRURL = account->GetBaseRURL();
	else
	{
		//mBaseRURLText->Hide();
		//mBaseRURL->Hide();
	}

	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		GetDlgItem(IDC_PREFS_ACCOUNT_ADBK_ALLOWDISCONNECT)->EnableWindow(false);
}

// Force update of data
bool CPrefsAddressIMSP::UpdateContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup);
	account->SetDisconnected(mAllowDisconnected);
	if (account->GetServerType() == CINETAccount::eCardDAVAdbk)
		account->SetBaseRURL(mBaseRURL);
	
	return true;
}
