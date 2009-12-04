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


// Source for CAddressContactPanel class

#include "CAddressContactPanel.h"

#include "CAdbkAddress.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressContactPanel::CAddressContactPanel(LStream *inStream)
		: CAddressPanelBase(inStream)
{
}

// Default destructor
CAddressContactPanel::~CAddressContactPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressContactPanel::FinishCreateSelf(void)
{
	// Do inherited
	CAddressPanelBase::FinishCreateSelf();

	// Get fields
	mAddress = (CTextDisplay*) FindPaneByID(paneid_AddressContactAddress);
	mURL = (CTextDisplay*) FindPaneByID(paneid_AddressContactURL);
	mPhoneWork = (CTextFieldX*) FindPaneByID(paneid_AddressContactPhoneWork);
	mPhoneHome = (CTextFieldX*) FindPaneByID(paneid_AddressContactPhoneHome);
	mFax = (CTextFieldX*) FindPaneByID(paneid_AddressContactFax);
}

void CAddressContactPanel::Focus()
{
	LCommander::SwitchTarget(mAddress);
}

// Set fields in dialog
void CAddressContactPanel::SetFields(const CAdbkAddress* addr)
{
	mAddress->SetText(addr ? const_cast<cdstring&>(addr->GetAddress(CAdbkAddress::eDefaultAddressType)) : cdstring::null_str);

	cdstring txt;
	if (addr)
		txt = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
	mPhoneWork->SetText(txt);

	if (addr)
		txt = addr->GetPhone(CAdbkAddress::eHomePhoneType);
	mPhoneHome->SetText(txt);

	if (addr)
		txt = addr->GetPhone(CAdbkAddress::eFaxType);
	mFax->SetText(txt);

	mURL->SetText(addr ? const_cast<cdstring&>(addr->GetURL()) : cdstring::null_str);
}

// Get fields from dialog
bool CAddressContactPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	bool done_edit = false;

	{
		// Copy text and null terminate
		cdstring atxt;
		mAddress->GetText(atxt);

		// Give to address if not same as previous
		if (addr->GetAddress(CAdbkAddress::eDefaultAddressType) != atxt)
		{
			addr->SetAddress(atxt, CAdbkAddress::eDefaultAddressType);
			done_edit = true;
		}

	}

	cdstring txt = mPhoneWork->GetText();
	if (addr->GetPhone(CAdbkAddress::eWorkPhoneType) != txt)
	{
		addr->SetPhone(txt, CAdbkAddress::eWorkPhoneType);
		done_edit = true;
	}

	txt = mPhoneHome->GetText();
	if (addr->GetPhone(CAdbkAddress::eHomePhoneType) != txt)
	{
		addr->SetPhone(txt, CAdbkAddress::eHomePhoneType);
		done_edit = true;
	}

	txt = mFax->GetText();
	if (addr->GetPhone(CAdbkAddress::eFaxType) != txt)
	{
		addr->SetPhone(txt, CAdbkAddress::eFaxType);
		done_edit = true;
	}

	{
		// Copy text and null terminate
		cdstring atxt;
		mURL->GetText(atxt);

		// Give to URL if not same as previous
		if (addr->GetURL() != atxt)
		{
			addr->SetURL(atxt);
			done_edit = true;
		}
	}

	return done_edit;
}
