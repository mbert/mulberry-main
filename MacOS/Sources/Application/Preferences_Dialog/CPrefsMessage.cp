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


// Source for CPrefsMessage class

#include "CPrefsMessage.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"

#include "CPrefsMessageGeneral.h"
#include "CPrefsMessageOptions.h"

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M E S S A G E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMessage::CPrefsMessage()
{
}

// Constructor from stream
CPrefsMessage::CPrefsMessage(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsMessage::~CPrefsMessage()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMessage::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_MessageTabs);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsMessageGeneral);
	mTabs->AddPanel(paneid_PrefsMessageOptions);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsMessageBtns);
}

// Toggle display of IC items
void CPrefsMessage::ToggleICDisplay(void)
{
}

// Handle buttons
void CPrefsMessage::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_MessageTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;
	}
}

// Set prefs
void CPrefsMessage::SetPrefs(CPreferences* copyPrefs)
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
void CPrefsMessage::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	// Update existing panel
	mTabs->UpdateData(mCopyPrefs);
}
