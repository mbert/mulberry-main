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


// Source for CPrefsSpellingSuggestions class

#include "CPrefsSpellingSuggestions.h"

#include "CPreferences.h"
#include "CSpellPlugin.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSpellingSuggestions::CPrefsSpellingSuggestions()
{
}

// Constructor from stream
CPrefsSpellingSuggestions::CPrefsSpellingSuggestions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsSpellingSuggestions::~CPrefsSpellingSuggestions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSpellingSuggestions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mPhoneticSuggestions = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsPhoneticSuggestions);
	mTypoSuggestions = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsTypoSuggestions);
	mSuggestSplit = (LCheckBox*) FindPaneByID(paneid_PrefsSpellOptionsSuggestSplit);

	mSuggestFast = (LRadioButton*) FindPaneByID(paneid_PrefsSpellOptionsSpellSuggestFast);
	mSuggestMedium = (LRadioButton*) FindPaneByID(paneid_PrefsSpellOptionsSpellSuggestMedium);
	mSuggestSlow = (LRadioButton*) FindPaneByID(paneid_PrefsSpellOptionsSpellSuggestSlow);
}

// Set prefs
void CPrefsSpellingSuggestions::SetData(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	// Copy info
	mPhoneticSuggestions->SetValue(speller->GetPreferences()->mPhoneticSuggestions.GetValue());
	mTypoSuggestions->SetValue(speller->GetPreferences()->mTypoSuggestions.GetValue());
	mSuggestSplit->SetValue(speller->GetPreferences()->mSuggestSplit.GetValue());

	mSuggestFast->SetValue(speller->GetPreferences()->mSuggestLevel.GetValue() == CSpellPreferencesDLL::eSuggest_Fast);
	mSuggestMedium->SetValue(speller->GetPreferences()->mSuggestLevel.GetValue() == CSpellPreferencesDLL::eSuggest_Medium);
	mSuggestSlow->SetValue(speller->GetPreferences()->mSuggestLevel.GetValue() == CSpellPreferencesDLL::eSuggest_Slow);
}

// Force update of prefs
void CPrefsSpellingSuggestions::UpdateData(void* data)
{
	CSpellPlugin* speller = static_cast<CSpellPlugin*>(data);

	// Copy info from panel into prefs
	speller->GetPreferences()->mPhoneticSuggestions.SetValue(mPhoneticSuggestions->GetValue());
	speller->GetPreferences()->mTypoSuggestions.SetValue(mTypoSuggestions->GetValue());
	speller->GetPreferences()->mSuggestSplit.SetValue(mSuggestSplit->GetValue());

	if (mSuggestFast->GetValue())
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Fast);
	else if (mSuggestMedium->GetValue())
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Medium);
	else if (mSuggestSlow->GetValue())
		speller->GetPreferences()->mSuggestLevel.SetValue(CSpellPreferencesDLL::eSuggest_Slow);
}
