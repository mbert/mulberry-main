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

#include "CWindow.h"
#include "CMulberryApp.h"

#include <JXTextMenu.h>
#include "CWindowsMenu.h"

#include "CCommands.h"

// Make sure commands route through application
CWindow::CWindow(JXDirector* owner)
		: CCommander(CMulberryApp::sApp), JXWindowDirector(owner)
{
}

JBoolean CWindow::Close()
{
	if (AttemptClose())
		return JXWindowDirector::Close();

	return kFalse;
}

bool CWindow::AttemptClose()
{
	return true;
}

void CWindow::Receive(JBroadcaster* sender, const Message& message)
{
	// Look for dynamic Windows menu update and redo dynamic items
	if (message.Is(JXMenu::kNeedsUpdate))
	{
		JXTextMenu* menu = dynamic_cast<JXTextMenu*>(sender);
		if (GetMainMenuID(menu) == CMainMenu::eWindows)
			CWindowsMenu::UpdateMenu(menu);
	}

	// Route to commander first
	if (ReceiveMenu(sender, message))
		return;

	JXWindowDirector::Receive(sender, message);
}

bool CWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eFileClose:
		Close();
		return true;
	default:;
	}

	if (menu)
	{
		// Special check for import/export
		if ((GetMainMenuID(menu->mMenu) == CMainMenu::eImportAddresses) ||
			(GetMainMenuID(menu->mMenu) == CMainMenu::eExportAddresses))
		{
			CMulberryApp::sApp->OnAppAddrImportExport();
			return true;
		}

		else if ((GetMainMenuID(menu->mMenu) == CMainMenu::eWindows) &&
					(menu->mIndex >= CMainMenu::kWindowsDynamicStart))
		{
			CMulberryApp::sApp->OnAppSelectWindow(menu->mIndex);
			return true;
		}
	}

	// Call inherited
	return CCommander::ObeyCommand(cmd, menu);
}

void CWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eFileClose:
	case CCommand::eAddressesImport:
	case CCommand::eAddressesExport:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;
	default:;
	}

	if (cmdui->mMenu)
	{
		// Special check for import/export
		if ((GetMainMenuID(cmdui->mMenu) == CMainMenu::eImportAddresses) ||
			(GetMainMenuID(cmdui->mMenu) == CMainMenu::eExportAddresses))
		{
			// Always enabled
			OnUpdateAlways(cmdui);
			return;
		}

		else if ((GetMainMenuID(cmdui->mMenu) == CMainMenu::eWindows) &&
					(cmdui->mMenuIndex >= CMainMenu::kWindowsDynamicStart))
		{
			CWindowsMenu::UpdateMenuItem(cmdui);
			return;
		}
	}

	// Call inherited
	CCommander::UpdateCommand(cmd, cmdui);
}

void CWindow::CreateMainMenu(JXContainer* wnd, unsigned short which, bool listen)
{
	CMainMenu::sMMenu->CreateMenuBar(which, wnd, &mainMenus);
	if (listen)
	{
		for (int i = 0; i < CMainMenu::eNumMenus; i++)
		{
			if (mainMenus[i]) 
				ListenTo(mainMenus[i]);
		}		
	}
}

CMainMenu::MenuId CWindow::GetMainMenuID(JXTextMenu* menu) const
{
	unsigned long pos;
	for (pos = 0; pos < CMainMenu::eNumMenus; pos++)
	{
		if (menu == mainMenus[pos])
			break;
	}

	return static_cast<CMainMenu::MenuId>(pos);
}
