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


// Source for CEditIdentitySecurity class

#include "CEditIdentitySecurity.h"

#include "CIdentity.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LCheckBoxGroupBox.h>
#include <LPopupButton.h>


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

enum
{
	eSignWith_Default = 1,
	eSignWith_From,
	eSignWith_ReplyTo,
	eSignWith_Sender,
	eSignWith_Other = eSignWith_Sender + 2
};

// Default constructor
CEditIdentitySecurity::CEditIdentitySecurity()
{
}

// Constructor from stream
CEditIdentitySecurity::CEditIdentitySecurity(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
}

// Default destructor
CEditIdentitySecurity::~CEditIdentitySecurity()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CEditIdentitySecurity::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mActive = (LCheckBoxGroupBox*) FindPaneByID(paneid_EditIdentitySecurityActive);
	mSign = (LCheckBox*) FindPaneByID(paneid_EditIdentitySign);
	mEncrypt = (LCheckBox*) FindPaneByID(paneid_EditIdentityEncrypt);
	mSignWithPopup = (LPopupButton*) FindPaneByID(paneid_EditIdentitySignWith);
	mSignOther = (CTextFieldX*) FindPaneByID(paneid_EditIdentitySignOther);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CEditIdentitySecurityBtns);
}

// Handle buttons
void CEditIdentitySecurity::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_EditIdentitySignWith:
		if ((*(long*) ioParam) == eSignWith_Other)
			mSignOther->Enable();
		else
			mSignOther->Disable();
		break;
	}
}

// Set prefs
void CEditIdentitySecurity::SetData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	mActive->SetValue(identity->GetUseSecurity());
	mSign->SetValue(identity->GetSign());
	mEncrypt->SetValue(identity->GetEncrypt());
	switch(identity->GetSignWith())
	{
	default:
		mSignWithPopup->SetValue(identity->GetSignWith() + 1);
		mSignOther->Disable();
		break;
	case CIdentity::eSignWithOther:
		mSignWithPopup->SetValue(identity->GetSignWith() + 2);
		break;
	}
	mSignOther->SetText(identity->GetSignOther());
}

// Force update of prefs
void CEditIdentitySecurity::UpdateData(void* data)
{
	CIdentity* identity = (CIdentity*) data;

	identity->SetUseSecurity(mActive->GetValue() == 1);
	identity->SetSign(mSign->GetValue() == 1);
	identity->SetEncrypt(mEncrypt->GetValue() == 1);

	cdstring other = mSignOther->GetText();
	switch(mSignWithPopup->GetValue())
	{
	default:
		identity->SetSignWith(static_cast<CIdentity::ESignWith>(mSignWithPopup->GetValue() - 1), other);
		break;
	case eSignWith_Other:
		identity->SetSignWith(static_cast<CIdentity::ESignWith>(mSignWithPopup->GetValue() - 2), other);
		break;
	}
}
