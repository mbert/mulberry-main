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


// Source for CPrefsAlerts class

#include "CPrefsAlerts.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"

#include "CPrefsAlertsAttachment.h"
#include "CPrefsAlertsMessage.h"

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S N E T W O R K
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAlerts::CPrefsAlerts()
{
}

// Constructor from stream
CPrefsAlerts::CPrefsAlerts(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsAlerts::~CPrefsAlerts()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAlerts::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_AlertsTabs);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsAlertsMessage);
	mTabs->AddPanel(paneid_PrefsAlertsAttachment);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAlertsBtns);
}

// Handle buttons
void CPrefsAlerts::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
		case msg_AlertsTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

	}
}

// Toggle display of IC items
void CPrefsAlerts::ToggleICDisplay(void)
{
	mTabs->ToggleICDisplay(mCopyPrefs->mUse_IC.GetValue());
}

// Set prefs
void CPrefsAlerts::SetPrefs(CPreferences* copyPrefs)
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
void CPrefsAlerts::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	// Update existing panel
	mTabs->UpdateData(mCopyPrefs);
}
