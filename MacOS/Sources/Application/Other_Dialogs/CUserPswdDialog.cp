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


// Source for CUserPswdDialog class

#include "CUserPswdDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CTextFieldX.h"
#include "CStaticText.h"

#include <LIconControl.h>

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C U S E R P S W D D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CUserPswdDialog::CUserPswdDialog()
{
}

// Constructor from stream
CUserPswdDialog::CUserPswdDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CUserPswdDialog::~CUserPswdDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CUserPswdDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Set up server name
	mIcon = (LIconControl*) FindPaneByID(paneid_Icon);
	mDescription = (CStaticText*) FindPaneByID(paneid_Description);
	mServerName = (CStaticText*) FindPaneByID(paneid_ServerName);
	mMethod = (CStaticText*) FindPaneByID(paneid_Method);
	mConnection = (CStaticText*) FindPaneByID(paneid_Connection);

	// Get caps lock pane
	mCapsLock = (CStaticText*) FindPaneByID(paneid_CapsLock);

	// Get edit & caption panes for user & pswd
	mUserTextEdit = (CTextFieldX*) FindPaneByID(paneid_UserIDEdit);
	mUserTextCaption = (CStaticText*) FindPaneByID(paneid_UserIDCaption);
	mPswdTextEdit = (CTextFieldX*) FindPaneByID(paneid_PswdEdit);
}

// Deal with change in caps lock
void CUserPswdDialog::CapsLockChange(bool caps_lock_down)
{
	if (caps_lock_down)
		mCapsLock->Show();
	else
		mCapsLock->Hide();
}

// Select the user or password text
void CUserPswdDialog::SetUserPass(const cdstring& uid, const cdstring& pswd,
									bool lock_user, bool lock_pswd,
									const cdstring& description, const cdstring& server,
									const cdstring& method, bool secure, ResIDT icon)
{
	mIcon->SetResourceID(icon);

	// Set description and server name
	mDescription->SetText(description);
	mServerName->SetText(server);
	mMethod->SetText(method);

	cdstring txt;
	txt.FromResource(secure ? "UI::UserPswd::Secure" : "UI::UserPswd::Insecure");
	mConnection->SetText(txt);

	// Copy text to text fields
	if (lock_user)
	{
		mUserTextCaption->SetText(uid);
		delete mUserTextEdit;
		mUserTextEdit = nil;
	}
	else
	{
		mUserTextEdit->SetText(uid);
		delete mUserTextCaption;
		mUserTextCaption = nil;
	}

	mPswdTextEdit->SetText(pswd);

	if (!lock_user && (uid.empty() || lock_pswd))
	{
		SetLatentSub((CTextFieldX*) mUserTextEdit);
		((CTextFieldX*) mUserTextEdit)->SelectAll();
	}
	else
	{
		SetLatentSub((CTextFieldX*) mPswdTextEdit);
		((CTextFieldX*) mPswdTextEdit)->SelectAll();
	}
}

// Get details from dialog
void CUserPswdDialog::GetUserPass(cdstring& uid, cdstring& pswd)
{
	if (mUserTextEdit)
		uid = mUserTextEdit->GetText();
	else
		uid = mUserTextCaption->GetText();

	pswd = mPswdTextEdit->GetText();
}


bool CUserPswdDialog::PoseDialog(cdstring& uid, cdstring& pswd, bool save_user, bool save_pswd,
									const cdstring& title, const cdstring& server_ip, const cdstring& method,
									bool secure, int icon)
{
	// Create the dialog
	CBalloonDialog	theHandler(paneid_UserPswdDialog, CMulberryApp::sApp);
	((CUserPswdDialog*) theHandler.GetDialog())->SetUserPass(uid, pswd, save_user, save_pswd,
																title, server_ip, method, secure, icon);
	theHandler.StartDialog();

	bool caps_lock = false;

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CUserPswdDialog*) theHandler.GetDialog())->GetUserPass(uid, pswd);
			return true;
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
				((CUserPswdDialog*) theHandler.GetDialog())->CapsLockChange(current_caps_lock);
				caps_lock = current_caps_lock;
			}
		}
	}
}
