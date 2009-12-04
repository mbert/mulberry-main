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


// CPrefsLocalOptions.cpp : implementation file
//

#include "CPrefsLocalOptions.h"

#include "CLocalCommon.h"
#include "CMailAccount.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

#include <unistd.h>

/////////////////////////////////////////////////////////////////////////////
// CPrefsLocalOptions dialog

IMPLEMENT_DYNAMIC(CPrefsLocalOptions, CTabPanel)

CPrefsLocalOptions::CPrefsLocalOptions()
	: CTabPanel(CPrefsLocalOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsLocalOptions)
	//}}AFX_DATA_INIT
	
	mLocalAddress = false;
	mDisconnected = false;
}


void CPrefsLocalOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsLocalOptions)
	DDX_Radio(pDX, IDC_PREFS_ACCOUNT_Local_USEDEFAULT, mUseDefault);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Local_USEDEFAULT, mUseDefaultCtrl);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Local_SPECIFY, mSpecifyCtrl);
	DDX_UTF8Text(pDX, IDC_PREFS_ACCOUNT_Local_PATH, mPath);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Local_PATH, mPathCtrl);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Local_RELATIVE, mRelativeCtrl);
	DDX_Control(pDX, IDC_PREFS_ACCOUNT_Local_ABSOLUTE, mAbsoluteCtrl);
	//DDX_Control(pDX, IDC_PREFS_ACCOUNT_Local_FILEFORMAT, mFileFormatPopup);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsLocalOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsLocalOptions)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Local_USEDEFAULT, OnPrefsAccountLocalUseDefault)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Local_SPECIFY, OnPrefsAccountLocalSpecify)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Local_CHOOSE, OnPrefsAccountLocalChoose)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Local_RELATIVE, OnPrefsAccountLocalRelative)
	ON_BN_CLICKED(IDC_PREFS_ACCOUNT_Local_ABSOLUTE, OnPrefsAccountLocalAbsolute)
	ON_COMMAND_RANGE(IDM_FILEFORMAT_AUTOMATIC, IDM_FILEFORMAT_WINDOWS, OnFileFormatPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsLocalOptions message handlers

BOOL CPrefsLocalOptions::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mFileFormatPopup.SubclassDlgItem(IDC_PREFS_ACCOUNT_Local_FILEFORMAT, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mFileFormatPopup.SetMenu(IDR_POPUP_FILEFORMATS);

	return true;
}

// Set data
void CPrefsLocalOptions::SetContent(void* data)
{
	CINETAccount* account;
	CINETAccount temp;
	if (mLocalAddress)
		account = &static_cast<CPreferences*>(data)->mLocalAdbkAccount.Value();
	else if (mDisconnected)
	{
		temp.GetCWD().SetName(static_cast<CPreferences*>(data)->mDisconnectedCWD.GetValue());
		account = &temp;
	}
	else
		account = static_cast<CINETAccount*>(data);

	SetPath(account->GetCWD().GetName());
	
	mUseDefault = account->GetCWD().GetName().empty() ? 0 : 1;
	SetUseLocal(!account->GetCWD().GetName().empty());

	// Set it in popup
	CMailAccount* lacct = dynamic_cast<CMailAccount*>(account);
	if (lacct && (lacct->GetServerType() == CINETAccount::eLocal))
		mFileFormatPopup.SetValue(IDM_FILEFORMAT_AUTOMATIC + lacct->GetEndl());
	else
	{
		mFileFormatPopup.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_PREFS_ACCOUNT_Local_FILEFORMAT_TITLE)->ShowWindow(SW_HIDE);
	}
}

// Force update of data
bool CPrefsLocalOptions::UpdateContent(void* data)
{
	CINETAccount* account;
	CINETAccount temp;
	if (mLocalAddress)
		account = &static_cast<CPreferences*>(data)->mLocalAdbkAccount.Value();
	else if (mDisconnected)
		account = &temp;
	else
		account = static_cast<CINETAccount*>(data);

	// Copy info from panel into prefs
	if (mSpecifyCtrl.GetCheck())
		account->GetCWD().SetName(mPath);
	else
		account->GetCWD().SetName(cdstring::null_str);

	CMailAccount* lacct = dynamic_cast<CMailAccount*>(account);
	if (lacct && (lacct->GetServerType() == CINETAccount::eLocal))
		lacct->SetEndl(static_cast<EEndl>(mFileFormatPopup.GetValue() - IDM_FILEFORMAT_AUTOMATIC));

	if (mDisconnected)
		static_cast<CPreferences*>(data)->mDisconnectedCWD.SetValue(temp.GetCWD().GetName());
	
	return true;
}

void CPrefsLocalOptions::SetUseLocal(bool use)
{
	GetDlgItem(IDC_PREFS_ACCOUNT_Local_PATH)->EnableWindow(use);
	GetDlgItem(IDC_PREFS_ACCOUNT_Local_CHOOSE)->EnableWindow(use);
	GetDlgItem(IDC_PREFS_ACCOUNT_Local_RELATIVE)->EnableWindow(use);
	GetDlgItem(IDC_PREFS_ACCOUNT_Local_ABSOLUTE)->EnableWindow(use);
}

void CPrefsLocalOptions::OnPrefsAccountLocalUseDefault() 
{
	SetUseLocal(false);
}

void CPrefsLocalOptions::OnPrefsAccountLocalSpecify() 
{
	SetUseLocal(true);
}

void CPrefsLocalOptions::OnPrefsAccountLocalChoose() 
{
	DoChooseLocalFolder();
}

void CPrefsLocalOptions::OnPrefsAccountLocalRelative() 
{
	if (!mRelativeCtrl.GetCheck())
		SetRelative();
}

void CPrefsLocalOptions::OnPrefsAccountLocalAbsolute() 
{
	if (!mAbsoluteCtrl.GetCheck())
		SetAbsolute();
}

void CPrefsLocalOptions::SetPath(const char* path)
{
	// Get CWD
	cdstring cwd = CConnectionManager::sConnectionManager.GetCWD();
	
	// Does path start with this?
	if (IsRelativePath(path) || !::strncmp(path, cwd, cwd.length()))
	{
		// Set relative mode
		mRelativeCtrl.SetCheck(1);
		mAbsoluteCtrl.SetCheck(0);
		
		mPath = IsRelativePath(path) ? path : &path[cwd.length() - 1];
		mPathCtrl.SetWindowText(mPath.win_str());
	}
	else
	{
		// Set relative mode
		mAbsoluteCtrl.SetCheck(1);
		mRelativeCtrl.SetCheck(0);
		
		mPath = path;
		CUnicodeUtils::SetWindowTextUTF8(&mPathCtrl, path);
	}
}

// Choose default folder
void CPrefsLocalOptions::DoChooseLocalFolder(void)
{
	cdstring temp;
	if (BrowseForFolder("Choose a Local Mailbox directory", temp, this))
		SetPath(temp);
}

void CPrefsLocalOptions::SetRelative()
{
	// Get current path
	CString copyStr;
	mPathCtrl.GetWindowText(copyStr);
	cdstring path = copyStr;

	// Get CWD
	cdstring cwd = CConnectionManager::sConnectionManager.GetCWD();
	
	// Path must start with CWD
	if (!::strncmp(path, cwd, cwd.length()))
	{
		mPath = &path.c_str()[cwd.length() - 1];
		mPathCtrl.SetWindowText(mPath.win_str());
		mRelativeCtrl.SetCheck(1);
		mAbsoluteCtrl.SetCheck(0);
	}
	else
	{
		// Stop relative mode
		mAbsoluteCtrl.SetCheck(1);
		mRelativeCtrl.SetCheck(0);
	}
}

void CPrefsLocalOptions::SetAbsolute()
{
	// Get current path
	CString copyStr;
	mPathCtrl.GetWindowText(copyStr);
	cdstring path = copyStr;

	// Get CWD
	cdstring cwd = CConnectionManager::sConnectionManager.GetCWD();
	
	// Path must be relative
	if (IsRelativePath(path))
	{
		mPath = cwd;
		mPath += (path.c_str() + 1);
		mPathCtrl.SetWindowText(mPath.win_str());
		mAbsoluteCtrl.SetCheck(1);
		mRelativeCtrl.SetCheck(0);
	}
	else
	{
		// Stop absolute mode
		mRelativeCtrl.SetCheck(1);
		mAbsoluteCtrl.SetCheck(0);
	}
}

void CPrefsLocalOptions::OnFileFormatPopup(UINT nID)
{
	mFileFormatPopup.SetValue(nID);
}
