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


// Source for CAdbkServerPopup class

#include "CAdbkServerPopup.h"

#include "CAddressAccount.h"
#include "CMailAccountManager.h"
#include "CMboxProtocol.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAdbkServerPopup::CAdbkServerPopup()
{
	mDirty = true;
}

// Default destructor
CAdbkServerPopup::~CAdbkServerPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Respond to list changes
void CAdbkServerPopup::ListenTo_Message(long msg, void* param)
{
#if 0
	//For time being reset entire menu
	switch(msg)
	{
	case CMailAccountManager::eBroadcast_BeginMailAccountChange:
	case CMailAccountManager::eBroadcast_EndMailAccountChange:
	case CMailAccountManager::eBroadcast_NewMailAccount:
	case CMailAccountManager::eBroadcast_RemoveMailAccount:
		// Just mark as dirty
		mDirty = true;
		break;
	default:;
	}
#endif
}

// Set size
void CAdbkServerPopup::SetValue(UINT value)
{
	CPopupButton::SetValue(value);

	// Force off so that any future selection will broadcast change
	mValue = 0;
			
	short pos = value - IDM_ADBK_SERVERS_Start;
	
	CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
	while(pos || ((*iter)->GetServerType() != mType))
	{
		if ((*iter)->GetServerType() != mType)
			iter++;
		else
		{
			iter++;
			pos--;
		}
	}
	(*iter)->SetLDAPSearch(!(*iter)->GetLDAPSearch());
	CPreferences::sPrefs->mAddressAccounts.SetDirty();
}

// Set up account menus
void CAdbkServerPopup::InitServersMenu(CINETAccount::EINETServerType type)
{
	mType = type;
	mDirty = true;
	
	// Count items
	unsigned long ctr = 0;
	CAddressAccount* acct = NULL;
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
		iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetServerType() == mType)
		{
			if (!ctr)
				acct = *iter;
			ctr++;
		}
	}

	// If there is a single lookup make it searchable by default
	if ((ctr == 1) && acct)
		acct->SetLDAPSearch(true);
}

void CAdbkServerPopup::SyncMenu(void)
{
	// Remove any existing items from main menu
	short num_menu = GetPopupMenu()->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		GetPopupMenu()->RemoveMenu(0, MF_BYPOSITION);
	
	int menu_id = IDM_ADBK_SERVERS_Start;
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
		iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetServerType() == mType)
			CUnicodeUtils::AppendMenuUTF8(GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());
	}

	// Reset value & max & min
	mValue = 0;
	
	// Reset dirty state
	mDirty = false;
}

void CAdbkServerPopup::SetupCurrentMenuItem(bool check)
{
	CMenu* pPopup = GetPopupMenu();
	
	// Reset menu items if dirty
	if (mDirty)
		SyncMenu();

	// Check current values
	short index = 0;
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
		iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetServerType() == mType)
			pPopup->CheckMenuItem(index++, ((*iter)->GetLDAPSearch() ? MF_CHECKED : MF_UNCHECKED) | MF_BYPOSITION);
	}
}

cdstrvect CAdbkServerPopup::GetServers()
{
	cdstrvect servers;

	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		// Insert Item
		if(((*iter)->GetServerType() == mType) && (*iter)->GetLDAPSearch())
			servers.push_back((*iter)->GetName());
	}
	
	return servers;
}
