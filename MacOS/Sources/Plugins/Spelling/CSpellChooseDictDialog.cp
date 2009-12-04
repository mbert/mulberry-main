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

// Source for CSpellChooseDictDialog class

#include "CSpellChooseDictDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CSpellPlugin.h"

#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellChooseDictDialog::CSpellChooseDictDialog()
{
}

// Constructor from stream
CSpellChooseDictDialog::CSpellChooseDictDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CSpellChooseDictDialog::~CSpellChooseDictDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellChooseDictDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mDictionaries = (LPopupButton*) FindPaneByID(paneid_SpellChooseDictDictionaries);
}

// Set the details
void CSpellChooseDictDialog::SetSpeller(CSpellPlugin* speller)
{
	InitDictionaries(speller);
}

// Get the details
void CSpellChooseDictDialog::GetOptions(CSpellPlugin* speller)
{
	Str255 dname;
	mDictionaries->GetMenuItemText(mDictionaries->GetValue(), dname);
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(dname));
}

// Set dictionary popup
void CSpellChooseDictDialog::InitDictionaries(CSpellPlugin* speller)
{
	// Delete previous items
	MenuHandle menuH = mDictionaries->GetMacMenuH();
	for(short i = 1; i <= ::CountMenuItems(menuH); i++)
		::DeleteMenuItem(menuH, 1);

	// Add each enabled password changing plugin
	const char* names;
	if (speller->GetDictionaries(&names))
	{
		const char** name = (const char**)(names);
		short menu_pos = 1;
		while(*name)
			::AppendItemToMenu(menuH, menu_pos++, *name++);
	}

	// Force max/min update
	mDictionaries->SetMenuMinMax();

	SetPopupByName(mDictionaries, speller->GetPreferences()->mDictionaryName.GetValue());
}

bool CSpellChooseDictDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_SpellChooseDictDialog, CMulberryApp::sApp);
	CSpellChooseDictDialog* dlog = (CSpellChooseDictDialog*) theHandler.GetDialog();
	dlog->SetSpeller(speller);
	dlog->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			dlog->GetOptions(speller);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
