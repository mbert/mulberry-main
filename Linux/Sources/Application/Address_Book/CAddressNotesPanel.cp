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

#include <JXStaticText.h>

#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAddressNotesPanel::CAddressNotesPanel(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
		: CAddressPanelBase(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// Default destructor
CAddressNotesPanel::~CAddressNotesPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressNotesPanel::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Notes:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,12, 55,20);
    assert( obj1 != NULL );

    mNotes =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 85,10, 400,230);
    assert( mNotes != NULL );

// end JXLayout1
}

void CAddressNotesPanel::Focus()
{
	mNotes->Focus();
}

// Set fields in dialog
void CAddressNotesPanel::SetFields(const CAdbkAddress* addr)
{
	cdstring	txt;

	if (addr)
		txt = addr->GetNotes();
	mNotes->SetText(txt);
}

// Get fields from dialog
bool CAddressNotesPanel::GetFields(CAdbkAddress* addr)
{
	// Only for valid group
	if (!addr)
		return false;

	cdstring txt;
	bool done_edit = false;

	txt = mNotes->GetText();
	if (addr->GetNotes() != txt)
	{
		addr->SetNotes(txt);
		done_edit = true;
	}

	return done_edit;
}
