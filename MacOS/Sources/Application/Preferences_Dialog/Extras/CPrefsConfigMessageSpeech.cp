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


// Source for CPrefsConfigMessageSpeech class

#include "CPrefsConfigMessageSpeech.h"

#include "CMessageSpeechTable.h"

#include <LPushButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C R E P L Y C H O O S E D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsConfigMessageSpeech::CPrefsConfigMessageSpeech()
{
}

// Constructor from stream
CPrefsConfigMessageSpeech::CPrefsConfigMessageSpeech(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CPrefsConfigMessageSpeech::~CPrefsConfigMessageSpeech()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsConfigMessageSpeech::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Table
	mItsTable = (CMessageSpeechTable*) FindPaneByID(paneid_SpeechTable);

	// Set buttons
	mNewBtn = (LPushButton*) FindPaneByID(paneid_SpeechNewBtn);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsConfigMessageSpeechBtns);

	// Set target
	SetLatentSub(mItsTable);
}

// Handle buttons
void CPrefsConfigMessageSpeech::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage) {

		case msg_NewSpeechItem:
			DoNewItem();
			break;

		default:
			break;
	}
}

// Set mappings table
void CPrefsConfigMessageSpeech::SetList(CMessageSpeakVector* list)
{
	mList = list;
	mItsTable->SetList(list);
}

// Create a new mapping entry
void CPrefsConfigMessageSpeech::DoNewItem(void)
{
	// Create new default item
	CMessageSpeak new_item;
	new_item.mItem = eMessageSpeakNone;

	// Add to list and refresh
	mList->push_back(new_item);
	mItsTable->ResetTable();

	// Select and auto edit
	STableCell aCell;
	mItsTable->GetTableSize(aCell.row, aCell.col);
	mItsTable->SelectCell(aCell);
	mItsTable->EditEntry(aCell);
}