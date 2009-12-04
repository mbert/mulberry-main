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


// Source for CCreateMailboxDialog class

#include "CCreateMailboxDialog.h"

#include "CBalloonDialog.h"
#include "CHelpAttach.h"
#include "CINETCommon.h"
#include "CMailAccountManager.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStaticText.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>


#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCreateMailboxDialog::CCreateMailboxDialog()
{
}

// Constructor from stream
CCreateMailboxDialog::CCreateMailboxDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CCreateMailboxDialog::~CCreateMailboxDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CCreateMailboxDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get name
	mMailbox = (LRadioButton*) FindPaneByID(paneid_CrMailbox);
	mDirectory = (LRadioButton*) FindPaneByID(paneid_CrDirectory);
	mMailboxName = (CTextFieldX*) FindPaneByID(paneid_CrName);

	// Get checkbox
	mSubscribe = (LCheckBox*) FindPaneByID(paneid_CrSubscribe);

	// Get radio buttons
	mFullPath = (LRadioButton*) FindPaneByID(paneid_CrFullPath);
	mUseDirectory = (LRadioButton*) FindPaneByID(paneid_CrUseDirectory);

	// Get captions
	mHierarchy = (CStaticText*) FindPaneByID(paneid_CrHierarchy);
	mAccountBox = (LView*) FindPaneByID(paneid_CrAccountBox);
	mAccount = (CStaticText*) FindPaneByID(paneid_CrAccount);
	mAccountPopup = (LPopupButton*) FindPaneByID(paneid_CrAccountPopup);
	InitAccountMenu();


	// Make text edit field active
	SetLatentSub(mMailboxName);
	mMailboxName->SelectAll();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CCreateMailboxDialogBtns);

}

// Handle OK button
void CCreateMailboxDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage) {

		case msg_SetFullPath:
			if (*((long*) ioParam))
				mHierarchy->Disable();
			break;

		case msg_SetUseDirectory:
			if (*((long*) ioParam))
				mHierarchy->Enable();
			break;

		case msg_CrAccountPopup:
			OnChangeAccount(*((long*) ioParam));
			break;

		default:
			LDialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Account popup changed
void CCreateMailboxDialog::OnChangeAccount(long index)
{
	// Get account
	CMailAccount* acct = CPreferences::sPrefs->mMailAccounts.GetValue()[index - 1];
	
	// Enable subscribe only for IMAP accounts
	if (acct->GetServerType() == CINETAccount::eIMAP)
	{
		mUseSubscribe = true;
		mSubscribe->Show();
	}
	else
	{
		mUseSubscribe = false;
		mSubscribe->Hide();
	}
}

// Set the details
void CCreateMailboxDialog::SetDetails(SCreateMailbox* create)
{
	// If no account use the popup
	if (create->account.empty())
	{
		mAccountBox->Hide();
		
		// Make sure subscribe option is properly setup
		OnChangeAccount(1);
	}
	else
	{
		mAccountPopup->Hide();
		mAccount->SetText(create->account);
		
		// Hide subscribed if not IMAP
		mUseSubscribe = (create->account_type == CINETAccount::eIMAP);
		if (!mUseSubscribe)
			mSubscribe->Hide();
	}

	mHierarchy->SetText(create->parent);

	if (create->use_wd && !create->parent.empty())
		mUseDirectory->SetValue(1);
	else
		mFullPath->SetValue(1);

	if (create->parent.empty())
	{
		mUseDirectory->Disable();
		mHierarchy->Disable();
	}
}

// Get the details
void CCreateMailboxDialog::GetDetails(SCreateMailbox* result)
{
	result->directory = (mDirectory->GetValue() == 1);
	result->new_name = mMailboxName->GetText();

	result->use_wd = (mFullPath->GetValue() != 1);

	result->subscribe = mUseSubscribe && (mSubscribe->GetValue() == 1);

	// Get account if not specified
	if (result->account.empty())
	{
		result->account = ::GetPopupMenuItemTextUTF8(mAccountPopup);
	}
}

// Called during idle
void CCreateMailboxDialog::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = 1;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
	{
		// Add to menu
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());
		
		// Disable if not logged in
		if (!CMailAccountManager::sMailAccountManager->GetProtocol((*iter)->GetName())->IsLoggedOn())
			::DisableItem(menuH, menu_pos);
	}

	// Force max/min update
	mAccountPopup->SetMenuMinMax();
}

bool CCreateMailboxDialog::PoseDialog(SCreateMailbox& create)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_CreateMailboxDialog, CMulberryApp::sApp);
	((CCreateMailboxDialog*) theHandler.GetDialog())->SetDetails(&create);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CCreateMailboxDialog*) theHandler.GetDialog())->GetDetails(&create);
			result = !create.new_name.empty();
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
