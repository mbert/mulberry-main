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


// CCreateAdbkDialog.cpp : implementation file
//

#include "CCreateAdbkDialog.h"

#include "CAdminLock.h"
#include "CAdbkProtocol.h"
#include "CAddressBookManager.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CCreateAdbkDialog dialog


CCreateAdbkDialog::CCreateAdbkDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCreateAdbkDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCreateAdbkDialog)
	mAdbkName = _T("");
	mAccount = _T("");
	mOpenOnStartup = FALSE;
	mUseNicknames = FALSE;
	mUseSearch = FALSE;
	//}}AFX_DATA_INIT
	mHasLocal = false;
	mHasRemote = false;
}


void CCreateAdbkDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateAdbkDialog)
	DDX_UTF8Text(pDX, IDC_CREATEADBK_NAME, mAdbkName);
	DDX_Radio(pDX, IDC_CREATEADBK_PERSONAL, mPersonal);
	DDX_Check(pDX, IDC_CREATEADBK_OPENONSTARTUP, mOpenOnStartup);
	DDX_Check(pDX, IDC_CREATEADBK_USENICKNAMES, mUseNicknames);
	DDX_Check(pDX, IDC_CREATEADBK_USESEARCH, mUseSearch);
	DDX_UTF8Text(pDX, IDC_CREATEADBK_ACCOUNT, mAccount);
	DDX_Control(pDX, IDC_CREATEADBK_ACCOUNT, mAccountCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateAdbkDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCreateAdbkDialog)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnAccountPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateAdbkDialog message handlers

BOOL CCreateAdbkDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_CREATEADBK_ACCOUNTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_AccountStart);

	// Disable account popup if aleady specified
	if (mAccount.empty())
	{
		mAccountCtrl.ShowWindow(SW_HIDE);
		OnAccountPopup(IDM_AccountStart);
	}
	else
	{
		mAccountPopup.ShowWindow(SW_HIDE);
		
		// Disable items if the one selected is the local account
		if (mAccount == CPreferences::sPrefs->mLocalAdbkAccount.GetValue().GetName())
		{
			GetDlgItem(IDC_CREATEADBK_TYPE)->EnableWindow(false);
			GetDlgItem(IDC_CREATEADBK_PERSONAL)->EnableWindow(false);
			GetDlgItem(IDC_CREATEADBK_GLOBAL)->EnableWindow(false);
		}
	}
	
	// Hide type items if local
	if (mHasLocal && !mHasRemote)
	{
		CRect rect1;
		GetDlgItem(IDC_CREATEADBK_TYPE)->GetWindowRect(rect1);
		CRect rect2;
		GetDlgItem(IDC_CREATEADBK_ACCOUNTTITLE)->GetWindowRect(rect2);
		int resizeby = rect1.top - rect2.top;

		GetDlgItem(IDC_CREATEADBK_TYPE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CREATEADBK_PERSONAL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CREATEADBK_GLOBAL)->ShowWindow(SW_HIDE);
		
		::MoveWindowBy(GetDlgItem(IDC_CREATEADBK_ACCOUNTTITLE), 0, resizeby);
		::MoveWindowBy(GetDlgItem(IDC_CREATEADBK_ACCOUNT), 0, resizeby);
		::MoveWindowBy(GetDlgItem(IDC_CREATEADBK_ACCOUNTPOPUP), 0, resizeby);
		::MoveWindowBy(GetDlgItem(IDC_CREATEADBK_OPENONSTARTUP), 0, resizeby);
		::MoveWindowBy(GetDlgItem(IDC_CREATEADBK_USENICKNAMES), 0, resizeby);
		::MoveWindowBy(GetDlgItem(IDC_CREATEADBK_USESEARCH), 0, resizeby);
		::ResizeWindowBy(this, 0, resizeby);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set the details
void CCreateAdbkDialog::SetDetails(SCreateAdbk* create)
{
	mPersonal = 0;
	mAccount = create->account;
	mOpenOnStartup = create->open_on_startup;
	mUseNicknames = create->use_nicknames;
	mUseSearch = create->use_search;
}

// Get the details
void CCreateAdbkDialog::GetDetails(SCreateAdbk* result)
{
	result->name = mAdbkName;
	result->personal = (mPersonal == 0);
	result->open_on_startup = mOpenOnStartup;
	result->use_nicknames = mUseNicknames;
	result->use_search = mUseSearch;

	// Get account if not specified
	if (result->account.empty())
	{
		result->account = CUnicodeUtils::GetMenuStringUTF8(mAccountPopup.GetPopupMenu(), mAccountPopup.GetValue(), MF_BYCOMMAND);
	}
}

void CCreateAdbkDialog::OnAccountPopup(UINT nID) 
{
	mAccountPopup.SetValue(nID);

	// Get account
	CMailAccount* acct = CPreferences::sPrefs->mMailAccounts.GetValue()[nID - IDM_AccountStart];
	
	// Enable personal only for IMSP/ACAP accounts
	if (mHasLocal && (nID == IDM_AccountStart))
	{
		GetDlgItem(IDC_CREATEADBK_TYPE)->EnableWindow(false);
		GetDlgItem(IDC_CREATEADBK_PERSONAL)->EnableWindow(false);
		GetDlgItem(IDC_CREATEADBK_GLOBAL)->EnableWindow(false);
	}
	else
	{
		GetDlgItem(IDC_CREATEADBK_TYPE)->EnableWindow(true);
		GetDlgItem(IDC_CREATEADBK_PERSONAL)->EnableWindow(true);
		GetDlgItem(IDC_CREATEADBK_GLOBAL)->EnableWindow(true);
		
		cdstring protoname = CUnicodeUtils::GetMenuStringUTF8(mAccountPopup.GetPopupMenu(), mAccountPopup.GetValue(), MF_BYCOMMAND);
		CAdbkProtocol* proto = CAddressBookManager::sAddressBookManager->GetProtocol(protoname);

		cdstring default_name = proto->GetUserPrefix();
		default_name += proto->GetAccount()->GetAuthenticator().GetAuthenticator()->GetActualUID();
		bool personal_allowed = !default_name.empty();

		if (!personal_allowed)
		{
			GetDlgItem(IDC_CREATEADBK_PERSONAL)->EnableWindow(false);
			static_cast<CButton*>(GetDlgItem(IDC_CREATEADBK_GLOBAL))->SetCheck(true);
		}

	}
}

void CCreateAdbkDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Now add current items
	int menu_id = IDM_AccountStart;
	if (!CAdminLock::sAdminLock.mNoLocalAdbks)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, CPreferences::sPrefs->mLocalAdbkAccount.GetValue().GetName());
		mHasLocal = true;
	}

	// Add each adbk account (only IMSP/ACAP)
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		// Only if IMSP/ACAP
		if (((*iter)->GetServerType() != CINETAccount::eIMSP) &&
			((*iter)->GetServerType() != CINETAccount::eACAP))
			continue;

		// Add to menu
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		mHasRemote = true;
		
		// Disable if not logged in
		if (!CAddressBookManager::sAddressBookManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			pPopup->EnableMenuItem(menu_id, MF_BYCOMMAND | MF_GRAYED);
		
		// Update menu id here after we have added an actual item
		menu_id++;
	}
}

// Do the dialog
bool CCreateAdbkDialog::PoseDialog(SCreateAdbk* details)
{
	// Create the dialog
	CCreateAdbkDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(details);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{					
		dlog.GetDetails(details);
		return true;
	}
	else
		return false;
}
