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

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CSpellPlugin.h"

#include <LCheckBox.h>
#include <LGAColorSwatchControl.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellOptionsDialog::CSpellOptionsDialog()
{
}

// Constructor from stream
CSpellOptionsDialog::CSpellOptionsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CSpellOptionsDialog::~CSpellOptionsDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellOptionsDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mIgnoreCapitalised = (LCheckBox*) FindPaneByID(paneid_SpellOptionsIgnoreCapitalised);
	mIgnoreAllCaps = (LCheckBox*) FindPaneByID(paneid_SpellOptionsIgnoreAllCaps);
	mIgnoreWordsNumbers = (LCheckBox*) FindPaneByID(paneid_SpellOptionsIgnoreWordsNumbers);
	mIgnoreMixedCase = (LCheckBox*) FindPaneByID(paneid_SpellOptionsIgnoreMixedCase);
	mIgnoreDomainNames = (LCheckBox*) FindPaneByID(paneid_SpellOptionsIgnoreDomainNames);
	mReportDoubledWords = (LCheckBox*) FindPaneByID(paneid_SpellOptionsReportDoubledWords);
	mCaseSensitive = (LCheckBox*) FindPaneByID(paneid_SpellOptionsCaseSensitive);
	mPhoneticSuggestions = (LCheckBox*) FindPaneByID(paneid_SpellOptionsPhoneticSuggestions);
	mTypoSuggestions = (LCheckBox*) FindPaneByID(paneid_SpellOptionsTypoSuggestions);
	mSuggestSplit = (LCheckBox*) FindPaneByID(paneid_SpellOptionsSuggestSplit);
	mAutoCorrect = (LCheckBox*) FindPaneByID(paneid_SpellOptionsAutoCorrect);

	mAutoPositionDialog = (LCheckBox*) FindPaneByID(paneid_SpellOptionsAutoPositionDialog);
	mSpellOnSend = (LCheckBox*) FindPaneByID(paneid_SpellOptionsSpellOnSend);
	mSpellAsYouType = (LCheckBox*) FindPaneByID(paneid_SpellOptionsSpellAsYouType);
	mSpellColourBackground = (LCheckBox*) FindPaneByID(paneid_SpellOptionsSpellColourBackground);
	mSpellBackgroundColour = (LGAColorSwatchControl*) FindPaneByID(paneid_SpellOptionsSpellBackgroundColour);

	mSuggestFast = (LRadioButton*) FindPaneByID(paneid_SpellOptionsSpellSuggestFast);
	mSuggestMedium = (LRadioButton*) FindPaneByID(paneid_SpellOptionsSpellSuggestMedium);
	mSuggestSlow = (LRadioButton*) FindPaneByID(paneid_SpellOptionsSpellSuggestSlow);
	
	mDictionaries = (LPopupButton*) FindPaneByID(paneid_SpellOptionsDictionaries);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSpellOptionsDialog);
}

// Set the details
void CSpellOptionsDialog::SetSpeller(CSpellPlugin* speller)
{
	mIgnoreCapitalised->SetValue(speller->GetPreferences()->mIgnoreCapitalised.GetValue());
	mIgnoreAllCaps->SetValue(speller->GetPreferences()->mIgnoreAllCaps.GetValue());
	mIgnoreWordsNumbers->SetValue(speller->GetPreferences()->mIgnoreWordsNumbers.GetValue());
	mIgnoreMixedCase->SetValue(speller->GetPreferences()->mIgnoreMixedCase.GetValue());
	mIgnoreDomainNames->SetValue(speller->GetPreferences()->mIgnoreDomainNames.GetValue());
	mReportDoubledWords->SetValue(speller->GetPreferences()->mReportDoubledWords.GetValue());
	mCaseSensitive->SetValue(speller->GetPreferences()->mCaseSensitive.GetValue());
	mPhoneticSuggestions->SetValue(speller->GetPreferences()->mPhoneticSuggestions.GetValue());
	mTypoSuggestions->SetValue(speller->GetPreferences()->mTypoSuggestions.GetValue());
	mSuggestSplit->SetValue(speller->GetPreferences()->mSuggestSplit.GetValue());
	mAutoCorrect->SetValue(speller->GetPreferences()->mAutoCorrect.GetValue());

	mAutoPositionDialog->SetValue(speller->GetPreferences()->mAutoPositionDialog.GetValue());
	mSpellOnSend->SetValue(speller->GetPreferences()->mSpellOnSend.GetValue());
	mSpellAsYouType->SetValue(speller->GetPreferences()->mSpellAsYouType.GetValue());
	mSpellColourBackground->SetValue(speller->GetPreferences()->mSpellColourBackground.GetValue());
	mSpellBackgroundColour->SetSwatchColor(speller->GetPreferences()->mSpellBackgroundColour.Value());
	if (!speller->GetPreferences()->mSpellColourBackground.GetValue())
		mSpellBackgroundColour->Disable();

	mSuggestFast->SetValue(speller->GetPreferences()->mSuggestLevel.GetValue() == CSpellPreferencesDLL::eSuggest_Fast);
	mSuggestMedium->SetValue(speller->GetPreferences()->mSuggestLevel.GetValue() == CSpellPreferencesDLL::eSuggest_Medium);
	mSuggestSlow->SetValue(speller->GetPreferences()->mSuggestLevel.GetValue() == CSpellPreferencesDLL::eSuggest_Slow);
	
	InitDictionaries(speller);
}

// Handle buttons
void CSpellOptionsDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
	case msg_SpellOptionsSpellColourBackground:
		if (*(long*) ioParam)
			mSpellBackgroundColour->Enable();
		else
			mSpellBackgroundColour->Disable();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

inline int operator==(const RGBColor& c1, const RGBColor& c2);
inline int operator==(const RGBColor& c1, const RGBColor& c2)
{
	return (c1.red == c2.red) && (c1.green == c2.green) && (c1.blue == c2.blue);
}

// Get the details
void CSpellOptionsDialog::GetOptions(CSpellPlugin* speller)
{
	speller->GetPreferences()->mIgnoreCapitalised.SetValue(mIgnoreCapitalised->GetValue());
	speller->GetPreferences()->mIgnoreAllCaps.SetValue(mIgnoreAllCaps->GetValue());
	speller->GetPreferences()->mIgnoreWordsNumbers.SetValue(mIgnoreWordsNumbers->GetValue());
	speller->GetPreferences()->mIgnoreMixedCase.SetValue(mIgnoreMixedCase->GetValue());
	speller->GetPreferences()->mIgnoreDomainNames.SetValue(mIgnoreDomainNames->GetValue());
	speller->GetPreferences()->mReportDoubledWords.SetValue(mReportDoubledWords->GetValue());
	speller->GetPreferences()->mCaseSensitive.SetValue(mCaseSensitive->GetValue());
	speller->GetPreferences()->mPhoneticSuggestions.SetValue(mPhoneticSuggestions->GetValue());
	speller->GetPreferences()->mTypoSuggestions.SetValue(mTypoSuggestions->GetValue());
	speller->GetPreferences()->mSuggestSplit.SetValue(mSuggestSplit->GetValue());
	speller->GetPreferences()->mAutoCorrect.SetValue(mAutoCorrect->GetValue());

	speller->GetPreferences()->mAutoPositionDialog.SetValue(mAutoPositionDialog->GetValue());
	speller->GetPreferences()->mSpellOnSend.SetValue(mSpellOnSend->GetValue());
	speller->GetPreferences()->mSpellAsYouType.SetValue(mSpellAsYouType->GetValue());
	speller->GetPreferences()->mSpellColourBackground.SetValue(mSpellColourBackground->GetValue());
	RGBColor temp;
	mSpellBackgroundColour->GetSwatchColor(temp);
	speller->GetPreferences()->mSpellBackgroundColour.SetValue(temp);

	if (mSuggestFast->GetValue())
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Fast);
	else if (mSuggestMedium->GetValue())
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Medium);
	else if (mSuggestSlow->GetValue())
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Slow);
	
	Str255 dname;
	mDictionaries->GetMenuItemText(mDictionaries->GetValue(), dname);
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(dname));

	speller->UpdatePreferences();
}

// Set dictionary popup
void CSpellOptionsDialog::InitDictionaries(CSpellPlugin* speller)
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

bool CSpellOptionsDialog::PoseDialog(CSpellPlugin* speller)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_SpellOptionsDialog, CMulberryApp::sApp);
	CSpellOptionsDialog* dlog = (CSpellOptionsDialog*) theHandler.GetDialog();
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
