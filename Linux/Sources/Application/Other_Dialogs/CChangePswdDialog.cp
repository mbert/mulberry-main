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
#include "CPasswordField.h"
#include "COptionsProtocol.h"
#include "CUserPswdDialog.h"
#include "CXStringResources.h"

#include <LGAIconSuite.h>

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
		: LGADialogBox(inStream)
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
	LGADialogBox::FinishCreateSelf();

	// Set up server name
	mIcon = (LGAIconSuite*) FindPaneByID(paneid_ChangePswdIcon);
	mDescription = FindPaneByID(paneid_ChangePswdDescription);
	mServerName = FindPaneByID(paneid_ChangePswdServerName);
	mUserName = FindPaneByID(paneid_ChangePswdUID);

	// Get caps lock pane
	mCapsLock = FindPaneByID(paneid_ChangePswdCapsLock);

	// Get edit & caption panes for user & pswd
	mOldPassword = FindPaneByID(paneid_ChangePswdOldPswd);
	mNew1Password = FindPaneByID(paneid_ChangePswdNewPswd1);
	mNew2Password = FindPaneByID(paneid_ChangePswdNewPswd2);
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
	LStr255	copyStr;

	mIcon->SetIconResourceID(icon);

	// Set description and server name
	copyStr = description;
	mDescription->SetDescriptor(copyStr);
	copyStr = server;
	mServerName->SetDescriptor(copyStr);
	copyStr = uid;
	mUserName->SetDescriptor(copyStr);
}

// Get details from dialog
void CChangePswdDialog::GetDetails(cdstring& old_pass, cdstring& new1_pass, cdstring& new2_pass)
{
	Str255 copyStr;
	
	mOldPassword->GetDescriptor(copyStr);
	old_pass = copyStr;

	mNew1Password->GetDescriptor(copyStr);
	new1_pass = copyStr;

	mNew2Password->GetDescriptor(copyStr);
	new2_pass = copyStr;
}

// Get details from dialog
void CChangePswdDialog::ResetNewPasswords()
{
	LStr255 copyStr;
	
	mNew1Password->SetDescriptor(copyStr);
	mNew2Password->SetDescriptor(copyStr);
	
	SwitchTarget((CPasswordField*) mNew1Password);
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
		icon = IDI_USERPSWD_IMAP;
		title = rsrc::GetString("Alerts::UserPswd::IMAPCHANGE");
		break;
	case CINETAccount::ePOP3:
	case CINETAccount::eLocal:
		icon = IDI_USERPSWD_POP3;
		title = rsrc::GetString("Alerts::UserPswd::POP3CHANGE");
		break;
	case CINETAccount::eSMTP:
		icon = IDI_USERPSWD_SMTP;
		title = rsrc::GetString("Alerts::UserPswd::SMTPCHANGE");
		break;
	case CINETAccount::eIMSP:
	case CINETAccount::eACAP:
		if (typeid(*acct->GetProtocol()) == typeid(COptionsProtocol))
		{
			icon = IDI_USERPSWD_PREFS;
			title = rsrc::GetString("Alerts::UserPswd::PREFSCHANGE");
		}
		else
		{
			icon = IDI_USERPSWD_ADBK;
			title = rsrc::GetString("Alerts::UserPswd::ADBKCHANGE");
		}
		break;
	case CINETAccount::eLDAP:
		icon = IDI_USERPSWD_LDAP;
		title = rsrc::GetString("Alerts::UserPswd::LDAPCHANGE");
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
			bool current_caps_lock;
			KeyMap km;
			::GetKeys(km);			// Get keyboard state
			// Check for caps lock
			current_caps_lock = ((((unsigned char*) km)[0x39>>3] >> (0x39 & 7) ) & 1);
			if (current_caps_lock != caps_lock)
			{
				((CChangePswdDialog*) theHandler.GetDialog())->CapsLockChange(current_caps_lock);
				caps_lock = current_caps_lock;
			}
		}
	}
}