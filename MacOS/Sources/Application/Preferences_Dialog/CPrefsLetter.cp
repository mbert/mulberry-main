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


// Source for CPrefsLetter class

#include "CPrefsLetter.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"

#include "CPrefsLetterGeneral.h"
#include "CPrefsLetterOptions.h"
#include "CPrefsLetterStyled.h"

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M A I L E R
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsLetter::CPrefsLetter()
{
}

// Constructor from stream
CPrefsLetter::CPrefsLetter(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsLetter::~CPrefsLetter()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsLetter::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_LetterTabs);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsLetterGeneral);
	mTabs->AddPanel(paneid_PrefsLetterOptions);
	mTabs->AddPanel(paneid_PrefsLetterStyled);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsLetterBtns);
}

// Toggle display of IC items
void CPrefsLetter::ToggleICDisplay(void)
{
	mTabs->ToggleICDisplay(mCopyPrefs->mUse_IC.GetValue());
}

// Handle buttons
void CPrefsLetter::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
		case msg_LetterTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

	}
}

// Set prefs
void CPrefsLetter::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay();

	// Set existing panel
	mTabs->SetData(mCopyPrefs);
}

// Force update of prefs
void CPrefsLetter::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	// Update existing panel
	mTabs->UpdateData(mCopyPrefs);
}
