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

// Source for CSpellAddDialog class

#include "CSpellAddDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CSpellPlugin.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>


#include <ctype.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellAddDialog::CSpellAddDialog()
{
	mSpeller = nil;
}

// Constructor from stream
CSpellAddDialog::CSpellAddDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mSpeller = nil;
}

// Default destructor
CSpellAddDialog::~CSpellAddDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellAddDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	for(int i = 0; i < 19; i++)
	{
		mCheck[i] = (LCheckBox*) FindPaneByID(paneid_SpellAddCheck[i]);
		mSuggestion[i] = (CTextFieldX*) FindPaneByID(paneid_SpellAddSuggestion[i]);
	}
	mCapitalise = (LCheckBox*) FindPaneByID(paneid_SpellAddCaps);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSpellAddDialogBtns);

}

// Handle OK button
void CSpellAddDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	int index = -1;
	for(int i = 0; i < 19; i++)
	{
		if (inMessage == msg_SpellAddCheck[i])
		{
			index = i;
			break;
		}
	}

	if (index >= 0)
	{
		if (mCheck[index]->GetValue())
			mSuggestion[index]->Enable();
		else
			mSuggestion[index]->Disable();
	}
	else
		switch (inMessage)
		{
			case msg_SpellAddCaps:
				Capitalise(mCapitalise->GetValue());
				break;

			default:
				LDialogBox::ListenToMessage(inMessage, ioParam);
				break;
		}
}

// Set the details
void CSpellAddDialog::SetDetails(CSpellPlugin* speller, const char* word)
{
	mSpeller = speller;

	// Add default word
	mSuggestion[0]->SetText(word);

	// Do suffixes for others
	for(int i = 1; i < 19; i++)
	{
		char buf[256];
		char* s1;
		const char* suffix = mSpeller->GetAddSuffix(i);
		::strcpy(buf, word);
		s1 = buf + ::strlen(buf);

		/* build conjugated string */
		if (i == 1)
		{
			if (*(s1-1) == 'y')
			{
				suffix = "ies";
				*(s1-1) = 0;
			}
		}
		else if ((i >= 4) && (i <= 6))
		{
			if (*(s1-1) == 'e')
				*(s1-1) = 0;
		}
		else if ((i == 14) || (i == 15))
		{
			switch(*(s1-1))
			{
			case 'a':
			case 'e':
			case 'i':
			case 'y':
				*(s1-1) = 0;
				break;
			default:;
			}
		}

		// Now add suffix
		::strcat(buf, suffix);

		// Add to text field
		mSuggestion[i]->SetText(buf);
	}

	// Turn on first item
	mCheck[0]->SetValue(1);

	// Now do capitalisation
	mCapitalise->SetValue(::isupper(*word) != 0);
}

void CSpellAddDialog::GetDetails(CSpellPlugin* speller)
{
	// Add requested words
	for(int i = 0; i < 19; i++)
	{
		if (mCheck[i]->GetValue())
		{
			cdstring str = mSuggestion[i]->GetText();
			speller->AddWord(str);
		}
	}
}

// Rotate default button
void CSpellAddDialog::Capitalise(bool capitals)
{
	// Convert all entries to upper/lower as required
	for(int i = 0; i < 19; i++)
	{
		cdstring str = mSuggestion[i]->GetText();
		if (!str.empty())
		{
			str[0UL] = (capitals ? ::toupper(str[0UL]) : ::tolower(str[0UL]));
			mSuggestion[i]->SetText(str);
		}
	}
}

bool CSpellAddDialog::PoseDialog(CSpellPlugin* speller, const char* add_word)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_SpellAddDialog, CMulberryApp::sApp);
	CSpellAddDialog* dlog = (CSpellAddDialog*) theHandler.GetDialog();
	dlog->SetDetails(speller, add_word);
	dlog->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			dlog->GetDetails(speller);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
