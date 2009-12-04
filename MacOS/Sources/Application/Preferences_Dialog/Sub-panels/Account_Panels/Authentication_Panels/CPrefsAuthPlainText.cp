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

#include "CTextFieldX.h"

#include <LCheckBox.h>



// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAuthPlainText::CPrefsAuthPlainText()
{
}

// Constructor from stream
CPrefsAuthPlainText::CPrefsAuthPlainText(LStream *inStream)
		: CPrefsAuthPanel(inStream)
{
}

// Default destructor
CPrefsAuthPlainText::~CPrefsAuthPlainText()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAuthPlainText::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsAuthPanel::FinishCreateSelf();

	mUID = (CTextFieldX*) FindPaneByID(paneid_AuthPlainTextUID);
	mSaveUser = (LCheckBox*) FindPaneByID(paneid_AuthPlainTextSaveUser);
	mSavePswd = (LCheckBox*) FindPaneByID(paneid_AuthPlainTextSavePswd);

	if (CAdminLock::sAdminLock.mLockSavePswd)
		mSavePswd->Disable();
}

// Toggle display of IC
void CPrefsAuthPlainText::ToggleICDisplay(bool IC_on)
{
	if (IC_on)
		mUID->Disable();
	else
		mUID->Enable();
}

// Set prefs
void CPrefsAuthPlainText::SetAuth(CAuthenticator* auth)
{
	mUID->SetText(static_cast<CAuthenticatorUserPswd*>(auth)->GetUID());
	mSaveUser->SetValue(static_cast<CAuthenticatorUserPswd*>(auth)->GetSaveUID());
	mSavePswd->SetValue(static_cast<CAuthenticatorUserPswd*>(auth)->GetSavePswd() && !CAdminLock::sAdminLock.mLockSavePswd);
}

// Force update of prefs
void CPrefsAuthPlainText::UpdateAuth(CAuthenticator* auth)
{
	static_cast<CAuthenticatorUserPswd*>(auth)->SetUID(mUID->GetText());
	static_cast<CAuthenticatorUserPswd*>(auth)->SetSaveUID(mSaveUser->GetValue()==1);
	static_cast<CAuthenticatorUserPswd*>(auth)->SetSavePswd((mSavePswd->GetValue()==1) && !CAdminLock::sAdminLock.mLockSavePswd);
}

// Change items states
void CPrefsAuthPlainText::UpdateItems(bool enable)
{
	if (enable)
	{
		mUID->Enable();
		mSaveUser->Enable();
		if (!CAdminLock::sAdminLock.mLockSavePswd)
			mSavePswd->Enable();
	}
	else
	{
		mUID->Disable();
		mSaveUser->Disable();
		mSavePswd->Disable();
	}
}
