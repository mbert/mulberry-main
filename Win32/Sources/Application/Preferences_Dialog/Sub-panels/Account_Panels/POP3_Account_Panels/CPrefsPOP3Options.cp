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


// CPrefsPOP3Options.cpp : implementation file
//

#include "CPrefsPOP3Options.h"

#include "CMailAccount.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsPOP3Options dialog

IMPLEMENT_DYNAMIC(CPrefsPOP3Options, CTabPanel)

CPrefsPOP3Options::CPrefsPOP3Options()
	: CTabPanel(CPrefsPOP3Options::IDD)
{
	//{{AFX_DATA_INIT(CPrefsPOP3Options)
	mLeaveOnServer = FALSE;
	mDeleteAfter = FALSE;
	mDeleteDays = 0;
	mDeleteExpunged = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsPOP3Options::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsPOP3Options)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_POP3_LEAVEMAIL, mLeaveOnServer);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_POP3_LEAVEMAIL, mLeaveOnServerCtrl);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_POP3_DELETEAFTER, mDeleteAfter);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_POP3_DELETEAFTER, mDeleteAfterCtrl);
	DDX_UTF8Text(pDX, IDC_PREFS_ACCOUNT_POP3_DAYS, mDeleteDays);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_POP3_DAYS, mDeleteDaysCtrl);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_POP3_DELETEEXPUNGE, mDeleteExpunged);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_POP3_DELETEEXPUNGE, mDeleteExpungedCtrl);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_POP3_USEMAXSIZE, mUseMaxSize);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_POP3_USEMAXSIZE, mUseMaxSizeCtrl);
	DDX_UTF8Text(pDX, IDC_PREFS_ACCOUNT_POP3_MAXSIZE, mMaxSize);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_POP3_MAXSIZE, mMaxSizeCtrl);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_POP3_USEAPOP, mUseAPOP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsPOP3Options, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsPOP3Options)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_POP3_LEAVEMAIL, OnPrefsAccountPOP3LeaveMail)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_POP3_DELETEAFTER, OnPrefsAccountPOP3DeleteAfter)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_POP3_USEMAXSIZE, OnPrefsAccountPOP3UseMaxSize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsPOP3Options message handlers

// Set data
void CPrefsPOP3Options::SetContent(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info
	mLeaveOnServer = account->GetLeaveOnServer();
	mDeleteAfter = account->GetDoTimedDelete();
	mDeleteDays = account->GetDeleteAfter();
	mDeleteExpunged = account->GetDeleteExpunged();
	mUseMaxSize = account->GetUseMaxSize();
	mMaxSize = account->GetMaxSize();
	mUseAPOP = account->GetUseAPOP();

	if (!account->GetDoTimedDelete())
		mDeleteDaysCtrl.EnableWindow(false);
	SetLeaveOnServer(account->GetLeaveOnServer());

	if (!mUseMaxSize)
		mMaxSizeCtrl.EnableWindow(false);
}

// Force update of data
bool CPrefsPOP3Options::UpdateContent(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info from panel into prefs
	account->SetLeaveOnServer(mLeaveOnServer);
	account->SetDoTimedDelete(mDeleteAfter);
	account->SetDeleteAfter(mDeleteDays);
	account->SetDeleteExpunged(mDeleteExpunged);
	account->SetUseMaxSize(mUseMaxSize);
	account->SetMaxSize(mMaxSize);
	account->SetUseAPOP(mUseAPOP);
	
	return true;
}

void CPrefsPOP3Options::OnPrefsAccountPOP3LeaveMail() 
{
	SetLeaveOnServer(mLeaveOnServerCtrl.GetCheck());
}

void CPrefsPOP3Options::OnPrefsAccountPOP3DeleteAfter() 
{
	mDeleteDaysCtrl.EnableWindow(mDeleteAfterCtrl.GetCheck());
}

void CPrefsPOP3Options::OnPrefsAccountPOP3UseMaxSize() 
{
	mMaxSizeCtrl.EnableWindow(mUseMaxSizeCtrl.GetCheck());
}

void CPrefsPOP3Options::SetLeaveOnServer(bool on)
{
	if (on)
	{
		mDeleteAfterCtrl.EnableWindow(true);
		if (mDeleteAfterCtrl.GetCheck())
			mDeleteDaysCtrl.EnableWindow(true);
		mDeleteExpungedCtrl.EnableWindow(true);
	}
	else
	{
		mDeleteAfterCtrl.EnableWindow(false);
		mDeleteDaysCtrl.EnableWindow(false);
		mDeleteExpungedCtrl.EnableWindow(false);
	}
}
