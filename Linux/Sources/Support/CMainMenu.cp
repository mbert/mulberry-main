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

#include "CMainMenu.h"

#include "CAdminLock.h"
#include "CMenuBar.h"
#include "CCommands.h"
#include "CCopyToMenu.h"
#include "CFontNameMenu.h"
#include "CFontSizeMenu.h"
#include "CMenu.h"
#include "CPluginManager.h"

#include <JXTextMenu.h>
#include <jXActions.h>

struct SMenuBuilder
{
	const char* item;
	const char* cmd;
};

#include "CMainMenuBuild.cp"
#include "CMainMenuContext.cp"

CMainMenu* CMainMenu::sMMenu = new CMainMenu;

JXTextMenu* CMainMenu::BuildMenu(JXMenuBar* menu_bar, const char* title, const SMenuBuilder* items)
{
	const JRect bounds = menu_bar->GetBounds();
	JXTextMenu* menu = new CMenu(title, menu_bar, JXWidget::kFixedLeft, JXWidget::kVElastic, bounds.left, bounds.top, 10, bounds.height());
	menu_bar->AppendMenu(menu);
	menu->SetTitleFontSize(10);
	const SMenuBuilder* p = items;
	while(p->item)
	{
		menu->AppendMenuItems(p->item);
		if (p->cmd)
			menu->SetItemID(menu->GetItemCount(), p->cmd);
		p++;
	}
	
	return menu;
}

void CMainMenu::BuildMenu(JXTextMenu* menu, const SMenuBuilder* items)
{
	const SMenuBuilder* p = items;
	while(p->item)
	{
		menu->AppendMenuItems(p->item);
		if (p->cmd)
			menu->SetItemID(menu->GetItemCount(), p->cmd);
		p++;
	}
}

JXMenuBar* CMainMenu::CreateMenuBar(unsigned short which,
													 JXContainer* enclosure,
													 MenusArray* arrayToFill)
{
	CMenuBar* pMenu = new CMenuBar(enclosure,
									 JXWidget::kHElastic,
									 JXWidget::kFixedTop,
									 0, 0, 10, CMenuBar::cMenuBarHeight);
	pMenu->FitToEnclosure(kTrue, kFalse);
	JXTextMenu* temp, *sub;
	if (which & fFile)
	{
		temp = BuildMenu(pMenu, "File", cFileMenuBuilder);
		//temp->SetShortcuts("@F");
		(*arrayToFill)[eFile] = temp;
	}
	else
		(*arrayToFill)[eFile] = NULL;

	if (which & fEdit)
	{
		temp = BuildMenu(pMenu, "Edit", cEditMenuBuilder);
		//temp->SetShortcuts("@E");
		(*arrayToFill)[eEdit] = temp;

		// Check for spelling and remove spell menu items
		if (!CPluginManager::sPluginManager.HasSpelling())
		{
			JIndex index;
			if (temp->ItemIDToIndex(CCommand::cEditCheckSpelling, &index))
				temp->RemoveItem(index);
			if (temp->ItemIDToIndex(CCommand::cEditChangePassword, &index))
				temp->ShowSeparatorAfter(index, kFalse);
		}
	}
	else
	{
		(*arrayToFill)[eEdit] = NULL;
	}

	if (which & fMailboxes)
	{
		temp = BuildMenu(pMenu, "Mailbox", cMailboxMenuBuilder);
		//temp->SetShortcuts("@M");
		(*arrayToFill)[eMailboxes] = temp;

		sub = new CMenu(temp, kMailboxDisplayHierarchy, pMenu);
		BuildMenu(sub, cDisplayHierarchyMenuBuilder);
		(*arrayToFill)[eDisplayHierarchy] = sub;

		sub = new CMenu(temp, kMailboxCabinets, pMenu);
		BuildMenu(sub, cCabinetsMenuBuilder);
		(*arrayToFill)[eCabinets] = sub;

		sub = new CMenu(temp, kMailboxSortBy, pMenu);
		BuildMenu(sub, cSortByMenuBuilder);
		(*arrayToFill)[eSortBy] = sub;		

		sub = new CMenu(temp, kMailboxApplyRules, pMenu);
		BuildMenu(sub, cApplyRulesMenuBuilder);
		(*arrayToFill)[eApplyRules] = sub;		
	}
	else
	{
		(*arrayToFill)[eMailboxes] = NULL;
		(*arrayToFill)[eDisplayHierarchy] = NULL;
		(*arrayToFill)[eCabinets] = NULL;
		(*arrayToFill)[eSortBy] = NULL;
		(*arrayToFill)[eApplyRules] = NULL;
	}

	if (which & fMessages)
	{
		temp = BuildMenu(pMenu, "Message", cMessagesMenuBuilder);
		//temp->SetShortcuts("@s");
		(*arrayToFill)[eMessages] = temp;

		sub = new CCopyToMenu::CCopyToMain(true, temp, kMessagesCopyTo, pMenu);
		BuildMenu(sub, cCopyToMenuBuilder);
		(*arrayToFill)[eCopyTo] = sub;

		sub = new CMenu(temp, kMessagesFlags, pMenu);
		BuildMenu(sub, cFlagsMenuBuilder);
		(*arrayToFill)[eFlags] = sub;

		sub = new CMenu(temp, kMessagesDecodeEmbedded, pMenu);
		BuildMenu(sub, cDecodeMenuBuilder);
		(*arrayToFill)[eDecode] = sub;

		// Check for security and remove security menu items
		if (!CPluginManager::sPluginManager.HasSecurity())
		{
			JIndex index;
			if (temp->ItemIDToIndex(CCommand::cMessagesVerifyDecrypt, &index))
				temp->RemoveItem(index);
			if (temp->ItemIDToIndex(CCommand::cMessagesDecodeEmbedded, &index))
				temp->ShowSeparatorAfter(index, kTrue);
		}
	}
	else
	{
		(*arrayToFill)[eMessages] = NULL;
		(*arrayToFill)[eFlags] = NULL;
		(*arrayToFill)[eDecode] = NULL;
		(*arrayToFill)[eCopyTo] = NULL;
	}

	if (which & fDraft)
	{
		temp = BuildMenu(pMenu, "Draft", cDraftMenuBuilder);
		//temp->SetShortcuts("@D");
		(*arrayToFill)[eDraft] = temp;

		sub = new CMenu(temp, kDraftNewTextPart, pMenu);
		BuildMenu(sub, cNewTextPartMenuBuilder);
		(*arrayToFill)[eNewTextPart] = sub;

		sub = new CMenu(temp, kDraftNewMultipart, pMenu);
		BuildMenu(sub, cNewMultipartMenuBuilder);
		(*arrayToFill)[eNewMultipart] = sub;

		sub = new CMenu(temp, kDraftCurrentTextPart, pMenu);
		BuildMenu(sub, cCurrentTextPartMenuBuilder);
		(*arrayToFill)[eCurrentTextPart] = sub;

		sub = new CCopyToMenu::CCopyToMain(false, temp, kDraftAppendTo, pMenu);
		BuildMenu(sub, cAppendToMenuBuilder);
		(*arrayToFill)[eAppendTo] = sub;

		sub = new CMenu(temp, kDraftNotifications, pMenu);
		BuildMenu(sub, cNotificationsMenuBuilder);
		(*arrayToFill)[eNotifications] = sub;

		CFontNameMenu* fnamemenu = new CFontNameMenu(temp, kDraftFont, pMenu);
		CFontSizeMenu* fsizemenu = new CFontSizeMenu(temp, kDraftSize, pMenu);
		fnamemenu->OnCreate();
		fnamemenu->BuildMenuNow(true);			// Quick screen font only build of menu
		fsizemenu->OnCreate(fnamemenu);
		(*arrayToFill)[eFont] = fnamemenu; 
		(*arrayToFill)[eSize] = fsizemenu;

		sub = new CMenu(temp, kDraftStyle, pMenu);
		BuildMenu(sub, cStyleMenuBuilder);
		(*arrayToFill)[eStyle] = sub; 

		sub = new CMenu(temp, kDraftAlignment, pMenu);
		BuildMenu(sub, cAlignmentMenuBuilder);
		(*arrayToFill)[eAlignment] = sub; 

		sub = new CMenu(temp, kDraftColour, pMenu);
		BuildMenu(sub, cColourMenuBuilder);
		(*arrayToFill)[eColour] = sub;

		// Check for security and remove security menu items
		if (!CPluginManager::sPluginManager.HasSecurity())
		{
			JIndex index;
			if (temp->ItemIDToIndex(CCommand::cDraftSign, &index))
				temp->RemoveItem(index);
			if (temp->ItemIDToIndex(CCommand::cDraftEncrypt, &index))
				temp->RemoveItem(index);
		}
	}
	else
	{
		(*arrayToFill)[eDraft] = NULL;
		(*arrayToFill)[eNewTextPart] = NULL;
		(*arrayToFill)[eNewMultipart] = NULL;
		(*arrayToFill)[eCurrentTextPart] = NULL;
		(*arrayToFill)[eAppendTo] = NULL;
		(*arrayToFill)[eNotifications] = NULL;
		(*arrayToFill)[eFont] = NULL; 
		(*arrayToFill)[eSize] = NULL; 
		(*arrayToFill)[eStyle] = NULL; 
		(*arrayToFill)[eAlignment] = NULL; 
		(*arrayToFill)[eColour] = NULL;
	}

	if (which & fAddresses)
	{
		temp = BuildMenu(pMenu, "Address", cAddressesMenuBuilder);
		//temp->SetShortcuts("@A");
		(*arrayToFill)[eAddresses] = temp;

		CMenu* importers = new CMenu(temp, kAddressesImport, pMenu);
		(*arrayToFill)[eImportAddresses] = importers;

		CMenu* exporters = new CMenu(temp, kAddressesExport, pMenu);
		(*arrayToFill)[eExportAddresses] = exporters;

		// Now get plugin manager to add the appropriate plugins
		CPluginManager::sPluginManager.SyncAdbkIOPluginMenu(importers, exporters);
	}
	else
	{
		(*arrayToFill)[eAddresses] = NULL;
		(*arrayToFill)[eImportAddresses] = NULL;
		(*arrayToFill)[eExportAddresses] = NULL;
	}

	// Don't allow this if admin locks it out
	if ((which & fCalendar) && !CAdminLock::sAdminLock.mPreventCalendars)
	{
		temp = BuildMenu(pMenu, "Calendar", cCalendarMenuBuilder);
		//temp->SetShortcuts("@A");
		(*arrayToFill)[eCalendar] = temp;
	}
	else
	{
		(*arrayToFill)[eCalendar] = NULL;
	}

	if (which & fWindows)
	{
		temp = BuildMenu(pMenu, "Window", cWindowsMenuBuilder);
		//temp->SetShortcuts("@W");
		(*arrayToFill)[eWindows] = temp;

		sub = new CMenu(temp, kWindowsToolbars, pMenu);
		BuildMenu(sub, cToolbarsMenuBuilder);
		(*arrayToFill)[eToolbar] = sub;

		sub = new CMenu(temp, kWindowsLayout, pMenu);
		BuildMenu(sub, cLayoutMenuBuilder);
		(*arrayToFill)[eLayout] = sub;
	}
	else
	{
		(*arrayToFill)[eWindows] = NULL;
		(*arrayToFill)[eToolbar] = NULL;
		(*arrayToFill)[eLayout] = NULL;
	}

	if (which & fHelp)
	{
		temp = BuildMenu(pMenu, "Help", cHelpMenuBuilder);
		//temp->SetShortcuts("@H");
		(*arrayToFill)[eHelp] = temp;
	}
	else
		(*arrayToFill)[eHelp] = NULL;

	return pMenu;
}

void CMainMenu::CreateContext(EContextMenu context, JXContainer* enclosure, JPtrArray<JXMenu>& menus)
{
	JXTextMenu* main = new CMenu("", enclosure, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
	JXTextMenu* sub = NULL;
	JIndex subitem;
	main->Hide();
	main->SetToHiddenPopupMenu(kTrue);
	main->SetUpdateAction(JXMenu::kDisableSingles);
	menus.Append(main);

	switch(context)
	{
	case eContextEdit:
		BuildMenu(main, cEditContext);
		break;
	case eContextServerTable:
		BuildMenu(main, cServerTableContext);
		CreateSubMenu(main, CCommand::cMailboxDisplayHierarchy, cDisplayHierarchyMenuBuilder, enclosure, menus);
		CreateSubMenu(main, CCommand::cMailboxCabinets, cCabinetsMenuBuilder, enclosure, menus);
		break;
	case eContextMailboxTable:
		BuildMenu(main, cMailboxTableContext);
		CreateSubMenu(main, CCommand::cMessagesFlags, cFlagsMenuNoShortcutBuilder, enclosure, menus);
		break;
	case eContextMessageAddr:
		BuildMenu(main, cMessageAddrContext);
		CreateSubMenu(main, CCommand::cMessagesFlags, cFlagsMenuNoShortcutBuilder, enclosure, menus);
		break;
	case eContextMessageSubj:
		BuildMenu(main, cMessageSubjContext);
		CreateSubMenu(main, CCommand::cMessagesFlags, cFlagsMenuNoShortcutBuilder, enclosure, menus);
		break;
	case eContextMessageParts:
		BuildMenu(main, cMessagePartsContext);
		break;
	case eContextMessageText:
		BuildMenu(main, cMessageTextContext);
		break;
	case eContextMessageTextURL:
		BuildMenu(main, cMessageTextURLContext);
		break;
	case eContextLetterAddr:
		BuildMenu(main, cLetterAddrContext);
		break;
	case eContextLetterParts:
		BuildMenu(main, cLetterPartsContext);
		CreateSubMenu(main, CCommand::cDraftNewTextPart, cNewTextPartMenuBuilder, enclosure, menus);
		CreateSubMenu(main, CCommand::cDraftNewMultipart, cNewMultipartMenuBuilder, enclosure, menus);
		break;
	case eContextLetterText:
		BuildMenu(main, cLetterTextContext);
		break;
	case eContextAdbkManager:
		BuildMenu(main, cAdbkManagerContext);
		break;
	case eContextAdbkSearch:
		BuildMenu(main, cAdbkSearchContext);
		break;
	case eContextAdbkTable:
		BuildMenu(main, cAdbkTableContext);
		break;
	case eContextAdbkGroup:
		BuildMenu(main, cGroupTableContext);
		break;
	case eContextCalendarStore:
		BuildMenu(main, cCalendarStoreContext);
		break;
	case eContextCalendarEventTable:
		BuildMenu(main, cEventTableContext);
		break;
	case eContextCalendarToDoTable:
		BuildMenu(main, cToDoTableContext);
		break;
	case eContextCalendarEventItem:
		BuildMenu(main, cEventItemContext);
		break;
	case eContextCalendarToDoItem:
		BuildMenu(main, cToDoItemContext);
		break;
	}
}

void CMainMenu::CreateSubMenu(JXTextMenu* parent, const char* cmd, const SMenuBuilder* items, JXContainer* enclosure, JPtrArray<JXMenu>& menus)
{
	JIndex index;
	assert (parent->ItemIDToIndex(cmd, &index) == kTrue);
	JXTextMenu* sub = new CMenu(parent, index, enclosure);

	menus.Append(sub);
	BuildMenu(sub, items);
}
