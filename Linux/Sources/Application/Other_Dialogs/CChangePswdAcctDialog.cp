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


// Source for CChangePswdAcctDialog class

#include "CChangePswdAcctDialog.h"

#include "CBalloonDialog.h"
#include "CErrorHandler.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CPrefsAccount.h"
#include "CPswdChangePlugin.h"

#include <LGAPopup.h>

// __________________________________________________________________________________________________
// C L A S S __ C U S E R P S W D D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CChangePswdAcctDialog::CChangePswdAcctDialog()
{
}

// Constructor from stream
CChangePswdAcctDialog::CChangePswdAcctDialog(LStream *inStream)
		: LGADialogBox(inStream)
{
}

// Default destructor
CChangePswdAcctDialog::~CChangePswdAcctDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CChangePswdAcctDialog::FinishCreateSelf(void)
{
	// Do inherited
	LGADialogBox::FinishCreateSelf();

	// Set up server name
	mAccountPopup = (LGAPopup*) FindPaneByID(paneid_ChangePswdAcctAcctPopup);
	mPluginPopup = (LGAPopup*) FindPaneByID(paneid_ChangePswdAcctPluginPopup);
	
	InitAccountMenu();
	InitPluginsMenu();
}

void CChangePswdAcctDialog::InitAccountMenu()
{
	// Delete previous items
	MenuHandle menuH = mAccountPopup->GetMacMenuH();
	for(short i = 1; i <= ::CountMenuItems(menuH); i++)
		::DeleteMenuItem(menuH, 1);

	// Add each mail account
	short menu_pos = 1;
	for(CMailAccountList::const_iterator iter = CPreferences::sPrefs->mMailAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mMailAccounts.GetValue().end(); iter++, menu_pos++)
	{
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_MailAccount);
		if (!(*iter)->GetAuthenticator().RequiresUserPswd())
			::DisableItem(menuH, menu_pos);
	}

	// Add each SMTP account
	for(CSMTPAccountList::const_iterator iter = CPreferences::sPrefs->mSMTPAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mSMTPAccounts.GetValue().end(); iter++, menu_pos++)
	{
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_SMTPAccount);
		if (!(*iter)->GetAuthenticator().RequiresUserPswd())
			::DisableItem(menuH, menu_pos);
	}

	// Add each remote account
	for(COptionsAccountList::const_iterator iter = CPreferences::sPrefs->mRemoteAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mRemoteAccounts.GetValue().end(); iter++, menu_pos++)
	{
		::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_RemoteAccount);
		if (!(*iter)->GetAuthenticator().RequiresUserPswd())
			::DisableItem(menuH, menu_pos);
	}

	// Add each address account
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++, menu_pos++)
	{
		// Insert item
		switch((*iter)->GetServerType())
		{
		case CINETAccount::eIMSP:
		case CINETAccount::eACAP:
			::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_AdbkAccount);
			break;
		case CINETAccount::eLDAP:
		case CINETAccount::eWHOISPP:
		case CINETAccount::eFinger:
			::AppendItemToMenu(menuH, menu_pos, (*iter)->GetName(), false, cicn_AddrSearchAccount);
			break;
		}
		if (!(*iter)->GetAuthenticator().RequiresUserPswd())
			::DisableItem(menuH, menu_pos);
	}

	// Force max/min update
	mAccountPopup->SetPopupMinMaxValues();
}

void CChangePswdAcctDialog::InitPluginsMenu()
{
	// Delete previous items
	MenuHandle menuH = mPluginPopup->GetMacMenuH();
	for(short i = 1; i <= ::CountMenuItems(menuH); i++)
		::DeleteMenuItem(menuH, 1);

	// Add each password changing plugin
	short menu_pos = 1;
	for(CPluginList::const_iterator iter = CPluginManager::sPluginManager.GetPlugins().begin();
			iter != CPluginManager::sPluginManager.GetPlugins().end(); iter++)
	{
		if ((*iter)->GetType() == CPlugin::ePluginPswdChange)
			::AppendItemToMenu(menuH, menu_pos++, static_cast<CPswdChangePlugin*>(*iter)->GetUIName());
	}

	// Force max/min update
	mPluginPopup->SetPopupMinMaxValues();
}

// Get details from dialog
void CChangePswdAcctDialog::GetDetails(CINETAccount*& acct, CPswdChangePlugin*& plugin)
{
	long acct_index = mAccountPopup->GetValue() - 1;
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
	
	Str255 currentItemTitle;
	mPluginPopup->GetCurrentItemTitle(currentItemTitle);
	cdstring pluginname = currentItemTitle;
	plugin = CPluginManager::sPluginManager.FindPswdChangePlugin(pluginname);
}

void CChangePswdAcctDialog::AcctPasswordChange()
{
	CINETAccount* acct = NULL;
	CPswdChangePlugin* plugin = NULL;
	
	if (PromptAcctPasswordChange(acct, plugin, CMulberryApp::sApp))
		plugin->DoPswdChange(acct);
}

bool CChangePswdAcctDialog::PromptAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin,
											LCommander* cmdr)
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
				return DoAcctPasswordChange(acct, plugin, cmdr);
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
				return DoAcctPasswordChange(acct, plugin, cmdr);
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
				return DoAcctPasswordChange(acct, plugin, cmdr);
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
				return DoAcctPasswordChange(acct, plugin, cmdr);
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
		return DoAcctPasswordChange(acct, plugin, cmdr);
	else
	{
		// Find the plugin
		for(CPluginList::const_iterator iter = CPluginManager::sPluginManager.GetPlugins().begin();
				iter != CPluginManager::sPluginManager.GetPlugins().end(); iter++)
		{
			if ((*iter)->GetType() == CPlugin::ePluginPswdChange)
			{
				plugin = static_cast<CPswdChangePlugin*>(*iter);
				break;
			}
		}
	}
	
	// Have a single account and a single plugin, just return these
	return true;
}

bool CChangePswdAcctDialog::DoAcctPasswordChange(CINETAccount*& acct, CPswdChangePlugin*& plugin,
											LCommander* cmdr)
{
	// Create the dialog
	CBalloonDialog	theHandler(paneid_ChangePswdAcctDialog, cmdr);
	theHandler.StartDialog();

	// Let DialogHandler process events
	while (true)
	{					
		MessageT hitMessage = theHandler.DoDialog();
		
		if (hitMessage == msg_OK)
		{
			((CChangePswdAcctDialog*) theHandler.GetDialog())->GetDetails(acct, plugin);
			return true;
		}
		else if (hitMessage == msg_Cancel)
		{
			return false;
		}
	}
}
