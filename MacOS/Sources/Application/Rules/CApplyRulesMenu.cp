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
#include "CMulberryCommon.h"
#include "CPreferences.h"

// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// Statics
CApplyRulesMenu* CApplyRulesMenu::sApplyRules = NULL;

LMenu* CApplyRulesMenu::sApplyRulesMenu = NULL;
bool CApplyRulesMenu::sApplyRulesReset = true;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CApplyRulesMenu::CApplyRulesMenu()
{
	sApplyRulesMenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_ApplyRules);
	ThrowIfNil_(sApplyRulesMenu);
	
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
		Reset(sApplyRulesMenu);
	sApplyRulesReset = false;
}

// Set the menu items from the various lists
void CApplyRulesMenu::Reset(LMenu* main_menu)
{
	// Remove any existing items from main menu
	MenuHandle menuH = main_menu->GetMacMenuH();
	short num_menu = ::CountMenuItems(menuH);
	for(short i = eApply_First; i <= num_menu; i++)
		::DeleteMenuItem(menuH, eApply_First);
	
	// Get all manual filters
	cdstrvect items;
	CPreferences::sPrefs->GetFilterManager()->GetManualFilters(items);
	
	// Add each one to menu
	short pos = eApply_First;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++)
		::AppendItemToMenu(menuH, pos++, *iter);
}
