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


// Source for CPrefsAccountIMAP class

#include "CPrefsAccountIMAP.h"

#include "CAdminLock.h"
#include "CMailAccount.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountIMAP::OnCreate()
{
// begin JXLayout1

    mLogonAtStartup =
        new JXTextCheckbox("Login at Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 115,20);
    assert( mLogonAtStartup != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Hierarchy Separator:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,37, 125,20);
    assert( obj1 != NULL );

    mDirectorySeparator =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 145,35, 20,20);
    assert( mDirectorySeparator != NULL );

    mAutoNamespace =
        new JXTextCheckbox("Automatic Hierarchy List", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 175,20);
    assert( mAutoNamespace != NULL );

    mDisconnected =
        new JXTextCheckbox("Allow Disconnected Operations", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,100, 205,20);
    assert( mDisconnected != NULL );

// end JXLayout1
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		mDisconnected->Deactivate();
}

// Set prefs
void CPrefsAccountIMAP::SetData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info
	mLogonAtStartup->SetState(JBoolean(account->GetLogonAtStart()));
	cdstring copyStr;
	if (account->GetDirDelim())
		copyStr = account->GetDirDelim();
	else
		copyStr[0UL] = 0;
	mDirectorySeparator->SetText(copyStr);
	mAutoNamespace->SetState(JBoolean(account->GetAutoNamespace()));
	mDisconnected->SetState(JBoolean(account->GetDisconnected()));
}

// Force update of prefs
bool CPrefsAccountIMAP::UpdateData(void* data)
{
	CMailAccount* account = (CMailAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup->IsChecked());

	cdstring copyStr(mDirectorySeparator->GetText());
	if (copyStr.length())
		account->SetDirDelim(copyStr[0UL]);
	else
		account->SetDirDelim(0);

	account->SetAutoNamespace(mAutoNamespace->IsChecked());
	account->SetDisconnected(mDisconnected->IsChecked());
	
	return true;
}
