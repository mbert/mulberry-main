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


// Source for CRejectActionDialog class

#include "CRejectActionDialog.h"

#include "CBalloonDialog.h"
#include "CIdentityPopup.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRejectActionDialog::CRejectActionDialog()
{
}

// Constructor from stream
CRejectActionDialog::CRejectActionDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CRejectActionDialog::~CRejectActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CRejectActionDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mReturnHeaders = (LRadioButton*) FindPaneByID(paneid_RejectActionReturnHeaders);
	mReturnMessage = (LRadioButton*) FindPaneByID(paneid_RejectActionReturnMessage);
	mUseTied = (LRadioButton*) FindPaneByID(paneid_RejectActionUseTied);
	mUseIdentity = (LRadioButton*) FindPaneByID(paneid_RejectActionUseIdentity);
	mIdentityPopup = (CIdentityPopup*) FindPaneByID(paneid_RejectActionIdentity);
	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());
	mCreateDraft = (LCheckBox*) FindPaneByID(paneid_RejectActionCreateDraft);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CRejectActionDialogBtns);
}

// Handle OK button
void CRejectActionDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_RejectActionUseTied:
		if (*(long*) ioParam)
			mIdentityPopup->Disable();
		break;

	case msg_RejectActionUseIdentity:
		if (*(long*) ioParam)
			mIdentityPopup->Enable();
		break;

	case msg_RejectActionIdentity:
		switch(*(long*) ioParam)
		{
		// New identity wanted
		case eIdentityPopup_New:
			mIdentityPopup->DoNewIdentity(CPreferences::sPrefs);
			break;

		// New identity wanted
		case eIdentityPopup_Edit:
			mIdentityPopup->DoEditIdentity(CPreferences::sPrefs);
			break;

		// Delete existing identity
		case eIdentityPopup_Delete:
			mIdentityPopup->DoDeleteIdentity(CPreferences::sPrefs);
			break;

		// Select an identity
		default:
			mCurrentIdentity = CPreferences::sPrefs->mIdentities.Value()[*(long*) ioParam - mIdentityPopup->FirstIndex()].GetIdentity();
			break;
		}
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the details
void CRejectActionDialog::SetDetails(CActionItem::CActionReject& details)
{
	mReturnHeaders->SetValue(!details.ReturnMessage());
	mReturnMessage->SetValue(details.ReturnMessage());

	mUseTied->SetValue(details.UseTiedIdentity());
	mUseIdentity->SetValue(!details.UseTiedIdentity());
	if (details.UseTiedIdentity())
		mIdentityPopup->Disable();
	
	mCurrentIdentity = details.GetIdentity();
	mIdentityPopup->SetIdentity(CPreferences::sPrefs, mCurrentIdentity);

	mCreateDraft->SetValue(details.CreateDraft());
}

// Get the details
void CRejectActionDialog::GetDetails(CActionItem::CActionReject& details)
{
	details.SetReturnMessage(mReturnMessage->GetValue());

	details.SetTiedIdentity(mUseTied->GetValue());

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft->GetValue());
}

bool CRejectActionDialog::PoseDialog(CActionItem::CActionReject& details)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_RejectActionDialog, CMulberryApp::sApp);
	((CRejectActionDialog*) theHandler.GetDialog())->SetDetails(details);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CRejectActionDialog*) theHandler.GetDialog())->GetDetails(details);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
