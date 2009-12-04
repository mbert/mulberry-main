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


// CEditAddressDialog.cpp : implementation file
//


#include "CEditAddressDialog.h"

#include "CAdbkAddress.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CEditAddressDialog dialog


CEditAddressDialog::CEditAddressDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

/////////////////////////////////////////////////////////////////////////////
// CEditAddressDialog message handlers

void CEditAddressDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 545,315, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 545,315);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Full Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,12, 70,20);
    assert( obj2 != NULL );

    mFullName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 200,20);
    assert( mFullName != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Nick-Name:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,37, 80,20);
    assert( obj3 != NULL );

    mNickName =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,35, 200,20);
    assert( mNickName != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Email:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,62, 55,20);
    assert( obj4 != NULL );

    mEmail =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,60, 200,20);
    assert( mEmail != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Calendar:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,87, 65,20);
    assert( obj5 != NULL );

    mCalendar =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,85, 200,20);
    assert( mCalendar != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Company:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,112, 65,20);
    assert( obj6 != NULL );

    mCompany =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,110, 200,20);
    assert( mCompany != NULL );

    JXStaticText* obj7 =
        new JXStaticText("Phone Work:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 295,12, 85,20);
    assert( obj7 != NULL );

    mPhoneWork =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 380,10, 150,20);
    assert( mPhoneWork != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Phone Home:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 295,35, 85,20);
    assert( obj8 != NULL );

    mPhoneHome =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 380,35, 150,20);
    assert( mPhoneHome != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Fax:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 295,62, 85,20);
    assert( obj9 != NULL );

    mFax =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 380,60, 150,20);
    assert( mFax != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Address:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,137, 60,20);
    assert( obj10 != NULL );

    mAddress =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,135, 260,50);
    assert( mAddress != NULL );

    JXStaticText* obj11 =
        new JXStaticText("URLs:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,192, 55,20);
    assert( obj11 != NULL );

    mURL =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,190, 260,50);
    assert( mURL != NULL );

    JXStaticText* obj12 =
        new JXStaticText("Notes:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,247, 55,20);
    assert( obj12 != NULL );

    mNotes =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,245, 260,50);
    assert( mNotes != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 365,280, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 455,280, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("Address Edit");
	SetButtons(mOKBtn, mCancelBtn);
}

void CEditAddressDialog::SetFields(CAdbkAddress* addr, bool allow_edit)
{
	mNickName->SetText(addr->GetADL());
	mFullName->SetText(addr->GetName());
	mEmail->SetText(addr->GetEmail(CAdbkAddress::eDefaultEmailType));
	mCalendar->SetText(addr->GetCalendar());
	mCompany->SetText(addr->GetCompany());
	mPhoneWork->SetText(addr->GetPhone(CAdbkAddress::eWorkPhoneType));
	mPhoneHome->SetText(addr->GetPhone(CAdbkAddress::eHomePhoneType));
	mFax->SetText(addr->GetPhone(CAdbkAddress::eFaxType));
	mAddress->SetText(addr->GetAddress(CAdbkAddress::eDefaultAddressType));
	mURL->SetText(addr->GetURL());
	mNotes->SetText(addr->GetNotes());
	
	if (!allow_edit)
	{
		mNickName->SetReadOnly(true);
		mFullName->SetReadOnly(true);
		mEmail->SetReadOnly(true);
		mCalendar->SetReadOnly(true);
		mCompany->SetReadOnly(true);
		mPhoneWork->SetReadOnly(true);
		mPhoneHome->SetReadOnly(true);
		mFax->SetReadOnly(true);
		mAddress->SetReadOnly(true);
		mURL->SetReadOnly(true);
		mNotes->SetReadOnly(true);
	}
}

bool CEditAddressDialog::GetFields(CAdbkAddress* addr)
{
	bool done_edit = false;

	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring txt = mNickName->GetText();
	::strreplace(txt.c_str_mod(), "@", '*');
	txt.trimspace();

	if (addr->GetADL() != txt)
	{
		addr->SetADL(txt);
		done_edit = true;
	}

	txt = mFullName->GetText();
	if (addr->GetName() != txt)
	{
		addr->SetName(txt);
		done_edit = true;
	}

	txt = mEmail->GetText();
	cdstring test_addr = addr->GetEmail(CAdbkAddress::eDefaultEmailType);
	if (test_addr != txt)
	{
		addr->SetEmail(txt, CAdbkAddress::eDefaultEmailType);
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

	txt = mAddress->GetText();

	// Give to address if not same as previous
	if (addr->GetAddress(CAdbkAddress::eDefaultAddressType) != txt)
	{
		addr->SetAddress(txt, CAdbkAddress::eDefaultAddressType);
		done_edit = true;
	}

	txt = mURL->GetText();

	// Give to URL if not same as previous
	if (addr->GetURL() != txt)
	{
		addr->SetURL(txt);
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

	txt = mNotes->GetText();
	if (addr->GetNotes() != txt)
	{
		addr->SetNotes(txt);
		done_edit = true;
	}

	return done_edit;
}

bool CEditAddressDialog::PoseDialog(CAdbkAddress* addr, bool allow_edit)
{
	bool result = false;

	CEditAddressDialog* dlog = new CEditAddressDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetFields(addr, allow_edit);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		// Get the address
		dlog->GetFields(addr);

		result = true;
		dlog->Close();
	}

	return result;
}
