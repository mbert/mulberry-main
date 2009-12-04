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


// Source for CChangePswdDialog class

#include "CChangePswdDialog.h"

#include "CBalloonDialog.h"
#include "CErrorHandler.h"
#include "CINETAccount.h"
#include "CMulberryCommon.h"
#include "CMulberryApp.h"
#include "COptionsProtocol.h"
#include "CResources.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CUserPswdDialog.h"

#include <LEditText.h>
#include <LIconControl.h>

// __________________________________________________________________________________________________
// C L A S S __ C U S E R P S W D D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CChangePswdDialog::CChangePswdDialog()
{
}

// Constructor from stream
CChangePswdDialog::CChangePswdDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CChangePswdDialog::~CChangePswdDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CChangePswdDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Set up server name
	mIcon = (LIconControl*) FindPaneByID(paneid_ChangePswdIcon);
	mDescription = (CStaticText*) FindPaneByID(paneid_ChangePswdDescription);
	mServerName = (CStaticText*) FindPaneByID(paneid_ChangePswdServerName);
	mUserName = (CStaticText*) FindPaneByID(paneid_ChangePswdUID);

	// Get caps lock pane
	mCapsLock = (CStaticText*) FindPaneByID(paneid_ChangePswdCapsLock);

	// Get edit & caption panes for user & pswd
	mOldPassword = (CTextFieldX*) FindPaneByID(paneid_ChangePswdOldPswd);
	mNew1Password = (CTextFieldX*) FindPaneByID(paneid_ChangePswdNewPswd1);
	mNew2Password = (CTextFieldX*) FindPaneByID(paneid_ChangePswdNewPswd2);
}

// Deal with change in caps lock
void CChangePswdDialog::CapsLockChange(bool caps_lock_down)
{
	if (caps_lock_down)
		mCapsLock->Show();
	else
		mCapsLock->Hide();
}

// Select the user or password text
void CChangePswdDialog::SetDetails(const cdstring& uid,
									const cdstring& description,
									const cdstring& server,
									ResIDT icon)
{
	mIcon->SetResourceID(icon);

	// Set description and server name
	mDescription->SetText(description);
	mServerName->SetText(server);
	mUserName->SetText(uid);
}

// Get details from dialog
void CChangePswdDialog::GetDetails(cdstring& old_pass, cdstring& new1_pass, cdstring& new2_pass)
{
	old_pass = mOldPassword->GetText();

	new1_pass = mNew1Password->GetText();

	new2_pass = mNew2Password->GetText();
}

// Get details from dialog
void CChangePswdDialog::ResetNewPasswords()
{
	mNew1Password->SetText(cdstring::null_str);
	mNew2Password->SetText(cdstring::null_str);

	SwitchTarget((CTextFieldX*) mNew1Password);
}

bool CChangePswdDialog::PromptPasswordChange(const CINETAccount* acct,
											cdstring& old_pass,
											cdstring& new_pass,
											LCommander* cmdr)
{
	ResIDT icon = ICNx_UserPswdMailbox;
	cdstring title;
	switch(acct->GetServerType())
	{
	case CINETAccount::eIMAP:
		icon = ICNx_UserPswdMailbox;
		title = LStr255(STRx_Standards, str_ChangePswdIMAP);
		break;
	case CINETAccount::ePOP3:
	case CINETAccount::eLocal:
		icon = ICNx_UserPswdPOP3Mailbox;
		title = LStr255(STRx_Standards, str_ChangePswdPOP3);
		break;
	case CINETAccount::eSMTP:
		icon = ICNx_UserPswdSMTP;
		title = LStr255(STRx_Standards, str_ChangePswdSMTP);
		break;
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
		if (typeid(*(CINETProtocol*)acct->GetProtocol()) == typeid(COptionsProtocol))
		{
			icon = ICNx_UserPswdPrefs;
			title = LStr255(STRx_Standards, str_ChangePswdRemotePrefs);
		}
		else
		{
			icon = ICNx_UserPswdAddressBook;
			title = LStr255(STRx_Standards, str_ChangePswdRemoteAddress);
		}
		break;
	case CINETAccount::eLDAP:
		icon = ICNx_UserPswdLDAP;
		title =LStr255(STRx_Standards, str_ChangePswdLDAP);
		break;
	}

	// Create the dialog
	CBalloonDialog	theHandler(paneid_ChangePswdDialog, cmdr);
	((CChangePswdDialog*) theHandler.GetDialog())->SetDetails(acct->GetAuthenticatorUserPswd()->GetUID(), title, acct->GetServerIP(), icon);
	theHandler.StartDialog();

	bool caps_lock = false;

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			cdstring temp_old_pass;
			cdstring temp_new1_pass;
			cdstring temp_new2_pass;
			((CChangePswdDialog*) theHandler.GetDialog())->GetDetails(temp_old_pass, temp_new1_pass, temp_new2_pass);

			// Must have new passwords the same
			if (temp_new1_pass != temp_new2_pass)
			{
				// Show error
				CErrorHandler::PutStopAlertRsrc("Alerts::UserPswd::NewPasswordMismatch");
				((CChangePswdDialog*) theHandler.GetDialog())->ResetNewPasswords();
			}
			else
			{
				old_pass = temp_old_pass;
				new_pass = temp_new1_pass;
				return true;
			}
		}
		else if (hitMessage == msg_Cancel)
		{
			return false;
		}
		else if (hitMessage == msg_Nothing)
		{
			// Check for caps lock
			bool current_caps_lock = ::GetCurrentKeyModifiers() & alphaLock;
			if (current_caps_lock != caps_lock)
			{
				((CChangePswdDialog*) theHandler.GetDialog())->CapsLockChange(current_caps_lock);
				caps_lock = current_caps_lock;
			}
		}
	}
}