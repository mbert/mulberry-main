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


// Source for CAddressNotesPanel class

#include "CAddressNotesPanel.h"

#include "CAdbkAddress.h"
#include "CTextDisplay.h"

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressNotesPanel::CAddressNotesPanel(LStream *inStream)
		: CAddressPanelBase(inStream)
{
}

// Default destructor
CAddressNotesPanel::~CAddressNotesPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressNotesPanel::FinishCreateSelf(void)
{
	// Do inherited
	CAddressPanelBase::FinishCreateSelf();

	// Get fields
	mNotes = (CTextDisplay*) FindPaneByID(paneid_AddressNotesNotes);
}

void CAddressNotesPanel::Focus()
{
	LCommander::SwitchTarget(mNotes);
}

// Set fields in dialog
void CAddressNotesPanel::SetFields(const CAdbkAddress* addr)
{
	mNotes->SetText(addr ? const_cast<cdstring&>(addr->GetNotes()) : cdstring::null_str);
}

// Get fields from dialog
bool CAddressNotesPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	bool done_edit = false;

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
