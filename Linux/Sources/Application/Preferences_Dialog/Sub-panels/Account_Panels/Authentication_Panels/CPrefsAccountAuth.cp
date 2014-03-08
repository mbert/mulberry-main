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
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPreferencesDialog.h"
#include "CPrefsAuthPanel.h"
#include "CPrefsAuthPlainText.h"
#include "CPrefsAuthKerberos.h"
#include "CPrefsAuthAnonymous.h"

#include "TPopupMenu.h"

#include <JXCardFile.h>
#include <JXDownRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>

#include <algorithm>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsAccountAuth::OnCreate()
{
// begin JXLayout1

    JXDownRect* obj1 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,173, 340,52);
    assert( obj1 != NULL );

    JXDownRect* obj2 =
        new JXDownRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,15, 340,142);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Method:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,8, 55,20);
    assert( obj3 != NULL );

    mAuthPopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,5, 180,20);
    assert( mAuthPopup != NULL );

    mAuthSubPanel =
        new JXCardFile(obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,15, 330,120);
    assert( mAuthSubPanel != NULL );

    mTLSPopupTitle =
        new JXStaticText("Secure:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,165, 55,20);
    assert( mTLSPopupTitle != NULL );

    mTLSPopup =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,162, 180,20);
    assert( mTLSPopup != NULL );

    mUseTLSClientCert =
        new JXTextCheckbox("Use Client Certificate", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,17, 145,20);
    assert( mUseTLSClientCert != NULL );

    mTLSClientCert =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,17, 180,20);
    assert( mTLSClientCert != NULL );

// end JXLayout1

	BuildCertPopup();

	// Initialise the panels
	AddPanel(new CPrefsAuthPlainText(mAuthSubPanel, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 330, 120));
	AddPanel(new CPrefsAuthKerberos(mAuthSubPanel, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 330, 120));
	AddPanel(new CPrefsAuthAnonymous(mAuthSubPanel, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 330, 120));

	// Start listening
	ListenTo(mAuthPopup);
	ListenTo(mTLSPopup);
	ListenTo(mUseTLSClientCert);
}

void CPrefsAccountAuth::AddPanel(CPrefsAuthPanel* panel)
{
	panel->OnCreate();
	mAuthSubPanel->AppendCard(panel);
}

// Handle buttons
void CPrefsAccountAuth::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mAuthPopup)
    	{
			OnAuthPopup();
			return;
		}
    	else if (sender == mTLSPopup)
    	{
			TLSItemsState();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
    	if (sender == mUseTLSClientCert)
    	{
			TLSItemsState();
			return;
		}
	}
	
	CTabPanel::Receive(sender, message);
}

// Set prefs
void CPrefsAccountAuth::SetData(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Build popup from panel
	BuildAuthPopup(account);

	if (mCurrentPanel)
		mCurrentPanel->SetAuth(account->GetAuthenticator().GetAuthenticator());

	InitTLSPopup(account);
	mTLSPopup->SetValue(account->GetTLSType() + 1);
	mUseTLSClientCert->SetState(JBoolean(account->GetUseTLSClientCert()));

	// Match fingerprint in list
	for(cdstrvect::const_iterator iter = mCertFingerprints.begin(); iter != mCertFingerprints.end(); iter++)
	{
		if (account->GetTLSClientCert() == *iter)
		{
			mTLSClientCert->SetPopupByName(mCertSubjects.at(iter - mCertFingerprints.begin()));
			break;
		}
	}

	TLSItemsState();
}

// Force update of prefs
bool CPrefsAccountAuth::UpdateData(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Copy info from panel into prefs
	cdstring temp(mAuthPopup->GetCurrentItemText());
	
	// Special for anonymous
	if (mAuthPopup->GetValue() == mAuthPopup->GetItemCount() - 1)
		temp = "None";

	account->GetAuthenticator().SetDescriptor(temp);

	if (mCurrentPanel)
		mCurrentPanel->UpdateAuth(account->GetAuthenticator().GetAuthenticator());

	account->SetTLSType((CINETAccount::ETLSType) (mTLSPopup->GetValue() - 1));
	
	account->SetUseTLSClientCert(mUseTLSClientCert->IsChecked());

	if (mCertFingerprints.size())
		account->SetTLSClientCert(mCertFingerprints.at(mTLSClientCert->GetValue() - 1));
	else
		account->SetTLSClientCert(cdstring::null_str);
	return true;
}

// Set auth panel
void CPrefsAccountAuth::SetAuthPanel(const cdstring& auth_type)
{
	// Find matching auth plugin
	CAuthPlugin* plugin = CPluginManager::sPluginManager.GetAuthPlugin(auth_type);

	CAuthPlugin::EAuthPluginUIType ui_type = plugin ? plugin->GetAuthUIType() :
												(auth_type == "Plain Text" ? CAuthPlugin::eAuthUserPswd : CAuthPlugin::eAuthAnonymous);

	int panel;
	switch(ui_type)
	{
	case CAuthPlugin::eAuthUserPswd:
		panel = 1;
		break;
	case CAuthPlugin::eAuthKerberos:
		panel = 2;
		break;
	case CAuthPlugin::eAuthAnonymous:
		panel = 3;
		break;
	}

	if (mAuthType != auth_type)
	{
		mAuthType = auth_type;
		mAuthSubPanel->ShowCard(panel);
		mCurrentPanel = static_cast<CPrefsAuthPanel*>(mAuthSubPanel->GetCurrentCard());
	}
}

void CPrefsAccountAuth::BuildAuthPopup(CINETAccount* account)
{
	// Copy info
	cdstring set_name;
	short set_value = 0;
	switch(account->GetAuthenticatorType())
	{
	case CAuthenticator::eNone:
		set_value = 0;
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
	default:;
	}

	// Remove any existing plugin items from main menu
	mAuthPopup->RemoveAllItems();
	cdstring menu_items = "Plain Text %r";

	cdstrvect plugin_names;
	CPluginManager::sPluginManager.GetAuthPlugins(plugin_names);
	std::sort(plugin_names.begin(), plugin_names.end());
	short index = 2;
	for(cdstrvect::const_iterator iter = plugin_names.begin(); iter != plugin_names.end(); iter++, index++)
	{
		menu_items += " |";
		menu_items += *iter;
		menu_items += " %r";

		if (*iter == set_name)
			set_value = index;
	}
	menu_items += " %l |Anonymous %r |SSL Client Certificate %r";
	mAuthPopup->SetMenuItems(menu_items);

	// Force max/min update
	mAuthPopup->SetUpdateAction(JXMenu::kDisableNone);
	
	// Set value
	{
		StStopListening _no_listen(this);
		mAuthPopup->SetValue(set_value ? set_value : index);
	}
	cdstring title(mAuthPopup->GetCurrentItemText());
	SetAuthPanel(title);
}

void CPrefsAccountAuth::InitTLSPopup(CINETAccount* account)
{
	mTLSPopup->SetMenuItems("No Security %r | SSLv23 %r | SSLv3 %r | STARTTLS - TLSv1 %r | STARTTLS - SSL %r");
	mTLSPopup->SetUpdateAction(JXMenu::kDisableNone);
	mTLSPopup->SetToPopupChoice(kTrue, 1);

	// Enable each item based on what the protocol supports
	bool enabled = false;
	for(int i = CINETAccount::eNoTLS; i <= CINETAccount::eTLSTypeMax; i++)
	{
		if (account->SupportsTLSType(static_cast<CINETAccount::ETLSType>(i)))
		{
			mTLSPopup->EnableItem(i + 1);
			enabled = true;
		}
		else
			mTLSPopup->DisableItem(i + 1);
	}
	
	// Hide if no plugin present or none enabled
	if (enabled && CPluginManager::sPluginManager.HasSSL())
	{
		mTLSPopup->Show();
		mTLSPopupTitle->Show();
	}
	else
	{
		mTLSPopup->Hide();
		mTLSPopupTitle->Hide();

		// Disable the auth item
		mAuthPopup->DisableItem(mAuthPopup->GetItemCount());
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
		mTLSClientCert->RemoveAllItems();

		cdstring menu_items;
		for(cdstrvect::const_iterator iter = mCertSubjects.begin(); iter != mCertSubjects.end(); iter++)
		{
			if (iter != mCertSubjects.begin())
				menu_items += " |";
			menu_items += *iter;
			menu_items += " %r";
		}
		mTLSClientCert->SetMenuItems(menu_items);
		if (!menu_items.empty())
			mTLSClientCert->SetValue(1);
	}	
}

void CPrefsAccountAuth::OnAuthPopup()
{
	cdstring title(mAuthPopup->GetCurrentItemText());
	SetAuthPanel(title);
}

void CPrefsAccountAuth::TLSItemsState()
{
	// TLS popup
	if (mTLSPopup->GetValue() - 1 == CINETAccount::eNoTLS)
	{
		mUseTLSClientCert->Deactivate();
		mTLSClientCert->Deactivate();
		if (mAuthPopup->GetValue() == mAuthPopup->GetItemCount())
		{
			mAuthPopup->SetValue(1);
			OnAuthPopup();
		}
		mAuthPopup->DisableItem(mAuthPopup->GetItemCount());
	}
	else
	{
		mUseTLSClientCert->Activate();

		if (mUseTLSClientCert->IsChecked())
		{
			mTLSClientCert->Activate();
			mAuthPopup->EnableItem(mAuthPopup->GetItemCount());
		}
		else
		{
			mTLSClientCert->Deactivate();
			if (mAuthPopup->GetValue() == mAuthPopup->GetItemCount())
			{
				mAuthPopup->SetValue(1);
				OnAuthPopup();
			}
			mAuthPopup->DisableItem(mAuthPopup->GetItemCount());
		}
	}
}
