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


// Source for CSaveDraftDialog class

#include "CSaveDraftDialog.h"

#include "CBalloonDialog.h"
#include "CMailboxPopup.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

#include <LRadioButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C O P E N M A I L B O X D I A L O G
// __________________________________________________________________________________________________

CSaveDraftDialog::SSaveDraft CSaveDraftDialog::sDetails;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSaveDraftDialog::CSaveDraftDialog()
{
}

// Constructor from stream
CSaveDraftDialog::CSaveDraftDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CSaveDraftDialog::~CSaveDraftDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSaveDraftDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mFile = (LRadioButton*) FindPaneByID(paneid_SaveDraftFile);
	mMailbox = (LRadioButton*) FindPaneByID(paneid_SaveDraftMailbox);
	mMailboxPopup = (CMailboxPopup*) FindPaneByID(paneid_SaveDraftMailboxPopup);
	mMailboxPopup->SetCopyTo(false);
	mMailboxPopup->SetPickPopup(false);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CSaveDraftDialogBtns);
}

// Handle OK button
void CSaveDraftDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_SaveDraftFile:
		// Disable popup
		if (*(long*)ioParam)
			mMailboxPopup->Disable();
		break;
	case msg_SaveDraftMailbox:
		// Enable popup
		if (*(long*)ioParam)
			mMailboxPopup->Enable();
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

// Set the dialogs info
void CSaveDraftDialog::SetDetails(bool allow_file)
{
	mFile->SetValue(allow_file && sDetails.mFile);
	if (!allow_file)
		mFile->Disable();

	mMailbox->SetValue(!allow_file || !sDetails.mFile);

	if (sDetails.mFile)
		mMailboxPopup->Disable();
	mMailboxPopup->SetSelectedMbox(sDetails.mMailboxName, sDetails.mMailboxName.empty(), sDetails.mMailboxName == "\1");
}

// Set the dialogs info
bool CSaveDraftDialog::GetDetails()
{
	bool result = true;

	sDetails.mFile = mFile->GetValue();
	
	// Only resolve mailbox name if saving to a mailbox
	if (!sDetails.mFile)
		result = mMailboxPopup->GetSelectedMboxName(sDetails.mMailboxName, true);
	
	return result;
}

bool CSaveDraftDialog::PoseDialog(SSaveDraft& details, bool allow_file)
{
	bool result = false;

	CBalloonDialog dlog(paneid_SaveDraftDialog, CMulberryApp::sApp);
	CSaveDraftDialog* sdlog = static_cast<CSaveDraftDialog*>(dlog.GetDialog());
	sdlog->SetDetails(allow_file);
	dlog.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = dlog.DoDialog();

		if (hitMessage == msg_OK)
		{
			result = sdlog->GetDetails();
			details.mFile = sDetails.mFile;
			details.mMailboxName = sDetails.mMailboxName;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
