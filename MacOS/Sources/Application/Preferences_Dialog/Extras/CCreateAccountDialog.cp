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


// Source for CCreateAccountDialog class

#include "CCreateAccountDialog.h"

#include "CAdminLock.h"
#include "CMulberryCommon.h"
#include "CPrefsAccount.h"
#include "CTextFieldX.h"

#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C G E T S T R I N G D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCreateAccountDialog::CCreateAccountDialog()
{
}

// Constructor from stream
CCreateAccountDialog::CCreateAccountDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CCreateAccountDialog::~CCreateAccountDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateAccountDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Make text edit field active
	mText = (CTextFieldX*) FindPaneByID(paneid_CreateAccountName);
	mType = (LPopupButton*) FindPaneByID(paneid_CreateAccountType);
	
	// Disable certain types
	if (CAdminLock::sAdminLock.mNoLocalMbox)
	{
		::DisableItem(mType->GetMacMenuH(), menu_AccountPOPMailbox);
		::DisableItem(mType->GetMacMenuH(), menu_AccountLocalMailbox);
	}
	if (CAdminLock::sAdminLock.mNoRemotePrefs)
	{
		::DisableItem(mType->GetMacMenuH(), menu_AccountIMSPOptions);
		::DisableItem(mType->GetMacMenuH(), menu_AccountACAPOptions);
	}
	if (CAdminLock::sAdminLock.mPreventCalendars)
	{
		::DisableItem(mType->GetMacMenuH(), menu_AccountCalDAVCalendar);
		::DisableItem(mType->GetMacMenuH(), menu_AccountWebDAVCalendar);
	}

	SetLatentSub(mText);
}

// Called during idle
void CCreateAccountDialog::GetDetails(cdstring& change, short& type)
{
	change = mText->GetText();
	type = mType->GetValue();
}
