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


// Source for CEditAddressDialog class

#include "CEditAddressDialog.h"

#include "CAdbkAddress.h"
#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CStringUtils.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

// __________________________________________________________________________________________________
// C L A S S __ C E D I T A D D R E S S D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditAddressDialog::CEditAddressDialog()
{
}

// Constructor from stream
CEditAddressDialog::CEditAddressDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CEditAddressDialog::~CEditAddressDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditAddressDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Do nick-name
	mNickName = (CTextFieldX*) FindPaneByID(paneid_EditAddressNickName);

	// Do full name
	mFullName = (CTextFieldX*) FindPaneByID(paneid_EditAddressFullName);

	// Do email address
	mEmail = (CTextFieldX*) FindPaneByID(paneid_EditAddressEmail);

	// Do calendar
	mCalendar = (CTextFieldX*) FindPaneByID(paneid_EditAddressCalendar);
	
	// Do company
	mCompany = (CTextFieldX*) FindPaneByID(paneid_EditAddressCompany);

	// Do address
	mAddress = (CTextDisplay*) FindPaneByID(paneid_EditAddressAddress);

	// Do URL
	mURL = (CTextDisplay*) FindPaneByID(paneid_EditAddressURL);

	// Do phone work
	mPhoneWork = (CTextFieldX*) FindPaneByID(paneid_EditAddressPhoneWork);

	// Do phone home
	mPhoneHome = (CTextFieldX*) FindPaneByID(paneid_EditAddressPhoneHome);

	// Do fax
	mFax = (CTextFieldX*) FindPaneByID(paneid_EditAddressFax);

	// Do notes
	mNotes = (CTextDisplay*) FindPaneByID(paneid_EditAddressNotes);

}

// Set fields in dialog
void CEditAddressDialog::SetFields(const CAdbkAddress* addr, bool allow_edit)
{
	mNickName->SetText(addr->GetADL());

	mFullName->SetText( addr->GetName());

	mEmail->SetText(addr->GetEmail(CAdbkAddress::eDefaultEmailType));

	mCalendar->SetText(addr->GetCalendar());

	mCompany->SetText(addr->GetCompany());

	mAddress->SetText(addr->GetAddress(CAdbkAddress::eDefaultAddressType));

	mURL->SetText(addr->GetURL());

	mPhoneWork->SetText(addr->GetPhone(CAdbkAddress::eWorkPhoneType));

	mPhoneHome->SetText(addr->GetPhone(CAdbkAddress::eHomePhoneType));

	mFax->SetText(addr->GetPhone(CAdbkAddress::eFaxType));

	mNotes->SetText(addr->GetNotes());

	mNickName->SelectAll();
	
	if (!allow_edit)
	{
		mFullName->SetReadOnly(true);
		mNickName->SetReadOnly(true);
		mEmail->SetReadOnly(true);
		mCalendar->SetReadOnly(true);
		mCompany->SetReadOnly(true);
		mAddress->SetReadOnly(true);
		mURL->SetReadOnly(true);
		mPhoneWork->SetReadOnly(true);
		mPhoneHome->SetReadOnly(true);
		mFax->SetReadOnly(true);
		mNotes->SetReadOnly(true);
	}
	else
		// Make text edit field active
		SetLatentSub(mFullName);
}

// Get fields from dialog
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

	{
		// Copy text And null terminate
		cdstring atxt;
		mNotes->GetText(atxt);

		// Give to address if not same as previous
		if (addr->GetNotes() != atxt)
		{
			addr->SetNotes(atxt);
			done_edit = true;
		}
	}

	return done_edit;
}

bool CEditAddressDialog::PoseDialog(CAdbkAddress* addr, bool allow_edit)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_EditAddress, CMulberryApp::sApp);
	CEditAddressDialog* dlog = (CEditAddressDialog*) theHandler.GetDialog();
	dlog->SetFields(addr, allow_edit);

	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			// Get strings and update address if changed
			dlog->GetFields(addr);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
