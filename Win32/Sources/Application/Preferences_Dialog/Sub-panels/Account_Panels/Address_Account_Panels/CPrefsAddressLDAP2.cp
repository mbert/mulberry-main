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


// CPrefsAddressLDAP2.cpp : implementation file
//

#include "CPrefsAddressLDAP2.h"

#include "CAdbkAddress.h"
#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAP2 dialog

IMPLEMENT_DYNAMIC(CPrefsAddressLDAP2, CTabPanel)

CPrefsAddressLDAP2::CPrefsAddressLDAP2()
	: CTabPanel(CPrefsAddressLDAP2::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddressLDAP2)
	mLDAPPut = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAddressLDAP2::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddressLDAP2)
	DDX_UTF8Text(pDX, IDC_LDAPWORK, mLDAPWork);
	DDX_UTF8Text(pDX, IDC_LDAPHOME, mLDAPHome);
	DDX_UTF8Text(pDX, IDC_LDAPFAX, mLDAPFax);
	DDX_UTF8Text(pDX, IDC_LDAPURL, mLDAPURLs);
	DDX_UTF8Text(pDX, IDC_LDAPNOTES, mLDAPNotes);
	DDX_Control(pDX, IDC_LDAPNOTES, mLDAPNotesCtrl);
	DDX_Check(pDX, IDC_LDAPPUTNOTES, mLDAPPut);
	DDX_Control(pDX, IDC_LDAPPUTNOTES, mLDAPPutBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddressLDAP2, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAddressLDAP2)
	ON_BN_CLICKED(IDC_LDAPPUTNOTES, OnLDAPPut)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAP2 message handlers

// Set data
void CPrefsAddressLDAP2::SetContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info
	mLDAPWork = account->GetLDAPMatch(CAdbkAddress::ePhoneWork);
	mLDAPHome = account->GetLDAPMatch(CAdbkAddress::ePhoneHome);
	mLDAPFax = account->GetLDAPMatch(CAdbkAddress::eFax);
	mLDAPURLs = account->GetLDAPMatch(CAdbkAddress::eURL);
	mLDAPNotes = account->GetLDAPMatch(CAdbkAddress::eNotes);
	mLDAPPut = account->GetLDAPNotes();
	if (account->GetLDAPNotes())
		mLDAPNotesCtrl.EnableWindow(false);

	// Lock out allfields
	if (CAdminLock::sAdminLock.mLockServerAddress)
		EnableWindow(false);
}

// Force update of data
bool CPrefsAddressLDAP2::UpdateContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info from panel into prefs
	account->SetLDAPMatch(CAdbkAddress::ePhoneWork, mLDAPWork);
	account->SetLDAPMatch(CAdbkAddress::ePhoneHome, mLDAPHome);
	account->SetLDAPMatch(CAdbkAddress::eFax, mLDAPFax);
	account->SetLDAPMatch(CAdbkAddress::eURL, mLDAPURLs);
	account->SetLDAPMatch(CAdbkAddress::eNotes, mLDAPNotes);
	account->SetLDAPNotes(mLDAPPut);
	
	return true;
}

void CPrefsAddressLDAP2::OnLDAPPut()
{
	mLDAPNotesCtrl.EnableWindow(!mLDAPPutBtn.GetCheck());
}

