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


// Source for CPrefsDisplay class

#include "CPrefsDisplay.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"

#include "CPrefsDisplayMailbox.h"
#include "CPrefsDisplayMessage.h"
#include "CPrefsDisplayLabel.h"
#include "CPrefsDisplayQuotes.h"
#include "CPrefsDisplayServer.h"
#include "CPrefsDisplayStyles.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsDisplay::CPrefsDisplay()
{
}

// Constructor from stream
CPrefsDisplay::CPrefsDisplay(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsDisplay::~CPrefsDisplay()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsDisplay::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_DisplayTabs);
	mAllowKeyboardShortcuts = (LCheckBox*) FindPaneByID(paneid_DisplayAllowKeyboardShortcuts);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsDisplayServer);
	mTabs->AddPanel(paneid_PrefsDisplayMailbox);
	mTabs->AddPanel(paneid_PrefsDisplayMessage);
	mTabs->AddPanel(paneid_PrefsDisplayLabel);
	mTabs->AddPanel(paneid_PrefsDisplayQuotes);
	mTabs->AddPanel(paneid_PrefsDisplayStyles);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsDisplayBtns);

}

// Handle buttons
void CPrefsDisplay::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_DisplayTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

		case msg_ResetWindows:
			mCopyPrefs->ResetAllStates(true);
			CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
			while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
				prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
			prefs_dlog->SetForceWindowReset(true);
			break;

		default:
			break;
	}
}

// Toggle display of IC items
void CPrefsDisplay::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsDisplay::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;
	CPreferences* local_prefs = copyPrefs;

	// Toggle IC display
	ToggleICDisplay();

	// Set existing panel
	mTabs->SetData(mCopyPrefs);

	mAllowKeyboardShortcuts->SetValue(copyPrefs->mAllowKeyboardShortcuts.GetValue());
}

// Force update of prefs
void CPrefsDisplay::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	// Update existing panel
	mTabs->UpdateData(mCopyPrefs);

	local_prefs->mAllowKeyboardShortcuts.SetValue(mAllowKeyboardShortcuts->GetValue());
}
