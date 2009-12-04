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


// Source for CPrefsAccountRemoteCalendar class

#include "CPrefsAccountRemoteCalendar.h"

#include "CAdminLock.h"
#include "CCalendarAccount.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountRemoteCalendar::OnCreate()
{
// begin JXLayout1

    mDisconnected =
        new JXTextCheckbox("Allow Disconnected Operations", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 205,20);
    assert( mDisconnected != NULL );

    mLogonAtStartup =
        new JXTextCheckbox("Login at Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 115,20);
    assert( mLogonAtStartup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Path:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,87, 35,20);
    assert( obj1 != NULL );

    mBaseURL =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,85, 280,20);
    assert( mBaseURL != NULL );

// end JXLayout1
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		mDisconnected->Deactivate();
}

// Set prefs
void CPrefsAccountRemoteCalendar::SetData(void* data)
{
	CCalendarAccount* account = (CCalendarAccount*) data;

	// Copy info
	mLogonAtStartup->SetState(JBoolean(account->GetLogonAtStart()));
	mDisconnected->SetState(JBoolean(account->GetDisconnected()));
	mBaseURL->SetText(account->GetBaseRURL());
}

// Force update of prefs
bool CPrefsAccountRemoteCalendar::UpdateData(void* data)
{
	CCalendarAccount* account = (CCalendarAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup->IsChecked());
	account->SetDisconnected(mDisconnected->IsChecked());
	account->SetBaseRURL(mBaseURL->GetText());
	
	return true;
}
