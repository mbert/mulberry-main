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
#include "CTextFieldX.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressPersonalPanel::CAddressPersonalPanel(LStream *inStream)
		: CAddressPanelBase(inStream)
{
}

// Default destructor
CAddressPersonalPanel::~CAddressPersonalPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressPersonalPanel::FinishCreateSelf(void)
{
	// Do inherited
	CAddressPanelBase::FinishCreateSelf();

	// Get fields
	mFullName = (CTextFieldX*) FindPaneByID(paneid_AddressPersonalFullName);
	mNickName = (CTextFieldX*) FindPaneByID(paneid_AddressPersonalNickName);
	mEmail = (CTextFieldX*) FindPaneByID(paneid_AddressPersonalEmail);
	mCalendar = (CTextFieldX*) FindPaneByID(paneid_AddressPersonalCalendar);
	mCompany = (CTextFieldX*) FindPaneByID(paneid_AddressPersonalCompany);
}

void CAddressPersonalPanel::Focus()
{
	LCommander::SwitchTarget(mFullName);
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

	bool done_edit = false;

	cdstring txt = mFullName->GetText();
	if (addr->GetName() != txt)
	{
		addr->SetName((char*) txt);
		done_edit = true;
	}

	txt = mNickName->GetText();

	// Nick-name cannot contain '@' and no spaces surrounding it
	cdstring nickname(txt);
	::strreplace(nickname.c_str_mod(), "@", '*');
	nickname.trimspace();

	if (addr->GetADL() != nickname)
	{
		addr->SetADL(nickname);
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

	return done_edit;
}
