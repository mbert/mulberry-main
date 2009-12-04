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


// Source for CAddressPersonalPanel class

#include "CAddressPersonalPanel.h"

#include "CAdbkAddress.h"
#include "CStringUtils.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <jXGlobals.h>

#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressPersonalPanel::CAddressPersonalPanel(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
		: CAddressPanelBase(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CAddressPersonalPanel::~CAddressPersonalPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressPersonalPanel::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Full Name:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,12, 70,20);
    assert( obj1 != NULL );

    mFullName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,10, 400,20);
    assert( mFullName != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Nick-Name:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,37, 80,20);
    assert( obj2 != NULL );

    mNickName =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,35, 400,20);
    assert( mNickName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Email:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,62, 55,20);
    assert( obj3 != NULL );

    mEmail =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,60, 400,20);
    assert( mEmail != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Calendar:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,87, 65,20);
    assert( obj4 != NULL );

    mCalendar =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,85, 400,20);
    assert( mCalendar != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Company:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,112, 65,20);
    assert( obj5 != NULL );

    mCompany =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 85,110, 400,20);
    assert( mCompany != NULL );

// end JXLayout1
}

void CAddressPersonalPanel::Focus()
{
	mFullName->Focus();
}

// Set fields in dialog
void CAddressPersonalPanel::SetFields(const CAdbkAddress* addr)
{
	cdstring txt;

	if (addr)
		txt = addr->GetName();
	mFullName->SetText(txt);

	if (addr)
		txt = addr->GetADL();
	mNickName->SetText(txt);

	if (addr)
		txt = addr->GetEmail(CAdbkAddress::eDefaultEmailType);
	mEmail->SetText(txt);

	if (addr)
		txt = addr->GetCalendar();
	mCalendar->SetText(txt);

	if (addr)
		txt = addr->GetCompany();
	mCompany->SetText(txt);
}

// Get fields from dialog
bool CAddressPersonalPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	cdstring txt;
	bool done_edit = false;

	txt = mFullName->GetText();
	if (addr->GetName() != txt)
	{
		addr->SetName(txt);
		done_edit = true;
	}

	txt = mNickName->GetText();
	::strreplace(txt.c_str_mod(), "@", '*');
	txt.trimspace();

	if (addr->GetADL() != txt)
	{
		addr->SetADL(txt);
		done_edit = true;
	}

	txt = mEmail->GetText();
	cdstring test_addr = addr->GetEmail(CAdbkAddress::eDefaultEmailType);
	if (test_addr != txt)
	{
		addr->CopyMailAddress(txt);
		done_edit = true;
	}

	txt = mCalendar->GetText();
	if (addr->GetCalendar() != txt)
	{
		addr->SetCalendar(txt);
		done_edit = true;
	}

	txt = mCompany->GetText();
	if (addr->GetCompany() != txt)
	{
		addr->SetCompany(txt);
		done_edit = true;
	}

	return done_edit;
}
