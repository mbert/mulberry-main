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


// Source for CPrefsMailboxDisconnected class

#include "CPrefsMailboxDisconnected.h"

#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"
#include "CPrefsMailboxOptions.h"
#include "CPrefsAccountLocal.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMailboxDisconnected::CPrefsMailboxDisconnected()
{
}

// Constructor from stream
CPrefsMailboxDisconnected::CPrefsMailboxDisconnected(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsMailboxDisconnected::~CPrefsMailboxDisconnected()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxDisconnected::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_MDTabs);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	mTabs->AddPanel(paneid_PrefsMailboxOptions);
	mTabs->AddPanel(paneid_PrefsAccountLocal);
	static_cast<CPrefsAccountLocal*>(mTabs->GetPanel(1))->SetDisconnected();
	mTabs->SetDisplayPanel(1);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsMailboxDisconnectedBtns);
}

// Handle buttons
void CPrefsMailboxDisconnected::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_MDTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

		default:;
	}
}

// Set prefs
void CPrefsMailboxDisconnected::SetData(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Copy info
	mTabs->SetData(copyPrefs);
}

// Force update of prefs
void CPrefsMailboxDisconnected::UpdateData(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Copy info from panel into prefs
	mTabs->UpdateData(copyPrefs);
}
