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


// CMultiUserDialog.cpp : implementation file
//


#include "CMultiUserDialog.h"

#include "CAdminLock.h"
#include "CConfigPlugin.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRegistration.h"

#include "TPopupMenu.h"
#include "HResourceMap.h"

#include <JXWindow.h>
#include <JXDisplay.h>
#include <JXUpRect.h>
#include <JXEngravedRect.h>
#include <JXImageWidget.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXInputField.h>
#include <JXPasswordInput.h>
#include "CInputField.h"

#include <jXGlobals.h>

/////////////////////////////////////////////////////////////////////////////
// CMultiUserDialog dialog


CMultiUserDialog::CMultiUserDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mCapsLockDown = false;
}

void CMultiUserDialog::OnCreate()
{
	if (!mUseRealName)
	{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 470,310, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 470,310);
    assert( obj1 != NULL );

    mLogo =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 260,100);
    assert( mLogo != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Version Number", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 265,5, 85,15);
    assert( obj2 != NULL );

    mVersion =
        new JXStaticText("v2.0", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,5, 115,15);
    assert( mVersion != NULL );

    JXEngravedRect* obj3 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 305,150);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Login to Server:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,10, 280,20);
    assert( obj4 != NULL );

    mServerTxt =
        new JXStaticText("Server name", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 6,30, 280,20);
    assert( mServerTxt != NULL );

    mServerPopup =
        new HPopupMenu("",obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 230,20);
    assert( mServerPopup != NULL );

    mUserIDCaption =
        new JXStaticText("User ID:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,65, 60,20);
    assert( mUserIDCaption != NULL );

    mUserID =
        new CInputField<JXInputField>(obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,65, 200,20);
    assert( mUserID != NULL );

    mPasswordCaption =
        new JXStaticText("Password:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,95, 70,20);
    assert( mPasswordCaption != NULL );

    mPassword =
        new CInputField<JXPasswordInput>(obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,95, 200,20);
    assert( mPassword != NULL );

    mCapsLock =
        new JXStaticText("Caps Lock\nis Down", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 320,195, 70,25);
    assert( mCapsLock != NULL );

    mAuth =
        new JXStaticText("", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,80, 280,20);
    assert( mAuth != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 380,275, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,275, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mSSLStateCaption =
        new JXStaticText("Connection:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,125, 80,20);
    assert( mSSLStateCaption != NULL );

    mSSLState =
        new JXStaticText("connection", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,125, 200,20);
    assert( mSSLState != NULL );

// end JXLayout
		window->SetTitle("Login to Mulberry");

		// Null out unused items
		mRealName = NULL;
	}
	else
	{
// begin JXLayout2

    JXWindow* window = new JXWindow(this, 470,330, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 470,330);
    assert( obj1 != NULL );

    mLogo =
        new JXImageWidget(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 260,100);
    assert( mLogo != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Version Number", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 265,5, 85,15);
    assert( obj2 != NULL );

    mVersion =
        new JXStaticText("v2.0", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,5, 115,15);
    assert( mVersion != NULL );

    JXEngravedRect* obj3 =
        new JXEngravedRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,115, 305,170);
    assert( obj3 != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Login to Server:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,10, 280,20);
    assert( obj4 != NULL );

    mServerTxt =
        new JXStaticText("Server name", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 6,30, 280,20);
    assert( mServerTxt != NULL );

    mServerPopup =
        new HPopupMenu("",obj3,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,30, 230,20);
    assert( mServerPopup != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Real Name:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,55, 75,20);
    assert( obj5 != NULL );

    mRealName =
        new CInputField<JXInputField>(obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,55, 200,20);
    assert( mRealName != NULL );

    mUserIDCaption =
        new JXStaticText("User ID:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,85, 60,20);
    assert( mUserIDCaption != NULL );

    mUserID =
        new CInputField<JXInputField>(obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,85, 200,20);
    assert( mUserID != NULL );

    mPasswordCaption =
        new JXStaticText("Password:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,115, 70,20);
    assert( mPasswordCaption != NULL );

    mPassword =
        new CInputField<JXPasswordInput>(obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,115, 200,20);
    assert( mPassword != NULL );

    mCapsLock =
        new JXStaticText("Caps Lock\nis Down", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 320,220, 60,25);
    assert( mCapsLock != NULL );

    mAuth =
        new JXStaticText("", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,100, 280,20);
    assert( mAuth != NULL );

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 380,295, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,295, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mSSLStateCaption =
        new JXStaticText("Connection:", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,145, 80,20);
    assert( mSSLStateCaption != NULL );

    mSSLState =
        new JXStaticText("connection", obj3,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 85,145, 200,20);
    assert( mSSLState != NULL );

// end JXLayout2
		window->SetTitle("Login to Mulberry");
	}

	mLogo->SetImage(bmpFromResource(IDB_LOGO, mLogo), kTrue);
	SetButtons(mOKBtn, mCancelBtn);

	JXKeyModifiers modifiers(GetDisplay());
	GetDisplay()->GetCurrentButtonKeyState(NULL, &modifiers);
	mCapsLockDown = modifiers.shiftLock();
	if (!mCapsLockDown)
		mCapsLock->Hide();

	// Need to update for every character typed
	mUserID->ShouldBroadcastAllTextChanged(kTrue);

	ListenTo(mUserID);
}

// Watch state of capslock
void CMultiUserDialog::Continue()
{
	if (mCapsLockDown != GetDisplay()->GetLatestKeyModifiers().shiftLock())
	{
		mCapsLockDown = !mCapsLockDown;
		if (mCapsLockDown)
			mCapsLock->Show();
		else
			mCapsLock->Hide();
	}
}

void CMultiUserDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if ((sender == mUserID) && message.Is(JTextEditor::kTextChanged))
	{
		if (mUserID->GetTextLength())
			mOKBtn->Activate();
		else
			mOKBtn->Deactivate();
		return;
	}
	else if ((sender == mServerPopup) && message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		OnServerPopup(index);
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}


void CMultiUserDialog::InitFields()
{
	// Set version details
	mVersion->SetText(CPreferences::sPrefs->GetVersionText());
#if 0
	mSerial->SetText(CRegistration::sRegistration.GetSerialNumber());
	cdstring licensee = CRegistration::sRegistration.GetLicensee();
	if (CRegistration::sRegistration.GetOrganisation().length())
	{
		licensee += "\n";
		licensee += CRegistration::sRegistration.GetOrganisation();
	}
	mLicensee->SetText(licensee);
#endif

	// Determine whether remote login to be used
	bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
					CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : 0);

	mServerTxt->SetText(remote ?
						CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetServerIP() :
						CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetServerIP());

	// Set up server name
	InitServerField();

	// Determine authenticator type
	bool use_uid = true;
	bool use_pswd = true;

	// See if configuration plugin is present
	if (CPluginManager::sPluginManager.HasConfig())
	{
		use_uid = CPluginManager::sPluginManager.GetConfig()->PromptUserID();
		use_pswd = CPluginManager::sPluginManager.GetConfig()->PromptPswd();
	}
	else
		// Determine from authenticator type
		use_uid = use_pswd = (remote ?
						CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().RequiresUserPswd() :
						CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetAuthenticator().RequiresUserPswd());

	// Check for use of uid & password
	if (use_uid)
	{
		mAuth->Hide();
	}
	else
	{
		mUserID->Hide();
		mUserIDCaption->Hide();
		
		if (!CPluginManager::sPluginManager.HasConfig())
		{
			cdstring auth = (remote ?
							CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().GetDescriptor() :
							CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetAuthenticator().GetDescriptor());
			cdstring title;
			title.FromResource("UI::MU::AuthMethod");
			title.Substitute(auth);
			
			mAuth->SetText(title);
		}

		mOKBtn->Activate();
	}

	if (use_pswd)
	{

		if (!remote &&
			CPreferences::sPrefs->mMailAccounts.GetValue().size() &&
			!CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetLogonAtStart())
		{
			mPassword->Hide();
			mPasswordCaption->Hide();
		}
	}
	else
	{
		mPassword->Hide();
		mPasswordCaption->Hide();
	}

	// Display connection field if requested
	if (CAdminLock::sAdminLock.mSSLStateDisplay)
	{
		bool secure = false;
		if (remote)
			secure = (CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetTLSType() != CINETAccount::eNoTLS);
		else
			secure = (CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetTLSType() != CINETAccount::eNoTLS);
		cdstring temp;
		temp.FromResource(secure ? "UI::UserPswd::Secure" : "UI::UserPswd::Insecure");
		mSSLState->SetText(temp);
	}
	else
	{
		mSSLState->Hide();
		mSSLStateCaption->Hide();
	}

}

// Initialise server fields
void CMultiUserDialog::InitServerField()
{
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
		mServerTxt->Hide();
		
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
	}
}

void CMultiUserDialog::InitServerMenu()
{
	// Add each mail account
	for(cdstrvect::const_iterator iter = mMailServers.begin(); iter != mMailServers.end(); iter++)
		mServerPopup->AppendItem(*iter, kFalse, kTrue);
	
	// Now add in other if required
	if (mUseOther)
	{
		mServerPopup->ShowSeparatorAfter(mMailServers.size(), kTrue);
		mServerPopup->AppendItem("Other...", kFalse, kTrue);
	}
	
	// Initial value
	mServerPopup->SetValue(1);
}

void CMultiUserDialog::OnServerPopup(JIndex nID)
{
	// Check for other
	if (mUseOther && (nID > mMailServers.size()))
	{
		// Get name from user
		cdstring temp_mail;
		cdstring temp_smtp;
		//if (CChooseServersDialog::PoseDialog(temp_mail, temp_smtp, this))
		{
			mMailServerName = temp_mail;
			mSMTPServerName = temp_smtp;
		}
		//else
		{
			mServerPopup->SetValue(1);
		}
	}
	else
	{
		cdstring title(mServerPopup->GetCurrentItemText());
		mMailServerName = title;
		mSMTPServerName = cdstring::null_str;
		mServerIndex = nID - 1;
	}
}

// Get new text
void CMultiUserDialog::UpdatePrefs(void)
{
	// Configure multiple servers
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

	// Configure prefs from MU
	cdstring str = mUserID->GetText().GetCString();
	cdstring real_name = (mRealName ? mRealName->GetText().GetCString() : "");

	// NB Don't configure the return address here. Instead wait for the first successful login
	// to complete and then do it. This prevents an incorrect MU dialog user id from being
	// put into the user's From address.
	CPreferences::sPrefs->MultiuserConfigure(str, real_name, false);

	// Store password
	CAdminLock::sAdminLock.mInitialPswd = mPassword->GetText().GetCString();

	// Now do any config update of prefs - this will override default MU startup
	if (CPluginManager::sPluginManager.HasConfig())
		UpdateConfigPrefs();
}

// Get new text
void CMultiUserDialog::UpdateConfigPrefs(void)
{
	CPluginManager::sPluginManager.GetConfig()->DoConfiguration(mUserID->GetText(), mPassword->GetText(), mRealName->GetText(), mMailServerName);

	// At this point prefs have been configured by plugin
}

bool CMultiUserDialog::PoseDialog()
{
	bool result = false;
	CMultiUserDialog* dlog = new CMultiUserDialog(JXGetApplication());
	dlog->mUseRealName = CAdminLock::sAdminLock.mAskRealName;
	dlog->OnCreate();
	dlog->InitFields();

	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		// Do configure multiple servers
		dlog->UpdatePrefs();
		dlog->Close();
		result = true;
	}

	return result;
}
