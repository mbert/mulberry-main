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


// Source for CPrefsEditMappings class

#include "CPrefsEditMappings.h"

#include "CBalloonDialog.h"
#include "CEditMappingsTable.h"
#include "CMIMEMap.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CPrefsEditMappingDialog.h"

#include <LPushButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsEditMappings::CPrefsEditMappings()
{
	mItsTable = NULL;
	mMappings = NULL;
	mCurrentMap = 0;
}

// Constructor from stream
CPrefsEditMappings::CPrefsEditMappings(LStream *inStream)
		: LDialogBox(inStream)
{
	mItsTable = NULL;
	mMappings = NULL;
	mCurrentMap = 0;
}

// Default destructor
CPrefsEditMappings::~CPrefsEditMappings()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsEditMappings::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Table
	mItsTable = (CEditMappingsTable*) FindPaneByID(paneid_MappingsTable);

	// Set buttons
	mNewBtn = (LPushButton*) FindPaneByID(paneid_MappingsNewBtn);

	mChangeBtn = (LPushButton*) FindPaneByID(paneid_MappingsChangeBtn);
	mChangeBtn->Disable();

	mDeleteBtn = (LPushButton*) FindPaneByID(paneid_MappingsDeleteBtn);
	mDeleteBtn->Disable();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsEditMappingsBtns);

	// Set target
	SetLatentSub(mItsTable);
}

// Handle buttons
void CPrefsEditMappings::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_NewMapping:
			DoNewMapping();
			break;

		case msg_ChangeMapping:
			DoChangeMapping();
			break;

		case msg_DeleteMapping:
			DoDeleteMapping();
			break;
	}
}

// Set mappings table
void CPrefsEditMappings::SetMappings(CMIMEMapVector* mappings)
{
	mMappings = mappings;
	mItsTable->SetMappings(mappings);
}

void CPrefsEditMappings::ShowMapping(unsigned long mapping)
{
	mCurrentMap = mapping;
	if (mCurrentMap)
	{
		mChangeBtn->Enable();
		mDeleteBtn->Enable();
	}
	else
	{
		mChangeBtn->Disable();
		mDeleteBtn->Disable();
	}
}

// Create a new mapping entry
void CPrefsEditMappings::DoNewMapping()
{
	// Create new one and add to end of list
	CMIMEMap new_map;
	if (CPrefsEditMappingDialog::PoseDialog(new_map))
	{
		// Add it and then sort
		(*mMappings).push_back(new_map);
		std::sort(mMappings->begin(), mMappings->end());

		// Reset table
		mItsTable->ResetTable();

		// Find position
		CMIMEMapVector::const_iterator found = std::find(mMappings->begin(), mMappings->end(), new_map);
		if (found != mMappings->end())
		{
			// Select and scroll new item
			STableCell cell(found - mMappings->begin() + 1, 1);
			mItsTable->FakeClickSelect(cell, false);
			mItsTable->ScrollCellIntoFrame(cell);
		}
	}
}

// Change the selected entry
void CPrefsEditMappings::DoChangeMapping()
{
	// Only do if proper row selected
	if (!mCurrentMap) return;

	CMIMEMap& mapping = (*mMappings)[mCurrentMap - 1];
	if (CPrefsEditMappingDialog::PoseDialog(mapping))
	{
		CMIMEMap temp(mapping);
		std::sort(mMappings->begin(), mMappings->end());

		// Reset table
		mItsTable->ResetTable();

		// Find position
		CMIMEMapVector::const_iterator found = std::find(mMappings->begin(), mMappings->end(), temp);
		if (found != mMappings->end())
		{
			// Select and scroll new item
			STableCell cell(found - mMappings->begin() + 1, 1);
			mItsTable->FakeClickSelect(cell, false);
			mItsTable->ScrollCellIntoFrame(cell);
		}
	}
}

// Choose an application
void CPrefsEditMappings::DoDeleteMapping()
{
	// Only do if proper row selected
	if (!mCurrentMap) return;

	mMappings->erase(mMappings->begin() + (mCurrentMap - 1));
	mItsTable->ResetTable();

}

bool CPrefsEditMappings::PoseDialog(CMIMEMapVector* mappings)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_PrefsEditMappings, CMulberryApp::sApp);
	CPrefsEditMappings* dlog = (CPrefsEditMappings*) theHandler.GetDialog();
	dlog->SetMappings(mappings);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
