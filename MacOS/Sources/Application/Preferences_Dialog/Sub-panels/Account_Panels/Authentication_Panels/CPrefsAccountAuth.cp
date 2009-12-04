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


// Source for CPrefsAccountAuth class

#include "CPrefsAccountAuth.h"

#include "CAuthPlugin.h"
#include "CCertificateManager.h"
#include "CINETAccount.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsAuthPanel.h"
#include "CPrefsAuthPlainText.h"
#include "CPrefsAuthKerberos.h"
#include "CPrefsAuthAnonymous.h"

#include <LCheckBox.h>
#include <LPopupButton.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsAccountAuth::CPrefsAccountAuth()
{
	mCurrentPanel = nil;
	mCurrentPanelNum = 0;
}

// Constructor from stream
CPrefsAccountAuth::CPrefsAccountAuth(LStream *inStream)
		: CPrefsTabSubPanel(inStream)
{
	mCurrentPanel = nil;
	mCurrentPanelNum = 0;
}

// Default destructor
CPrefsAccountAuth::~CPrefsAccountAuth()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountAuth::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsTabSubPanel::FinishCreateSelf();

	// Get controls
	mAuthPopup = (LPopupButton*) FindPaneByID(paneid_AccountAuthPopup);
	mAuthSubPanel = (LView*) FindPaneByID(paneid_AccountAuthPanel);
	mTLSPopup = (LPopupButton*) FindPaneByID(paneid_AccountTLSPopup);
	mUseTLSClientCert = (LCheckBox*) FindPaneByID(paneid_AccountUseTLSClientCert);
	mTLSClientCert = (LPopupButton*) FindPaneByID(paneid_AccountTLSClientCert);
	BuildCertPopup();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this,this,RidL_CPrefsAccountAuthBtns);

	// Start with first panel
	SetAuthPanel(cdstring::null_str);
}

// Handle buttons
void CPrefsAccountAuth::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case msg_MailAccountAuth:
	{
		cdstring temp = ::GetPopupMenuItemTextUTF8(mAuthPopup);
		SetAuthPanel(temp);
		break;
	}
	case msg_AccountTLSPopup:
		TLSItemsState();
		break;
	case msg_AccountUseTLSClientCert:
		TLSItemsState();
		break;
	}
}

// Toggle display of IC
void CPrefsAccountAuth::ToggleICDisplay(bool IC_on)
{
	if (mCurrentPanel)
		mCurrentPanel->ToggleICDisplay(IC_on);
}

// Set prefs
void CPrefsAccountAuth::SetData(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Build popup from panel
	BuildAuthPopup(account);

	if (mCurrentPanel)
		mCurrentPanel->SetAuth(account->GetAuthenticator().GetAuthenticator());
	
	InitTLSItems(account);
	mTLSPopup->SetValue(account->GetTLSType() + 1);
	mUseTLSClientCert->SetValue(account->GetUseTLSClientCert());

	// Match fingerprint in list
	for(cdstrvect::const_iterator iter = mCertFingerprints.begin(); iter != mCertFingerprints.end(); iter++)
	{
		if (account->GetTLSClientCert() == *iter)
		{
			::SetPopupByName(mTLSClientCert, mCertSubjects.at(iter - mCertFingerprints.begin()));
			break;
		}
	}

	TLSItemsState();
}

// Force update of prefs
void CPrefsAccountAuth::UpdateData(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Copy info from panel into prefs
	cdstring temp = ::GetPopupMenuItemTextUTF8(mAuthPopup);
	
	// Special for anonymous
	if (mAuthPopup->GetValue() == ::CountMenuItems(mAuthPopup->GetMacMenuH()) - 1)
		temp = "None";

	account->GetAuthenticator().SetDescriptor(temp);

	if (mCurrentPanel)
		mCurrentPanel->UpdateAuth(account->GetAuthenticator().GetAuthenticator());

	int popup = mTLSPopup->GetValue() - 1;
	account->SetTLSType((CINETAccount::ETLSType) (mTLSPopup->GetValue() - 1));
	
	account->SetUseTLSClientCert(mUseTLSClientCert->GetValue());

	if (account->GetUseTLSClientCert() && ::CountMenuItems(mTLSClientCert->GetMacMenuH()))
		account->SetTLSClientCert(mCertFingerprints.at(mTLSClientCert->GetValue() - 1));
	else
		account->SetTLSClientCert(cdstring::null_str);
}

// Set auth panel
void CPrefsAccountAuth::SetAuthPanel(const cdstring& auth_type)
{
	// Find matching auth plugin
	CAuthPlugin* plugin = CPluginManager::sPluginManager.GetAuthPlugin(auth_type);

	CAuthPlugin::EAuthPluginUIType ui_type = plugin ? plugin->GetAuthUIType() :
												(auth_type == "Plain Text" ? CAuthPlugin::eAuthUserPswd : CAuthPlugin::eAuthAnonymous);

	ResIDT panel;
	switch(ui_type)
	{
	case CAuthPlugin::eAuthUserPswd:
		panel = paneid_PrefsAuthPlainText;
		break;
	case CAuthPlugin::eAuthKerberos:
		panel = paneid_PrefsAuthKerberos;
		break;
	case CAuthPlugin::eAuthAnonymous:
		panel = paneid_PrefsAuthAnonymous;
		break;
	}

	if (mAuthType != auth_type)
	{
		mAuthType = auth_type;

		// First remove and update any existing panel
		mAuthSubPanel->DeleteAllSubPanes();

		// Update to new panel id
		mCurrentPanelNum = panel;

		// Make panel area default so new panel is automatically added to it
		if (panel)
		{
			SetDefaultView(mAuthSubPanel);
			mAuthSubPanel->Hide();
			CPreferencesDialog* prefs_dlog = (CPreferencesDialog*) mSuperView;
			while(prefs_dlog->GetPaneID() != paneid_PreferencesDialog)
				prefs_dlog = (CPreferencesDialog*) prefs_dlog->GetSuperView();
			LCommander* defCommander;
			prefs_dlog->GetSubCommanders().FetchItemAt(1, defCommander);
			prefs_dlog->SetDefaultCommander(defCommander);
			mCurrentPanel = (CPrefsAuthPanel*) UReanimator::ReadObjects('PPob', mCurrentPanelNum);
			mCurrentPanel->FinishCreate();
			mAuthSubPanel->Show();
		}
		else
		{
			mAuthSubPanel->Refresh();
			mCurrentPanel = nil;
		}
	}
}

void CPrefsAccountAuth::BuildAuthPopup(CINETAccount* account)
{
	// Copy info
	cdstring set_name;
	short set_value = -1;
	switch(account->GetAuthenticatorType())
	{
	case CAuthenticator::eNone:
		set_value = -1;
		break;
	case CAuthenticator::ePlainText:
		set_value = 1;
		break;
	case CAuthenticator::eSSL:
		set_value = 0;
		break;
	case CAuthenticator::ePlugin:
		set_name = account->GetAuthenticator().GetDescriptor();
		break;
	}

	// Remove any existing plugin items from main menu
	MenuHandle menuH = mAuthPopup->GetMacMenuH();
	short num_remove = ::CountMenuItems(menuH) - 4;
	for(short i = 0; i < num_remove; i++)
		::DeleteMenuItem(menuH, 2);

	cdstrvect plugin_names;
	CPluginManager::sPluginManager.GetAuthPlugins(plugin_names);
	std::sort(plugin_names.begin(), plugin_names.end());
	short index = 1;
	for(cdstrvect::const_iterator iter = plugin_names.begin(); iter != plugin_names.end(); iter++, index++)
	{
		::InsertMenuItem(menuH, "\p?", index);
		::SetMenuItemTextUTF8(menuH, index + 1, *iter);
		
		if (*iter == set_name)
			set_value = index + 1;
	}

	// Force max/min update
	mAuthPopup->SetMenuMinMax();
	
	// Set value
	StopListening();
	mAuthPopup->SetValue((set_value > 0) ? set_value : index + 3 + set_value);
	StartListening();
	cdstring temp = ::GetPopupMenuItemTextUTF8(mAuthPopup);
	SetAuthPanel(temp);
}

void CPrefsAccountAuth::InitTLSItems(CINETAccount* account)
{
	// Enable each item based on what the protocol supports
	bool enabled = false;
	for(int i = CINETAccount::eNoTLS; i <= CINETAccount::eTLSTypeMax; i++)
	{
		if (account->SupportsTLSType(static_cast<CINETAccount::ETLSType>(i)))
		{
			::EnableItem(mTLSPopup->GetMacMenuH(), i + 1);
			enabled = true;
		}
		else
			::DisableItem(mTLSPopup->GetMacMenuH(), i + 1);
	}
	
	// Hide if no plugin present or none enabled
	if (enabled && CPluginManager::sPluginManager.HasSSL())
	{
		mTLSPopup->Show();
		FindPaneByID(paneid_AccountTLSGroup)->Show();
		//mUseTLSClientCert->Show();
		//mTLSClientCert->Show();
	}
	else
	{
		mTLSPopup->Hide();
		FindPaneByID(paneid_AccountTLSGroup)->Hide();
		//mUseTLSClientCert->Hide();
		//mTLSClientCert->Hide();

		// Disable the auth item
		::DisableItem(mAuthPopup->GetMacMenuH(), mAuthPopup->GetMaxValue());
	}
}

void CPrefsAccountAuth::BuildCertPopup()
{
	// Only if certs are available
	if (CCertificateManager::HasCertificateManager())
	{
		// Get list of private certificates
		CCertificateManager::sCertificateManager->GetPrivateCertificates(mCertSubjects, mCertFingerprints);

		// Remove any existing items from main menu
		MenuHandle menuH = mTLSClientCert->GetMacMenuH();
		while(::CountMenuItems(menuH))
			::DeleteMenuItem(menuH, 1);

		short index = 1;
		for(cdstrvect::const_iterator iter = mCertSubjects.begin(); iter != mCertSubjects.end(); iter++, index++)
			::AppendItemToMenu(menuH, index, (*iter).c_str());

		// Force max/min update
		mTLSClientCert->SetMenuMinMax();
	}	
}

void CPrefsAccountAuth::TLSItemsState()
{
	// TLS popup
	if (mTLSPopup->GetValue() - 1 == CINETAccount::eNoTLS)
	{
		mUseTLSClientCert->Disable();
		mTLSClientCert->Disable();
		if (mAuthPopup->GetValue() == mAuthPopup->GetMaxValue())
			mAuthPopup->SetValue(1);
		::DisableItem(mAuthPopup->GetMacMenuH(), mAuthPopup->GetMaxValue());
	}
	else
	{
		mUseTLSClientCert->Enable();

		if (mUseTLSClientCert->GetValue())
		{
			mTLSClientCert->Enable();
			::EnableItem(mAuthPopup->GetMacMenuH(), mAuthPopup->GetMaxValue());
		}
		else
		{
			mTLSClientCert->Disable();
			if (mAuthPopup->GetValue() == mAuthPopup->GetMaxValue())
				mAuthPopup->SetValue(1);
			::DisableItem(mAuthPopup->GetMacMenuH(), mAuthPopup->GetMaxValue());
		}
	}
}
