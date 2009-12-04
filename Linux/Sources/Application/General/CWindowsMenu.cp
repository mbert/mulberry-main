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

#include "CWindowsMenu.h"

#include "CMulberryApp.h"
#include "CWindow.h"

#include <JXTextMenu.h>
#include <JXWindow.h>
#include <algorithm>
// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// Statics
CWindowsMenu::CWindowList CWindowsMenu::sWindowList;
long CWindowsMenu::sNumServers = 0;
bool CWindowsMenu::sDirty = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

const JCharacter CWindowsMenu::AddWindowToMenu::shortcut_keys_[][7] =
{"Meta-1", "Meta-2", "Meta-3", "Meta-4", "Meta-5", "Meta-6", "Meta-7", 
 "Meta-8", "Meta-9", "Meta-0"};
	
//Not very useful to have the def'n of an inline function in the .cp file
//intead of the .h file, you say?  Well here it is ok, because the only place
//this method ever really gets called is in CWindowsMenu::UpdateMenu, so 
//as long as this can be seen when UpdateMenu is compiled, we're in business
void CWindowsMenu::AddWindowToMenu::operator()(JXWindowDirector* w)
{
	if (shortcut_ < 10)
		menu_->AppendItem(w->GetWindow()->GetTitle(), kFalse, kTrue, NULL, shortcut_keys_[shortcut_]);
	else
		menu_->AppendItem(w->GetWindow()->GetTitle(), kFalse, kTrue);

	if (w->GetWindow()->HasFocus())
		menu_->CheckItem(shortcut_ + CMainMenu::kWindowsDynamicStart);

	shortcut_++;
}

void CWindowsMenu::Init()
{
	sNumServers = 0;
}

// Get window corresponding to menu item
void CWindowsMenu::AddWindow(JXWindowDirector* theWindow, bool server)
{
	if (server)
		// Insert into array
		sWindowList.insert(sWindowList.begin() + sNumServers++, theWindow);
	else
		// Insert into array
		sWindowList.push_back(theWindow);
	
	// Set flag to force refresh
	sDirty = true;
}



// O T H E R  M E T H O D S ____________________________________________________________________________

void CWindowsMenu::RemoveWindow(JXWindowDirector* theWindow)
{
	CWindowList::iterator found = find(sWindowList.begin(), sWindowList.end(), theWindow);
	if ((found - sWindowList.begin()) + 1 <= sNumServers)
		sNumServers--;
	sWindowList.erase(found);
	
	// Set flag to force refresh
	sDirty = true;
}

void CWindowsMenu::RenamedWindow()
{ 
	sDirty = true;
}

void CWindowsMenu::UpdateMenu(JXTextMenu* menu)
{
	// Only do if changed
	if (sDirty)
	{
		// Remove any existing items
		short num_menu = menu->GetItemCount();
		for(short i = num_menu; i >= CMainMenu::kWindowsDynamicStart; i--)
			menu->RemoveItem(i);
		
		// Add windows to list
		for_each(sWindowList.begin(), sWindowList.end(), AddWindowToMenu(menu));
		
		sDirty = false;
	}
}

void CWindowsMenu::UpdateMenuItem(CCommander::CCmdUI* cmdui)
{
	if (cmdui->mMenuIndex < CMainMenu::kWindowsDynamicStart)
		return;

	// Always enabled
	cmdui->Enable(true);
	
	// Check if it has the focus
	cmdui->SetCheck(GetWindow(cmdui->mMenuIndex)->GetWindow()->HasFocus());
}

// Get window corresponding to menu item
JXWindowDirector* CWindowsMenu::GetWindow(SInt16 item_num)
{
	if (item_num < CMainMenu::kWindowsDynamicStart) {
		return NULL;
	}

	CWindowList::iterator i = sWindowList.begin();
	i += item_num - CMainMenu::kWindowsDynamicStart;
	return *i;
}
