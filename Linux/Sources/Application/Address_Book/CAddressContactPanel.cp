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
#include "CTextField.h"

#include <JXStaticText.h>

#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressContactPanel::CAddressContactPanel(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
		: CAddressPanelBase(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CAddressContactPanel::~CAddressContactPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressContactPanel::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Address:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,12, 60,20);
    assert( obj1 != NULL );

    mAddress =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 90,10, 400,70);
    assert( mAddress != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Phone Work:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,92, 85,20);
    assert( obj2 != NULL );

    mPhoneWork =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 90,90, 400,20);
    assert( mPhoneWork != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Phone Home:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,115, 85,20);
    assert( obj3 != NULL );

    mPhoneHome =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 90,115, 400,20);
    assert( mPhoneHome != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Fax:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,142, 85,20);
    assert( obj4 != NULL );

    mFax =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 90,140, 400,20);
    assert( mFax != NULL );

    JXStaticText* obj5 =
        new JXStaticText("URLs:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,172, 55,20);
    assert( obj5 != NULL );

    mURL =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 90,170, 400,70);
    assert( mURL != NULL );

// end JXLayout1
}

void CAddressContactPanel::Focus()
{
	mAddress->Focus();
}

// Set fields in dialog
void CAddressContactPanel::SetFields(const CAdbkAddress* addr)
{
	cdstring	txt;

	if (addr)
		txt = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
	mAddress->SetText(txt);

	if (addr)
		txt = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
	mPhoneWork->SetText(txt);

	if (addr)
		txt = addr->GetPhone(CAdbkAddress::eHomePhoneType);
	mPhoneHome->SetText(txt);

	if (addr)
		txt = addr->GetPhone(CAdbkAddress::eFaxType);
	mFax->SetText(txt);

	if (addr)
		txt = addr->GetURL();
	mURL->SetText(txt);
}

// Get fields from dialog
bool CAddressContactPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	cdstring txt;
	bool done_edit = false;

	txt = mAddress->GetText();
	if (addr->GetAddress(CAdbkAddress::eDefaultAddressType) != txt)
	{
		addr->SetAddress(txt, CAdbkAddress::eDefaultAddressType);
		done_edit = true;
	}

	txt = mPhoneWork->GetText();
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

	txt = mURL->GetText();
	if (addr->GetURL() != txt)
	{
		addr->SetURL(txt);
		done_edit = true;
	}

	return done_edit;
}
