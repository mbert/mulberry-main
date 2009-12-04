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


// Source for CPrefsAddressLDAP1 class

#include "CPrefsAddressLDAP1.h"

#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CTextFieldX.h"


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddressLDAP1::CPrefsAddressLDAP1()
{
}

// Constructor from stream
CPrefsAddressLDAP1::CPrefsAddressLDAP1(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAddressLDAP1::~CPrefsAddressLDAP1()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressLDAP1::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLDAPRoot = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPRoot);
	mLDAPName = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPName);
	mLDAPEmail = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPEmail);
	mLDAPNickName = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPNickName);
	mLDAPAddress = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPAddress);
	mLDAPCompany = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPCompany);

	// Lock out fields
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mLDAPRoot->Disable();
		mLDAPName->Disable();
		mLDAPEmail->Disable();
		mLDAPNickName->Disable();
		mLDAPAddress->Disable();
		mLDAPCompany->Disable();
	}
}

// Set prefs
void CPrefsAddressLDAP1::SetData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	mLDAPRoot->SetText(account->GetLDAPRoot());
	mLDAPName->SetText(account->GetLDAPMatch(CAdbkAddress::eName));
	mLDAPEmail->SetText(account->GetLDAPMatch(CAdbkAddress::eEmail));
	mLDAPNickName->SetText(account->GetLDAPMatch(CAdbkAddress::eNickName));
	mLDAPAddress->SetText(account->GetLDAPMatch(CAdbkAddress::eAddress));
	mLDAPCompany->SetText(account->GetLDAPMatch(CAdbkAddress::eCompany));
}

// Force update of prefs
void CPrefsAddressLDAP1::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	account->SetLDAPRoot(mLDAPRoot->GetText());
	account->SetLDAPMatch(CAdbkAddress::eName, mLDAPName->GetText());
	account->SetLDAPMatch(CAdbkAddress::eEmail, mLDAPEmail->GetText());
	account->SetLDAPMatch(CAdbkAddress::eNickName, mLDAPNickName->GetText());
	account->SetLDAPMatch(CAdbkAddress::eAddress, mLDAPAddress->GetText());
	account->SetLDAPMatch(CAdbkAddress::eCompany, mLDAPCompany->GetText());
}
