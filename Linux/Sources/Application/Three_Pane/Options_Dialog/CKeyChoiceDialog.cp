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

#include "CMulberryApp.h"

#include <JXColormap.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

CKeyTrap::CKeyTrap(const JCharacter* text, JXContainer* enclosure,
			 const HSizingOption hSizing, const VSizingOption vSizing,
			 const JCoordinate x, const JCoordinate y,
			 const JCoordinate w, const JCoordinate h)
	: JXStaticText(text, enclosure, hSizing, vSizing, x, y, w, h)
{
	mKey = 0;
	WantInput(kTrue, kTrue, kTrue, kTrue);
}

void CKeyTrap::HandleKeyPress(const int key, const JXKeyModifiers& modifiers)
{
	// Ignore if not >0 < 128
	if ((key < 0) || (key > 127))
		return;

	// Just grap the key
	mKey = key;
	mMods = CKeyModifiers(modifiers);
	
	// Now end the dialog
	mDialog->EndDialog(CDialogDirector::kDialogClosed_OK);
}

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CKeyChoiceDialog::CKeyChoiceDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor, kTrue)
{
	mDone = false;
}

// Default destructor
CKeyChoiceDialog::~CKeyChoiceDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CKeyChoiceDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 220,55, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 220,55);
    assert( obj1 != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,15, 70,25);
    assert( mCancelBtn != NULL );

    mKeyTrap =
        new CKeyTrap("Type a Key...",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,15, 90,20);
    assert( mKeyTrap != NULL );

// end JXLayout
	window->SetTitle("Type a Key");
	SetButtons(mCancelBtn, NULL);
	
	mKeyTrap->mDialog = this;
	mKeyTrap->SetBackgroundColor(GetColormap()->GetDefaultBackColor());
}

void CKeyChoiceDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mCancelBtn)
		{
			EndDialog(kDialogClosed_Cancel);
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

bool CKeyChoiceDialog::PoseDialog(unsigned char& key, CKeyModifiers& mods)
{
	bool result = false;

	// Create the dialog
	CKeyChoiceDialog* dlog = new CKeyChoiceDialog(JXGetApplication());

	// Let DialogHandler process events
	if (dlog->DoModal() == kDialogClosed_OK)
	{
		key = dlog->mKeyTrap->mKey;
		mods = dlog->mKeyTrap->mMods;
		dlog->Close();
		result = true;
	}

	return result;
}
