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


// CPrefsLDAPAccount.cpp : implementation file
//

#include "CPrefsLDAPAccount.h"

#include "CPrefsAccountAuth.h"
#include "CPrefsAddressLDAPOptions.h"
#include "CPrefsAddressLDAP1.h"
#include "CPrefsAddressLDAP2.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsLDAPAccount dialog


CPrefsLDAPAccount::CPrefsLDAPAccount()
	: CPrefsAccountPanel(CPrefsLDAPAccount::IDD, IDC_PREFS_ACCOUNT_ADDRESS_TABS)
{
	//{{AFX_DATA_INIT(CPrefsLDAPAccount)
	//}}AFX_DATA_INIT
}

BEGIN_MESSAGE_MAP(CPrefsLDAPAccount, CPrefsAccountPanel)
	//{{AFX_MSG_MAP(CPrefsLDAPAccount)
	ON_NOTIFY(TCN_SELCHANGE, IDC_PREFS_ACCOUNT_ADDRESS_TABS, OnSelChangePrefsAccountTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsLDAPAccount message handlers

void CPrefsLDAPAccount::InitTabs()
{
	// Create tab panels
	CPrefsAccountAuth* auth = new CPrefsAccountAuth;
	mTabs.AddPanel(auth);
	CPrefsAddressLDAPOptions* address_ldapoptions = new CPrefsAddressLDAPOptions;
	mTabs.AddPanel(address_ldapoptions);
	CPrefsAddressLDAP1* address_ldap1 = new CPrefsAddressLDAP1;
	mTabs.AddPanel(address_ldap1);
	CPrefsAddressLDAP2* address_ldap2 = new CPrefsAddressLDAP2;
	mTabs.AddPanel(address_ldap2);
}
