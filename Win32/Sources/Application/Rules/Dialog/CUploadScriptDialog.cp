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


// CUploadScriptDialog.cpp : implementation file
//

#include "CUploadScriptDialog.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

CUploadScriptDialog::SUploadScript CUploadScriptDialog::sLastChoice = {false, 0, true, true};

/////////////////////////////////////////////////////////////////////////////
// CUploadScriptDialog dialog


CUploadScriptDialog::CUploadScriptDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CUploadScriptDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CUploadScriptDialog)
	//}}AFX_DATA_INIT
}


void CUploadScriptDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CUploadScriptDialog)
	DDX_Radio(pDX, IDC_UPLOADSCRIPT_FILE, mFile);
	DDX_Control(pDX, IDC_UPLOADSCRIPT_ACCOUNTPOPUP, mAccountPopup);
	DDX_Radio(pDX, IDC_UPLOADSCRIPT_UPLOAD, mUploadGroup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CUploadScriptDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CUploadScriptDialog)
	ON_COMMAND(IDC_UPLOADSCRIPT_FILE, OnFileBtn)
	ON_COMMAND(IDC_UPLOADSCRIPT_SERVER, OnServerBtn)
	ON_COMMAND_RANGE(IDM_AccountStart, IDM_AccountStop, OnAccountPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CUploadScriptDialog message handlers

BOOL CUploadScriptDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_UPLOADSCRIPT_ACCOUNTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();
	mAccountPopup.SetValue(IDM_AccountStart + mAccountValue);

	// Set file button if previously used or if no accounts
	if (mFile == 0)
		OnFileBtn();
	
	// Disable server choice if no accounts
	if (mAccountPopup.GetPopupMenu()->GetMenuItemCount() == 0)
		GetDlgItem(IDC_UPLOADSCRIPT_SERVER)->EnableWindow(false);
		

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Set the details
void CUploadScriptDialog::SetDetails(SUploadScript& create)
{
	mFile = (create.mFile || (CPreferences::sPrefs->mSIEVEAccounts.GetValue().size() == 0)) ? 0 : 1;

	// Init acct
	mAccountValue = create.mAccountIndex;

	// Init group
	if (create.mUpload && !create.mActivate)
		mUploadGroup = 0;
	else if (create.mUpload && create.mActivate)
		mUploadGroup = 1;
	else
		mUploadGroup = 2;
}

// Get the details
void CUploadScriptDialog::GetDetails(SUploadScript& result)
{
	// Get items
	result.mFile = (mFile == 0);
	result.mAccountIndex = mAccountValue;
	result.mUpload = (mUploadGroup != 2);
	result.mActivate = (mUploadGroup == 1);
}

void CUploadScriptDialog::OnFileBtn() 
{
	mAccountPopup.EnableWindow(false);
	GetDlgItem(IDC_UPLOADSCRIPT_UPLOAD)->EnableWindow(false);
	GetDlgItem(IDC_UPLOADSCRIPT_UPLOADACTIVATE)->EnableWindow(false);
	GetDlgItem(IDC_UPLOADSCRIPT_DELETE)->EnableWindow(false);
}

void CUploadScriptDialog::OnServerBtn() 
{
	mAccountPopup.EnableWindow(true);
	GetDlgItem(IDC_UPLOADSCRIPT_UPLOAD)->EnableWindow(true);
	GetDlgItem(IDC_UPLOADSCRIPT_UPLOADACTIVATE)->EnableWindow(true);
	GetDlgItem(IDC_UPLOADSCRIPT_DELETE)->EnableWindow(true);
}

void CUploadScriptDialog::OnAccountPopup(UINT nID) 
{
	mAccountPopup.SetValue(nID);
	mAccountValue = nID - IDM_AccountStart;
}

void CUploadScriptDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add each filter account
	int menu_id = IDM_AccountStart;
	for(CManageSIEVEAccountList::const_iterator iter = CPreferences::sPrefs->mSIEVEAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mSIEVEAccounts.GetValue().end(); iter++, menu_id++)
	{
		// Add to menu
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
	}
}

// Do the dialog
bool CUploadScriptDialog::PoseDialog(SUploadScript& details)
{
	// Create the dialog
	CUploadScriptDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.SetDetails(sLastChoice);

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{					
		dlog.GetDetails(sLastChoice);
		details = sLastChoice;
		return true;
	}
	else
		return false;
}
