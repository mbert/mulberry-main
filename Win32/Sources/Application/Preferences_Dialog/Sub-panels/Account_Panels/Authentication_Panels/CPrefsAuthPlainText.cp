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


// CPrefsAuthPlainText.cpp : implementation file
//

#include "CPrefsAuthPlainText.h"

#include "CAdminLock.h"
#include "CAuthenticator.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAuthPlainText dialog

IMPLEMENT_DYNAMIC(CPrefsAuthPlainText, CTabPanel)

CPrefsAuthPlainText::CPrefsAuthPlainText()
	: CTabPanel(CPrefsAuthPlainText::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAuthPlainText)
	mSaveUser = FALSE;
	mSavePswd = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAuthPlainText::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAuthPlainText)
	DDX_UTF8Text(pDX, IDC_PREFS_AUTHENTICATE_PLAIN_UID, mUID);
	DDX_Check(pDX, IDC_PREFS_AUTHENTICATE_PLAIN_SAVEUSER, mSaveUser);
	DDX_Check(pDX, IDC_PREFS_AUTHENTICATE_PLAIN_SAVEPSWD, mSavePswd);
	DDX_Control(pDX, IDC_PREFS_AUTHENTICATE_PLAIN_SAVEPSWD, mSavePswdCtrl);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAuthPlainText, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAuthPlainText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAuthPlainText message handlers

BOOL CPrefsAuthPlainText::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Disable button
	if (CAdminLock::sAdminLock.mLockSavePswd)
		mSavePswdCtrl.EnableWindow(false);

	return true;
}

// Set data
void CPrefsAuthPlainText::SetContent(void* data)
{
	CAuthenticator* auth = (CAuthenticator*) data;

	// Copy info
	mUID = ((CAuthenticatorUserPswd*) auth)->GetUID();
	mSaveUser = ((CAuthenticatorUserPswd*) auth)->GetSaveUID();
	mSavePswd = ((CAuthenticatorUserPswd*) auth)->GetSavePswd();
}

// Force update of data
bool CPrefsAuthPlainText::UpdateContent(void* data)
{
	CAuthenticator* auth = (CAuthenticator*) data;

	// Copy info from panel into prefs
	((CAuthenticatorUserPswd*) auth)->SetUID(mUID);
	((CAuthenticatorUserPswd*) auth)->SetSaveUID(mSaveUser);
	((CAuthenticatorUserPswd*) auth)->SetSavePswd(mSavePswd && !CAdminLock::sAdminLock.mLockSavePswd);

	return true;
}
