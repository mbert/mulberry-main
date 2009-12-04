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


// Source for CPrefsAddressLDAP2 class

#include "CPrefsAddressLDAP2.h"

#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressLDAP2::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Work Phone:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 80,20);
    assert( obj1 != NULL );

    mLDAPPhoneWork =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,10, 255,20);
    assert( mLDAPPhoneWork != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Home Phone:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,35, 85,20);
    assert( obj2 != NULL );

    mLDAPPhoneHome =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,35, 255,20);
    assert( mLDAPPhoneHome != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Fax:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 75,20);
    assert( obj3 != NULL );

    mLDAPFax =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,60, 255,20);
    assert( mLDAPFax != NULL );

    JXStaticText* obj4 =
        new JXStaticText("URLs:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,85, 60,20);
    assert( obj4 != NULL );

    mLDAPURLs =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,85, 255,20);
    assert( mLDAPURLs != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Notes:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,110, 65,20);
    assert( obj5 != NULL );

    mLDAPNotes =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 95,110, 255,20);
    assert( mLDAPNotes != NULL );

    mLDAPPut =
        new JXTextCheckbox("Put All Attributes in Notes", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,145, 180,20);
    assert( mLDAPPut != NULL );

// end JXLayout1

	// Lock out fields
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mLDAPPhoneWork->Deactivate();
		mLDAPPhoneHome->Deactivate();
		mLDAPFax->Deactivate();
		mLDAPURLs->Deactivate();
		mLDAPNotes->Deactivate();
		mLDAPPut->Deactivate();
	}

	// Start listening
	ListenTo(mLDAPPut);
}

// Handle buttons
void CPrefsAddressLDAP2::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mLDAPPut)
		{
			if (mLDAPPut->IsChecked())
				mLDAPNotes->Deactivate();
			else
				mLDAPNotes->Activate();
			return;
		}
	}

	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAddressLDAP2::SetData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	mLDAPPhoneWork->SetText(account->GetLDAPMatch(CAdbkAddress::ePhoneWork));
	mLDAPPhoneHome->SetText(account->GetLDAPMatch(CAdbkAddress::ePhoneHome));
	mLDAPFax->SetText(account->GetLDAPMatch(CAdbkAddress::eFax));
	mLDAPURLs->SetText(account->GetLDAPMatch(CAdbkAddress::eURL));
	mLDAPNotes->SetText(account->GetLDAPMatch(CAdbkAddress::eNotes));
	mLDAPPut->SetState(JBoolean(account->GetLDAPNotes()));
	if (account->GetLDAPNotes())
		mLDAPNotes->Deactivate();
}

// Force update of prefs
bool CPrefsAddressLDAP2::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	account->SetLDAPMatch(CAdbkAddress::ePhoneWork, mLDAPPhoneWork->GetText());
	account->SetLDAPMatch(CAdbkAddress::ePhoneHome, mLDAPPhoneHome->GetText());
	account->SetLDAPMatch(CAdbkAddress::eFax, mLDAPFax->GetText());
	account->SetLDAPMatch(CAdbkAddress::eURL, mLDAPURLs->GetText());
	account->SetLDAPMatch(CAdbkAddress::eNotes, mLDAPNotes->GetText());
	account->SetLDAPNotes(mLDAPPut->IsChecked());
	
	return true;
}
