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


// CPrefsMailIMAP.cpp : implementation file
//

#include "CPrefsMailIMAP.h"

#include "CAdminLock.h"
#include "CMailAccount.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailIMAP dialog

IMPLEMENT_DYNAMIC(CPrefsMailIMAP, CTabPanel)

CPrefsMailIMAP::CPrefsMailIMAP()
	: CTabPanel(CPrefsMailIMAP::IDD)
{
	//{{AFX_DATA_INIT(CPrefsMailIMAP)
	mLogonAtStartup = FALSE;
	mAutoNamespace = FALSE;
	mAllowDisconnected = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsMailIMAP::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsMailIMAP)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_IMAP_LOGINATSTART, mLogonAtStartup);
	DDX_UTF8Text(pDX, IDC_PREFS_ACCOUNT_IMAP_DIRSEPARATOR, mDirDelim);
	DDV_UTF8MaxChars(pDX, mDirDelim, 1);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_IMAP_AUTOHIERARCHY, mAutoNamespace);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_IMAP_ALLOWDISCONNECT, mAllowDisconnected);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsMailIMAP, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsMailIMAP)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailIMAP message handlers

// Set data
void CPrefsMailIMAP::SetContent(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info
	mLogonAtStartup = account->GetLogonAtStart();
	mDirDelim = account->GetDirDelim();
	mAutoNamespace = account->GetAutoNamespace();
	mAllowDisconnected = account->GetDisconnected();
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		GetDlgItem(IDC_PREFS_ACCOUNT_IMAP_ALLOWDISCONNECT)->EnableWindow(false);
}

// Force update of data
bool CPrefsMailIMAP::UpdateContent(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup);
	account->SetDirDelim(mDirDelim[(cdstring::size_type)0]);
	account->SetAutoNamespace(mAutoNamespace);
	account->SetDisconnected(mAllowDisconnected);
	
	return true;
}
