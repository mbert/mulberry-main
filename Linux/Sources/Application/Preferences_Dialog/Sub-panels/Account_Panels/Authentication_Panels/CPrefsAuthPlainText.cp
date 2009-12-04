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


// Source for CPrefsAuthPlainText class

#include "CPrefsAuthPlainText.h"

#include "CAdminLock.h"
#include "CAuthenticator.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAuthPlainText::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("User ID:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,12, 55,15);
    assert( obj1 != NULL );

    mUID =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,10, 225,20);
    assert( mUID != NULL );

    mSaveUser =
        new JXTextCheckbox("Save User ID", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 105,20);
    assert( mSaveUser != NULL );

    mSavePswd =
        new JXTextCheckbox("Save Password", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 115,20);
    assert( mSavePswd != NULL );

// end JXLayout1
	
	// Disable certain items
	if (CAdminLock::sAdminLock.mLockSavePswd)
		mSavePswd->Deactivate();
}

// Set prefs
void CPrefsAuthPlainText::SetAuth(CAuthenticator* auth)
{
	mUID->SetText(static_cast<CAuthenticatorUserPswd*>(auth)->GetUID());
	mSaveUser->SetState(JBoolean(static_cast<CAuthenticatorUserPswd*>(auth)->GetSaveUID()));
	mSavePswd->SetState(JBoolean(static_cast<CAuthenticatorUserPswd*>(auth)->GetSavePswd() && !CAdminLock::sAdminLock.mLockSavePswd));
}

// Force update of prefs
void CPrefsAuthPlainText::UpdateAuth(CAuthenticator* auth)
{
	static_cast<CAuthenticatorUserPswd*>(auth)->SetUID(cdstring(mUID->GetText()));
	static_cast<CAuthenticatorUserPswd*>(auth)->SetSaveUID(mSaveUser->IsChecked());
	static_cast<CAuthenticatorUserPswd*>(auth)->SetSavePswd(mSavePswd->IsChecked() && !CAdminLock::sAdminLock.mLockSavePswd);

}

// Change items states
void CPrefsAuthPlainText::UpdateItems(bool enable)
{
	if (enable)
	{
		mUID->Activate();
		mSaveUser->Activate();
		if (!CAdminLock::sAdminLock.mLockSavePswd)
			mSavePswd->Activate();
	}
	else
	{
		mUID->Deactivate();
		mSaveUser->Deactivate();
		mSavePswd->Deactivate();
	}
}
