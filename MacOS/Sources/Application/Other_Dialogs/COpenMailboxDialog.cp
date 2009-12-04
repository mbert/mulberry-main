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


// Source for COpenMailboxDialog class

#include "COpenMailboxDialog.h"

#include "CBalloonDialog.h"
#include "CBrowseMailboxDialog.h"
#include "CMboxRef.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextFieldX.h"

#include <LControl.h>
#include <LPopupButton.h>
#include <LPushButton.h>

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C O P E N M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
COpenMailboxDialog::COpenMailboxDialog()
{
	mBrowsed = nil;
}

// Constructor from stream
COpenMailboxDialog::COpenMailboxDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mBrowsed = nil;
}

// Default destructor
COpenMailboxDialog::~COpenMailboxDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void COpenMailboxDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mOKButton = (LPushButton*) FindPaneByID(paneid_OpenMailboxOKbtn);
	mOKButton->Disable();
	mAccountPopup = (LPopupButton*) FindPaneByID(paneid_OpenMailboxAccountPopup);
	InitAccountMenu();
	mMailboxName = (CTextFieldX*) FindPaneByID(paneid_OpenMailboxName);
	mMailboxName->AddListener(this);

	// Make text edit field active
	SetLatentSub(mMailboxName);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_COpenMailboxDialogBtns);
}

// Handle OK button
void COpenMailboxDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
		case msg_OpenMailboxBrowse:
		{
			// Create the dialog
			bool ignore = false;
			CBrowseMailboxDialog::PoseDialog(true, false, mBrowsed, ignore);
			break;
		}

		case msg_OpenMailbox:
			// Check for any text
			if (!mMailboxName->GetText().empty())
				mOKButton->Enable();
			else
				mOKButton->Disable();
			break;

		default:
			LDialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Set the dialogs return info
CMbox* COpenMailboxDialog::GetSelectedMbox(void)
{
	// If browse result, use that
	if (mBrowsed)
		return mBrowsed;

	// Get account name for mailbox
	cdstring acct_name;

	acct_name += ::GetPopupMenuItemTextUTF8(mAccountPopup);
	acct_name += cMailAccountSeparator;

	acct_name += mMailboxName->GetText();

	// Get diur delim for account
	short acct_num = mAccountPopup->GetValue() - 1;
	char dir_delim = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_num)->GetDirDelim();

	// Try to resolve reference - force creation
	CMboxRef ref(acct_name, dir_delim);

	return ref.ResolveMbox(true);
}

// Called during idle
void COpenMailboxDialog::InitAccountMenu(void)
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = 1;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName());

	// Force max/min update
	mAccountPopup->SetMenuMinMax();
}

bool COpenMailboxDialog::PoseDialog(CMbox*& mbox)
{
	bool result = false;

	CBalloonDialog	dlog(paneid_OpenMailboxDialog, CMulberryApp::sApp);
	dlog.StartDialog();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = dlog.DoDialog();

		if ((hitMessage == msg_OK) || ((COpenMailboxDialog*) dlog.GetDialog())->IsDone())
		{
			mbox = ((COpenMailboxDialog*) dlog.GetDialog())->GetSelectedMbox();
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}

	return result;
}
