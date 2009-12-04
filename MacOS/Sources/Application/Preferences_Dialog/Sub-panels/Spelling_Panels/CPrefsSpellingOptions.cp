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
#include "CSpellPlugin.h"

#include <LCheckBox.h>
#include <LGAColorSwatchControl.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSpellingOptions::CPrefsSpellingOptions()
{
}

// Constructor from stream
CPrefsSpellingOptions::CPrefsSpellingOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsSpellingOptions::~CPrefsSpellingOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSpellingOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mCaseSensitive = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsCaseSensitive);
	mIgnoreCapitalised = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsIgnoreCapitalised);
	mIgnoreAllCaps = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsIgnoreAllCaps);
	mIgnoreWordsNumbers = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsIgnoreWordsNumbers);
	mIgnoreMixedCase = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsIgnoreMixedCase);
	mIgnoreDomainNames = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsIgnoreDomainNames);
	mReportDoubledWords = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsReportDoubledWords);
	mAutoCorrect = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsAutoCorrect);

	mAutoPositionDialog = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsAutoPositionDialog);
	mSpellOnSend = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsSpellOnSend);
	mSpellAsYouType = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsSpellAsYouType);
	mSpellColourBackground = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsSpellColourBackground);
	mSpellBackgroundColour = (LGAColorSwatchControl*) FindPaneByID(paneid_PrefsSpellOptionsSpellBackgroundColour);
}

// Handle buttons
void CPrefsSpellingOptions::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_PrefsSpellOptionsSpellColourBackground:
		if (*(long*) ioParam)
			mSpellBackgroundColour->Enable();
		else
			mSpellBackgroundColour->Disable();
		break;
	}
}

inline int operator==(const RGBColor& c1, const RGBColor& c2);
inline int operator==(const RGBColor& c1, const RGBColor& c2)
{
	return (c1.red == c2.red) && (c1.green == c2.green) && (c1.blue == c2.blue);
}

// Set prefs
void CPrefsSpellingOptions::SetData(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	// Copy info
	mCaseSensitive->SetValue(speller->GetPreferences()->mCaseSensitive.GetValue());
	mIgnoreCapitalised->SetValue(speller->GetPreferences()->mIgnoreCapitalised.GetValue());
	mIgnoreAllCaps->SetValue(speller->GetPreferences()->mIgnoreAllCaps.GetValue());
	mIgnoreWordsNumbers->SetValue(speller->GetPreferences()->mIgnoreWordsNumbers.GetValue());
	mIgnoreMixedCase->SetValue(speller->GetPreferences()->mIgnoreMixedCase.GetValue());
	mIgnoreDomainNames->SetValue(speller->GetPreferences()->mIgnoreDomainNames.GetValue());
	mReportDoubledWords->SetValue(speller->GetPreferences()->mReportDoubledWords.GetValue());
	mAutoCorrect->SetValue(speller->GetPreferences()->mAutoCorrect.GetValue());

	mAutoPositionDialog->SetValue(speller->GetPreferences()->mAutoPositionDialog.GetValue());
	mSpellOnSend->SetValue(speller->GetPreferences()->mSpellOnSend.GetValue());
	mSpellAsYouType->SetValue(speller->GetPreferences()->mSpellAsYouType.GetValue());
	mSpellColourBackground->SetValue(speller->GetPreferences()->mSpellColourBackground.GetValue());
	mSpellBackgroundColour->SetSwatchColor(speller->GetPreferences()->mSpellBackgroundColour.Value());
	if (!speller->GetPreferences()->mSpellColourBackground.GetValue())
		mSpellBackgroundColour->Disable();
}

// Force update of prefs
void CPrefsSpellingOptions::UpdateData(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	// Copy info from panel into prefs
	speller->GetPreferences()->mCaseSensitive.SetValue(mCaseSensitive->GetValue());
	speller->GetPreferences()->mIgnoreCapitalised.SetValue(mIgnoreCapitalised->GetValue());
	speller->GetPreferences()->mIgnoreAllCaps.SetValue(mIgnoreAllCaps->GetValue());
	speller->GetPreferences()->mIgnoreWordsNumbers.SetValue(mIgnoreWordsNumbers->GetValue());
	speller->GetPreferences()->mIgnoreMixedCase.SetValue(mIgnoreMixedCase->GetValue());
	speller->GetPreferences()->mIgnoreDomainNames.SetValue(mIgnoreDomainNames->GetValue());
	speller->GetPreferences()->mReportDoubledWords.SetValue(mReportDoubledWords->GetValue());
	speller->GetPreferences()->mAutoCorrect.SetValue(mAutoCorrect->GetValue());

	speller->GetPreferences()->mAutoPositionDialog.SetValue(mAutoPositionDialog->GetValue());
	speller->GetPreferences()->mSpellOnSend.SetValue(mSpellOnSend->GetValue());
	speller->GetPreferences()->mSpellAsYouType.SetValue(mSpellAsYouType->GetValue());
	speller->GetPreferences()->mSpellColourBackground.SetValue(mSpellColourBackground->GetValue());
	RGBColor temp;
	mSpellBackgroundColour->GetSwatchColor(temp);
	speller->GetPreferences()->mSpellBackgroundColour.SetValue(temp);
}
