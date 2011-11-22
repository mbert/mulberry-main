/*
    Copyright (c) 2007-2011 Cyrus Daboo. All rights reserved.
    
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


// Source for CEditAddressAdvancedDialog class

#include "CEditAddressAdvancedDialog.h"

#include "CAdbkAddress.h"
#include "CAddressFieldContainer.h"
#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CStringUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C E D I T A D D R E S S D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CEditAddressAdvancedDialog::CEditAddressAdvancedDialog()
{
}

// Constructor from stream
CEditAddressAdvancedDialog::CEditAddressAdvancedDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CEditAddressAdvancedDialog::~CEditAddressAdvancedDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditAddressAdvancedDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	mFields = (CAddressFieldContainer*) FindPaneByID(paneid_EditAddressAdvancedFields);

}

// Set fields in dialog
void CEditAddressAdvancedDialog::SetFields(const CAdbkAddress* addr, bool allow_edit)
{
    mFields->SetAddress(addr);
}

// Get fields from dialog
bool CEditAddressAdvancedDialog::GetFields(CAdbkAddress* addr)
{
    return mFields->GetAddress(addr);
}

bool CEditAddressAdvancedDialog::PoseDialog(CAdbkAddress* addr, bool allow_edit)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_EditAddressAdvanced, CMulberryApp::sApp);
	CEditAddressAdvancedDialog* dlog = (CEditAddressAdvancedDialog*) theHandler.GetDialog();
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
