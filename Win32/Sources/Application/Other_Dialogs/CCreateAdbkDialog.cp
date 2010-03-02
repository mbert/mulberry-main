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
}


void CCreateAdbkDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateAdbkDialog)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		GetDetails(mData);
	}
	else
	{
		InitControls();
		SetDetails(mData);
	}
}


BEGIN_MESSAGE_MAP(CCreateAdbkDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCreateAdbkDialog)
	ON_COMMAND(IDC_CREATEADBK_ADDRESSBOOK, OnAddressBook)
	ON_COMMAND(IDC_CREATEADBK_DIRECTORY, OnDirectory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateAdbkDialog message handlers

void CCreateAdbkDialog::InitControls()
{
	// Get name
	mAddressBook.SubclassDlgItem(IDC_CREATEADBK_ADDRESSBOOK, this);
	mDirectory.SubclassDlgItem(IDC_CREATEADBK_DIRECTORY, this);
	mAdbkName.SubclassDlgItem(IDC_CREATEADBK_NAME, this);

	// Get checkboxes
	mOpenOnStartup.SubclassDlgItem(IDC_CREATEADBK_OPENONSTARTUP, this);
	mNicknames.SubclassDlgItem(IDC_CREATEADBK_USENICKNAMES, this);
	mSearch.SubclassDlgItem(IDC_CREATEADBK_USESEARCH, this);

	// Get radio buttons
	mFullPath.SubclassDlgItem(IDC_CREATEADBK_FULL, this);
	mUseDirectory.SubclassDlgItem(IDC_CREATEADBK_HIER, this);

	// Get captions
	mHierarchy.SubclassDlgItem(IDC_CREATEADBK_HIERARCHY, this);
	mAccount.SubclassDlgItem(IDC_CREATEADBK_ACCOUNT, this);
	mAccountPopup.SubclassDlgItem(IDC_CREATEADBK_ACCOUNTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_AccountStart);
}

void CCreateAdbkDialog::OnAddressBook()
{
	mOpenOnStartup.EnableWindow(true);
	mNicknames.EnableWindow(true);
	mSearch.EnableWindow(true);
}

void CCreateAdbkDialog::OnDirectory()
{
	mOpenOnStartup.EnableWindow(false);
	mNicknames.EnableWindow(false);
	mSearch.EnableWindow(false);
}

// Set the details
void CCreateAdbkDialog::SetDetails(SCreateAdbk* create)
{
	mAddressBook.SetCheck(1);

	// If no account use the popup
	if (create->account.empty())
	{
		mAccount.ShowWindow(SW_HIDE);
	}
	else
	{
		mAccountPopup.ShowWindow(SW_HIDE);
		CUnicodeUtils::SetWindowTextUTF8(&mAccount, create->account);
	}

	CUnicodeUtils::SetWindowTextUTF8(mHierarchy, create->parent);

	if (create->use_wd && !create->parent.empty())
		mUseDirectory.SetCheck(1);
	else
		mFullPath.SetCheck(1);

	if (create->parent.empty())
	{
		mUseDirectory.EnableWindow(false);
		mHierarchy.EnableWindow(false);
	}
}

// Get the details
void CCreateAdbkDialog::GetDetails(SCreateAdbk* result)
{
	result->directory = (mDirectory.GetCheck() == 1);
	result->name = mAdbkName.GetText();

	result->use_wd = (mFullPath.GetCheck() != 1);

	result->open_on_startup = (mOpenOnStartup.GetCheck() == 1);
	result->use_nicknames = (mNicknames.GetCheck() == 1);
	result->use_search = (mSearch.GetCheck() == 1);

	// Get account if not specified
	if (result->account.empty())
	{
		result->account = mAccountPopup.GetValueText();
	}
}

// Called during idle
void CCreateAdbkDialog::InitAccountMenu(void)
{
	// Delete previous items
	CMenu* pPopup = mAccountPopup.GetPopupMenu();
	UINT num_menu = pPopup->GetMenuItemCount();
	for(UINT i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add to menu
	UINT menu_id = IDM_AccountStart;
	if (!CAdminLock::sAdminLock.mNoLocalAdbks)
	{
		// Convert from UTF8 data
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, CPreferences::sPrefs->mLocalAdbkAccount.GetValue().GetName());
	}

	// Add each mail account
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++, menu_id++)
	{
		// Only if IMSP/ACAP/CardDAV
		if (((*iter)->GetServerType() != CINETAccount::eIMSP) &&
			((*iter)->GetServerType() != CINETAccount::eACAP) &&
			((*iter)->GetServerType() != CINETAccount::eCardDAVAdbk))
			continue;

		// Add to menu
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		
		// Disable if not logged in
		if (!CAddressBookManager::sAddressBookManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			pPopup->EnableMenuItem(menu_id, MF_BYCOMMAND | MF_GRAYED);
	}
}

// Do the dialog
bool CCreateAdbkDialog::PoseDialog(SCreateAdbk* details)
{
	bool result = false;

	// Create the dialog
	CCreateAdbkDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mData = details;

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{					
		result = !details->name.empty();
	}

	return result;
}
