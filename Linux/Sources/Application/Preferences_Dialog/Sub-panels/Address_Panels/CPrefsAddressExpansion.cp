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


// Source for CPrefsAddressExpansion class

#include "CPrefsAddressExpansion.h"

#include "CPreferences.h"

#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressExpansion::OnCreate()
{
// begin JXLayout1

    mExpandNoNicknames =
        new JXTextCheckbox("Use Expansion instead of Nick-names", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,20, 245,20);
    assert( mExpandNoNicknames != NULL );

    mExpandFailedNicknames =
        new JXTextCheckbox("Expand when Nick-name Fails", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 205,20);
    assert( mExpandFailedNicknames != NULL );

    mExpandFullName =
        new JXTextCheckbox("Expand using Full Name", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,80, 170,20);
    assert( mExpandFullName != NULL );

    mExpandNickName =
        new JXTextCheckbox("Expand using Nick Name", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,100, 170,20);
    assert( mExpandNickName != NULL );

    mExpandEmail =
        new JXTextCheckbox("Expand using Email Address", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,120, 190,20);
    assert( mExpandEmail != NULL );

    mSkipLDAP =
        new JXTextCheckbox("Skip LDAP if Address Book Results are Found", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,160, 320,20);
    assert( mSkipLDAP != NULL );

// end JXLayout1

	// Start listening
	ListenTo(mExpandNoNicknames);
}

void CPrefsAddressExpansion::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mExpandNoNicknames)
		{
			OnExpandNoNickname();
			return;
		}
	}
}

// Set prefs
void CPrefsAddressExpansion::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info to into panel
	mExpandNoNicknames->SetState(JBoolean(copyPrefs->mExpandNoNicknames.GetValue()));
	if (copyPrefs->mExpandNoNicknames.GetValue())
		mExpandFailedNicknames->Deactivate();
	mExpandFailedNicknames->SetState(JBoolean(copyPrefs->mExpandFailedNicknames.GetValue()));
	mExpandFullName->SetState(JBoolean(copyPrefs->mExpandFullName.GetValue()));
	mExpandNickName->SetState(JBoolean(copyPrefs->mExpandNickName.GetValue()));
	mExpandEmail->SetState(JBoolean(copyPrefs->mExpandEmail.GetValue()));
	mSkipLDAP->SetState(JBoolean(copyPrefs->mSkipLDAP.GetValue()));
}

// Force update of prefs
bool CPrefsAddressExpansion::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mExpandNoNicknames.SetValue(mExpandNoNicknames->IsChecked());
	copyPrefs->mExpandFailedNicknames.SetValue(mExpandFailedNicknames->IsChecked());
	copyPrefs->mExpandFullName.SetValue(mExpandFullName->IsChecked());
	copyPrefs->mExpandNickName.SetValue(mExpandNickName->IsChecked());
	copyPrefs->mExpandEmail.SetValue(mExpandEmail->IsChecked());
	copyPrefs->mSkipLDAP.SetValue(mSkipLDAP->IsChecked());
	
	return true;
}

void CPrefsAddressExpansion::OnExpandNoNickname()
{
	if (mExpandNoNicknames->IsChecked())
		mExpandFailedNicknames->Deactivate();
	else
		mExpandFailedNicknames->Activate();
}
