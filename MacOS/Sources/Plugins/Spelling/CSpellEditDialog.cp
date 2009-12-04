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

// Source for CSpellEditDialog class

#include "CSpellEditDialog.h"

#include "CBalloonDialog.h"
#include "CDictionaryPageScroller.h"
#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CSpellAddDialog.h"
#include "CSpellPlugin.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CTextTable.h"

#include <LPushButton.h>
#include <LScrollBar.h>

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellEditDialog::CSpellEditDialog()
{
	mSpeller = NULL;
}

// Constructor from stream
CSpellEditDialog::CSpellEditDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mSpeller = NULL;
}

// Default destructor
CSpellEditDialog::~CSpellEditDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellEditDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mDictName = (CStaticText*) FindPaneByID(paneid_SpellEditDictName);
	mScroller = (CDictionaryPageScroller*) FindPaneByID(paneid_SpellEditScroller);
	mWordTop = (CStaticText*) FindPaneByID(paneid_SpellEditWordTop);
	mWordBottom = (CStaticText*) FindPaneByID(paneid_SpellEditWordBottom);
	mScroller->SetRangeCaptions(mWordTop, mWordBottom);
	mScroller->DisplayPages(true);
	mList = (CTextTable*) FindPaneByID(paneid_SpellEditList);
	mList->AddListener(this);
	mWord = (CTextFieldX*) FindPaneByID(paneid_SpellEditWord);
	mAdd = (LPushButton*) FindPaneByID(paneid_SpellEditAdd);
	mFind = (LPushButton*) FindPaneByID(paneid_SpellEditFind);
	mRemove = (LPushButton*) FindPaneByID(paneid_SpellEditRemove);
	mCancel = (LPushButton*) FindPaneByID(paneid_SpellEditCancel);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSpellEditDialogBtns);

}

Boolean CSpellEditDialog::HandleKeyPress ( const EventRecord	&inKeyEvent )
{
	bool		keyHandled = false;
	LControl*	keyButton = NULL;

	switch (inKeyEvent.message & charCodeMask)
	{

	case char_Tab:
		// Check for command key
		if (inKeyEvent.modifiers & cmdKey)
		{
			RotateDefault();
			keyHandled = true;
			break;
		}

		// Fall through

	default:
		keyHandled = LDialogBox::HandleKeyPress(inKeyEvent);
		break;
	}

	return keyHandled;

}	//	LDialogBox::HandleKeyPress

// Handle OK button
void CSpellEditDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_SpellEditList:
		// Transfer selected word to suggestion and default to replace
		{
			STableCell aCell = mList->GetFirstSelectedCell();
			if (aCell.row)
				mRemove->Enable();
			else
				mRemove->Disable();
		}
		break;

	case paneid_SpellEditScroller:
		mRemove->Disable();
		break;

	case msg_SpellEditAdd:
		DoAdd();
		break;

	case msg_SpellEditFind:
		DoFind();
		break;

	case msg_SpellEditRemove:
		DoRemove();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the details
void CSpellEditDialog::SetSpeller(CSpellPlugin* speller)
{
	mSpeller = speller;

	// Set details in scroller
	mScroller->SetDetails(mSpeller, this);

	// Start at first page
	mScroller->SetPage(0);

	// Do after dictionary has loaded as possible change to page number
	mScroller->GetHorizScrollBar()->SetMinValue(0);
	mScroller->GetHorizScrollBar()->SetMaxValue(mSpeller->CountPages());
	mScroller->GetHorizScrollBar()->SetValue(mSpeller->CurrentPage());

	// Remove Find button if only one page
	if (!mSpeller->CountPages())
		mFind->Hide();

	// Set dictionary name
	mDictName->SetText(mSpeller->GetPreferences()->mDictionaryName.GetValue());
}

// Rotate default button
void CSpellEditDialog::RotateDefault(void)
{
	switch(mDefaultButtonID)
	{
	case paneid_SpellEditAdd:
		SetDefaultButton(paneid_SpellEditFind);
		break;
	case paneid_SpellEditFind:
		SetDefaultButton(mRemove->IsEnabled() ? paneid_SpellEditRemove : paneid_SpellEditCancel);
		break;
	case paneid_SpellEditRemove:
		SetDefaultButton(paneid_SpellEditCancel);
		break;
	case paneid_SpellEditCancel:
		SetDefaultButton(paneid_SpellEditAdd);
		break;
	}
}

// Add word to dictionary
void CSpellEditDialog::DoAdd(void)
{
	cdstring add_word = mWord->GetText();

	// Must contain something
	if (add_word.empty())
		return;

	// Must not be in dictionary
	if (mSpeller->ContainsWord(add_word))
	{
		// Put error alert
		CErrorHandler::PutNoteAlert(STRx_Spell, str_CannotAddDuplicate);
		return;
	}

	// Do the dialog
	if (CSpellAddDialog::PoseDialog(mSpeller, add_word))
	{
		// Words will be added by add dialog

		// Refresh dict view
		mScroller->DisplayDictionary(add_word);
	}
}

// Remove word from dictionary
void CSpellEditDialog::DoFind(void)
{
	cdstring find_word = mWord->GetText();

	// Must contain something
	if (find_word.empty())
		return;

	// Refresh dict view
	mScroller->DisplayDictionary(find_word, true);
}

// Replace word
void CSpellEditDialog::DoRemove(void)
{
	STableCell aCell(0, 0);
	while(mList->GetNextSelectedCell(aCell))
	{
		char str[256];
		UInt32	len = sizeof(str);
		mList->GetCellData(aCell, str, len);

		// Delete in dict
		mSpeller->RemoveWord(str);
	}

	// Refresh dict view
	mScroller->DisplayDictionary(NULL, true);
}

bool CSpellEditDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_SpellEditDialog, CMulberryApp::sApp);
	((CSpellEditDialog*) theHandler.GetDialog())->SetSpeller(speller);
	theHandler.GetDialog()->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
