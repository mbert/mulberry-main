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

// Source for CSpellOptionsDialog class

#include "CSpellOptionsDialog.h"

#include "CSpellPlugin.h"

#include "TPopupMenu.h"
#include "JXColorButton.h"

#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellOptionsDialog::CSpellOptionsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CSpellOptionsDialog::~CSpellOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellOptionsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 340,220, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,220);
    assert( obj1 != NULL );

    JXEngravedRect* obj2 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 320,110);
    assert( obj2 != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,185, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,185, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    JXStaticText* obj3 =
        new JXStaticText("Checking:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,10, 65,15);
    assert( obj3 != NULL );

    mAutoPositionDialog =
        new JXTextCheckbox("Automatically Position Dialog", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 250,20);
    assert( mAutoPositionDialog != NULL );

    mSpellOnSend =
        new JXTextCheckbox("Always Check Spelling before Sending", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 250,20);
    assert( mSpellOnSend != NULL );

    mSpellAsYouType =
        new JXTextCheckbox("Check Spelling whilst Typing", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,50, 250,20);
    assert( mSpellAsYouType != NULL );

    mSpellColourBackground =
        new JXTextCheckbox("Use Background Colour for Errors:", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,70, 220,20);
    assert( mSpellColourBackground != NULL );

    mSpellBackgroundColour =
        new JXColorButton(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,70, 40,20);
    assert( mSpellBackgroundColour != NULL );

    mDictionaries =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,145, 210,25);
    assert( mDictionaries != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Main Dictionary:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,150, 105,20);
    assert( obj4 != NULL );

// end JXLayout

	SetButtons(mOKBtn, mCancelBtn);
	GetWindow()->SetTitle("Spelling Options");

	// Colour button needs this
	mSpellBackgroundColour->SetDirector(this);

	// Listen to the buttons
	ListenTo(mSpellColourBackground);
}

// Handle OK button
void CSpellOptionsDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mSpellColourBackground)
		{
			OnSpellColourBackground();
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

// Set the options
void CSpellOptionsDialog::SetOptions(CSpellPlugin* speller)
{
	mAutoPositionDialog->SetState(JBoolean(speller->GetPreferences()->mAutoPositionDialog.GetValue()));
	mSpellOnSend->SetState(JBoolean(speller->GetPreferences()->mSpellOnSend.GetValue()));
	mSpellAsYouType->SetState(JBoolean(speller->GetPreferences()->mSpellAsYouType.GetValue()));
	mSpellColourBackground->SetState(JBoolean(speller->GetPreferences()->mSpellColourBackground.GetValue()));
	mSpellBackgroundColour->SetColor(speller->GetPreferences()->mSpellBackgroundColour.GetValue());
	if (!speller->GetPreferences()->mSpellColourBackground.GetValue())
		mSpellBackgroundColour->Deactivate();

	InitDictionaries(speller);
}

// Get the options
void CSpellOptionsDialog::GetOptions(CSpellPlugin* speller)
{
	speller->GetPreferences()->mAutoPositionDialog.SetValue(mAutoPositionDialog->IsChecked());
	speller->GetPreferences()->mSpellOnSend.SetValue(mSpellOnSend->IsChecked());
	speller->GetPreferences()->mSpellAsYouType.SetValue(mSpellAsYouType->IsChecked());
	speller->GetPreferences()->mSpellColourBackground.SetValue(mSpellColourBackground->IsChecked());
	speller->GetPreferences()->mSpellBackgroundColour.SetValue(mSpellBackgroundColour->GetColor());
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(mDictionaries->GetCurrentItemText()));
	speller->UpdatePreferences();
}

void CSpellOptionsDialog::OnSpellColourBackground()
{
	if (mSpellColourBackground->IsChecked())
		mSpellBackgroundColour->Activate();
	else
		mSpellBackgroundColour->Deactivate();
}

// Called during idle
void CSpellOptionsDialog::InitDictionaries(CSpellPlugin* speller)
{
	mDictionaries->RemoveAllItems();

	// Add each enabled password changing plugin
	const char* names;
	if (speller->GetDictionaries(&names))
	{
		const char** name = (const char**)(names);
		JIndex menu_pos = 1;
		JIndex choice_pos = 1;
		while(*name)
		{
			// Get the index of the one we want to initially select
			if (speller->GetPreferences()->mDictionaryName.GetValue() == *name)
				choice_pos = menu_pos;

			mDictionaries->AppendItem(*name++, kFalse, kTrue);
			menu_pos++;
		}

		// Set intitial item
		mDictionaries->SetToPopupChoice(kTrue, choice_pos);
	}
}

bool CSpellOptionsDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	CSpellOptionsDialog* dlog = new CSpellOptionsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetOptions(speller);

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetOptions(speller);
		dlog->Close();
		result = true;
	}

	return result;
}
