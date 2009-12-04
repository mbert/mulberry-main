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

// C O N S T R U C T I O N / D E S T R U C T I O N	M E T H O D S

// Constructor from stream
CAdbkServerPopup::CAdbkServerPopup
(
 const JCharacter*	title,
 JXContainer*		enclosure,
 const HSizingOption	hSizing,
 const VSizingOption	vSizing,
 const JCoordinate	x,
 const JCoordinate	y,
 const JCoordinate	w,
 const JCoordinate	h
 ) : HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h)
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
}

// Set size
void CAdbkServerPopup::SetValue(JIndex value)
{
	if (!value)
		return;

	// Force off so that any future selection will broadcast change
	mValue = 0;
			
	short pos = value - 1;
	
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


void CAdbkServerPopup::SetupCurrentMenuItem(bool check)
{
	// Make sure everything is enabled
	EnableAll();

	if (!check)
		return;

	// Check dirty state and resync
	if (mDirty)
		SyncMenu();

	short index = 1;
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
		iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		if ((*iter)->GetServerType() == mType)
		{
			if ((*iter)->GetLDAPSearch())
      			CheckItem(index);
      		index++;
      	}
	}
}

void CAdbkServerPopup::Receive(JBroadcaster* sender, const Message& message)
{
  if (sender == this && message.Is(JXMenu::kNeedsUpdate)) {
    SetupCurrentMenuItem(true); //What param to set?
  } else {
    HPopupMenu::Receive(sender, message);
  }
  
}

void CAdbkServerPopup::AdjustPopupChoiceTitle (const JIndex index)
{
  //we don't want to do the title thing here, so override this
  //to do nothing

  SetValue(index);
  return;
}

void CAdbkServerPopup::SyncMenu(void)
{
	// Delete Previous items
	RemoveAllItems();
		
	for(CAddressAccountList::const_iterator iter = CPreferences::sPrefs->mAddressAccounts.GetValue().begin();
			iter != CPreferences::sPrefs->mAddressAccounts.GetValue().end(); iter++)
	{
		// Insert Item
		if((*iter)->GetServerType() == mType)
		    AppendItem((*iter)->GetName(), kTrue, kFalse);
	}
	
	// Reset dirty state
	mDirty = false;

	SetToPopupChoice(kTrue, 0);
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

