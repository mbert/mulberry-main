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

#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsMailboxOptions::OnCreate()
{
// begin JXLayout1

    mPromptStartup =
        new JXTextCheckbox("Prompt on Startup if Required", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 195,20);
    assert( mPromptStartup != NULL );

    mAutoDial =
        new JXTextCheckbox("Automatic Modem Dial/Hangup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 205,20);
    assert( mAutoDial != NULL );

    mUIDValidityMismatch =
        new JXTextCheckbox("Warn if Mailboxes do not Match", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 210,20);
    assert( mUIDValidityMismatch != NULL );

    mOSDefaultLocation =
        new JXTextCheckbox("Use OS Specific Default Locations", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,80, 220,20);
    assert( mOSDefaultLocation != NULL );

    mSleepReconnect =
        new JXTextCheckbox("Connect after Wake if Network Connections Found", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 320,20);
    assert( mSleepReconnect != NULL );

// end JXLayout1
}

// Set prefs
void CPrefsMailboxOptions::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info
	mPromptStartup->SetState(JBoolean(copyPrefs->mPromptDisconnected.GetValue()));
	mAutoDial->SetState(JBoolean(copyPrefs->mAutoDial.GetValue()));
	mUIDValidityMismatch->SetState(JBoolean(copyPrefs->mUIDValidityMismatch.GetValue()));
	mOSDefaultLocation->SetState(JBoolean(copyPrefs->mOSDefaultLocation.GetValue()));
	mSleepReconnect->SetState(JBoolean(copyPrefs->mSleepReconnect.GetValue()));
}

// Force update of prefs
bool CPrefsMailboxOptions::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mPromptDisconnected.SetValue(mPromptStartup->IsChecked());
	copyPrefs->mAutoDial.SetValue(mAutoDial->IsChecked());
	copyPrefs->mUIDValidityMismatch.SetValue(mUIDValidityMismatch->IsChecked());
	copyPrefs->mOSDefaultLocation.SetValue(mOSDefaultLocation->IsChecked());
	copyPrefs->mSleepReconnect.SetValue(mSleepReconnect->IsChecked());
	
	return true;
}
