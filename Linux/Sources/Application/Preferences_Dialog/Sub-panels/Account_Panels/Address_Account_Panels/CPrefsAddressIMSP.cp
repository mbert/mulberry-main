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


// Source for CPrefsAddressIMSP class

#include "CPrefsAddressIMSP.h"

#include "CAddressAccount.h"
#include "CAdminLock.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressIMSP::OnCreate()
{
// begin JXLayout1

    mLogonAtStartup =
        new JXTextCheckbox("Login at Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 125,20);
    assert( mLogonAtStartup != NULL );

    mDisconnected =
        new JXTextCheckbox("Allow Allow Disconnected Operations", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 245,20);
    assert( mDisconnected != NULL );

    mBaseRURLText =
        new JXStaticText("Path:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,92, 35,20);
    assert( mBaseRURLText != NULL );

    mBaseRURL =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,90, 280,20);
    assert( mBaseRURL != NULL );

// end JXLayout1

	// Disable certain items
	if (CAdminLock::sAdminLock.mNoDisconnect)
		mDisconnected->Deactivate();
}

// Set prefs
void CPrefsAddressIMSP::SetData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info
	mLogonAtStartup->SetState(JBoolean(account->GetLogonAtStart()));
	mDisconnected->SetState(JBoolean(account->GetDisconnected()));

	if (account->GetServerType() == CINETAccount::eCardDAVAdbk)
		mBaseRURL->SetText(account->GetBaseRURL());
	else
	{
		mBaseRURLText->Hide();
		mBaseRURL->Hide();
	}
}

// Force update of prefs
bool CPrefsAddressIMSP::UpdateData(void* data)
{
	CAddressAccount* account = (CAddressAccount*) data;

	// Copy info from panel into prefs
	account->SetLoginAtStart(mLogonAtStartup->IsChecked());
	account->SetDisconnected(mDisconnected->IsChecked());
	if (account->GetServerType() == CINETAccount::eCardDAVAdbk)
		account->SetBaseRURL(mBaseRURL->GetText());
	
	return true;
}
