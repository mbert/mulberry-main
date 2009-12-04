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


// CPrefsSpelling.cp : implementation file
//


#include "CPrefsSpelling.h"

#include "CPluginManager.h"
#include "CPreferences.h"
#include "CTabController.h"
#include "CSpellPlugin.h"

#include "CPrefsSpellingOptions.h"

#include "TPopupMenu.h"

#include <JXStaticText.h>

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpelling property page

CPrefsSpelling::CPrefsSpelling(JXContainer* enclosure,
								const HSizingOption hSizing, 
								const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpelling message handlers

void CPrefsSpelling::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Spelling Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 130,20);
    assert( obj1 != NULL );

    mTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,25, 380,330);
    assert( mTabs != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Main Dictionary:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 8,368, 100,20);
    assert( obj2 != NULL );

    mDictionaries =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 108,365, 220,20);
    assert( mDictionaries != NULL );

// end JXLayout1

	// Create tab panels
	CTabPanel* card = new CPrefsSpellingOptions(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 380, 305);
	mTabs->AppendCard(card, "Options");
	mHasDictionaries = false;
}

// Set prefs
void CPrefsSpelling::SetPrefs(CPreferences* copyPrefs)
{
	CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
	
	if (speller != NULL)
	{
		CSpellPlugin::StLoadSpellPlugin _load(speller);

		SetSpeller(speller);

		// Set existing panel
		mTabs->SetData(speller);
	}
}

// Force update of prefs
void CPrefsSpelling::UpdatePrefs(CPreferences* prefs)
{
	CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
	
	if (speller != NULL)
	{
		CSpellPlugin::StLoadSpellPlugin _load(speller);

		// Update existing panel
		mTabs->UpdateData(speller);

		GetOptions(speller);
	}
}

// Set the details
void CPrefsSpelling::SetSpeller(CSpellPlugin* speller)
{
	InitDictionaries(speller);
}

// Get the details
void CPrefsSpelling::GetOptions(CSpellPlugin* speller)
{
	if (mHasDictionaries)
		speller->GetPreferences()->mDictionaryName.SetValue(cdstring(mDictionaries->GetCurrentItemText()));

	speller->UpdatePreferences();
}

// Called during idle
void CPrefsSpelling::InitDictionaries(CSpellPlugin* speller)
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
		mHasDictionaries = true;
	}
}

