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

#include "CAdminLock.h"
#include "CAuthenticator.h"

#include "CTextFieldX.h"

#include <LCheckBox.h>



// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAuthKerberos::CPrefsAuthKerberos()
{
}

// Constructor from stream
CPrefsAuthKerberos::CPrefsAuthKerberos(LStream *inStream)
		: CPrefsAuthPanel(inStream)
{
}

// Default destructor
CPrefsAuthKerberos::~CPrefsAuthKerberos()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAuthKerberos::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsAuthPanel::FinishCreateSelf();

	mDefaultPrincipal = (LCheckBox*) FindPaneByID(paneid_AuthKerberosDefaultPrincipal);
	mDefaultPrincipal->SetValue(1);
	mPrincipal = (CTextFieldX*) FindPaneByID(paneid_AuthKerberosPrincipal);
	mPrincipal->Disable();
	mDefaultUID = (LCheckBox*) FindPaneByID(paneid_AuthKerberosDefaultUID);
	mDefaultUID->SetValue(1);
	mUID = (CTextFieldX*) FindPaneByID(paneid_AuthKerberosUID);
	mUID->Disable();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsAuthKerberosBtns);
}

// Handle buttons
void CPrefsAuthKerberos::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_AuthKerberosDefaultPrincipal:
			if (*(long*) ioParam)
				mPrincipal->Disable();
			else
				mPrincipal->Enable();
			break;

		case msg_AuthKerberosDefaultUID:
			if (*(long*) ioParam)
				mUID->Disable();
			else
				mUID->Enable();
			break;

		default:
			break;
	}
}

// Toggle display of IC
void CPrefsAuthKerberos::ToggleICDisplay(bool IC_on)
{
	// Does nothing!
}

// Set prefs
void CPrefsAuthKerberos::SetAuth(CAuthenticator* auth)
{
	mDefaultPrincipal->SetValue(static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultPrincipal());
	mPrincipal->SetText(static_cast<CAuthenticatorKerberos*>(auth)->GetServerPrincipal());

	mDefaultUID->SetValue(static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultUID());
	mUID->SetText(static_cast<CAuthenticatorKerberos*>(auth)->GetUID());

	if (static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultPrincipal())
		mPrincipal->Disable();
	if (static_cast<CAuthenticatorKerberos*>(auth)->GetDefaultUID())
		mUID->Disable();
}

// Force update of prefs
void CPrefsAuthKerberos::UpdateAuth(CAuthenticator* auth)
{
	static_cast<CAuthenticatorKerberos*>(auth)->SetUsePrincipal(mDefaultPrincipal->GetValue() == 1);
	static_cast<CAuthenticatorKerberos*>(auth)->SetServerPrincipal(mPrincipal->GetText());
	static_cast<CAuthenticatorKerberos*>(auth)->SetUseUID(mDefaultUID->GetValue() == 1);
	static_cast<CAuthenticatorKerberos*>(auth)->SetUID(mUID->GetText());
}

// Change items states
void CPrefsAuthKerberos::UpdateItems(bool enable)
{
	if (enable)
	{
		if (!CAdminLock::sAdminLock.mLockServerAddress)
		{
			mDefaultPrincipal->Enable();
			if (mDefaultPrincipal->GetValue())
				mPrincipal->Disable();
			else
				mPrincipal->Enable();
			mDefaultUID->Enable();
			if (mDefaultUID->GetValue())
				mUID->Disable();
			else
				mUID->Enable();
		}
	}
	else
	{
		mDefaultPrincipal->Disable();
		mPrincipal->Disable();
		mDefaultUID->Disable();
		mUID->Disable();
	}
}
