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


// Source for CSynchroniseDialog class

#include "CSynchroniseDialog.h"

#include "CBalloonDialog.h"
#include "CConnectionManager.h"
#include "CMbox.h"
#include "CMboxList.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSynchroniseDialog::CSynchroniseDialog()
{
}

// Constructor from stream
CSynchroniseDialog::CSynchroniseDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CSynchroniseDialog::~CSynchroniseDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSynchroniseDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get radio buttons
	mAll = (LRadioButton*) FindPaneByID(paneid_SynchroniseAll);
	mNew = (LRadioButton*) FindPaneByID(paneid_SynchroniseNew);
	mSelected = (LRadioButton*) FindPaneByID(paneid_SynchroniseSelected);
	mFull = (LRadioButton*) FindPaneByID(paneid_SynchroniseFull);
	mBelow = (LRadioButton*) FindPaneByID(paneid_SynchroniseBelow);
	mSize = (CTextFieldX*) FindPaneByID(paneid_SynchroniseSize);
	mPartial = (LRadioButton*) FindPaneByID(paneid_SynchronisePartial);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CSynchroniseDialogBtns);

}

// Handle OK button
void CSynchroniseDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_SynchroniseFull:
		case msg_SynchronisePartial:
			if (*((long*) ioParam))
				mSize->Disable();
			break;

		case msg_SynchroniseBelow:
			if (*((long*) ioParam))
			{
				mSize->Enable();
				LCommander::SwitchTarget(mSize);
				mSize->SelectAll();
			}
			break;

		default:
			LDialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Set the details
void CSynchroniseDialog::SetDetails(bool has_selection)
{
	// Get disconnection options from prefs
	const CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.GetValue();

	if (!has_selection)
	{
		mAll->SetValue(disconnect.mMboxSync != CConnectionManager::eNewMessages);
		mNew->SetValue(disconnect.mMboxSync == CConnectionManager::eNewMessages);
		mSelected->Disable();
	}
	else
		mSelected->SetValue(1);

	mFull->SetValue(disconnect.mMsgSync == CConnectionManager::eEntireMessage);
	mBelow->SetValue(disconnect.mMsgSync == CConnectionManager::eMessageBelow);
	mPartial->SetValue(disconnect.mMsgSync == CConnectionManager::eFirstDisplayable);
	mSize->SetNumber(disconnect.mMsgSyncSize/1024);
	if (disconnect.mMsgSync != CConnectionManager::eMessageBelow)
		mSize->Disable();

}

// Get the details
void CSynchroniseDialog::GetDetails(bool& fast, bool& partial, unsigned long& size, bool& selection)
{
	// Get disconnection options from prefs
	CConnectionManager::CDisconnectOptions orig_disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();
	CConnectionManager::CDisconnectOptions& disconnect = CPreferences::sPrefs->mDisconnectOptions.Value();

	fast = mNew->GetValue();
	partial = mPartial->GetValue();
	selection = mSelected->GetValue();

	// Sync if requested
	if (mAll->GetValue())
		disconnect.mMboxSync = CConnectionManager::eAllMessages;

	// Fast if requested
	else if (mNew->GetValue())
		disconnect.mMboxSync = CConnectionManager::eNewMessages;

	// Message options
	if (mFull->GetValue())
		disconnect.mMsgSync = CConnectionManager::eEntireMessage;
	else if (mBelow->GetValue())
		disconnect.mMsgSync = CConnectionManager::eMessageBelow;
	else
		disconnect.mMsgSync = CConnectionManager::eFirstDisplayable;

	size = mSize->GetNumber() * 1024;
	disconnect.mMsgSyncSize = size;

	if (!mBelow->GetValue())
		size = 0;
	
	// Mark prefs as dirty if needed
	if (!(orig_disconnect == disconnect))
		CPreferences::sPrefs->mDisconnectOptions.SetDirty();
}

void CSynchroniseDialog::PoseDialog(CMboxList* mbox_list)
{
	bool fast = false;
	bool partial = false;
	unsigned long size = 0;
	bool selected = false;

	if (PoseDialog(fast, partial, size, selected))
	{
		// Synchonise each mailbox
		for(CMboxList::iterator iter = mbox_list->begin(); iter != mbox_list->end(); iter++)
			static_cast<CMbox*>(*iter)->Synchronise(fast, partial, size);
	}
}

void CSynchroniseDialog::PoseDialog(CMbox* mbox, ulvector& selection)
{
	bool fast = false;
	bool partial = false;
	unsigned long size = 0;
	bool selected = selection.size();

	if (PoseDialog(fast, partial, size, selected))
	{
		if (selected)
			// Synchronise the messages
			mbox->SynchroniseMessage(selection, false, fast, partial, size, true);
		else
			// Full synchonise of mailbox
			mbox->Synchronise(fast, partial, size);
	}
}

bool CSynchroniseDialog::PoseDialog(bool& fast, bool& partial, unsigned long& size, bool& selected)
{
	// Create the dialog
	CBalloonDialog theHandler(paneid_SynchroniseDialog, CMulberryApp::sApp);

	((CSynchroniseDialog*) theHandler.GetDialog())->SetDetails(selected);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		switch(hitMessage)
		{
		case msg_OK:
			((CSynchroniseDialog*) theHandler.GetDialog())->GetDetails(fast, partial, size, selected);
			return true;
		case msg_Cancel:
			return false;
		}
	}

}
