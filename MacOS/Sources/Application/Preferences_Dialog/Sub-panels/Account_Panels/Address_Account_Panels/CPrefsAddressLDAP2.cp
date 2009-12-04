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
#include "CTextFieldX.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddressLDAP2::CPrefsAddressLDAP2()
{
}

// Constructor from stream
CPrefsAddressLDAP2::CPrefsAddressLDAP2(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAddressLDAP2::~CPrefsAddressLDAP2()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressLDAP2::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mLDAPPhoneWork = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPPhoneWork);
	mLDAPPhoneHome = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPPhoneHome);
	mLDAPFax = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPFax);
	mLDAPURLs = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPURLs);
	mLDAPNotes = (CTextFieldX*) FindPaneByID(paneid_LDAPAccountLDAPNotes);
	mLDAPPut = (LCheckBox*) FindPaneByID(paneid_LDAPAccountLDAPPut);

	// Lock out fields
	if (CAdminLock::sAdminLock.mLockServerAddress)
	{
		mLDAPPhoneWork->Disable();
		mLDAPPhoneHome->Disable();
		mLDAPFax->Disable();
		mLDAPURLs->Disable();
		mLDAPNotes->Disable();
		mLDAPPut->Disable();
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsAddressLDAP2Btns);
}

// Handle buttons
void CPrefsAddressLDAP2::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_LDAPAccountLDAPPut:
		if (*(long*) ioParam)
			mLDAPNotes->Disable();
		else
			mLDAPNotes->Enable();
		break;
	}
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
	mLDAPPut->SetValue(account->GetLDAPNotes());
	if (account->GetLDAPNotes())
		mLDAPNotes->Disable();
}

// Force update of prefs
void CPrefsAddressLDAP2::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	account->SetLDAPMatch(CAdbkAddress::ePhoneWork, mLDAPPhoneWork->GetText());
	account->SetLDAPMatch(CAdbkAddress::ePhoneHome, mLDAPPhoneHome->GetText());
	account->SetLDAPMatch(CAdbkAddress::eFax, mLDAPFax->GetText());
	account->SetLDAPMatch(CAdbkAddress::eURL, mLDAPURLs->GetText());
	account->SetLDAPMatch(CAdbkAddress::eNotes, mLDAPNotes->GetText());
	account->SetLDAPNotes(mLDAPPut->GetValue()==1);
}
