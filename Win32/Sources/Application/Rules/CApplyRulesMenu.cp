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


// Source for CWindowsMenu class

#include "CApplyRulesMenu.h"

#include "CFilterManager.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CUnicodeUtils.h"

// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// Statics
CApplyRulesMenu* CApplyRulesMenu::sApplyRules = NULL;

CMenu CApplyRulesMenu::sApplyRulesMenu;
bool CApplyRulesMenu::sApplyRulesReset = true;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CApplyRulesMenu::CApplyRulesMenu()
{
	// Get main menu bar menus
	CMenu* menu = CMulberryApp::sApp->m_pMainWnd->GetMenu();
	CMenu* mailbox_menu = menu->GetSubMenu(2);
	for(int i = 0; i < mailbox_menu->GetMenuItemCount(); i++)
	{
		if (mailbox_menu->GetMenuItemID(i) == IDM_MAILBOX_RULES)
		{
			CMenu* apply_menu = mailbox_menu->GetSubMenu(i + 1);
			sApplyRulesMenu.Attach(*apply_menu);
			break;
		}
	}

	// Listen to filter manager
	CPreferences::sPrefs->GetFilterManager()->Add_Listener(this);
}

// Default destructor
CApplyRulesMenu::~CApplyRulesMenu()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Respond to list changes
void CApplyRulesMenu::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CFilterManager::eBroadcast_RulesChanged:
		sApplyRulesReset = true;
		break;
	default:
		CListener::ListenTo_Message(msg, param);
		break;
	}
}

// Set the menu items from the various lists
void CApplyRulesMenu::ResetMenuList(void)
{
	// Only do if required
	if (sApplyRulesReset)
		Reset(&sApplyRulesMenu);
	sApplyRulesReset = false;
}

// Set the menu items from the various lists
void CApplyRulesMenu::Reset(CMenu* main_menu)
{
	// Remove any existing items from main menu
	int num_menu = main_menu->GetMenuItemCount();
	for(int i = eApply_First; i < num_menu; i++)
		main_menu->RemoveMenu(eApply_First, MF_BYPOSITION);
	
	// Get all manual filters
	cdstrvect items;
	CPreferences::sPrefs->GetFilterManager()->GetManualFilters(items);
	
	// Add each one to menu
	int pos = 0;
	int append_pos = 0;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
		CUnicodeUtils::AppendMenuUTF8(main_menu, MF_STRING | ::AppendMenuFlags(append_pos, false), pos++ + IDM_APPLY_RULESStart, *iter);
}
