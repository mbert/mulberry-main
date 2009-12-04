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


// Source for C3PaneContactsAccount class

#include "C3PaneContactsAccount.h"

#include "CAdbkManagerView.h"
#include "CAdbkManagerTable.h"
#include "CAddressBookManager.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneContactsAccount::C3PaneContactsAccount()
{
}

// Constructor from stream
C3PaneContactsAccount::C3PaneContactsAccount(LStream *inStream)
		: C3PaneViewPanel(inStream)
{
}

// Default destructor
C3PaneContactsAccount::~C3PaneContactsAccount()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneContactsAccount::FinishCreateSelf()
{
	// Do inherited
	C3PaneViewPanel::FinishCreateSelf();

	// Get sub-panes
	mContactsView = (CAdbkManagerView*) FindPaneByID(paneid_3PaneView);

	mContactsView->GetTable()->SetManager(CAddressBookManager::sAddressBookManager);
	mContactsView->ResetState();
	mContactsView->ResetTable();
}

CBaseView* C3PaneContactsAccount::GetBaseView() const
{
	return mContactsView;
}
