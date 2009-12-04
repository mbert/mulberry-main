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

#include "CCommands.h"
#include "CMulberryCommon.h"
#include "CResources.h"

#include "MyCFString.h"

// Statics
LMenu* CWindowsMenu::sMenu = nil;
LArray* CWindowsMenu::sWindowList = nil;
unsigned long CWindowsMenu::sNumServers = 0;
bool CWindowsMenu::sDirty = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CWindowsMenu::CWindowsMenu()
{
	sMenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_Windows);
	ThrowIfNil_(sMenu);

	sWindowList = new LArray(sizeof(LWindow*));

	sNumServers = 0;
	sDirty = false;
}

// Default destructor
CWindowsMenu::~CWindowsMenu()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Get window corresponding to menu item
void CWindowsMenu::AddWindow(LWindow* theWindow, bool server)
{
	if (server)
		// Insert into array
		sWindowList->InsertItemsAt(1, sNumServers++, &theWindow);
	else
		// Insert into array
		sWindowList->InsertItemsAt(1, LArray::index_Last, &theWindow);
	
	// Mark as dirty to force refresh
	sDirty = true;
	LCommander::SetUpdateCommandStatus(true);
}

// Get window corresponding to menu item
void CWindowsMenu::RemoveWindow(LWindow* theWindow)
{
	// Check for server
	if (sWindowList->FetchIndexOf(&theWindow) <= sNumServers)
		sNumServers--;

	// Remove from array
	sWindowList->Remove(&theWindow);
	
	// Mark as dirty to force refresh
	sDirty = true;
	LCommander::SetUpdateCommandStatus(true);
}

void CWindowsMenu::RenamedWindow()
{ 
	sDirty = true;
	LCommander::SetUpdateCommandStatus(true);
}

// Get window corresponding to menu item
void CWindowsMenu::UpdateMenuList()
{
	// Only update if dirty
	if (!sDirty)
		return;

	SInt16 status_index = sMenu->IndexFromCommand(cmd_Status);

	short num_menu = ::CountMenuItems(sMenu->GetMacMenuH());
	short old_count = num_menu - status_index;
	short new_count = sWindowList->GetCount();
	if (old_count > new_count)
	{
		// Remove any existing items
		for(short i = old_count; i > new_count; i--)
			::DeleteMenuItem(sMenu->GetMacMenuH(), i + status_index);
	}
	else if (old_count < new_count)
	{
		for(short i = old_count + 1; i <= new_count; i++)
		{
			Str255 item = "\p ";
			::AppendMenu(sMenu->GetMacMenuH(), item);
		}
	}

	// Add windows to list
	LArrayIterator iterator(*sWindowList, LArrayIterator::from_Start);
	LWindow* aWindow;
	short menuPos = status_index + 1;

	while(iterator.Next(&aWindow))
	{
		char cmd[256];
		*cmd = 0;
		Str255 item = "\p ";
		if (menuPos - status_index - 1 < 10)
			::snprintf(cmd, 256, "%d", menuPos - status_index - 1);

		MyCFString name(aWindow->CopyCFDescriptor());
		cdstring title = name.GetString();
		if (!aWindow->IsVisible())
		{
			cdstring temp = "(";
			temp += title;
			temp += ")";
			title = temp;
		}
		::SetMenuItemTextUTF8(sMenu->GetMacMenuH(), menuPos, title);

		if (*cmd)
			::SetItemCmd(sMenu->GetMacMenuH(), menuPos, *cmd);
		::EnableItem(sMenu->GetMacMenuH(), menuPos);
		menuPos++;
	}
	
	// Remove dirty indicator since menu now up to date
	sDirty = false;
}

// Get window corresponding to menu item
LWindow* CWindowsMenu::GetWindow(SInt16 menu_num, SInt16 item_num)
{
	LWindow* theWindow = nil;

	SInt16 dynamic_index = sMenu->IndexFromCommand(cmd_Status) + 1;

	sWindowList->FetchItemAt(item_num - dynamic_index + 1, &theWindow);

	return theWindow;
}

