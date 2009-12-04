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


// CPrefsAddressExpansion.cpp : implementation file
//

#include "CPrefsAddressExpansion.h"

#include "CPreferences.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressExpansion dialog

IMPLEMENT_DYNAMIC(CPrefsAddressExpansion, CTabPanel)

CPrefsAddressExpansion::CPrefsAddressExpansion()
	: CTabPanel(CPrefsAddressExpansion::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddressExpansion)
	mExpandNoNicknames = FALSE;
	mExpandFailedNicknames = FALSE;
	mExpandFullName = FALSE;
	mExpandNickName = FALSE;
	mExpandEmail = FALSE;
	mSkipLDAP = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAddressExpansion::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddressExpansion)
	DDX_Check(pDX, IDC_PREFS_ADDRESS_EXPANDNONICKNAMES, mExpandNoNicknames);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_EXPANDFAILEDNICKNAMES, mExpandFailedNicknames);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_EXPANDFULLNAME, mExpandFullName);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_EXPANDNICKNAME, mExpandNickName);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_EXPANDEMAIL, mExpandEmail);
	DDX_Check(pDX, IDC_PREFS_ADDRESS_SKIPLDAP, mSkipLDAP);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddressExpansion, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAddressExpansion)
	ON_BN_CLICKED(IDC_PREFS_ADDRESS_EXPANDNONICKNAMES, OnExpandNoNicknames)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressExpansion message handlers

// Set data
void CPrefsAddressExpansion::SetContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mExpandNoNicknames = copyPrefs->mExpandNoNicknames.GetValue();
	mExpandFailedNicknames = copyPrefs->mExpandFailedNicknames.GetValue();
	mExpandFullName = copyPrefs->mExpandFullName.GetValue();
	mExpandNickName = copyPrefs->mExpandNickName.GetValue();
	mExpandEmail = copyPrefs->mExpandEmail.GetValue();
	mSkipLDAP = copyPrefs->mSkipLDAP.GetValue();
	
	GetDlgItem(IDC_PREFS_ADDRESS_EXPANDFAILEDNICKNAMES)->EnableWindow(!mExpandNoNicknames);
}

// Force update of data
bool CPrefsAddressExpansion::UpdateContent(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mExpandNoNicknames.SetValue(mExpandNoNicknames);
	copyPrefs->mExpandFailedNicknames.SetValue(mExpandFailedNicknames);
	copyPrefs->mExpandFullName.SetValue(mExpandFullName);
	copyPrefs->mExpandNickName.SetValue(mExpandNickName);
	copyPrefs->mExpandEmail.SetValue(mExpandEmail);
	copyPrefs->mSkipLDAP.SetValue(mSkipLDAP);
	
	return true;
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressExpansion message handlers

void CPrefsAddressExpansion::OnExpandNoNicknames()
{
	// TODO: Add your control notification handler code here
	GetDlgItem(IDC_PREFS_ADDRESS_EXPANDFAILEDNICKNAMES)->EnableWindow(!static_cast<CButton*>(GetDlgItem(IDC_PREFS_ADDRESS_EXPANDNONICKNAMES))->GetCheck());
}

