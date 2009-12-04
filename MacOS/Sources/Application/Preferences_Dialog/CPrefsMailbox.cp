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


// Source for CPrefsMailbox class

#include "CPrefsMailbox.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"

#include "CPrefsMailboxBasic.h"
#include "CPrefsMailboxAdvanced.h"
#include "CPrefsMailboxDisconnected.h"

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S M A I L B O X
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMailbox::CPrefsMailbox()
{
}

// Constructor from stream
CPrefsMailbox::CPrefsMailbox(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsMailbox::~CPrefsMailbox()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailbox::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_MailboxTabs);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsMailboxBasic);
	mTabs->AddPanel(paneid_PrefsMailboxAdvanced);
	mTabs->AddPanel(paneid_PrefsMailboxDisconnected);
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsMailboxBtns);
}

// Handle buttons
void CPrefsMailbox::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage)
	{
		case msg_MailboxTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

	}
}

// Toggle display of IC items
void CPrefsMailbox::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsMailbox::SetPrefs(CPreferences* copyPrefs)
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
void CPrefsMailbox::UpdatePrefs(void)
{
	CPreferences* local_prefs = mCopyPrefs;

	// Update existing panel
	mTabs->UpdateData(mCopyPrefs);
}
