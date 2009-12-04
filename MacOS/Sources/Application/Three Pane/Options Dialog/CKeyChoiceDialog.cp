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


// Source for CKeyChoiceDialog class

#include "CKeyChoiceDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CKeyChoiceDialog::CKeyChoiceDialog()
{
	mDone = false;
}

// Constructor from stream
CKeyChoiceDialog::CKeyChoiceDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mDone = false;
}

// Default destructor
CKeyChoiceDialog::~CKeyChoiceDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CKeyChoiceDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CKeyChoiceDialogBtns);
}

// Handle key presses
Boolean CKeyChoiceDialog::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Look for preview/full view based on key stroke
	char key_press = (inKeyEvent.message & charCodeMask);
	if (key_press == char_Enter)
		key_press = char_Return;

	// Cache key press value
	mKey = key_press;

	// Cache modifiers
	mMods = CKeyModifiers(inKeyEvent.modifiers);

	// Force close
	mDone = true;
	
	return true;
}

bool CKeyChoiceDialog::PoseDialog(unsigned char& key, CKeyModifiers& mods)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_KeyChoiceDialog, CMulberryApp::sApp);
	CKeyChoiceDialog* dlog = static_cast<CKeyChoiceDialog*>(theHandler.GetDialog());
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if ((hitMessage == msg_OK) || dlog->IsDone())
		{
			key = dlog->mKey;
			mods = dlog->mMods;
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
