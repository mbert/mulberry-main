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


// CGetPassphraseDialog.cp : implementation file
//


#include "CGetPassphraseDialog.h"

#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

/////////////////////////////////////////////////////////////////////////////
// CRenameMailboxDialog dialog


CGetPassphraseDialog::CGetPassphraseDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CGetPassphraseDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CRenameMailboxDialog)
	mPassphrase1 = _T("");
	mPassphrase2 = _T("");
	mHideTyping = TRUE;
	//}}AFX_DATA_INIT
}


void CGetPassphraseDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGetPassphraseDialog)
	DDX_Control(pDX, IDC_PASSPHRASE1_TEXT, mPassphrase1Ctrl);
	DDX_UTF8Text(pDX, IDC_PASSPHRASE1_TEXT, mPassphrase1);
	DDX_Control(pDX, IDC_PASSPHRASE2_TEXT, mPassphrase2Ctrl);
	DDX_UTF8Text(pDX, IDC_PASSPHRASE2_TEXT, mPassphrase2);
	//DDX_Control(pDX, IDC_PASSPHRASE_KEYPOPUP, mKeyPopup);
	DDX_Check(pDX, IDC_PASSPHRASE_HIDETYPING, mHideTyping);
	DDX_Control(pDX, IDC_PASSPHRASE_HIDETYPING, mHideTypingCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CGetPassphraseDialog, CDialog)
	//{{AFX_MSG_MAP(CGetPassphraseDialog)
	ON_BN_CLICKED(IDC_PASSPHRASE_HIDETYPING, OnHideTyping)
	ON_COMMAND_RANGE(IDM_PswdChangeStart, IDM_PswdChangeStop, OnKeyPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGetPassphraseDialog message handlers

BOOL CGetPassphraseDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Subclass buttons
	mKeyPopup.SubclassDlgItem(IDC_PASSPHRASE_KEYPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mKeyPopup.SetMenu(IDR_POPUP_PLUGINS);
	InitKeysMenu();

	mPassphrase2Ctrl.ShowWindow(SW_HIDE);

	return true;
}

void CGetPassphraseDialog::InitKeysMenu(void) 
{
	if (mKeys)
	{
		CMenu* pPopup = mKeyPopup.GetPopupMenu();

		// Remove any existing items
		short num_menu = pPopup->GetMenuItemCount();
		for(short i = 0; i < num_menu; i++)
			pPopup->RemoveMenu(0, MF_BYPOSITION);

		// Add each mail account
		// Now add current items
		int menu_id = IDM_PswdChangeStart;
		const char** p = mKeys;
		while(*p)
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, *p++);
		
		// Refresh its display
		mKeyPopup.SetValue(IDM_PswdChangeStart);
		mKeyPopupText = CUnicodeUtils::GetWindowTextUTF8(&mKeyPopup);
	}
	else
	{
		GetDlgItem(IDC_PASSPHRASE_KEYPOPUPTITLE)->ShowWindow(SW_HIDE);
		mKeyPopup.ShowWindow(SW_HIDE);
	}
}

void CGetPassphraseDialog::OnHideTyping() 
{
	if (mHideTypingCtrl.GetCheck())
	{
		cdstring temp = CUnicodeUtils::GetWindowTextUTF8(&mPassphrase2Ctrl);
		CUnicodeUtils::SetWindowTextUTF8(&mPassphrase1Ctrl, temp);
		mPassphrase2Ctrl.ShowWindow(SW_HIDE);
		mPassphrase1Ctrl.ShowWindow(SW_SHOW);
	}
	else
	{
		cdstring temp = CUnicodeUtils::GetWindowTextUTF8(&mPassphrase1Ctrl);
		CUnicodeUtils::SetWindowTextUTF8(&mPassphrase2Ctrl, temp);
		mPassphrase1Ctrl.ShowWindow(SW_HIDE);
		mPassphrase2Ctrl.ShowWindow(SW_SHOW);
	}
}

void CGetPassphraseDialog::OnKeyPopup(UINT nID) 
{
	mKeyPopup.SetValue(nID);
	mKeyPopupText = CUnicodeUtils::GetWindowTextUTF8(&mKeyPopup);
}

bool CGetPassphraseDialog::PoseDialog(cdstring& passphrase, const char* title)
{
	cdstring dummy1;
	unsigned long dummy2;
	return PoseDialog(passphrase, NULL, dummy1, dummy2, title);
}

bool CGetPassphraseDialog::PoseDialog(cdstring& change, const char** keys, cdstring& chosen_key, unsigned long& index, const char* title)
{
	CGetPassphraseDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mPassphrase1 = change;
	dlog.mPassphrase2 = change;
	dlog.mKeys = keys;
	if (title != NULL)
		CUnicodeUtils::SetWindowTextUTF8(&dlog, rsrc::GetString(title));

	if (dlog.DoModal() == IDOK)
	{
		change = dlog.mHideTyping ? dlog.mPassphrase1 : dlog.mPassphrase2;
		chosen_key = dlog.mKeyPopupText;
		index = dlog.mKeyPopup.GetValue() - IDM_PswdChangeStart;
		
		::memset((void*) dlog.mPassphrase1.c_str_mod(), 'x', dlog.mPassphrase1.length());
		::memset((void*) dlog.mPassphrase2.c_str_mod(), 'x', dlog.mPassphrase2.length());
		return true;
	}
	else
		return false;
}
