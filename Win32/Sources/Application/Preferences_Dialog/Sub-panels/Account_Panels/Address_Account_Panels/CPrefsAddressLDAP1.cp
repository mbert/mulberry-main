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


// CPrefsAddressLDAP1.cpp : implementation file
//

#include "CPrefsAddressLDAP1.h"

#include "CAdbkAddress.h"
#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAP1 dialog

IMPLEMENT_DYNAMIC(CPrefsAddressLDAP1, CTabPanel)

CPrefsAddressLDAP1::CPrefsAddressLDAP1()
	: CTabPanel(CPrefsAddressLDAP1::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddressLDAP1)
	//}}AFX_DATA_INIT
}


void CPrefsAddressLDAP1::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddressLDAP1)
	DDX_UTF8Text(pDX, IDC_LDAPROOT, mLDAPRoot);
	DDX_UTF8Text(pDX, IDC_LDAPNAME, mLDAPName);
	DDX_UTF8Text(pDX, IDC_LDAPEMAIL, mLDAPEmail);
	DDX_UTF8Text(pDX, IDC_LDAPNICKNAME, mLDAPNickName);
	DDX_UTF8Text(pDX, IDC_LDAPADDR, mLDAPAddr);
	DDX_UTF8Text(pDX, IDC_LDAPCOMPANY, mLDAPCompany);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddressLDAP1, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAddressLDAP1)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAP1 message handlers

// Set data
void CPrefsAddressLDAP1::SetContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info
	mLDAPRoot = account->GetLDAPRoot();
	mLDAPName = account->GetLDAPMatch(CAdbkAddress::eName);
	mLDAPEmail = account->GetLDAPMatch(CAdbkAddress::eEmail);
	mLDAPNickName = account->GetLDAPMatch(CAdbkAddress::eNickName);
	mLDAPAddr = account->GetLDAPMatch(CAdbkAddress::eAddress);
	mLDAPCompany = account->GetLDAPMatch(CAdbkAddress::eCompany);

	// Lock out allfields
	if (CAdminLock::sAdminLock.mLockServerAddress)
		EnableWindow(false);
}

// Force update of data
bool CPrefsAddressLDAP1::UpdateContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info from panel into prefs
	account->SetLDAPRoot(mLDAPRoot);
	account->SetLDAPMatch(CAdbkAddress::eName, mLDAPName);
	account->SetLDAPMatch(CAdbkAddress::eEmail, mLDAPEmail);
	account->SetLDAPMatch(CAdbkAddress::eNickName, mLDAPNickName);
	account->SetLDAPMatch(CAdbkAddress::eAddress, mLDAPAddr);
	account->SetLDAPMatch(CAdbkAddress::eCompany, mLDAPCompany);
	
	return true;
}
