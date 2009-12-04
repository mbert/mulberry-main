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


// Source for CPrefsSpellingOptions class

#include "CPrefsSpellingOptions.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CSpellPlugin.h"

#include "JXColorButton.h"

#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSpellingOptions::CPrefsSpellingOptions(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSpellingOptions::OnCreate()
{
// begin JXLayout1

    mAutoPositionDialog =
        new JXTextCheckbox("Automatically Position Dialog", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,30, 250,20);
    assert( mAutoPositionDialog != NULL );

    mSpellOnSend =
        new JXTextCheckbox("Always Check Spelling before Sending", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,50, 250,20);
    assert( mSpellOnSend != NULL );

    mSpellAsYouType =
        new JXTextCheckbox("Check Spelling whilst Typing", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,70, 250,20);
    assert( mSpellAsYouType != NULL );

    mSpellColourBackground =
        new JXTextCheckbox("Use Background Colour for Errors:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,90, 220,20);
    assert( mSpellColourBackground != NULL );

    mSpellBackgroundColour =
        new JXColorButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,90, 40,20);
    assert( mSpellBackgroundColour != NULL );

// end JXLayout1

	// Colour button needs this
	mSpellBackgroundColour->SetDirector(CPreferencesDialog::sPrefsDlog);

	// Listen to the buttons
	ListenTo(mSpellColourBackground);
}

void CPrefsSpellingOptions::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mSpellColourBackground)
		{
			OnSpellColourBackground();
			return;
		}
	}
	
	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsSpellingOptions::SetData(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	mAutoPositionDialog->SetState(JBoolean(speller->GetPreferences()->mAutoPositionDialog.GetValue()));
	mSpellOnSend->SetState(JBoolean(speller->GetPreferences()->mSpellOnSend.GetValue()));
	mSpellAsYouType->SetState(JBoolean(speller->GetPreferences()->mSpellAsYouType.GetValue()));
	mSpellColourBackground->SetState(JBoolean(speller->GetPreferences()->mSpellColourBackground.GetValue()));
	mSpellBackgroundColour->SetColor(speller->GetPreferences()->mSpellBackgroundColour.GetValue());
	if (!speller->GetPreferences()->mSpellColourBackground.GetValue())
		mSpellBackgroundColour->Deactivate();
}

// Force update of prefs
bool CPrefsSpellingOptions::UpdateData(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	speller->GetPreferences()->mAutoPositionDialog.SetValue(mAutoPositionDialog->IsChecked());
	speller->GetPreferences()->mSpellOnSend.SetValue(mSpellOnSend->IsChecked());
	speller->GetPreferences()->mSpellAsYouType.SetValue(mSpellAsYouType->IsChecked());
	speller->GetPreferences()->mSpellColourBackground.SetValue(mSpellColourBackground->IsChecked());
	speller->GetPreferences()->mSpellBackgroundColour.SetValue(mSpellBackgroundColour->GetColor());
	
	return true;
}

void CPrefsSpellingOptions::OnSpellColourBackground()
{
	if (mSpellColourBackground->IsChecked())
		mSpellBackgroundColour->Activate();
	else
		mSpellBackgroundColour->Deactivate();
}
