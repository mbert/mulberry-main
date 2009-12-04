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


// CPrefsAccountAuth.cpp : implementation file
//

#include "CPrefsAccountAuth.h"

#include "CAuthPlugin.h"
#include "CCertificateManager.h"
#include "CINETAccount.h"
#include "CPluginManager.h"
#include "CPrefsAuthPlainText.h"
#include "CPrefsAuthKerberos.h"
#include "CUnicodeUtils.h"

#include "resource1.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountAuth dialog


CPrefsAccountAuth::CPrefsAccountAuth()
	: CTabPanel(CPrefsAccountAuth::IDD)
{
	//{{AFX_DATA_INIT(CPrefsAccountAuth)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CPrefsAccountAuth::DoDataExchange(CDataExchange* pDX)
{
	CTabPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsAccountAuth)
	//}}AFX_DATA_MAP
	
	// Update items now
	if (pDX->m_bSaveAndValidate)
	{
		mPanels.DoDataExchange(pDX);
	}
}


BEGIN_MESSAGE_MAP(CPrefsAccountAuth, CTabPanel)
	//{{AFX_MSG_MAP(CPrefsAccountAuth)
	ON_COMMAND_RANGE(IDM_AUTHENTICATE_METHOD_PLAIN, IDM_AUTHENTICATE_METHOD_LAST, OnAuthPopup)
	ON_COMMAND_RANGE(IDM_TLSPOPUP_NOSECURITY, IDM_TLSPOPUP_TLSBROKEN, OnTLSPopup)
	ON_COMMAND(IDC_PREFS_AUTHENTICATE_USETLSCERT, OnUseTLSCert)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountAuth message handlers

BOOL CPrefsAccountAuth::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Subclass buttons
	mAuthPopup.SubclassDlgItem(IDC_PREFS_AUTHENTICATE_POPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAuthPopup.SetMenu(IDR_POPUP_AUTHENTICATE_METHOD);

	mPanels.SubclassDlgItem(IDC_PREFS_AUTHENTICATE_PANELS, this);

	// Create tab panels
	mPanels.AddPanel(new CPrefsAuthPlainText);
	mPanels.AddPanel(new CPrefsAuthKerberos);

	// Set initial tab
	mPanels.SetPanel(0);

	mTLSPopup.SubclassDlgItem(IDC_PREFS_AUTHENTICATE_TLSPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mTLSPopup.SetMenu(IDR_POPUP_AUTHENTICATE_TLS);

	mTLSUseCert.SubclassDlgItem(IDC_PREFS_AUTHENTICATE_USETLSCERT, this);

	mTLSCert.SubclassDlgItem(IDC_PREFS_AUTHENTICATE_TLSCERT, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mTLSCert.SetMenu(IDR_POPUP_AUTHENTICATE_TLSCERT);
	BuildCertPopup();

	return true;
}

void CPrefsAccountAuth::OnAuthPopup(UINT nID) 
{
	cdstring choice = CUnicodeUtils::GetMenuStringUTF8(mAuthPopup.GetPopupMenu(), nID, MF_BYCOMMAND);
	SetAuthPanel(choice);
	mAuthPopup.SetValue(nID);
}

void CPrefsAccountAuth::OnTLSPopup(UINT nID) 
{
	mTLSPopup.SetValue(nID);
	TLSItemsState();
}

void CPrefsAccountAuth::OnUseTLSCert() 
{
	TLSItemsState();
}

// Set data
void CPrefsAccountAuth::SetContent(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Build popup from panel
	BuildAuthPopup(account);

	mPanels.SetPanelContent(account->GetAuthenticator().GetAuthenticator());
	
	InitTLSItems(account);
	mTLSPopup.SetValue(account->GetTLSType() + IDM_TLSPOPUP_NOSECURITY);
	mTLSUseCert.SetCheck(account->GetUseTLSClientCert());

	// Match fingerprint in list
	if (CCertificateManager::HasCertificateManager())
	{
		UINT id = IDM_AUTHENTICATE_TLSCERT_Start;
		mTLSCert.SetValue(id);
		for(cdstrvect::const_iterator iter = mCertFingerprints.begin(); iter != mCertFingerprints.end(); iter++, id++)
		{
			if (account->GetTLSClientCert() == *iter)
			{
				mTLSCert.SetValue(id);
				break;
			}
		}
	}

	TLSItemsState();
}

// Force update of data
bool CPrefsAccountAuth::UpdateContent(void* data)
{
	CINETAccount* account = (CINETAccount*) data;

	// Copy info from panel into prefs
	cdstring choice = CUnicodeUtils::GetMenuStringUTF8(mAuthPopup.GetPopupMenu(), mAuthPopup.GetValue(), MF_BYCOMMAND);
	
	// Special for anonymous
	if (mAuthPopup.GetValue() == IDM_AUTHENTICATE_METHOD_ANONYMOUS)
		choice = "None";
	account->GetAuthenticator().SetDescriptor(choice);

	mPanels.UpdatePanelContent(account->GetAuthenticator().GetAuthenticator());

	account->SetTLSType((CINETAccount::ETLSType) (mTLSPopup.GetValue() - IDM_TLSPOPUP_NOSECURITY));
	
	account->SetUseTLSClientCert(mTLSUseCert.GetCheck());

	// Popup may be empty
	if (CCertificateManager::HasCertificateManager())
	{
		if (account->GetUseTLSClientCert() && mTLSCert.GetPopupMenu()->GetMenuItemCount())
			account->SetTLSClientCert(mCertFingerprints.at(mTLSCert.GetValue() - IDM_AUTHENTICATE_TLSCERT_Start));
		else
			account->SetTLSClientCert(cdstring::null_str);
	}

	return true;
}

// Set auth panel
void CPrefsAccountAuth::SetAuthPanel(const cdstring& auth_type)
{
	// Find matching auth plugin
	CAuthPlugin* plugin = CPluginManager::sPluginManager.GetAuthPlugin(auth_type);

	CAuthPlugin::EAuthPluginUIType ui_type = plugin ? plugin->GetAuthUIType() :
												(auth_type == "Plain Text" ? CAuthPlugin::eAuthUserPswd : CAuthPlugin::eAuthAnonymous);

	short panel;
	switch(ui_type)
	{
	case CAuthPlugin::eAuthUserPswd:
		panel = 0;
		break;
	case CAuthPlugin::eAuthKerberos:
		panel = 1;
		break;
	case CAuthPlugin::eAuthAnonymous:
		panel = -1;
		break;
	}

	if (mAuthType != auth_type)
	{
		mAuthType = auth_type;
		mPanels.SetPanel(panel);
	}
}

void CPrefsAccountAuth::BuildAuthPopup(CINETAccount* account)
{
	// Copy info
	cdstring set_name;
	short set_value = IDM_AUTHENTICATE_METHOD_ANONYMOUS;
	switch(account->GetAuthenticatorType())
	{
	case CAuthenticator::eNone:
		set_value = IDM_AUTHENTICATE_METHOD_ANONYMOUS;
		break;
	case CAuthenticator::ePlainText:
		set_value = IDM_AUTHENTICATE_METHOD_PLAIN;
		break;
	case CAuthenticator::eSSL:
		set_value = IDM_AUTHENTICATE_METHOD_SSL;
		break;
	case CAuthenticator::ePlugin:
		set_name = account->GetAuthenticator().GetDescriptor();
		break;
	}

	// Remove any existing plugin items from main menu
	CMenu* pPopup = mAuthPopup.GetPopupMenu();
	short num_remove = pPopup->GetMenuItemCount() - 4;
	for(short i = 0; i < num_remove; i++)
		pPopup->RemoveMenu(1, MF_BYPOSITION);

	cdstrvect plugin_names;
	CPluginManager::sPluginManager.GetAuthPlugins(plugin_names);
	::sort(plugin_names.begin(), plugin_names.end());
	short index = 1;
	short cmd = IDM_AUTHENTICATE_METHOD_SSL + 1;
	for(cdstrvect::const_iterator iter = plugin_names.begin(); iter != plugin_names.end(); iter++, index++, cmd++)
	{
		CUnicodeUtils::InsertMenuUTF8(pPopup, index, MF_BYPOSITION, cmd, *iter);		
	
		if (*iter == set_name)
			set_value = cmd;
	}

	// Set value
	mAuthPopup.SetValue(set_value);

	cdstring choice = CUnicodeUtils::GetMenuStringUTF8(pPopup, mAuthPopup.GetValue(), MF_BYCOMMAND);
	SetAuthPanel(choice);
}

void CPrefsAccountAuth::InitTLSItems(CINETAccount* account)
{
	// Enable each item based on what the protocol supports
	CMenu* pPopup = mTLSPopup.GetPopupMenu();
	bool enabled = false;
	for(int i = CINETAccount::eNoTLS; i <= CINETAccount::eTLSTypeMax; i++)
	{
		if (account->SupportsTLSType((CINETAccount::ETLSType) i))
		{
			pPopup->EnableMenuItem(i + IDM_TLSPOPUP_NOSECURITY, MF_ENABLED);
			enabled = true;
		}
		else
			pPopup->EnableMenuItem(i + IDM_TLSPOPUP_NOSECURITY, MF_GRAYED);
	}
	
	// Hide if no plugin present or none enabled
	if (enabled && CPluginManager::sPluginManager.HasSSL())
	{
		GetDlgItem(IDC_PREFS_AUTHENTICATE_TLSGROUP)->ShowWindow(SW_SHOW);
		mTLSPopup.ShowWindow(SW_SHOW);
		mTLSUseCert.ShowWindow(SW_SHOW);
		mTLSCert.ShowWindow(SW_SHOW);
	}
	else
	{
		GetDlgItem(IDC_PREFS_AUTHENTICATE_TLSGROUP)->ShowWindow(SW_HIDE);
		mTLSPopup.ShowWindow(SW_HIDE);
		mTLSUseCert.ShowWindow(SW_HIDE);
		mTLSCert.ShowWindow(SW_HIDE);

		// Disable the auth item
		mAuthPopup.GetPopupMenu()->EnableMenuItem(IDM_AUTHENTICATE_METHOD_SSL, MF_GRAYED | MF_BYCOMMAND);
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
		CMenu* pPopup = mTLSCert.GetPopupMenu();
		short num_remove = pPopup->GetMenuItemCount();
		for(short i = 0; i < num_remove; i++)
			pPopup->RemoveMenu(0, MF_BYPOSITION);

		UINT index = IDM_AUTHENTICATE_TLSCERT_Start;
		for(cdstrvect::const_iterator iter = mCertSubjects.begin(); iter != mCertSubjects.end(); iter++, index++)
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, index, *iter);		
	}	
}

void CPrefsAccountAuth::TLSItemsState()
{
	// TLS popup
	if (mTLSPopup.GetValue() - IDM_TLSPOPUP_NOSECURITY == CINETAccount::eNoTLS)
	{
		mTLSUseCert.EnableWindow(false);
		mTLSCert.EnableWindow(false);
		if (mAuthPopup.GetValue() == IDM_AUTHENTICATE_METHOD_SSL)
			OnAuthPopup(IDM_AUTHENTICATE_METHOD_PLAIN);
		mAuthPopup.GetPopupMenu()->EnableMenuItem(IDM_AUTHENTICATE_METHOD_SSL, MF_GRAYED | MF_BYCOMMAND);
	}
	else
	{
		mTLSUseCert.EnableWindow(true);

		if (mTLSUseCert.GetCheck())
		{
			mTLSCert.EnableWindow(true);
			mAuthPopup.GetPopupMenu()->EnableMenuItem(IDM_AUTHENTICATE_METHOD_SSL, MF_BYCOMMAND);
		}
		else
		{
			mTLSCert.EnableWindow(false);
			if (mAuthPopup.GetValue() == IDM_AUTHENTICATE_METHOD_SSL)
				OnAuthPopup(IDM_AUTHENTICATE_METHOD_PLAIN);
			mAuthPopup.GetPopupMenu()->EnableMenuItem(IDM_AUTHENTICATE_METHOD_SSL, MF_GRAYED | MF_BYCOMMAND);
		}
	}
}
