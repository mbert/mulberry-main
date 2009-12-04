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
#include "CChooseServersDialog.h"
#include "CConfigPlugin.h"
#include "CMulberryApp.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CStringOptionsMap.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CMultiUserDialog dialog


CMultiUserDialog::CMultiUserDialog(bool real_name, CWnd* pParent /*=NULL*/)
	: CHelpDialog(real_name ? CMultiUserDialog::IDD2 : CMultiUserDialog::IDD1, pParent)
{
	mRealNameUse = real_name;

	//{{AFX_DATA_INIT(CMultiUserDialog)
	//}}AFX_DATA_INIT
}


void CMultiUserDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMultiUserDialog)
	DDX_Control(pDX, IDOK, mOKBtn);
	DDX_Control(pDX, IDC_MUPASSWORD, mPasswordCtrl);
	DDX_Control(pDX, IDC_MUPASSWORD_TXT, mPasswordCaptionCtrl);
	DDX_Control(pDX, IDC_MUUSERID, mUserIDCtrl);
	DDX_Control(pDX, IDC_MUUSERID_TXT, mUserIDCaptionCtrl);
	DDX_Control(pDX, IDC_MUSECURE, mSSLStateCtrl);
	DDX_Control(pDX, IDC_MUSECURE_TXT, mSSLStateCaptionCtrl);
	DDX_Control(pDX, IDC_MUAUTH, mAuthCtrl);
	DDX_UTF8Text(pDX, IDC_MUSERVER, mServerTxt);
	DDX_Control(pDX, IDC_MUSERVER_POPUP, mServerPopup);
	if (mRealNameUse)
		DDX_UTF8Text(pDX, IDC_MUREALNAME, mRealName);
	DDX_UTF8Text(pDX, IDC_MUUSERID, mUserID);
	DDX_UTF8Text(pDX, IDC_MUPASSWORD, mPassword);
	DDX_UTF8Text(pDX, IDC_MUAUTH, mAuth);
	DDX_UTF8Text(pDX, IDC_MUVERSION, mVersion);
	//DDX_UTF8Text(pDX, IDC_MUSERIAL, mSerial);
	//DDX_UTF8Text(pDX, IDC_MULICENSE, mLicensedTo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMultiUserDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CMultiUserDialog)
	ON_EN_CHANGE(IDC_MUUSERID, OnChangeEntry)
	ON_EN_CHANGE(IDC_MUPASSWORD, OnChangeEntry)
	ON_COMMAND_RANGE(IDM_MUSERVERStart, IDM_MUSERVEREnd, OnServerPopup)
	ON_COMMAND(IDM_MUSERVEROther, OnServerOther)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMultiUserDialog message handlers

// Called during startup
BOOL CMultiUserDialog::OnInitDialog(void)
{
	CHelpDialog::OnInitDialog();

	// Hide password field of not logon at startup and not remote
	bool remote = (CPreferences::sPrefs->mRemoteAccounts.GetValue().size() ?
					CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetLogonAtStart() && !CAdminLock::sAdminLock.mNoRemotePrefs : false);

	// Set up server name
	InitServerField();

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

		mAuthCtrl.ShowWindow(SW_HIDE);
	}
	else
	{
		mUserIDCtrl.ShowWindow(SW_HIDE);
		mUserIDCaptionCtrl.ShowWindow(SW_HIDE);
		
		if (!CPluginManager::sPluginManager.HasConfig())
		{
			cdstring auth = (remote ?
							CPreferences::sPrefs->mRemoteAccounts.GetValue().front()->GetAuthenticator().GetDescriptor() :
							CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetAuthenticator().GetDescriptor());
			cdstring temp;
			temp.FromResource("UI::MU::AuthMethod");
			temp.Substitute(auth);
			
			CUnicodeUtils::SetWindowTextUTF8(&mAuthCtrl, temp);
		}

		mOKBtn.EnableWindow(true);
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
		CUnicodeUtils::SetWindowTextUTF8(&mSSLStateCtrl, temp);
	}
	else
	{
		mSSLStateCtrl.ShowWindow(SW_HIDE);
		mSSLStateCaptionCtrl.ShowWindow(SW_HIDE);
	}

	if (mUsePswd)
	{

		if (!remote &&
			CPreferences::sPrefs->mMailAccounts.GetValue().size() &&
			!CPreferences::sPrefs->mMailAccounts.GetValue().front()->GetLogonAtStart())
		{
			mPasswordCtrl.ShowWindow(SW_HIDE);
			mPasswordCaptionCtrl.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		mPasswordCtrl.ShowWindow(SW_HIDE);
		mPasswordCaptionCtrl.ShowWindow(SW_HIDE);
	}

	return true;
}

void CMultiUserDialog::OnChangeEntry()
{
	cdstring userid = CUnicodeUtils::GetWindowTextUTF8(mUserIDCtrl);

	if (userid.empty())
		mOKBtn.EnableWindow(false);
	else
		mOKBtn.EnableWindow(true);
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
	cdstring str = mUserID;
	cdstring real_name = mRealName;

	// Configure prefs from MU
	// NB Don't configure the return address here. Instead wait for the first successful login
	// to complete and then do it. This prevents an incorrect MU dialog user id from being
	// put into the user's From address.
	CPreferences::sPrefs->MultiuserConfigure(str, real_name, false);

	// Store password
	CAdminLock::sAdminLock.mInitialPswd = mPassword;

	// Now do any config update of prefs - this will override default MU startup
	if (CPluginManager::sPluginManager.HasConfig())
		UpdateConfigPrefs();
}

// Get new text
void CMultiUserDialog::UpdateConfigPrefs(void)
{
	CPluginManager::sPluginManager.GetConfig()->DoConfiguration(mUserID, mPassword, mRealName, mMailServerName);

	// At this point prefs have been configured by plugin
}

// Initialise server fields
void CMultiUserDialog::InitServerField()
{
	mServerPopup.SubclassDlgItem(IDC_MUSERVER_POPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mServerPopup.SetMenu(IDR_POPUP_MUSERVER);

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
		GetDlgItem(IDC_MUSERVER)->ShowWindow(SW_HIDE);
		
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
		mServerPopup.ShowWindow(SW_HIDE);
	}
}

void CMultiUserDialog::InitServerMenu()
{
	CMenu* pPopup = mServerPopup.GetPopupMenu();

	// Get name of other
	cdstring other = CUnicodeUtils::GetMenuStringUTF8(pPopup, 1, MF_BYPOSITION);

	// Delete other items
	for(short i = pPopup->GetMenuItemCount() - 1; i >= 0; i--)
		pPopup->RemoveMenu(i, MF_BYPOSITION);

	// Add each mail account
	int menu_id = IDM_MUSERVERStart;
	for(cdstrvect::const_iterator iter = mMailServers.begin(); iter != mMailServers.end(); iter++, menu_id++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, *iter);
	
	// Now add in other if required
	if (mUseOther)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_SEPARATOR);
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, IDM_MUSERVEROther, other);
	}
	
	// Initial value
	mServerPopup.SetValue(IDM_MUSERVERStart);
}

void CMultiUserDialog::OnServerPopup(UINT nID)
{
	mServerPopup.SetValue(nID);
	mMailServerName = CUnicodeUtils::GetWindowTextUTF8(&mServerPopup);
	mSMTPServerName = cdstring::null_str;
	mServerIndex = nID - IDM_MUSERVERStart;
}

void CMultiUserDialog::OnServerOther()
{
	// Get name from user
	cdstring temp_mail;
	cdstring temp_smtp;
	if (CChooseServersDialog::PoseDialog(temp_mail, temp_smtp))
	{
		mMailServerName = temp_mail;
		mSMTPServerName = temp_smtp;
		mServerPopup.SetValue(IDM_MUSERVEROther);
	}
	else
	{
		mServerPopup.SetValue(IDM_MUSERVERStart);
		mMailServerName = CUnicodeUtils::GetWindowTextUTF8(&mServerPopup);
		mSMTPServerName = cdstring::null_str;
		mServerIndex = 0;
	}
}
