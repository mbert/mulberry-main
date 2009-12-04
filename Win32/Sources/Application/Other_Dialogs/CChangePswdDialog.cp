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


// CChangePswdDialog.cpp : implementation file
//


#include "CChangePswdDialog.h"

#include "CErrorHandler.h"
#include "COptionsProtocol.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CChangePswdDialog dialog


CChangePswdDialog::CChangePswdDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CChangePswdDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangePswdDialog)
	//}}AFX_DATA_INIT
	mTimerID = 0;
}


void CChangePswdDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangePswdDialog)
	DDX_Control(pDX, IDC_CHANGEPSWDICON, mIcon);
	DDX_UTF8Text(pDX, IDC_CHANGEPSWDDESCRIPTION, mDescription);
	DDX_UTF8Text(pDX, IDC_CHANGEPSWDSERVER, mServerName);
	DDX_UTF8Text(pDX, IDC_CHANGEPSWDUSER, mUserName);
	DDX_UTF8Text(pDX, IDC_CHANGEPSWDOLD, mOldPassword);
	DDX_UTF8Text(pDX, IDC_CHANGEPSWDNEW1, mNew1Password);
	DDX_UTF8Text(pDX, IDC_CHANGEPSWDNEW2, mNew2Password);
	DDX_Control(pDX, IDC_CHANGEPSWDCAPSLOCK, mCapsLock);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangePswdDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CChangePswdDialog)
		ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangePswdDialog message handlers

BOOL CChangePswdDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// TODO: Add extra initialization here

	// Set icon
	mIcon.SetIcon(::LoadIcon(::AfxFindResourceHandle(MAKEINTRESOURCE(mIconID), RT_GROUP_ICON), MAKEINTRESOURCE(mIconID)));

	mCapsLock.ShowWindow(SW_HIDE);
	mCaps = false;

	// Start a timer
	mTimerID = SetTimer('Cswd', 250, NULL);

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangePswdDialog::OnTimer(UINT nIDEvent)
{
	if ((::GetKeyState(VK_CAPITAL) & 0x0001) ^ mCaps)
	{
		mCaps = !mCaps;
		mCapsLock.ShowWindow(mCaps ? SW_SHOW : SW_HIDE);
	}
}

void CChangePswdDialog::OnOK()
{
	// Update the data
	UpdateData();
	
	if (mNew1Password != mNew2Password)
	{
		// Put alert
		CErrorHandler::PutStopAlertRsrc("Alerts::UserPswd::NewPasswordMismatch");
		
		// Focus back on new password
		mNew1Password = cdstring::null_str;
		mNew2Password = cdstring::null_str;
		UpdateData(false);

		GetDlgItem(IDC_CHANGEPSWDNEW1)->SetFocus();
		
		return;
	}

	// Do default OK action
	EndDialog(IDOK);
}


bool CChangePswdDialog::PromptPasswordChange(const CINETAccount* acct,
											cdstring& old_pass,
											cdstring& new_pass)
{
	int icon = IDI_USERPSWD_IMAP;
	cdstring title;
	switch(acct->GetServerType())
	{
	case CINETAccount::eIMAP:
		icon = IDI_USERPSWD_IMAP;
		title = rsrc::GetString("Alerts::UserPswd::IMAPCHANGE");
	case CINETAccount::ePOP3:
	case CINETAccount::eLocal:
		icon = IDI_USERPSWD_POP3;
		title = rsrc::GetString("Alerts::UserPswd::POP3CHANGE");
		break;
	case CINETAccount::eSMTP:
		icon = IDI_USERPSWD_SMTP;
		title = rsrc::GetString("Alerts::UserPswd::SMTPCHANGE");
		break;
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
		if (dynamic_cast<COptionsProtocol*>((CINETProtocol*)acct->GetProtocol()) != NULL)
		{
			icon = IDI_USERPSWD_PREFS;
			title = rsrc::GetString("Alerts::UserPswd::PREFSCHANGE");
		}
		else
		{
			icon = IDI_USERPSWD_ADBK;
			title = rsrc::GetString("Alerts::UserPswd::ADBKCHANGE");
		}
		break;
	case CINETAccount::eLDAP:
		icon = IDI_USERPSWD_LDAP;
		title = rsrc::GetString("Alerts::UserPswd::LDAPCHANGE");
		break;
	}

	// Create the dialog
	CChangePswdDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mIconID = icon;
	dlog.mDescription = title;
	dlog.mServerName = acct->GetServerIP();
	dlog.mUserName = acct->GetAuthenticatorUserPswd()->GetUID();

	// Do the dialog
	int result = dlog.DoModal();
	if (dlog.mTimerID != 0)
		dlog.KillTimer(dlog.mTimerID);

	if (result == IDOK)
	{
		old_pass = dlog.mOldPassword;
		new_pass = dlog.mNew1Password;
		return true;
	}
	else
		return false;
}