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

#include "CServerViewPopup.h"

#include "CMailAccountManager.h"
#include "CMboxProtocol.h"

#include <JXTextMenu.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CServerViewPopup::CServerViewPopup(const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: CToolbarPopupButton(label, enclosure, hSizing, vSizing, x,y, w,h)
{
	mDirty = true;
}

// Default destructor
CServerViewPopup::~CServerViewPopup()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CServerViewPopup::OnCreate()
{
	GetPopupMenu()->SetMenuItems("New... %l");

	if (CMailAccountManager::sMailAccountManager != NULL)
	{
		SyncMenu();
		CMailAccountManager::sMailAccountManager->Add_Listener(this);
	}
}

// Respond to list changes
void CServerViewPopup::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMboxProtocol::eBroadcast_NewList:
	case CTreeNodeList::eBroadcast_ResetList:
	case CMboxProtocol::eBroadcast_RemoveList:
		// Just mark as dirty
		mDirty = true;
		break;
	default:;
	}
}

void CServerViewPopup::SyncMenu(void)
{
  // Remove any existing items from main menu (except new and seperator)
	for (short i = GetPopupMenu()->GetItemCount(); i >= eServerView_First; i--)
		GetPopupMenu()->RemoveItem(i);
    
	int menu_id = IDM_ServerViewStart;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
		iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
	    GetPopupMenu()->AppendItem((*iter)->GetName(), kTrue, kFalse);

	// Reset dirty state
	mDirty = false;
}

void CServerViewPopup::UpdateMenu()
{
	// Make sure everything is enabled
	GetPopupMenu()->EnableAll();

	// Reset menu items if dirty
	if (mDirty)
	    SyncMenu();
  
	// Check current values
	short index = eServerView_First;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
		iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++, index++)
	{
		if ((*iter)->IsVisible())
			GetPopupMenu()->CheckItem(index);
	} 
}
