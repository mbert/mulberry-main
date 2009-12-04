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


// Source for CUploadScriptDialog class

#include "CUploadScriptDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

#include <LPopupButton.h>
#include <LRadioButton.h>
#include <LTextGroupBox.h>

CUploadScriptDialog::SUploadScript CUploadScriptDialog::sLastChoice = {false, 0, true, true};

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CUploadScriptDialog::CUploadScriptDialog()
{
}

// Constructor from stream
CUploadScriptDialog::CUploadScriptDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CUploadScriptDialog::~CUploadScriptDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CUploadScriptDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get account bits
	mAccountPopup = (LPopupButton*) FindPaneByID(paneid_UploadScriptAccountPopup);
	InitAccountMenu();

	// Get radio groups
	mFile = (LRadioButton*) FindPaneByID(paneid_UploadScriptFile);
	mServer = (LRadioButton*) FindPaneByID(paneid_UploadScriptServer);

	mGroup = (LTextGroupBox*) FindPaneByID(paneid_UploadScriptGroup);

	mUploadOnly = (LRadioButton*) FindPaneByID(paneid_UploadScriptUploadOnly);
	mUploadActivate = (LRadioButton*) FindPaneByID(paneid_UploadScriptUploadActvate);
	mDelete = (LRadioButton*) FindPaneByID(paneid_UploadScriptDelete);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CUploadScriptDialogBtns);

}

// Handle buttons
void CUploadScriptDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_UploadScriptFile:
		if (*((long*) ioParam))
			mGroup->Disable();
		break;

	case msg_UploadScriptServer:
		if (*((long*) ioParam))
			mGroup->Enable();
		break;
	
	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
	}
}

// Set the details
void CUploadScriptDialog::SetDetails(SUploadScript& create)
{
	// Set file/server group
	if (create.mFile || (::CountMenuItems(mAccountPopup->GetMacMenuH()) == 0))
	{
		mFile->SetValue(1);
		mGroup->Disable();
	}
	else
	{
		mServer->SetValue(1);
		mGroup->Enable();
	}

	// Disable server if no accounts
	if (::CountMenuItems(mAccountPopup->GetMacMenuH()) == 0)
		mServer->Disable();
	else
		// Init acct
		mAccountPopup->SetValue(create.mAccountIndex + 1);

	// Init group
	if (create.mUpload && !create.mActivate)
		mUploadOnly->SetValue(1);
	else if (create.mUpload && create.mActivate)
		mUploadActivate->SetValue(1);
	else
		mDelete->SetValue(1);
}

// Get the details
void CUploadScriptDialog::GetDetails(SUploadScript& result)
{
	// Get items
	result.mFile = (mFile->GetValue() == 1);
	result.mAccountIndex = mAccountPopup->GetValue() -1;
	result.mUpload = (mDelete->GetValue() == 0);
	result.mActivate = (mUploadActivate->GetValue() == 1);
}

// Called during idle
void CUploadScriptDialog::InitAccountMenu()
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add each filter account
	short menu_pos = 1;
	for(CManageSIEVEAccountList::const_iterator iter = CPreferences::sPrefs->mSIEVEAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mSIEVEAccounts.GetValue().end(); iter++, menu_pos++)
	{
		// Add to menu
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());
	}

	// Force max/min update
	mAccountPopup->SetMenuMinMax();
}

// Do the dialog
bool CUploadScriptDialog::PoseDialog(SUploadScript& details)
{
	bool result = false;

	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_UploadScriptDialog, CMulberryApp::sApp);
		CUploadScriptDialog* dlog = (CUploadScriptDialog*) theHandler.GetDialog();
		dlog->SetDetails(sLastChoice);

		theHandler.StartDialog();

		// Let DialogHandler process events
		MessageT hitMessage;
		while (true)
		{					
			hitMessage = theHandler.DoDialog();
			
			if (hitMessage == msg_OK)
			{
				dlog->GetDetails(sLastChoice);
				details = sLastChoice;
				result = true;
				break;
			}
			else if (hitMessage == msg_Cancel)
				break;
		}
	}

	return result;
}
