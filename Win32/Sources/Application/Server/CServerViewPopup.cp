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
#include "CUnicodeUtils.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CServerViewPopup::CServerViewPopup()
{
	mDirty = true;
}

// Default destructor
CServerViewPopup::~CServerViewPopup()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

BOOL CServerViewPopup::Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nTitle,
							UINT nIDIcon, UINT nIDIconSel, UINT nIDIconPushed, UINT nIDIconPushedSel, bool frame)
{
	if (!CToolbarButton::Create(title, rect, pParentWnd, nID, nTitle, nIDIcon, nIDIconSel, nIDIconPushed, nIDIconPushedSel, frame))
		return false;
	
	SetMenu(IDR_POPUP_SERVER_VIEW);
	if (CMailAccountManager::sMailAccountManager != NULL)
	{
		SyncMenu();
		CMailAccountManager::sMailAccountManager->Add_Listener(this);
	}
	
	return true;
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
	// Remove any existing items from main menu
	short num_menu = GetPopupMenu()->GetMenuItemCount();
	for(short i = eServerView_First; i < num_menu; i++)
		GetPopupMenu()->RemoveMenu(eServerView_First, MF_BYPOSITION);
	
	int menu_id = IDM_ServerViewStart;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
			iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(GetPopupMenu(), MF_STRING, menu_id++, (*iter)->GetName());

	// Reset value & max & min
	mValue = 0;
	
	// Reset dirty state
	mDirty = false;

	// Always make sure it is listening
	CMailAccountManager::sMailAccountManager->Add_Listener(this);
}

void CServerViewPopup::SetupCurrentMenuItem(bool check)
{
	CMenu* pPopup = GetPopupMenu();
	
	// Reset menu items if dirty
	if (mDirty)
		SyncMenu();

	// Check current values
	short index = eServerView_First;
	for(CFavourites::const_iterator iter = CMailAccountManager::sMailAccountManager->GetFavourites().begin();
		iter != CMailAccountManager::sMailAccountManager->GetFavourites().end(); iter++)
		pPopup->CheckMenuItem(index++, ((check && (*iter)->IsVisible()) ? MF_CHECKED : MF_UNCHECKED) | MF_BYPOSITION);
}

