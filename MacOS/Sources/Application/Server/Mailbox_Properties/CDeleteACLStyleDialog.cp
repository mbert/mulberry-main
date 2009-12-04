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


// Source for CDeleteACLStyleDialog class

#include "CDeleteACLStyleDialog.h"

#include "CBalloonDialog.h"
#include "CPreferences.h"
#include "CPropMailboxACL.h"
#include "CTextTable.h"

#include <LGAPushButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C D E L E T E A C L S T Y L E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CDeleteACLStyleDialog::CDeleteACLStyleDialog()
{
}

// Constructor from stream
CDeleteACLStyleDialog::CDeleteACLStyleDialog(LStream *inStream)
		: LGADialogBox(inStream)
{
}

// Default destructor
CDeleteACLStyleDialog::~CDeleteACLStyleDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CDeleteACLStyleDialog::FinishCreateSelf(void)
{
	// Do inherited
	LGADialogBox::FinishCreateSelf();

	// Get items
	mStyleList = (CTextTable*) FindPaneByID(paneid_DeleteACLStyleList);
	mStyleList->AddListener(this);
	mStyleList->AddListener((CBalloonDialog*) mSuperCommander);
	mDeleteBtn = (LGAPushButton*) FindPaneByID(paneid_DeleteACLStyleDeleteBtn);
	mDeleteBtn->Disable();

	// Link controls to this window
	UReanimator::LinkListenerToControls(this,this,RidL_CDeleteACLStyleDialogBtns);
}

void CDeleteACLStyleDialog::SetList(const SACLStyleList* aList)
{
	cdstrvect items;
	for(SACLStyleList::const_iterator iter = aList->begin(); iter != aList->end(); iter++)
		items.push_back((*iter).first);

	mStyleList->SetContents(items);
}

// Handle buttons
void CDeleteACLStyleDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {

		case msg_SelectStyle:
			if (mStyleList->IsSelectionValid())
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
ulvector CDeleteACLStyleDialog::GetDetails(void)
{
	ulvector selection;
	mStyleList->GetSelection(selection);

	return selection;
}

