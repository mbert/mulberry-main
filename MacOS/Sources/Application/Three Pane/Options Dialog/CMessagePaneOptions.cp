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


// Source for CMessagePaneOptions class

#include "CMessagePaneOptions.h"

#include "CMailViewOptions.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LRadioButton.h>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMessagePaneOptions::CMessagePaneOptions(LStream *inStream)
		: CCommonViewOptions(inStream)
{
}

// Default destructor
CMessagePaneOptions::~CMessagePaneOptions()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CMessagePaneOptions::FinishCreateSelf()
{
	// Do inherited
	CCommonViewOptions::FinishCreateSelf();

	mMailboxUseTabs = (LCheckBox*) FindPaneByID(paneid_MailViewMailboxUseTabs);
	mMailboxRestoreTabs = (LCheckBox*) FindPaneByID(paneid_MailViewMailboxRestoreTabs);

	mMessageAddress = (LCheckBox*) FindPaneByID(paneid_MailViewMessageAddress);
	mMessageSummary = (LCheckBox*) FindPaneByID(paneid_MailViewMessageSummary);
	mMessageParts = (LCheckBox*) FindPaneByID(paneid_MailViewMessageParts);

	mMarkSeen = (LRadioButton*) FindPaneByID(paneid_MailViewMessageMarkSeen);
	mMarkSeenAfter = (LRadioButton*) FindPaneByID(paneid_MailViewMessageMarkSeenAfter);
	mNoMarkSeen = (LRadioButton*) FindPaneByID(paneid_MailViewMessageNoMarkSeen);

	mSeenDelay = (CTextFieldX*) FindPaneByID(paneid_MailViewMessageSeenDelay);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CMessagePaneOptionsBtns);
}

void CMessagePaneOptions::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_MailViewMailboxUseTabs:
		if (*(long*) ioParam)
			mMailboxRestoreTabs->Enable();
		else
			mMailboxRestoreTabs->Disable();
		break;

	case msg_MailViewMessageMarkSeen:
	case msg_MailViewMessageNoMarkSeen:
		mSeenDelay->Disable();
		break;
	case msg_MailViewMessageMarkSeenAfter:
		mSeenDelay->Enable();
		LCommander::SwitchTarget(mSeenDelay);
		break;
	}
}

void CMessagePaneOptions::SetData(const CUserAction& listPreview,
									const CUserAction& listFullView,
									const CUserAction& itemsPreview,
									const CUserAction& itemsFullView,
									const CMailViewOptions& options,
									bool is3pane)
{
	SetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView, is3pane);

	mMailboxUseTabs->SetValue(options.GetUseTabs());
	mMailboxRestoreTabs->SetValue(options.GetRestoreTabs());
	if (!options.GetUseTabs())
		mMailboxRestoreTabs->Disable();
	if (!is3pane)
	{
		mMailboxUseTabs->Disable();
		mMailboxRestoreTabs->Disable();
	}

	mMessageAddress->SetValue(options.GetShowAddressPane());
	mMessageSummary->SetValue(options.GetShowSummary());
	mMessageParts->SetValue(options.GetShowParts());
	
	mMarkSeen->SetValue(options.GetPreviewFlagging() == CMailViewOptions::eMarkSeen);
	mMarkSeenAfter->SetValue(options.GetPreviewFlagging() == CMailViewOptions::eDelayedSeen);
	mNoMarkSeen->SetValue(options.GetPreviewFlagging() == CMailViewOptions::eNoSeen);

	mSeenDelay->SetNumber(options.GetPreviewDelay());
	
	if (options.GetPreviewFlagging() != CMailViewOptions::eDelayedSeen)
		mSeenDelay->Disable();
}

void CMessagePaneOptions::GetData(CUserAction& listPreview,
									CUserAction& listFullView,
									CUserAction& itemsPreview,
									CUserAction& itemsFullView,
									CMailViewOptions& options)
{
	GetCommonViewData(listPreview, listFullView, itemsPreview, itemsFullView);

	options.SetUseTabs(mMailboxUseTabs->GetValue());
	options.SetRestoreTabs(mMailboxRestoreTabs->GetValue());

	options.SetShowAddressPane(mMessageAddress->GetValue());
	options.SetShowSummary(mMessageSummary->GetValue());
	options.SetShowParts(mMessageParts->GetValue());
	
	if (mMarkSeen->GetValue())
		options.SetPreviewFlagging(CMailViewOptions::eMarkSeen);
	else if (mMarkSeenAfter->GetValue())
		options.SetPreviewFlagging(CMailViewOptions::eDelayedSeen);
	else if (mNoMarkSeen->GetValue())
		options.SetPreviewFlagging(CMailViewOptions::eNoSeen);
	
	options.SetPreviewDelay(mSeenDelay->GetNumber());
}
