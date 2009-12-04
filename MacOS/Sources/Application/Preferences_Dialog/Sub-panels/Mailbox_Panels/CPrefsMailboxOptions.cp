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


// Source for CPrefsMailboxOptions class

#include "CPrefsMailboxOptions.h"

#include "CPreferences.h"

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsMailboxOptions::CPrefsMailboxOptions()
{
}

// Constructor from stream
CPrefsMailboxOptions::CPrefsMailboxOptions(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsMailboxOptions::~CPrefsMailboxOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxOptions::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mPromptStartup = (LCheckBox*) FindPaneByID(paneid_MDOPromptStartup);
	mAutoDial = (LCheckBox*) FindPaneByID(paneid_MDOAutoDial);
	mUIDValidityMismatch = (LCheckBox*) FindPaneByID(paneid_MDOUIDValidityMismatch);
	mOSDefaultLocation = (LCheckBox*) FindPaneByID(paneid_MDOOSDefaultLocation);
	mSleepReconnect = (LCheckBox*) FindPaneByID(paneid_MDOOSSleepReconnect);
}

// Set prefs
void CPrefsMailboxOptions::SetData(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Copy info
	mPromptStartup->SetValue((copyPrefs->mPromptDisconnected.GetValue() ? Button_On : Button_Off));
	mAutoDial->SetValue((copyPrefs->mAutoDial.GetValue() ? Button_On : Button_Off));
	mUIDValidityMismatch->SetValue((copyPrefs->mUIDValidityMismatch.GetValue() ? Button_On : Button_Off));
	mOSDefaultLocation->SetValue((copyPrefs->mOSDefaultLocation.GetValue() ? Button_On : Button_Off));
	mSleepReconnect->SetValue((copyPrefs->mSleepReconnect.GetValue() ? Button_On : Button_Off));
}

// Force update of prefs
void CPrefsMailboxOptions::UpdateData(void* data)
{
	CPreferences* copyPrefs = static_cast<CPreferences*>(data);

	// Copy info from panel into prefs
	copyPrefs->mPromptDisconnected.SetValue((mPromptStartup->GetValue() == Button_On));
	copyPrefs->mAutoDial.SetValue((mAutoDial->GetValue() == Button_On));
	copyPrefs->mUIDValidityMismatch.SetValue((mUIDValidityMismatch->GetValue() == Button_On));
	copyPrefs->mOSDefaultLocation.SetValue((mOSDefaultLocation->GetValue() == Button_On));
	copyPrefs->mSleepReconnect.SetValue((mSleepReconnect->GetValue() == Button_On));
}
