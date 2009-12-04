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


// CPrefsSMTPOptions.cpp : implementation file
//

#include "CPrefsSMTPOptions.h"

#include "CAdminLock.h"
#include "CSMTPAccount.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSMTPOptions dialog

IMPLEMENT_DYNAMIC(CPrefsSMTPOptions, CTabPanel)

CPrefsSMTPOptions::CPrefsSMTPOptions()
	: CTabPanel(CPrefsSMTPOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsSMTPOptions)
	mUseQueue = FALSE;
	mHoldMessages = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsSMTPOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsSMTPOptions)
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_SMTP_UseQueue, mUseQueue);
	DDX_Check(pDX, IDC_PREFS_ACCOUNT_SMTP_HoldMessages, mHoldMessages);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsSMTPOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsSMTPOptions)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_SMTP_UseQueue, OnUseQueue)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsSMTPOptions message handlers

// Set data
void CPrefsSMTPOptions::SetContent(void* data)
{
	CSMTPAccount* account = (CSMTPAccount*) data;

	// Copy info
	mUseQueue = account->GetUseQueue();
	mHoldMessages = account->GetHoldMessages();
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoSMTPQueues)
	{
		GetDlgItem(IDC_PREFS_ACCOUNT_SMTP_UseQueue)->EnableWindow(false);
		GetDlgItem(IDC_PREFS_ACCOUNT_SMTP_HoldMessages)->EnableWindow(false);
	}
	else if (!mUseQueue)
		GetDlgItem(IDC_PREFS_ACCOUNT_SMTP_HoldMessages)->EnableWindow(false);
}

// Force update of data
bool CPrefsSMTPOptions::UpdateContent(void* data)
{
	CSMTPAccount* account = (CSMTPAccount*) data;

	// Copy info from panel into prefs
	account->SetUseQueue(mUseQueue);
	account->SetHoldMessages(mHoldMessages);
	
	return true;
}

void CPrefsSMTPOptions::OnUseQueue() 
{
	GetDlgItem(IDC_PREFS_ACCOUNT_SMTP_HoldMessages)->EnableWindow(static_cast<CButton*>(GetDlgItem(IDC_PREFS_ACCOUNT_SMTP_UseQueue))->GetCheck());
}
