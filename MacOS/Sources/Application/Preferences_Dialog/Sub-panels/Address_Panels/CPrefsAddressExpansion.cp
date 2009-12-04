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

#include <LCheckBox.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAddressExpansion::CPrefsAddressExpansion()
{
}

// Constructor from stream
CPrefsAddressExpansion::CPrefsAddressExpansion(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CPrefsAddressExpansion::~CPrefsAddressExpansion()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAddressExpansion::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	mExpandNoNicknames = (LCheckBox*) FindPaneByID(paneid_AEExpandNoNicknames);
	mExpandFailedNicknames = (LCheckBox*) FindPaneByID(paneid_AEExpandFailedNicknames);
	mExpandFullName = (LCheckBox*) FindPaneByID(paneid_AEExpandFullName);
	mExpandNickName = (LCheckBox*) FindPaneByID(paneid_AEExpandNickName);
	mExpandEmail = (LCheckBox*) FindPaneByID(paneid_AEExpandEmail);
	mSkipLDAP = (LCheckBox*) FindPaneByID(paneid_AESkipLDAP);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CPrefsAddressExpansionBtns);
}

// Handle buttons
void CPrefsAddressExpansion::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_AEExpandNoNicknames:
		if (*((long*) ioParam))
			mExpandFailedNicknames->Disable();
		else
			mExpandFailedNicknames->Enable();
		break;
	}
}

// Set prefs
void CPrefsAddressExpansion::SetData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info to into panel
	mExpandNoNicknames->SetValue(copyPrefs->mExpandNoNicknames.GetValue());
	if (copyPrefs->mExpandNoNicknames.GetValue())
		mExpandFailedNicknames->Disable();
	mExpandFailedNicknames->SetValue(copyPrefs->mExpandFailedNicknames.GetValue());
	mExpandFullName->SetValue(copyPrefs->mExpandFullName.GetValue());
	mExpandNickName->SetValue(copyPrefs->mExpandNickName.GetValue());
	mExpandEmail->SetValue(copyPrefs->mExpandEmail.GetValue());
	mSkipLDAP->SetValue(copyPrefs->mSkipLDAP.GetValue());
}

// Force update of prefs
void CPrefsAddressExpansion::UpdateData(void* data)
{
	CPreferences* copyPrefs = (CPreferences*) data;

	// Copy info from panel into prefs
	copyPrefs->mExpandNoNicknames.SetValue(mExpandNoNicknames->GetValue()==1);
	copyPrefs->mExpandFailedNicknames.SetValue(mExpandFailedNicknames->GetValue()==1);
	copyPrefs->mExpandFullName.SetValue(mExpandFullName->GetValue()==1);
	copyPrefs->mExpandNickName.SetValue(mExpandNickName->GetValue()==1);
	copyPrefs->mExpandEmail.SetValue(mExpandEmail->GetValue()==1);
	copyPrefs->mSkipLDAP.SetValue(mSkipLDAP->GetValue()==1);
}
