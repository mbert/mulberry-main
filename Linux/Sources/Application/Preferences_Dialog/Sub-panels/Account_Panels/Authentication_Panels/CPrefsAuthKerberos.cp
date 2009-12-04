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


// Source for CPrefsAuthKerberos class

#include "CPrefsAuthKerberos.h"

#include "CAuthenticator.h"
#include "CTextField.h"

#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <cassert>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAuthKerberos::OnCreate()
{
// begin JXLayout1

    mDefaultPrincipal =
        new JXTextCheckbox("Use Default Server Principal", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,0, 190,20);
    assert( mDefaultPrincipal != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Server\nPrincipal:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,18, 60,35);
    assert( obj1 != NULL );

    mPrincipal =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,30, 225,20);
    assert( mPrincipal != NULL );

    mDefaultUID =
        new JXTextCheckbox("Use Default User ID", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,60, 190,20);
    assert( mDefaultUID != NULL );

    JXStaticText* obj2 =
        new JXStaticText("User ID:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,87, 60,20);
    assert( obj2 != NULL );

    mUID =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,85, 225,20);
    assert( mUID != NULL );

// end JXLayout1

	// Start Listening
	ListenTo(mDefaultPrincipal);
	ListenTo(mDefaultUID);
}

// Handle buttons
void CPrefsAuthKerberos::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mDefaultPrincipal)
		{
			if (mDefaultPrincipal->IsChecked())
				mPrincipal->Deactivate();
			else
				mPrincipal->Activate();
			return;
		}
		else if (sender == mDefaultUID)
		{
			if (mDefaultUID->IsChecked())
				mUID->Deactivate();
			else
				mUID->Activate();
			return;
		}
	}

	CPrefsAuthPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAuthKerberos::SetAuth(CAuthenticator* auth)
{
	mDefaultPrincipal->SetState(JBoolean(static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultPrincipal()));
	mPrincipal->SetText(static_cast<CAuthenticatorKerberos*>(auth)->GetServerPrincipal());

	mDefaultUID->SetState(JBoolean(static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultUID()));
	mUID->SetText(static_cast<CAuthenticatorKerberos*>(auth)->GetUID());

	if (static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultPrincipal())
		mPrincipal->Deactivate();
	if (static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultUID())
		mUID->Deactivate();
}

// Force update of prefs
void CPrefsAuthKerberos::UpdateAuth(CAuthenticator* auth)
{
	static_cast<CAuthenticatorKerberos*>(auth)->SetUsePrincipal(mDefaultPrincipal->IsChecked());
	static_cast<CAuthenticatorKerberos*>(auth)->SetServerPrincipal(cdstring(mPrincipal->GetText()));

	static_cast<CAuthenticatorKerberos*>(auth)->SetUseUID(mDefaultUID->IsChecked());
	static_cast<CAuthenticatorKerberos*>(auth)->SetUID(cdstring(mUID->GetText()));
}

// Change items states
void CPrefsAuthKerberos::UpdateItems(bool enable)
{
	if (enable)
	{
		mDefaultPrincipal->Activate();
		if (mDefaultPrincipal->IsChecked())
			mPrincipal->Deactivate();
		else
			mPrincipal->Activate();
		mDefaultUID->Activate();
		if (mDefaultUID->IsChecked())
			mUID->Deactivate();
		else
			mUID->Activate();
	}
	else
	{
		mDefaultPrincipal->Deactivate();
		mPrincipal->Deactivate();
		mDefaultUID->Deactivate();
		mUID->Deactivate();
	}
}
