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


// Source for CPrefsAccountPanel class

#include "CPrefsAccountPanel.h"

#include "CPreferencesDialog.h"
#include "CPrefsSubTab.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountPanel::CPrefsAccountPanel()
{
}

// Constructor from stream
CPrefsAccountPanel::CPrefsAccountPanel(LStream *inStream)
		: LView(inStream)
{
}

// Default destructor
CPrefsAccountPanel::~CPrefsAccountPanel()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountPanel::FinishCreateSelf(void)
{
	// Do inherited
	LView::FinishCreateSelf();

	// Get controls
	mTabs = (CPrefsSubTab*) FindPaneByID(paneid_PrefsAccountTabs);
	mTabs->AddListener(this);

	// Set tabber as default commander
	CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) GetSuperView();
	while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
		prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
	LCommander* defCommander;
	prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
	defCommander->SetDefaultCommander(defCommander);

	// Create tab sub-panels
	InitTabs();
	mTabs->SetDisplayPanel(1);

}

// Handle buttons
void CPrefsAccountPanel::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_PrefsAccountTabs:
			mTabs->SetDisplayPanel(*(unsigned long*) ioParam);
			break;

		default:
			break;
	}
}

// Toggle display of IC
void CPrefsAccountPanel::ToggleICDisplay(bool IC_on)
{
	if (mTabs)
		mTabs->ToggleICDisplay(IC_on);
}

// Set prefs
void CPrefsAccountPanel::SetData(void* data)
{
	mTabs->SetData(data);
}

// Force update of prefs
void CPrefsAccountPanel::UpdateData(void* data)
{
	mTabs->UpdateData(data);
}
