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


// COpenMailboxDialog.cpp : implementation file
//

#include "COpenMailboxDialog.h"

#include "CBrowseMailboxDialog.h"
#include "CMboxRef.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// COpenMailboxDialog dialog


COpenMailboxDialog::COpenMailboxDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(COpenMailboxDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenMailboxDialog)
	mMailboxName = _T("");
	//}}AFX_DATA_INIT
	mBrowsed = nil;

}


void COpenMailboxDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenMailboxDialog)
	DDX_UTF8Text(pDX, IDC_OPENMAILBOX_NAME, mMailboxName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpenMailboxDialog, CHelpDialog)
	//{{AFX_MSG_MAP(COpenMailboxDialog)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnOpenMailboxAccount)
	ON_BN_CLICKED(IDC_OPENMAILBOX_BROWSE, OnOpenMailboxBrowse)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenMailboxDialog message handlers

BOOL COpenMailboxDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_OPENMAILBOX_ACCOUNT, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_AccountStart);

	return true;
}

// Set the dialogs return info
CMbox* COpenMailboxDialog::GetSelectedMbox(void)
{
	// If browse result, use that
	if (mBrowsed)
		return mBrowsed;
	
	// Get account name for mailbox
	cdstring acct_name;
	
	short acct_num = mAccountPopup.GetValue() - IDM_AccountStart;
	acct_name += CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetName();
	acct_name += cMailAccountSeparator;
	
	acct_name += mMailboxName;
	
	// Get diur delim for account
	char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetDirDelim();

	// Try to resolve reference - force creation
	CMboxRef ref(acct_name, dir_delim);
	
	return ref.ResolveMbox(true);
}

void COpenMailboxDialog::OnOpenMailboxAccount(UINT nID) 
{
	mAccountPopup.SetValue(nID);
}

void COpenMailboxDialog::OnOpenMailboxBrowse() 
{
	bool ignore = false;
	if (CBrowseMailboxDialog::PoseDialog(true, false, mBrowsed, ignore))
		EndDialog(IDOK);
}

void COpenMailboxDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, (*iter)->GetName());
}

bool COpenMailboxDialog::PoseDialog(CMbox*& mbox)
{
	bool result = false;

	COpenMailboxDialog dlog(CSDIFrame::GetAppTopWindow());

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		mbox = dlog.GetSelectedMbox();
		result = true;
	}

	return result;
}
