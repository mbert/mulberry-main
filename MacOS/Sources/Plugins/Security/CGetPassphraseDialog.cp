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


// Source for CGetPassphraseDialog class

#include "CGetPassphraseDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGetPassphraseDialog::CGetPassphraseDialog()
{
	mHidden = true;
}

// Constructor from stream
CGetPassphraseDialog::CGetPassphraseDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mHidden = true;
}

// Default destructor
CGetPassphraseDialog::~CGetPassphraseDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CGetPassphraseDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mTextP = (CTextFieldX*) FindPaneByID(paneid_GetPassphraseTextP);
	mTextC = (CTextFieldX*) FindPaneByID(paneid_GetPassphraseTextC);

	mTextC->Hide();

	mKeys = (LPopupButton*) FindPaneByID(paneid_GetPassphraseKeys);

	((LCheckBox*) FindPaneByID(paneid_GetPassphraseHide))->SetValue(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CGetPassphraseDialogBtns);

	// Make text edit field active
	SetLatentSub(mTextP);
}

// Called during idle
void CGetPassphraseDialog::SetUpDetails(cdstring& change, const char** keys)
{
	// Give text to pane
	CTextFieldX* theText = mHidden ? mTextP : mTextC;
	theText->SetText(change);
	theText->SelectAll();

	if (keys)
	{
		// Delete previous items
		MenuHandle menuH = mKeys->GetMacMenuH();
		for(short i = ::CountMenuItems(menuH); i >= 1; i--)
			::DeleteMenuItem(menuH, i);

		// Add each key
		short menu_pos = 1;
		const char** p = keys;
		while(*p)
			// Insert item
			::AppendItemToMenu(menuH, menu_pos++, *p++);

		// Force max/min update
		mKeys->SetMenuMinMax();
	}
	else
	{
		// Hide popup if no keys
		mKeys->Hide();
	}
}

void CGetPassphraseDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_GetPassphraseHide:
		HideText(*(long*) ioParam);
		break;
	}
}

void CGetPassphraseDialog::HideText(bool hide)
{
	if (mHidden ^ hide)
	{
		// Copy from current one into the other one
		CTextFieldX* to_hide = NULL;
		CTextFieldX* to_show = NULL;
		
		if (hide)
		{
			to_hide = mTextC;
			to_show = mTextP;
		}
		else
		{
			to_hide = mTextP;
			to_show = mTextC;
		}
		
		to_show->SetText(to_hide->GetText());
		
		SInt32 selStart;
		SInt32 selEnd;
		to_hide->GetSelection(&selStart, &selEnd);
		to_hide->Hide();

		to_show->Show();
		SwitchTarget(to_show);
		to_show->SetSelection(selStart, selEnd);

		mHidden = hide;
	}
}

bool CGetPassphraseDialog::PoseDialog(cdstring& change, const char** keys, cdstring& chosen_key, unsigned long& index)
{
	bool result = false;

	// Create the dialog
	{
		CBalloonDialog	theHandler(paneid_GetPassphraseDialog, CMulberryApp::sApp);
		CGetPassphraseDialog* dlog = (CGetPassphraseDialog*) theHandler.GetDialog();

		// Set dlog info
		dlog->SetUpDetails(change, keys);

		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{
			hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				CTextFieldX* theText = dlog->mHidden ? dlog->mTextP : dlog->mTextC;
				change = theText->GetText();

				if (keys)
				{
					chosen_key = ::GetPopupMenuItemTextUTF8(dlog->mKeys);
					index = dlog->mKeys->GetValue() - 1;
				}

				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
