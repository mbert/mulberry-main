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


// Source for CReplyActionDialog class

#include "CReplyActionDialog.h"

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
CReplyActionDialog::CReplyActionDialog()
{
}

// Constructor from stream
CReplyActionDialog::CReplyActionDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CReplyActionDialog::~CReplyActionDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CReplyActionDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mReplyTo = (LRadioButton*) FindPaneByID(paneid_ReplyActionReplyTo);
	mReplySender = (LRadioButton*) FindPaneByID(paneid_ReplyActionSender);
	mReplyFrom = (LRadioButton*) FindPaneByID(paneid_ReplyActionFrom);
	mReplyAll = (LRadioButton*) FindPaneByID(paneid_ReplyActionAll);
	mQuote = (LCheckBox*) FindPaneByID(paneid_ReplyActionQuote);
	mUseTied = (LRadioButton*) FindPaneByID(paneid_ReplyActionUseTied);
	mUseIdentity = (LRadioButton*) FindPaneByID(paneid_ReplyActionUseIdentity);
	mIdentityPopup = (CIdentityPopup*) FindPaneByID(paneid_ReplyActionIdentity);
	mIdentityPopup->Reset(CPreferences::sPrefs->mIdentities.GetValue());
	mCreateDraft = (LCheckBox*) FindPaneByID(paneid_ReplyActionCreateDraft);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CReplyActionDialogBtns);
}

// Handle OK button
void CReplyActionDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_ReplyActionText:
		SetText();
		break;

	case msg_ReplyActionUseTied:
		if (*(long*) ioParam)
			mIdentityPopup->Disable();
		break;

	case msg_ReplyActionUseIdentity:
		if (*(long*) ioParam)
			mIdentityPopup->Enable();
		break;

	case msg_ReplyActionIdentity:
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
void CReplyActionDialog::SetDetails(CActionItem::CActionReply& details)
{
	switch(details.GetReplyType())
	{
	case NMessage::eReplyTo:
		mReplyTo->SetValue(1);
		break;
	case NMessage::eReplySender:
		mReplySender->SetValue(1);
		break;
	case NMessage::eReplyFrom:
		mReplyFrom->SetValue(1);
		break;
	case NMessage::eReplyAll:
		mReplyAll->SetValue(1);
		break;
	}

	mQuote->SetValue(details.Quote());

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
void CReplyActionDialog::GetDetails(CActionItem::CActionReply& details)
{
	if (mReplyTo->GetValue())
		details.SetReplyType(NMessage::eReplyTo);
	else if (mReplySender->GetValue())
		details.SetReplyType(NMessage::eReplySender);
	else if (mReplyFrom->GetValue())
		details.SetReplyType(NMessage::eReplyFrom);
	else if (mReplyAll->GetValue())
		details.SetReplyType(NMessage::eReplyAll);

	details.SetQuote(mQuote->GetValue());

	details.SetText(mText);

	details.SetTiedIdentity(mUseTied->GetValue());

	details.SetIdentity(mCurrentIdentity);

	details.SetCreateDraft(mCreateDraft->GetValue());
}

// Called during idle
void CReplyActionDialog::SetText()
{
	CPrefsEditHeadFoot::PoseDialog("Set Reply Text", mText, false,
									CPreferences::sPrefs->spaces_per_tab.GetValue(),
									CPreferences::sPrefs->wrap_length.GetValue());
}

bool CReplyActionDialog::PoseDialog(CActionItem::CActionReply& details)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_ReplyActionDialog, CMulberryApp::sApp);
	((CReplyActionDialog*) theHandler.GetDialog())->SetDetails(details);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CReplyActionDialog*) theHandler.GetDialog())->GetDetails(details);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
