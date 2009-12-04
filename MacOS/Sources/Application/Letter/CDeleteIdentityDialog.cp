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


// Source for CDeleteIdentityDialog class

#include "CDeleteIdentityDialog.h"

#include "CBalloonDialog.h"
#include "CPreferences.h"
#include "CIdentity.h"
#include "CTextTable.h"

#include <LGAPushButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C D E L E T E I D E N T I T Y D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CDeleteIdentityDialog::CDeleteIdentityDialog()
{
}

// Constructor from stream
CDeleteIdentityDialog::CDeleteIdentityDialog(LStream *inStream)
		: LGADialogBox(inStream)
{
}

// Default destructor
CDeleteIdentityDialog::~CDeleteIdentityDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CDeleteIdentityDialog::FinishCreateSelf(void)
{
	// Do inherited
	LGADialogBox::FinishCreateSelf();

	// Get items
	mIdentityList = (CTextTable*) FindPaneByID(paneid_DeleteIdentityList);
	mIdentityList->AddListener(this);
	mIdentityList->AddListener((CBalloonDialog*) mSuperCommander);
	mDeleteBtn = (LGAPushButton*) FindPaneByID(paneid_DeleteIdentityDeleteBtn);
	mDeleteBtn->Disable();

	// Link controls to this window
	UReanimator::LinkListenerToControls(this,this,RidL_CDeleteIdentityDialogBtns);
}

void CDeleteIdentityDialog::SetList(const CIdentityList* aList)
{
	cdstrvect items;
	for(CIdentityList::const_iterator iter = aList->begin(); iter != aList->end(); iter++)
		items.push_back((*iter).GetIdentity());
	mIdentityList->SetContents(items);
}

// Handle buttons
void CDeleteIdentityDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {

		case msg_SelectIdentity:
			if (mIdentityList->IsSelectionValid())
				mDeleteBtn->Enable();
			else
				mDeleteBtn->Disable();
			break;

		default:
			LGADialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Get details from dialog
ulvector CDeleteIdentityDialog::GetDetails(void)
{
	ulvector selection;
	mIdentityList->GetSelection(selection);

	return selection;
}

