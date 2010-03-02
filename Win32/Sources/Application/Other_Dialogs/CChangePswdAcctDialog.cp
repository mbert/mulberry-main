/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CChangePswdAcctDialog.cpp : implementation file
//

#include "CChangePswdAcctDialog.h"

#include "CErrorHandler.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPswdChangePlugin.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CChangePswdAcctDialog dialog


CChangePswdAcctDialog::CChangePswdAcctDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CChangePswdAcctDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CChangePswdAcctDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CChangePswdAcctDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChangePswdAcctDialog)
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChangePswdAcctDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CChangePswdAcctDialog)
	ON_COMMAND_RANGE(IDM_ACCOUNTStart, IDM_ACCOUNTEnd, OnChangePswdAcctAcctPopup)
	ON_COMMAND_RANGE(IDM_PswdChangeStart, IDM_PswdChangeStop, OnChanegPswdAcctPluginPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CChangePswdAcctDialog message handlers

BOOL CChangePswdAcctDialog::OnInitDialog()
{
	CHelpDialog::OnInitDialog();

	// Load menu bitmaps
	mMaiboxBMP.LoadBitmap(IDB_ACCOUNTMAILBOX);
	mSMTPBMP.LoadBitmap(IDB_ACCOUNTSMTP);
	mRemoteBMP.LoadBitmap(IDB_ACCOUNTREMOTE);
	mAdbkBMP.LoadBitmap(IDB_ACCOUNTADBK);
	mAddrSearchBMP.LoadBitmap(IDB_ACCOUNTADDRSEARCH);

	// Subclass buttons
	mAccountPopup.SubclassDlgItem(IDC_CHANGEPSWDACCT_ACCTPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAccountPopup.SetMenu(IDR_POPUP_MAILBOX_SEARCH_ACCOUNTS);
	InitAccountMenu();

	mPluginPopup.SubclassDlgItem(IDC_CHANEGPSWDACCT_PLUGINPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mPluginPopup.SetMenu(IDR_POPUP_PLUGINS);
	InitPluginsMenu();

	return true;
}

void CChangePswdAcctDialog::OnChangePswdAcctAcctPopup(UINT nID) 
{
	mAccountPopup.SetValue(nID);
}

void CChangePswdAcctDialog::OnChanegPswdAcctPluginPopup(UINT nID) 
{
	mPluginPopup.SetValue(nID);
	mPlugin = mPluginPopup.GetValueText();
}

void CChangePswdAcctDialog::InitAccountMenu(void) 
{
	CMenu* pPopup = mAccountPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add each mail account
	// Now add current items
	int menu_id = IDM_ACCOUNTStart;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		pPopup->SetMenuItemBitmaps(menu_id, MF_BYCOMMAND, &mMaiboxBMP, nil);
	}

	// Add each SMTP account
	for(CSMTPAccountList::const_iterator iter = CPreferences::sPrefs->mSMTPAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mSMTPAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		pPopup->SetMenuItemBitmaps(menu_id, MF_BYCOMMAND, &mSMTPBMP, nil);
	}

	// Add each remote account
	for(COptionsAccountList::const_iterator iter = CPreferences::sPrefs->mRemoteAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mRemoteAccounts.GetValue().end(); iter++, menu_id++)
	{
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		pPopup->SetMenuItemBitmaps(menu_id, MF_BYCOMMAND, &mRemoteBMP, nil);
	}

	// Add each address account
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++, menu_id++)
	{
		// Insert item
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, (*iter)->GetName());
		switch((*iter)->GetServerType())
		{
		case CINETAccount::eIMSP:
		case CINETAccount::eACAP:
			pPopup->SetMenuItemBitmaps(menu_id, MF_BYCOMMAND, &mAdbkBMP, nil);
			break;
		case CINETAccount::eLDAP:
		case CINETAccount::eWHOISPP:
		case CINETAccount::eFinger:
			pPopup->SetMenuItemBitmaps(menu_id, MF_BYCOMMAND, &mAddrSearchBMP, nil);
			break;
		}
	}
	
	// Refresh its display
	mAccountPopup.RefreshValue();
	mAccountPopup.SetValue(IDM_ACCOUNTStart);
}

void CChangePswdAcctDialog::InitPluginsMenu(void) 
{
	CMenu* pPopup = mPluginPopup.GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add each mail account
	// Now add current items
	int menu_id = IDM_PswdChangeStart;
	for(CPluginList::const_iterator iter = CPluginManager::sPluginManager.GetPlugins().begin();
			iter != CPluginManager::sPluginManager.GetPlugins().end(); iter++)
	{
		if (((*iter)->GetType() == CPlugin::ePluginPswdChange) &&
			static_cast<CPswdChangePlugin*>(*iter)->Enabled())
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, static_cast<CPswdChangePlugin*>(*iter)->GetUIName());
	}
	
	// Refresh its display
	mPluginPopup.RefreshValue();
	mPluginPopup.SetValue(IDM_PswdChangeStart);
	mPlugin = mPluginPopup.GetValueText();
}

// Get details from dialog
void CChangePswdAcctDialog::GetDetails(CINETAccount*& acct, CPswdChangePlugin*& plugin)
{
	long acct_index = mAccountPopup.GetValue() - IDM_ACCOUNTStart;
	acct = NULL;

	// Adjust index for actual account
	if (acct_index >= CPreferences::sPrefs->mMailAccounts.GetValue().size())
	{
		acct_index -= CPreferences::sPrefs->mMailAccounts.GetValue().size();
		if (acct_index >= CPreferences::sPrefs->mSMTPAccounts.GetValue().size())
		{
			acct_index -= CPreferences::sPrefs->mSMTPAccounts.GetValue().size();
			if (acct_index >= CPreferences::sPrefs->mRemoteAccounts.GetValue().size())
			{
				acct_index -= CPreferences::sPrefs->mRemoteAccounts.GetValue().size();
				acct = CPreferences::sPrefs->mAddressAccounts.GetValue().at(acct_index);
			}
			else
				acct = CPreferences::sPrefs->mRemoteAccounts.GetValue().at(acct_index);
		}
		else
			acct = CPreferences::sPrefs->mSMTPAccounts.GetValue().at(acct_index);
	}
	else
		acct = CPreferences::sPrefs->mMailAccounts.GetValue().at(acct_index);
	
	cdstring pluginname = mPlugin;
	plugin = CPluginManager::sPluginManager.FindPswdChangePlugin(pluginname);
}

void CChangePswdAcctDialog::AcctPasswordChange()
{
	CINETAccount* acct = NULL;
	CPswdChangePlugin* plugin = NULL;
	
	if (PromptAcctPasswordChange(acct, plugin))
		plugin->DoPswdChange(acct);
}

bool CChangePswdAcctDialog::PromptAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin)
{
	// First see if more than one account with password based authenticator
	acct = NULL;
	
	// Check each mail account
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			if (acct)
				return DoAcctPasswordChange(acct, plugin);
			else
				acct = *iter;
		}
	}

	// Check each SMTP account
	for(CSMTPAccountList::const_iterator iter = CPreferences::sPrefs->mSMTPAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mSMTPAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			if (acct)
				return DoAcctPasswordChange(acct, plugin);
			else
				acct = *iter;
		}
	}
	
	// Check each remote account
	for(COptionsAccountList::const_iterator iter = CPreferences::sPrefs->mRemoteAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mRemoteAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			if (acct)
				return DoAcctPasswordChange(acct, plugin);
			else
				acct = *iter;
		}
	}

	// Check each address account
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetAuthenticator().RequiresUserPswd())
		{
			if (acct)
				return DoAcctPasswordChange(acct, plugin);
			else
				acct = *iter;
		}
	}

	// If no account then there's nothing to change!
	if (!acct)
	{
		CErrorHandler::PutStopAlertRsrc("Alerts::UserPswd::ChangePasswordNone");
		return false;
	}

	// Check for multiple plugins
	if (CPluginManager::sPluginManager.CountPswdChange() > 1)
		return DoAcctPasswordChange(acct, plugin);
	else
	{
		// Find the plugin
		for(CPluginList::const_iterator iter = CPluginManager::sPluginManager.GetPlugins().begin();
				iter != CPluginManager::sPluginManager.GetPlugins().end(); iter++)
		{
			if (((*iter)->GetType() == CPlugin::ePluginPswdChange) &&
				static_cast<CPswdChangePlugin*>(*iter)->Enabled())
			{
				plugin = static_cast<CPswdChangePlugin*>(*iter);
				break;
			}
		}
	}
	
	// Have a single account and a single plugin, just return these
	return true;
}

bool CChangePswdAcctDialog::DoAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin)
{
	// Create the dialog
	CChangePswdAcctDialog dlog(CSDIFrame::GetAppTopWindow());

	// Do the dialog
	if (dlog.DoModal() == IDOK)
	{
		dlog.GetDetails(acct, plugin);
		return true;
	}
	else
		return false;
}
