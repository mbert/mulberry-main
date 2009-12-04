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


// Source for CCreateCalendarDialog class

#include "CCreateCalendarDialog.h"

#include "CAdminLock.h"
#include "CINETCommon.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

#include "CCalendarAccount.h"
#include "CCalendarProtocol.h"
#include "CCalendarStoreManager.h"

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CCreateCalendarDialog::CCreateCalendarDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CCreateCalendarDialog::IDD, pParent)
{
}

// Default destructor
CCreateCalendarDialog::~CCreateCalendarDialog()
{
}

void CCreateCalendarDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateCalendarDialog)
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


BEGIN_MESSAGE_MAP(CCreateCalendarDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CCreateCalendarDialog)
	ON_COMMAND(IDC_CALENDAR_CREATE_CALENDAR, OnCalendar)
	ON_COMMAND(IDC_CALENDAR_CREATE_DIRECTORY, OnDirectory)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

void CCreateCalendarDialog::InitControls()
{
	// Get name
	mCalendar.SubclassDlgItem(IDC_CALENDAR_CREATE_CALENDAR, this);
	mDirectory.SubclassDlgItem(IDC_CALENDAR_CREATE_DIRECTORY, this);
	mCalendarName.SubclassDlgItem(IDC_CALENDAR_CREATE_NAME, this);

	// Get checkbox
	mSubscribe.SubclassDlgItem(IDC_CALENDAR_CREATE_SUBSCRIBE, this);

	// Get radio buttons
	mFullPath.SubclassDlgItem(IDC_CALENDAR_CREATE_FULL, this);
	mUseDirectory.SubclassDlgItem(IDC_CALENDAR_CREATE_HIER, this);

	// Get captions
	mHierarchy.SubclassDlgItem(IDC_CALENDAR_CREATE_HIERARCHY, this);
	mAccount.SubclassDlgItem(IDC_CALENDAR_CREATE_ACCOUNT, this);
	mAccountPopup.SubclassDlgItem(IDC_CALENDAR_CREATE_ACCOUNTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_AccountStart);
}

void CCreateCalendarDialog::OnCalendar()
{
	mSubscribe.EnableWindow(true);
	mUseSubscribe = true;
}

void CCreateCalendarDialog::OnDirectory()
{
	mSubscribe.EnableWindow(false);
	mUseSubscribe = false;
}

// Set the details
void CCreateCalendarDialog::SetDetails(SCreateCalendar* create)
{
	mCalendar.SetCheck(1);

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
void CCreateCalendarDialog::GetDetails(SCreateCalendar* result)
{
	result->directory = (mDirectory.GetCheck() == 1);
	result->new_name = mCalendarName.GetText();

	result->use_wd = (mFullPath.GetCheck() != 1);

	result->subscribe = mUseSubscribe && (mSubscribe.GetCheck() == 1);

	// Get account if not specified
	if (result->account.empty())
	{
		result->account = mAccountPopup.GetValueText();
	}
}

// Called during idle
void CCreateCalendarDialog::InitAccountMenu(void)
{
	// Delete previous items
	CMenu* pPopup = mAccountPopup.GetPopupMenu();
	UINT num_menu = pPopup->GetMenuItemCount();
	for(UINT i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add to menu
	UINT menu_id = IDM_AccountStart;
	if (!CAdminLock::sAdminLock.mNoLocalCalendars)
	{
		// Convert from UTF8 data
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, CPreferences::sPrefs->mLocalCalendarAccount.GetValue().GetName());
		mHasLocal = true;
	}
	
	// Add each mail account
	for(CCalendarAccountList::const_iterator iter = CPreferences::sPrefs->mCalendarAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mCalendarAccounts.GetValue().end(); iter++, menu_id++)
	{
		// Add to menu
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		mHasRemote = true;
		
		// Disable if not logged in
		if (!calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			pPopup->EnableMenuItem(menu_id, MF_BYCOMMAND | MF_GRAYED);
	}
}

bool CCreateCalendarDialog::PoseDialog(SCreateCalendar& create)
{
	bool result = false;

	// Create the dialog
	CCreateCalendarDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mData = &create;

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		result = !create.new_name.empty();
	}

	return result;
}
