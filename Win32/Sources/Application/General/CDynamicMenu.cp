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


// Source for CDynamicMenu class


#include "CDynamicMenu.h"

#include "CApplyRulesMenu.h"
#include "CCopyToMenu.h"
#include "CFontMenu.h"
#include "CMulberryApp.h"
#include "CPluginManager.h"
#include "CSpeechSynthesis.h"

void CDynamicMenu::CreateEditMenu(CMenu* menu, bool add_shared)
{
	// Check for spelling and remove spell menu items
	if (!CPluginManager::sPluginManager.HasSpelling())
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_EDIT_SPELLCHECK)
			{
				menu->DeleteMenu(i, MF_BYPOSITION);
				break;
			}
		}
	}

	// Check for speech and remove speech menu items
	if (!CSpeechSynthesis::Available())
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_EDIT_SPEAK)
			{
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->DeleteMenu(i - 1, MF_BYPOSITION);
				break;
			}
		}
	}
}

void CDynamicMenu::DestroyEditMenu(CMenu* menu)
{
	// Nothing to do
}

void CDynamicMenu::CreateMailboxMenu(CMenu* menu, bool add_shared)
{
	if (add_shared)
	{
		for(int i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_MAILBOX_RULES)
			{
				i++;

				// Add shared menu
				CString old_text;
				menu->GetMenuString(i, old_text, MF_BYPOSITION);
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->InsertMenu(i, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT) CApplyRulesMenu::GetMenu()->m_hMenu, old_text);

				break;
			}
		}
	}
}

void CDynamicMenu::DestroyMailboxMenu(CMenu* menu)
{
	for(int i = 0; i < menu->GetMenuItemCount(); i++)
	{
		if (menu->GetMenuItemID(i) == IDM_MAILBOX_RULES)
		{
			menu->RemoveMenu(i + 1, MF_BYPOSITION);
			break;
		}
	}
}

void CDynamicMenu::CreateMessagesMenu(CMenu* menu, bool add_shared)
{
	// Check for security and remove security menu items
	if (!CPluginManager::sPluginManager.HasSecurity())
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_MESSAGES_VERIFY_DECRYPT)
			{
				menu->DeleteMenu(i, MF_BYPOSITION);
				break;
			}
		}
	}

	if (add_shared)
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_MESSAGES_COPY_NOW)
			{
				i--;

				// Add shared menu
				CString old_text;
				menu->GetMenuString(i, old_text, MF_BYPOSITION);
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->InsertMenu(i, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT) CCopyToMenu::GetMenu(true)->m_hMenu, old_text);

				break;
			}
		}
	}
}

void CDynamicMenu::DestroyMessagesMenu(CMenu* menu)
{
	for(int i = 0; i < menu->GetMenuItemCount(); i++)
	{
		if (menu->GetMenuItemID(i) == IDM_MESSAGES_COPY_NOW)
		{
			menu->RemoveMenu(i - 1, MF_BYPOSITION);
			break;
		}
	}
}

const unsigned long cFontOffset = 5;

void CDynamicMenu::CreateDraftMenu(CMenu* menu, bool add_shared)
{
	// Check for security and remove security menu items
	if (!CPluginManager::sPluginManager.HasSecurity())
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_DRAFT_SIGN)
			{
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->DeleteMenu(i - 1, MF_BYPOSITION);
				break;
			}
		}
	}

	if (add_shared)
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_DRAFT_APPEND_NOW)
			{
				i--;

				// Add shared menu Append To
				CString old_text;
				menu->GetMenuString(i, old_text, MF_BYPOSITION);
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->InsertMenu(i, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT) CCopyToMenu::GetMenu(false)->m_hMenu, old_text);
				break;
			}
		}
	
		// Add shared menu Fonts
		int font_pos = menu->GetMenuItemCount() - cFontOffset;
		CString old_text;
		menu->GetMenuString(font_pos, old_text, MF_BYPOSITION);
		menu->DeleteMenu(font_pos, MF_BYPOSITION);
		menu->InsertMenu(font_pos, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT) CFontPopup::GetMainMenu()->m_hMenu, old_text);
	}
}

void CDynamicMenu::DestroyDraftMenu(CMenu* menu)
{
	for(int i = 0; i < menu->GetMenuItemCount(); i++)
	{
		if (menu->GetMenuItemID(i) == IDM_DRAFT_APPEND_NOW)
		{
			menu->RemoveMenu(i - 1, MF_BYPOSITION);
			break;
		}
	}

	int font_pos = menu->GetMenuItemCount() - cFontOffset;
	menu->RemoveMenu(font_pos, MF_BYPOSITION);		// NB -1 because item above removed!
}

void CDynamicMenu::CreateHelpMenu(CMenu* menu, bool add_shared)
{
	// Check for spelling and remove spell menu items
	if (!CMulberryApp::sApp->IsDemo())
	{
		for(UINT i = 0; i < menu->GetMenuItemCount(); i++)
		{
			if (menu->GetMenuItemID(i) == IDM_HELP_BUY_MULBERRY)
			{
				menu->DeleteMenu(i, MF_BYPOSITION);
				menu->DeleteMenu(i - 1, MF_BYPOSITION);
				break;
			}
		}
	}
}

void CDynamicMenu::DestroyHelpMenu(CMenu* menu)
{
	// Nothing to do
}

// Get main menu copy to submenu
CMenu* CDynamicMenu::GetCopytoSubMenu()
{
	// Get main menu bar menus
	CMenu* menu = CMulberryApp::sApp->m_pMainWnd->GetMenu();
	CMenu* msgs_menu = menu->GetSubMenu(3);

	for(int i = 0; i < msgs_menu->GetMenuItemCount(); i++)
	{
		if (msgs_menu->GetMenuItemID(i) == IDM_MESSAGES_COPY_NOW)
			return msgs_menu->GetSubMenu(i - 1);
	}

	return NULL;
}

// Get main menu append to submenu
CMenu* CDynamicMenu::GetAppendToSubMenu()
{
	// Get main menu bar menus
	CMenu* menu = CMulberryApp::sApp->m_pMainWnd->GetMenu();
	CMenu* draft_menu = menu->GetSubMenu(4);

	for(int i = 0; i < draft_menu->GetMenuItemCount(); i++)
	{
		if (draft_menu->GetMenuItemID(i) == IDM_DRAFT_APPEND_NOW)
			return draft_menu->GetSubMenu(i - 1);
	}

	return NULL;
}

