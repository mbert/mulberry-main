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


// CCreateAccountDialog.cpp : implementation file
//

#include "CCreateAccountDialog.h"

#include "CAdminLock.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateAccountDialog dialog


CCreateAccountDialog::CCreateAccountDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCreateAccountDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateAccountDialog)
	//}}AFX_DATA_INIT
}


void CCreateAccountDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateAccountDialog)
	DDX_Control(pDX, IDC_NEWACCOUNT_TYPE, mType);
	DDX_UTF8Text(pDX, IDC_NEWACCOUNT_NAME, mText);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateAccountDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCreateAccountDialog)
	ON_COMMAND_RANGE(IDM_ACCOUNT_IMAP_MAILBOX, IDM_ACCOUNT_WEBDAVCALENDAR, OnNewAccountType)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateAccountDialog message handlers

BOOL CCreateAccountDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Load menu bitmaps
	mMaiboxBMP.LoadBitmap(IDB_ACCOUNTMAILBOX);
	mSMTPBMP.LoadBitmap(IDB_ACCOUNTSMTP);
	mRemoteBMP.LoadBitmap(IDB_ACCOUNTREMOTE);
	mAdbkBMP.LoadBitmap(IDB_ACCOUNTADBK);
	mAddrSearchBMP.LoadBitmap(IDB_ACCOUNTADDRSEARCH);
	mSIEVEBMP.LoadBitmap(IDB_ACCOUNTSIEVE);

	// Subclass buttons
	mType.SubclassDlgItem(IDC_NEWACCOUNT_TYPE, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mType.SetMenu(IDR_POPUP_ACCOUNTTYPE);
	InitTypeMenu();
	mType.SetValue(IDM_ACCOUNT_IMAP_MAILBOX);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Called during idle
void CCreateAccountDialog::GetDetails(cdstring& change, short& type)
{
	change = mText;
	type = mType.GetValue();
}

void CCreateAccountDialog::OnNewAccountType(UINT nID) 
{
	mType.SetValue(nID);
}

void CCreateAccountDialog::InitTypeMenu()
{
	CMenu* pPopup = mType.GetPopupMenu();

	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_IMAP_MAILBOX, MF_BYCOMMAND, &mMaiboxBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_POP3_MAILBOX, MF_BYCOMMAND, &mMaiboxBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_LOCAL_MAILBOX, MF_BYCOMMAND, &mMaiboxBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_SMTP, MF_BYCOMMAND, &mSMTPBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_IMSP_OPTIONS, MF_BYCOMMAND, &mRemoteBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_ACAP_OPTIONS, MF_BYCOMMAND, &mRemoteBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_IMSP_ADBK, MF_BYCOMMAND, &mAdbkBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_ACAP_ADBK, MF_BYCOMMAND, &mAdbkBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_LDAP, MF_BYCOMMAND, &mAddrSearchBMP, NULL);
	pPopup->SetMenuItemBitmaps(IDM_ACCOUNT_SIEVE, MF_BYCOMMAND, &mSIEVEBMP, NULL);

	// Disable certain types
	if (CAdminLock::sAdminLock.mNoLocalMbox)
	{
		pPopup->EnableMenuItem(IDM_ACCOUNT_POP3_MAILBOX, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_ACCOUNT_LOCAL_MAILBOX, MF_GRAYED | MF_BYCOMMAND);
	}
	if (CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		pPopup->EnableMenuItem(IDM_ACCOUNT_IMSP_OPTIONS, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_ACCOUNT_ACAP_OPTIONS, MF_GRAYED | MF_BYCOMMAND);
	}
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		pPopup->EnableMenuItem(IDM_ACCOUNT_CALDAVCALENDAR, MF_GRAYED | MF_BYCOMMAND);
		pPopup->EnableMenuItem(IDM_ACCOUNT_WEBDAVCALENDAR, MF_GRAYED | MF_BYCOMMAND);
	}
}

