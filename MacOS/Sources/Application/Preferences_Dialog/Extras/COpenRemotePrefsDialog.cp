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


// Source for COpenRemotePrefsDialog class

#include "COpenRemotePrefsDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CPreferenceKeys.h"
#include "CRemotePrefsSets.h"
#include "CTextTable.h"

#include <LGAPushButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C O P E N R E M O T E P R E F S D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
COpenRemotePrefsDialog::COpenRemotePrefsDialog()
{
}

// Constructor from stream
COpenRemotePrefsDialog::COpenRemotePrefsDialog(LStream *inStream)
		: LGADialogBox(inStream)
{
}

// Default destructor
COpenRemotePrefsDialog::~COpenRemotePrefsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void COpenRemotePrefsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LGADialogBox::FinishCreateSelf();

	// Get items
	mRemoteList = (CTextTable*) FindPaneByID(paneid_OpenRemotePrefsList);
	mRemoteList->AddListener(this);
	mRemoteList->AddListener((CBalloonDialog*) mSuperCommander);
	mRemoteList->SetSingleSelection();
	mOpenBtn = (LGAPushButton*) FindPaneByID(paneid_OpenRemotePrefsOpenBtn);
	mOpenBtn->Disable();

	cdstrvect items;

	// Add default name first
	items.push_back(cDefaultPrefsSetKey_2_0);

	// Add the rest
	for(cdstrvect::const_iterator iter = CMulberryApp::sRemotePrefs->GetRemoteSets().begin();
			iter != CMulberryApp::sRemotePrefs->GetRemoteSets().end(); iter++)
		items.push_back(*iter);

	mRemoteList->SetContents(items);
	mRemoteList->SelectCell(STableCell(1, 1));

	// Link controls to this window
	UReanimator::LinkListenerToControls(this, this, RidL_COpenRemotePrefsDialogBtns);
}

// Handle buttons
void COpenRemotePrefsDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {

		case msg_SelectRemotePrefs:
			if (mRemoteList->IsSelectionValid())
				mOpenBtn->Enable();
			else
				mOpenBtn->Disable();
			break;

		default:
			LGADialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Get details from dialog
cdstring COpenRemotePrefsDialog::GetDetails(void)
{
	cdstring selection;
	STableCell aCell(0, 0);

	if (mRemoteList->GetNextSelectedCell(aCell))
	{
		if (aCell.row == 1)
			selection = cDefaultPrefsSetKey_2_0;
		else
			selection = CMulberryApp::sRemotePrefs->GetRemoteSets().at(aCell.row - 2);
	}

	return selection;
}

