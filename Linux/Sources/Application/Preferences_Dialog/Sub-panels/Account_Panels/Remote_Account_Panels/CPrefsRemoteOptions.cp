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


// Source for CPrefsRemoteOptions class

#include "CPrefsRemoteOptions.h"

#include "COptionsAccount.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsRemoteOptions::OnCreate()
{
// begin JXLayout1

    mUseRemote =
        new JXTextCheckbox("Use Remote Preferences at Startup", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 225,20);
    assert( mUseRemote != NULL );

    mBaseRURLText =
        new JXStaticText("Path:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,57, 35,20);
    assert( mBaseRURLText != NULL );

    mBaseRURL =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,55, 280,20);
    assert( mBaseRURL != NULL );

// end JXLayout1
}

// Set prefs
void CPrefsRemoteOptions::SetData(void* data)
{
	COptionsAccount* acct = (COptionsAccount*) data;

	// Copy info
	mUseRemote->SetState(JBoolean(acct->GetLogonAtStart()));

	if (acct->GetServerType() == CINETAccount::eWebDAVPrefs)
		mBaseRURL->SetText(acct->GetBaseRURL());
	else
	{
		mBaseRURLText->Hide();
		mBaseRURL->Hide();
	}
}

// Force update of prefs
bool CPrefsRemoteOptions::UpdateData(void* data)
{
	COptionsAccount* acct = (COptionsAccount*) data;

	acct->SetLoginAtStart(mUseRemote->IsChecked());
	if (acct->GetServerType() == CINETAccount::eWebDAVPrefs)
		acct->SetBaseRURL(mBaseRURL->GetText());
	
	return true;
}
