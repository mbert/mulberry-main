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


// CPrefsAddressLDAPOptions.cpp : implementation file
//

#include "CPrefsAddressLDAPOptions.h"

#include "CAdbkAddress.h"
#include "CAddressAccount.h"
#include "CAdminLock.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAPOptions dialog

IMPLEMENT_DYNAMIC(CPrefsAddressLDAPOptions, CTabPanel)

CPrefsAddressLDAPOptions::CPrefsAddressLDAPOptions()
	: CTabPanel(CPrefsAddressLDAPOptions::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAddressLDAPOptions)
	mLDAPUseExpansion = FALSE;
	mLDAPSearch = FALSE;
	//}}AFX_DATA_INIT
}


void CPrefsAddressLDAPOptions::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAddressLDAPOptions)
	DDX_Check(pDX, IDC_LDAPUSEEXPANSION, mLDAPUseExpansion);
	DDX_Check(pDX, IDC_LDAPSEARCH, mLDAPSearch);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsAddressLDAPOptions, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAddressLDAPOptions)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddressLDAPOptions message handlers

// Set data
void CPrefsAddressLDAPOptions::SetContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info
	mLDAPUseExpansion = account->GetLDAPUseExpansion();
	mLDAPSearch = account->GetLDAPSearch();
}

// Force update of data
bool CPrefsAddressLDAPOptions::UpdateContent(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info from panel into prefs
	account->SetLDAPUseExpansion(mLDAPUseExpansion);
	account->SetLDAPSearch(mLDAPSearch);
	
	return true;
}
