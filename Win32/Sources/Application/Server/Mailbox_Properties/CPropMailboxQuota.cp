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


// CPropMailboxQuota.cp : implementation file
//

#include "CPropMailboxQuota.h"

#include "CMbox.h"
#include "CMboxList.h"

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxQuota property page

IMPLEMENT_DYNCREATE(CPropMailboxQuota, CHelpPropertyPage)

CPropMailboxQuota::CPropMailboxQuota() : CHelpPropertyPage(CPropMailboxQuota::IDD)
{
	//{{AFX_DATA_INIT(CPropMailboxQuota)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPropMailboxQuota::~CPropMailboxQuota()
{
	mTable.Detach();
}

void CPropMailboxQuota::DoDataExchange(CDataExchange* pDX)
{
	CHelpPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPropMailboxQuota)
	//DDX_Control(pDX, IDC_MAILBOXQUOTA_TABLE, mTable);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPropMailboxQuota, CHelpPropertyPage)
	//{{AFX_MSG_MAP(CPropMailboxQuota)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPropMailboxQuota message handlers
// Set mbox list

BOOL CPropMailboxQuota::OnInitDialog()
{
	CHelpPropertyPage::OnInitDialog();

	// Subclass table
	mTitles.SubclassDlgItem(IDC_MAILBOXQUOTA_TITLES, this);
	mTable.SubclassDlgItem(IDC_MAILBOXQUOTA_TABLE, this);

	// Setup titles to sync with table and add titles
	mTitles.SyncTable(&mTable, true);
	mTitles.LoadTitles("UI::Titles::Quota", 4);

	// For now just use first item
	SetMbox((CMbox*) mMboxList->front());

	return true;
}

void CPropMailboxQuota::SetMboxList(CMboxList* mbox_list)
{
	// Save list
	mMboxList = mbox_list;
}


// Set mbox list
void CPropMailboxQuota::SetMbox(CMbox* mbox)
{
	mTable.SetList(mbox->GetQuotas());
}