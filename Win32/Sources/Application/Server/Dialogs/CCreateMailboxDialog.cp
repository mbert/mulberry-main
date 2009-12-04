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


// CCreateMailboxDialog.cpp : implementation file
//

#include "CCreateMailboxDialog.h"

#include "CMailAccountManager.h"
#include "CMboxProtocol.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateMailboxDialog dialog


CCreateMailboxDialog::CCreateMailboxDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCreateMailboxDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateMailboxDialog)
	mDirectory = 0;
	mSubscribe = FALSE;
	mFullPath = -1;
	//}}AFX_DATA_INIT
}


void CCreateMailboxDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateMailboxDialog)
	DDX_Radio(pDX, IDC_CREATEMAILBOX_MAILBOX, mDirectory);
	DDX_UTF8Text(pDX, IDC_CREATEMAILBOX_NAME, mMailboxName);
	DDX_Check(pDX, IDC_CREATEMAILBOX_SUBSCRIBE, mSubscribe);
	DDX_Radio(pDX, IDC_CREATEMAILBOX_FULL, mFullPath);
	DDX_Control(pDX, IDC_CREATEMAILBOX_HIER, mCreateInHierarchyBtn);
	DDX_UTF8Text(pDX, IDC_CREATEMAILBOX_HIERARCHY, mHierarchy);
	DDX_UTF8Text(pDX, IDC_CREATEMAILBOX_ACCOUNT, mAccount);
	DDX_Control(pDX, IDC_CREATEMAILBOX_ACCOUNT, mAccountCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateMailboxDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCreateMailboxDialog)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnAccountPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateMailboxDialog message handlers

BOOL CCreateMailboxDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_CREATEMAILBOX_ACCOUNTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_AccountStart);

	// Disable radio if no parent
	if (mHierarchy.empty())
		mCreateInHierarchyBtn.EnableWindow(false);

	// Disable account popup if aleady specified
	if (mAccount.empty())
		mAccountCtrl.ShowWindow(SW_HIDE);
	else
		mAccountPopup.ShowWindow(SW_HIDE);
	
	// Check status of subscribe
	if (mAccount.empty())
		// Set via current popup choice
		OnAccountPopup(IDM_AccountStart);

	// Hide subscribe if not needed
	else if (!mUseSubscribe)
		GetDlgItem(IDC_CREATEMAILBOX_SUBSCRIBE)->ShowWindow(SW_HIDE);

	return true;
}

// Set the details
void CCreateMailboxDialog::SetDetails(SCreateMailbox* create)
{
	mAccount = create->account;
	
	mHierarchy = create->parent;
	
	mFullPath = ((create->use_wd && !create->parent.empty()) ? 1 : 0);

	// No subscribed if not IMAP
	mUseSubscribe = mAccount.empty() ? true : (create->account_type == CINETAccount::eIMAP);
}

// Get the details
void CCreateMailboxDialog::GetDetails(SCreateMailbox* result)
{
	result->directory = (mDirectory == 1);

	result->new_name = mMailboxName;

	result->use_wd = (mFullPath == 1);

	result->subscribe = mUseSubscribe && mSubscribe;

	if (mAccount.empty())
	{
		short acct_num = mAccountPopup.GetValue() - IDM_AccountStart;
		result->account = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetName();
	}
}

void CCreateMailboxDialog::OnAccountPopup(UINT nID) 
{
	mAccountPopup.SetValue(nID);

	// Get account
	CMailAccount* acct = CPreferences::sPrefs->mMailAccounts.GetValue()[nID - IDM_AccountStart];
	
	// Enable subscribe only for IMAP accounts
	mUseSubscribe = (acct->GetServerType() == CINETAccount::eIMAP);
	GetDlgItem(IDC_CREATEMAILBOX_SUBSCRIBE)->ShowWindow(mUseSubscribe ? SW_SHOW : SW_HIDE);
}

void CCreateMailboxDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_id++)
	{
		// Add to menu
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		
		// Disable if not logged in
		if (!CMailAccountManager::sMailAccountManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			pPopup->EnableMenuItem(menu_id, MF_BYCOMMAND | MF_GRAYED);
	}
}

bool CCreateMailboxDialog::PoseDialog(SCreateMailbox& create)
{
	bool result = false;

	// Create the dialog
	CCreateMailboxDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(&create);

	// Let Dialog process events
	if (dlog.DoModal() == IDOK)
	{
		dlog.GetDetails(&create);

		// Flag success
		result = !create.new_name.empty();
	}

	return result;
}
