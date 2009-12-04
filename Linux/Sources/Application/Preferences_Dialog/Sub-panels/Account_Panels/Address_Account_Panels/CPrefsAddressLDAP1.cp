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
#include "CTextField.h"

#include <JXStaticText.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressLDAP1::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Root:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 40,20);
    assert( obj1 != NULL );

    mLDAPRoot =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,10, 295,45);
    assert( mLDAPRoot != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Name:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,65, 45,20);
    assert( obj2 != NULL );

    mLDAPName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,65, 260,20);
    assert( mLDAPName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Email:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,90, 45,20);
    assert( obj3 != NULL );

    mLDAPEmail =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,90, 260,20);
    assert( mLDAPEmail != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Nick-Name:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 75,20);
    assert( obj4 != NULL );

    mLDAPNickName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,115, 260,20);
    assert( mLDAPNickName != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Address:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,140, 60,20);
    assert( obj5 != NULL );

    mLDAPAddress =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,140, 260,20);
    assert( mLDAPAddress != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Company:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,165, 65,20);
    assert( obj6 != NULL );

    mLDAPCompany =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,165, 260,20);
    assert( mLDAPCompany != NULL );

// end JXLayout1

	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mLDAPRoot->Deactivate();
		mLDAPName->Deactivate();
		mLDAPEmail->Deactivate();
		mLDAPNickName->Deactivate();
		mLDAPAddress->Deactivate();
		mLDAPCompany->Deactivate();
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
bool CPrefsAddressLDAP1::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	account->SetLDAPRoot(mLDAPRoot->GetText());
	account->SetLDAPMatch(CAdbkAddress::eName, mLDAPName->GetText());
	account->SetLDAPMatch(CAdbkAddress::eEmail, mLDAPEmail->GetText());
	account->SetLDAPMatch(CAdbkAddress::eNickName, mLDAPNickName->GetText());
	account->SetLDAPMatch(CAdbkAddress::eAddress, mLDAPAddress->GetText());
	account->SetLDAPMatch(CAdbkAddress::eCompany, mLDAPCompany->GetText());
	
	return true;
}
