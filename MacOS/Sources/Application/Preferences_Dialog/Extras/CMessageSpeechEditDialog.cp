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


// Source for CMessageSpeechEditDialog class

#include "CMessageSpeechEditDialog.h"

#include "CHelpAttach.h"
#include "CSpeechSynthesis.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>


// __________________________________________________________________________________________________
// C L A S S __ C E D I T G R O U P D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMessageSpeechEditDialog::CMessageSpeechEditDialog()
{
}

// Constructor from stream
CMessageSpeechEditDialog::CMessageSpeechEditDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CMessageSpeechEditDialog::~CMessageSpeechEditDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMessageSpeechEditDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Do nick-name
	mItem = (LPopupButton*) FindPaneByID(paneid_MessageSpeechEditItem);
	mText = (CTextFieldX*) FindPaneByID(paneid_MessageSpeechEditText);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CMessageSpeechEditDialogBtns);
}

// Set item in dialog
void CMessageSpeechEditDialog::SetItem(CMessageSpeak* item)
{
	short menu_pos = item->mItem;

	if (menu_pos > 2) menu_pos++;
	if (menu_pos > 5) menu_pos++;
	if (menu_pos > 8) menu_pos++;
	if (menu_pos > 12) menu_pos++;
	if (menu_pos > 15) menu_pos++;

	mItem->SetValue(menu_pos);

	mText->SetText(item->mItemText);

}

// Get item from dialog
void CMessageSpeechEditDialog::GetItem(CMessageSpeak* item)
{
	short menu_pos = mItem->GetValue();

	if (menu_pos > 16) menu_pos--;
	if (menu_pos > 13) menu_pos--;
	if (menu_pos > 9) menu_pos--;
	if (menu_pos > 6) menu_pos--;
	if (menu_pos > 3) menu_pos--;

	item->mItem = (EMessageSpeakItem) menu_pos;

	item->mItemText = mText->GetText();
}
