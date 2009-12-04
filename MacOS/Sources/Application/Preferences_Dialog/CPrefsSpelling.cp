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


// Source for CPrefsSpelling class

#include "CPrefsSpelling.h"

#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"
#include "CSpellPlugin.h"

#include "CPrefsSpellingOptions.h"
#include "CPrefsSpellingSuggestions.h"

#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsSpelling::CPrefsSpelling()
{
}

// Constructor from stream
CPrefsSpelling::CPrefsSpelling(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsSpelling::~CPrefsSpelling()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsSpelling::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_SpellingTabs);

	mDictionaries = (LPopupButton*) FindPaneByID(paneid_SpellingDictionary);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsSpellingOptions);
	mTabs->AddPanel(paneid_PrefsSpellingSuggestions);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsSpellingBtns);

}

// Handle buttons
void CPrefsSpelling::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_SpellingTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

		default:
			break;
	}
}

// Toggle display of IC items
void CPrefsSpelling::ToggleICDisplay(void)
{
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
void CPrefsSpelling::UpdatePrefs(void)
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
	Str255 dname;
	mDictionaries->GetMenuItemText(mDictionaries->GetValue(), dname);
	speller->GetPreferences()->mDictionaryName.SetValue(cdstring(dname));

	speller->UpdatePreferences();
}

// Set dictionary popup
void CPrefsSpelling::InitDictionaries(CSpellPlugin* speller)
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
