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


// Source for CServerViewPopup class

#include "CAdbkServerPopup.h"

#include "CAddressAccount.h"
#include "CMailAccountManager.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

const int cTickIcon = 1;
const int cNoTickIcon = 2;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CAdbkServerPopup::CAdbkServerPopup(LStream *inStream)
		: CPickPopup(inStream)
{
	mActive = false;
	mDirty = true;
	//CMailAccountManager::sMailAccountManager->AddListener(this);
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
void CAdbkServerPopup::SetValue(SInt32 inValue)
{
	CPickPopup::SetValue(inValue);

	// Force off so that any future selection will broadcast change
	mValue = 0;

	// Only if active - this prevents unwanted data changes while setting menu up
	if (!mActive)
		return;

	short pos = inValue - 1;

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
void CAdbkServerPopup::InitAccountMenu(CINETAccount::EINETServerType type)
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

Boolean CAdbkServerPopup::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always sync menu state when clicked
	SetupCurrentMenuItem(GetMacMenuH(), 0);
	
	// Do default
	return CPickPopup::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}


void CAdbkServerPopup::SetupCurrentMenuItem(MenuHandle inMenuH, SInt16 inCurrentItem)
{
	// ¥ If the current item has changed then make it so, this
	// also involves removing the mark from any old
	if ( inMenuH )
	{
		// Check dirty state and resync
		if (mDirty)
			SyncMenu();

		short index = 1;
		for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
		{
			if ((*iter)->GetServerType() == mType)
				::SetItemIcon(inMenuH, index++, (*iter)->GetLDAPSearch() ? cTickIcon : cNoTickIcon);
		}
	}
}

void CAdbkServerPopup::SyncMenu(void)
{
	// Delete Previous items
	MenuHandle menuH = GetMacMenuH();
	short menu_pos = eServerPopup;

	for(short i = ::CountMenuItems(menuH); i >= eServerPopup; i--)
		::DeleteMenuItem(menuH, i);

	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		// Insert Item
		if((*iter)->GetServerType() == mType){
			::AppendItemToMenu(GetMacMenuH(), menu_pos, (*iter)->GetName());
			menu_pos++;
		}
	}

	// Make it inactive when changing max/min to prevent unwanted value changes
	mActive = false;
	SetMenuMinMax();
	mActive = true;

	// Reset dirty state
	mDirty = false;
}

cdstrvect CAdbkServerPopup::GetServers()
{
	cdstrvect servers;

	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		// Insert Item
		if(((*iter)->GetServerType() == mType) && (*iter)->GetLDAPSearch()){
			servers.push_back((*iter)->GetName());// record the name;
		}
	}

	return servers;
}

