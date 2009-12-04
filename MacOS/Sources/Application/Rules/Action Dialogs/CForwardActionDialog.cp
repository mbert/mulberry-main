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


// Source for CForwardActionDialog class

#include "CForwardActionDialog.h"

#include "CBalloonDialog.h"
#include "CIdentityPopup.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPrefsEditHeadFoot.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CForwardActionDialog::CForwardActionDialog()
{
}

// Constructor from stream
CForwardActionDialog::CForwardActionDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CForwardActionDialog::~CForwardActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CForwardActionDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mTo = (CTextFieldX*) FindPaneByID(paneid_ForwardActionTo);
	mCC = (CTextFieldX*) FindPaneByID(paneid_ForwardActionCC);
	mBcc = (CTextFieldX*) FindPaneByID(paneid_ForwardActionBcc);
	mQuote = (LCheckBox*) FindPaneByID(paneid_ForwardActionQuote);
	mAttach = (LCheckBox*) FindPaneByID(paneid_ForwardActionAttach);
	mUseTied = (LRadioButton*) FindPaneByID(paneid_ForwardActionUseTied);
	mUseIdentity = (LRadioButton*) FindPaneByID(paneid_ForwardActionUseIdentity);
	mIdentityPopup = (CIdentityPopup*) FindPaneByID(paneid_ForwardActionIdentity);
	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());
	mCreateDraft = (LCheckBox*) FindPaneByID(paneid_ForwardActionCreateDraft);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CForwardActionDialogBtns);
}

// Handle OK button
void CForwardActionDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_ForwardActionText:
		SetText();
		break;

	case msg_ForwardActionUseTied:
		if (*(long*) ioParam)
			mIdentityPopup->Disable();
		break;

	case msg_ForwardActionUseIdentity:
		if (*(long*) ioParam)
			mIdentityPopup->Enable();
		break;

	case msg_ForwardActionIdentity:
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
void CForwardActionDialog::SetDetails(CActionItem::CActionForward& details)
{
	mTo->SetText(details.Addrs().mTo);

	mCC->SetText(details.Addrs().mCC);

	mBcc->SetText(details.Addrs().mBcc);

	mQuote->SetValue(details.Quote());

	mAttach->SetValue(details.Attach());

	mText = details.GetText();

	mUseTied->SetValue(details.UseTiedIdentity());
	mUseIdentity->SetValue(!details.UseTiedIdentity());
	if (details.UseTiedIdentity())
		mIdentityPopup->Disable();
	
	mCurrentIdentity = details.GetIdentity();
	mIdentityPopup->SetIdentity(CPreferences::sPrefs, mCurrentIdentity);

	mCreateDraft->SetValue(details.CreateDraft());
}

// Get the details
void CForwardActionDialog::GetDetails(CActionItem::CActionForward& details)
{
	details.Addrs().mTo = mTo->GetText();

	details.Addrs().mCC = mCC->GetText();

	details.Addrs().mBcc = mBcc->GetText();

	details.SetQuote(mQuote->GetValue());

	details.SetAttach(mAttach->GetValue());

	details.SetText(mText);

	details.SetTiedIdentity(mUseTied->GetValue());

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft->GetValue());
}

// Called during idle
void CForwardActionDialog::SetText()
{
	CPrefsEditHeadFoot::PoseDialog("Set Forward Text", mText, false,
									CPreferences::sPrefs->spaces_per_tab.GetValue(),
									CPreferences::sPrefs->wrap_length.GetValue());
}

bool CForwardActionDialog::PoseDialog(CActionItem::CActionForward& details)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_ForwardActionDialog, CMulberryApp::sApp);
	((CForwardActionDialog*) theHandler.GetDialog())->SetDetails(details);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CForwardActionDialog*) theHandler.GetDialog())->GetDetails(details);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
