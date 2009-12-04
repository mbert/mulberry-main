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


// Source for CAboutDialog class

#include "CMultiUserDialog.h"

#include "CAdminLock.h"
#include "CChooseServersDialog.h"
#include "CConfigPlugin.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRegistration.h"
#include "CStaticText.h"
#include "CStringOptionsMap.h"
#include "CTextFieldX.h"
#include "CUserPswdDialog.h"

#include <LPopupButton.h>
#include <LPushButton.h>

#include <stdio.h>

// __________________________________________________________________________________________________
// C L A S S __ C M U L T I U S E R D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMultiUserDialog::CMultiUserDialog()
{
}

// Constructor from stream
CMultiUserDialog::CMultiUserDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}

// Default destructor
CMultiUserDialog::~CMultiUserDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CMultiUserDialog::FinishCreateSelf(void)
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get OK
	mOKBtn = (LPushButton*) FindPaneByID(paneid_MUOKBtn);
	mOKBtn->Disable();

	bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
					CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);

	// Set up server name
	InitServerField();

	// Get caps lock pane
	mCapsLock = (CStaticText*) FindPaneByID(paneid_MUCapsLock);

	// Do real name
	mRealName = (CTextFieldX*) FindPaneByID(paneid_MURealName);

	// Do real name locked
	mRealNameLocked = (CStaticText*) FindPaneByID(paneid_MURealNameLocked);

	// Delete the one not required
	if (CPreferences::sPrefs->mUse_IC.GetValue() && mRealName && mRealNameLocked)
	{
		delete mRealName;
		mRealName = NULL;

		// Copy text to edit fields
		cdstring real_name;
		if (!CPreferences::sPrefs->mIdentities.GetValue().empty())
		{
			CAddress addr(CPreferences::sPrefs->mIdentities.GetValue().front().GetFrom());
			real_name = addr.GetName();
		}
		mRealNameLocked->SetText(real_name);
	}
	else if (mRealName && mRealNameLocked)
	{
		delete mRealNameLocked;
		mRealNameLocked = NULL;
	}

	// See if configuration plugin is present
	if (CPluginManager::sPluginManager.HasConfig())
	{
		mUseUID = CPluginManager::sPluginManager.GetConfig()->PromptUserID();
		mUsePswd = CPluginManager::sPluginManager.GetConfig()->PromptPswd();
	}
	else
		// Determine from authenticator type
		mUseUID = mUsePswd = (remote ?
							CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().RequiresUserPswd() :
							CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetAuthenticator().RequiresUserPswd());

	// Check for use of uid & password
	if (mUseUID)
	{
		// Do user id
		mUserID = (CTextFieldX*) FindPaneByID(paneid_MUUserId);
		mUserID->AddListener(this);

		// Do user id locked
		mUserIDLocked = (CStaticText*) FindPaneByID(paneid_MUUserIdLocked);

		// Delete the one not required
		if (CPreferences::sPrefs->mUse_IC.GetValue() && !remote)
		{
			delete mUserID;
			mUserID = NULL;

			// Copy text to edit fields
			cdstring txt;
			if (CPreferences::sPrefs->mMailAccounts.GetValue().size())
				txt = CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetAuthenticatorUserPswd()->GetUID().c_str();
			mUserIDLocked->SetText(txt);

			mOKBtn->Enable();
		}
		else
		{
			delete mUserIDLocked;
			mUserIDLocked = NULL;
		}

		// Delete authentication description
		delete FindPaneByID(paneid_MUAuthenticate);
	}
	else
	{
		// Delete entire set of uid items
		delete FindPaneByID(paneid_MUUserId);
		delete FindPaneByID(paneid_MUUserIdLocked);
		delete FindPaneByID(paneid_MUUserIdCaption);
		mUserID = NULL;
		mUserIDLocked = NULL;

		// Set up authenticator name
		if (!CPluginManager::sPluginManager.HasConfig())
		{
			CStaticText* caption = (CStaticText*) FindPaneByID(paneid_MUAuthenticate);
			cdstring auth = (remote ?
							CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().GetDescriptor() :
							CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetAuthenticator().GetDescriptor());
			cdstring title;
			title.FromResource("UI::MU::AuthMethod");
			title.Substitute(auth);

			caption->SetText(title);
		}

		mOKBtn->Enable();
	}

	// Check for use of uid & password
	if (mUsePswd)
	{
		// Do password - delete if not logon at startup and not remote
		mPassword = (CTextFieldX*)FindPaneByID(paneid_MUPassword);
		if (!remote &&
			CPreferences::sPrefs->mMailAccounts.GetValue().size() &&
			!CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetLogonAtStart())
		{
			delete mPassword;
			mPassword = NULL;
			delete FindPaneByID(paneid_MUPasswordCaption);
		}
	}
	else
	{
		// Delete entire set of password items
		delete FindPaneByID(paneid_MUPassword);
		delete FindPaneByID(paneid_MUPasswordCaption);
		mPassword = NULL;
	}

	// Display connection field if requested
	if (CAdminLock::sAdminLock.mSSLStateDisplay)
	{
		bool secure = false;
		if (remote)
			secure = (CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetTLSType() != CINETAccount::eNoTLS);
		else
			secure = (CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetTLSType() != CINETAccount::eNoTLS);
		mConnection = (CStaticText*) FindPaneByID(paneid_MUConnection);
		cdstring temp;
		temp.FromResource(secure ? "UI::UserPswd::Secure" : "UI::UserPswd::Insecure");
		mConnection->SetText(temp);
	}
	else
	{
		FindPaneByID(paneid_MUConnection)->Hide();
		FindPaneByID(paneid_MUConnectionCaption)->Hide();
	}

	// Set up version number
	CStaticText* aPane = (CStaticText*) FindPaneByID(paneid_MUVersionNumber);
	aPane->SetText(CPreferences::sPrefs->GetVersionText());

	// Set up serial number
	aPane = (CStaticText*) FindPaneByID(paneid_MUSerialNumber);
	aPane->SetText(CRegistration::sRegistration.GetSerialNumber());

	// Set up licensee (and organisation)
	aPane = (CStaticText*) FindPaneByID(paneid_MULicensee);
	cdstring txt = CRegistration::sRegistration.GetLicensee();
	if (CRegistration::sRegistration.GetOrganisation().length() > 0)
	{
		txt += "\r";
		txt += CRegistration::sRegistration.GetOrganisation();
	}
	aPane->SetText(txt);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CMultiUserDialogBtns);
}

// Get new text
void CMultiUserDialog::UpdatePrefs(void)
{
	// Configure multiple servers if not done by plugin
	if (mMailServers.size())
	{
		bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
						CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);

		// Get the server string
		if (remote)
			CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->SetServerIP(mMailServerName);
		else if (CPreferences::sPrefs->mMailAccounts.GetValue().size())
			CPreferences::sPrefs->mMailAccounts.GetValue().front()->SetServerIP(mMailServerName);
		
		// See if SMTP server name already set
		if (mSMTPServerName.empty())
		{
			// Now check SMTP servers
			cdstring temp = CPreferences::sPrefs->mSMTPAccounts.GetValue().front()->GetServerIP();
			
			// Is it multiple?
			if (::strchr(temp.c_str(), ','))
			{
				// Parse into list
				cdstrvect servers;
				
				const char* p = ::strtok(temp, ",");
				while(p && *p)
				{
					while(*p == ' ') p++;
					servers.push_back(p);
					p = ::strtok(NULL, ",");
				}

				// Determine the appropriate name
				if (mServerIndex < servers.size())
					mSMTPServerName = servers[mServerIndex];
				else
					mSMTPServerName = servers[0];

				CPreferences::sPrefs->mSMTPAccounts.GetValue().front()->SetServerIP(mSMTPServerName);
			}
		}
		else
			// SMTP server chosen as Other... - just set it
			CPreferences::sPrefs->mSMTPAccounts.GetValue().front()->SetServerIP(mSMTPServerName);
			
	}

	// Get uid
	cdstring uid;
	if (mUserID)
		uid = mUserID->GetText();
	else if (mUserIDLocked)
	{
		Str255 temp;
		mUserIDLocked->GetDescriptor(temp);
		uid = temp;
	}

	// Get real name
	cdstring real_name;
	if (mRealName)
		real_name = mRealName->GetText();
	else if (mRealNameLocked)
	{
		real_name = mRealNameLocked->GetText();
	}

	// Configure prefs from MU
	// NB Don't configure the return address here. Instead wait for the first successful login
	// to complete and then do it. This prevents an incorrect MU dialog user id from being
	// put into the user's From address.
	CPreferences::sPrefs->MultiuserConfigure(uid, real_name, false);

	// Initiate logon by storing password
	cdstring pass;
	if (mPassword)
	{
		pass = mPassword->GetText();
	}

	// Store password
	CAdminLock::sAdminLock.mInitialPswd = pass;

	// Now do any config update of prefs - this will override default MU startup
	if (CPluginManager::sPluginManager.HasConfig())
		UpdateConfigPrefs();
}

// Get new text
void CMultiUserDialog::UpdateConfigPrefs(void)
{
	cdstring uid;
	cdstring pswd;
	cdstring real_name;
	cdstring server;
	
	// Get uid
	if (mUserID)
		uid = mUserID->GetText();
	else if (mUserIDLocked)
	{
		uid = mUserIDLocked->GetText();
	}

	// Get pswd
	if (mPassword)
	{
		pswd = mPassword->GetText();
	}

	// Get real name
	if (mRealName)
		real_name = mRealName->GetText();
	else if (mRealNameLocked)
	{
		real_name = mRealNameLocked->GetText();
	}

	server = mMailServerName;
	
	CPluginManager::sPluginManager.GetConfig()->DoConfiguration(uid.c_str(), pswd.c_str(), real_name.c_str(), server.c_str());

	// At this point prefs have been configured by plugin
}

void CMultiUserDialog::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch (inMessage) {

		case paneid_MUUserId:
			// Now check that there's text in uid field
			if (!mUserID->GetText().empty())
			{
				if (!mOKBtn->IsEnabled())
				{
					mOKBtn->Enable();
				}
			}
			else
			{
				if (mOKBtn->IsEnabled())
				{
					mOKBtn->Disable();
				}
			}
			break;

		case msg_MUServerPopup:
			{
				// Check for other
				if (mUseOther && (*(long*) ioParam > mMailServers.size()))
				{
					// Get name from user
					cdstring temp_mail;
					cdstring temp_smtp;
					if (CChooseServersDialog::PoseDialog(temp_mail, temp_smtp))
					{
						mMailServerName = temp_mail;
						mSMTPServerName = temp_smtp;
					}
					else
					{
						StopListening();
						mServerPopup->SetValue(1);
						StartListening();
						mMailServerName = ::GetPopupMenuItemTextUTF8(mServerPopup);
						mSMTPServerName = cdstring::null_str;
						mServerIndex = 0;
					}
				}
				else
				{
					mMailServerName = ::GetPopupMenuItemTextUTF8(mServerPopup);
					mSMTPServerName = cdstring::null_str;
					mServerIndex = (*(long*) ioParam) - 1;
				}
			}
			break;

		default:
			// Pass up
			LDialogBox::ListenToMessage(inMessage, ioParam);
	}

}


// Deal with change in caps lock
void CMultiUserDialog::CapsLockChange(bool caps_lock_down)
{
	if (caps_lock_down && (mUseUID || mUsePswd))
		mCapsLock->Show();
	else
		mCapsLock->Hide();
}

// Initialise server fields
void CMultiUserDialog::InitServerField()
{
	CStaticText* aPane = (CStaticText*) FindPaneByID(paneid_MUServerName);
	mServerPopup = (LPopupButton*) FindPaneByID(paneid_MUServerPopup);

	bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
					CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);

	// Get the server string
	cdstring temp;
	
	if (CPluginManager::sPluginManager.HasConfig() &&
		CPluginManager::sPluginManager.GetConfig()->PromptServers())
		temp = CPluginManager::sPluginManager.GetConfig()->GetServers();
	else if (remote)
		temp = CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetServerIP();
	else if (CPreferences::sPrefs->mMailAccounts.GetValue().size())
		temp = CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetServerIP();
	
	// Is it multiple?
	if (::strchr(temp.c_str(), ','))
	{
		// Hide server name
		aPane->Hide();
		
		// Get list of servers
		mUseOther = (temp[temp.length() - 1] == ',');
		
		const char* p = ::strtok(temp, ",");
		while(p && *p)
		{
			while(*p == ' ') p++;
			mMailServers.push_back(p);
			p = ::strtok(NULL, ",");
		}
		mMailServerName = mMailServers.front();
		mServerIndex = 0;

		// Initialise popup
		InitServerMenu();
	}
	else
	{
		// Hide popup and set single server name
		mServerPopup->Hide();
		aPane->SetText(temp);
	}
}

void CMultiUserDialog::InitServerMenu()
{
	MenuHandle menuH = mServerPopup->GetMacMenuH();

	// Get name of other
	cdstring other = ::GetMenuItemTextUTF8(menuH, 2);

	// Delete other items
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add each mail account
	short menu_pos = 1;
	for(cdstrvect::const_iterator iter = mMailServers.begin(); iter != mMailServers.end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, *iter);
	
	// Now add in other if required
	if (mUseOther)
	{
		::AppendMenu(menuH,"\p(-");
		::AppendItemToMenu(menuH, ++menu_pos, other);
	}

	// Force max/min update
	mServerPopup->SetMenuMinMax();
}
