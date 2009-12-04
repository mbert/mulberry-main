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

#include "CCopyToMenu.h"

#include "CBalloonDialog.h"
#include "CBrowseMailboxDialog.h"
#include "CIMAPCommon.h"
#include "CMailAccountManager.h"
#include "CMailboxPopup.h"
#include "CMailboxToolbarPopup.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMboxRefList.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CStringUtils.h"

#include "cdstring.h"

const short cMaxMailbox = 300;
const ResIDT cSubCopyToMenuFirst = 50;
const ResIDT cSubAppendToMenuFirst = 100;
const ResIDT cMaxSubMenu = 150;

const short cCopyToChoose = 1;
const short cPopupCopyNone = -1;
const short cPopupCopyChoose = 1;
const short cPopupCopySingleINBOX = 3;
const short cPopupAppendNone = 1;
const short cPopupAppendChoose = 2;
const short cPopupAppendSingleINBOX = 4;

// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// Statics
CCopyToMenu* CCopyToMenu::sMailboxMainMenu = NULL;

LMenu* CCopyToMenu::sCopyToMenu = NULL;
LMenu* CCopyToMenu::sAppendToMenu = NULL;
CCopyToMenu::CMenuList CCopyToMenu::sCopyToMenuList;
CCopyToMenu::CMenuList CCopyToMenu::sAppendToMenuList;
MenuHandle CCopyToMenu::sCopyToPopupMenu = NULL;
ulvector CCopyToMenu::sCopyToPopupServerBreaks;
MenuHandle CCopyToMenu::sAppendToPopupMenu = NULL;
ulvector CCopyToMenu::sAppendToPopupServerBreaks;
bool CCopyToMenu::sCopyToReset = true;
bool CCopyToMenu::sAppendToReset = true;
bool CCopyToMenu::sUseCopyToCabinet = false;
bool CCopyToMenu::sUseAppendToCabinet = false;
bool CCopyToMenu::sStripSubscribePrefix = false;
bool CCopyToMenu::sStripFavouritePrefix = false;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CCopyToMenu::CCopyToMenu()
{
	sCopyToMenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_CopyTo);
	ThrowIfNil_(sCopyToMenu);
	sAppendToMenu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_AppendTo);
	ThrowIfNil_(sAppendToMenu);

	// Create menu if not already
	sCopyToPopupMenu = ::GetMenu(MENU_CopyMailboxPopupID);
	ThrowIfNil_(sCopyToPopupMenu);
	::InsertMenu(sCopyToPopupMenu, hierMenu);
	sAppendToPopupMenu = ::GetMenu(MENU_AppendMailboxPopupID);
	ThrowIfNil_(sAppendToPopupMenu);
	::InsertMenu(sAppendToPopupMenu, hierMenu);
}

// Default destructor
CCopyToMenu::~CCopyToMenu()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Respond to list changes
void CCopyToMenu::ListenTo_Message(long msg, void* param)
{
	// For time being reset entire menu
	switch(msg)
	{
	case CMailAccountManager::eBroadcast_BeginMailAccountChange:
	case CMboxProtocol::eBroadcast_BeginListChange:
	case CMboxProtocol::eBroadcast_BeginListUpdate:
		break;
	case CMailAccountManager::eBroadcast_EndMailAccountChange:
	case CMboxProtocol::eBroadcast_EndListChange:
	case CMboxProtocol::eBroadcast_EndListUpdate:
		break;

	case CMailAccountManager::eBroadcast_MRUCopyToChange:
		if (sUseCopyToCabinet)
		{
			CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteCopyTo);
			ResetFavourite(sCopyToMenu, sCopyToPopupMenu, sCopyToPopupServerBreaks, *list, CMailAccountManager::sMailAccountManager->GetMRUCopyTo(), true);
			CMailboxPopup::ResetMenuList();
			CMailboxToolbarPopup::ResetMenuList();
		}
		break;

	case CMailAccountManager::eBroadcast_MRUAppendToChange:
		if (sUseAppendToCabinet)
		{
			CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteAppendTo);
			ResetFavourite(sAppendToMenu, sAppendToPopupMenu, sAppendToPopupServerBreaks, *list, CMailAccountManager::sMailAccountManager->GetMRUAppendTo(), false);
			CMailboxPopup::ResetMenuList();
			CMailboxToolbarPopup::ResetMenuList();
		}
		break;

	case CMailAccountManager::eBroadcast_NewMailAccount:
	case CMailAccountManager::eBroadcast_RemoveMailAccount:
		sCopyToReset = true;
		sAppendToReset = true;
		break;
	case CMboxProtocol::eBroadcast_NewList:
	case CMboxProtocol::eBroadcast_RemoveList:
	case CTreeNodeList::eBroadcast_ResetList:
		ChangedList(static_cast<CTreeNodeList*>(param));
		break;
	case CTreeNodeList::eBroadcast_AddNode:
	case CTreeNodeList::eBroadcast_ChangeNode:
	case CTreeNodeList::eBroadcast_DeleteNode:
		CTreeNodeList* aList = NULL;
		switch(msg)
		{
			case CTreeNodeList::eBroadcast_AddNode:
				aList = static_cast<CTreeNodeList::SBroadcastAddNode*>(param)->mList;
				break;
			case CTreeNodeList::eBroadcast_ChangeNode:
				aList = static_cast<CTreeNodeList::SBroadcastChangeNode*>(param)->mList;
				break;
			case CTreeNodeList::eBroadcast_DeleteNode:
				aList = static_cast<CTreeNodeList::SBroadcastDeleteNode*>(param)->mList;
				break;
		}
		ChangedList(aList);
		break;
	}
}

void CCopyToMenu::ChangedList(CTreeNodeList* list)
{
	if (typeid(*static_cast<CTreeNodeList*>(list)) == typeid(CMboxList))
	{
		// Only reset if not favourites
		if (!sUseCopyToCabinet)
			sCopyToReset = true;
		if (!sUseAppendToCabinet)
			sAppendToReset = true;

		// Must tell popups of change too
		CMailboxPopup::ChangeMenuList();
		CMailboxToolbarPopup::ChangeMenuList();
	}
	else if (typeid(*static_cast<CTreeNodeList*>(list)) == typeid(CMboxRefList))
	{
		// Only reset if favourites
		if (sUseCopyToCabinet &&
			CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteCopyTo)
			sCopyToReset = true;
		if (sUseAppendToCabinet &&
			CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteAppendTo)
			sAppendToReset = true;
	}
}

void CCopyToMenu::SetMenuOptions(bool use_copyto_cabinet, bool use_appendto_cabinet)
{
	// Look for change in state
	if ((sUseCopyToCabinet != use_copyto_cabinet) ||
		(sUseAppendToCabinet != use_appendto_cabinet))
		DirtyMenuList();

	sUseCopyToCabinet = use_copyto_cabinet;
	sUseAppendToCabinet = use_appendto_cabinet;
}

// Set the menu items from the various lists
void CCopyToMenu::ResetMenuList(void)
{
	// Only do if required
	if (!sCopyToReset && !sAppendToReset)
		return;

	// Always delete existing menu arrays
	if (sCopyToMenuList.size() && sCopyToReset)
	{
		// Delete all submenus
		for(CMenuList::iterator iter = sCopyToMenuList.begin(); iter != sCopyToMenuList.end(); iter++)
		{
			LMenuBar::GetCurrentMenuBar()->RemoveMenu(*iter);
			delete *iter;
		}
		sCopyToMenuList.clear();
	}

	// Always delete existing menu arrays
	if (sAppendToMenuList.size() && sAppendToReset)
	{
		// Delete all submenus
		for(CMenuList::iterator iter = sAppendToMenuList.begin(); iter != sAppendToMenuList.end(); iter++)
		{
			LMenuBar::GetCurrentMenuBar()->RemoveMenu(*iter);
			delete *iter;
		}
		sAppendToMenuList.clear();
	}

	// Can only do this is accounts already setup
	if (CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->GetProtocolCount())
	{
		if (sCopyToReset)
		{
			if (sUseCopyToCabinet)
			{
				CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteCopyTo);
				ResetFavourite(sCopyToMenu, sCopyToPopupMenu, sCopyToPopupServerBreaks, *list, CMailAccountManager::sMailAccountManager->GetMRUCopyTo(), true);
			}
			else
				ResetAll(sCopyToMenu, sCopyToPopupMenu, sCopyToPopupServerBreaks, true);
		}

		if (sAppendToReset)
		{
			if (sUseAppendToCabinet)
			{
				CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteAppendTo);
				ResetFavourite(sAppendToMenu, sAppendToPopupMenu, sAppendToPopupServerBreaks, *list, CMailAccountManager::sMailAccountManager->GetMRUAppendTo(), false);
			}
			else
				ResetAll(sAppendToMenu, sAppendToPopupMenu, sAppendToPopupServerBreaks, false);
		}

		// Always resync popups after change
		CMailboxPopup::ResetMenuList();
		CMailboxToolbarPopup::ResetMenuList();
	}

	// Always do this last as resolve may force reset
	sCopyToReset = false;
	sAppendToReset = false;
}

// Is menu one of the copy to's
bool CCopyToMenu::IsCopyToMenu(short menu_id)
{
	if ((menu_id == MENU_CopyTo) || (menu_id == MENU_AppendTo))
		return true;
	else if (sCopyToMenuList.size() &&
			((menu_id >= cSubCopyToMenuFirst) && (menu_id <= cSubCopyToMenuFirst + sCopyToMenuList.size() - 1)))
		return true;
	else if (sAppendToMenuList.size() &&
			((menu_id >= cSubAppendToMenuFirst) && (menu_id <= cSubAppendToMenuFirst + sAppendToMenuList.size() - 1)))
		return true;
	else
		return false;
}

bool CCopyToMenu::TestPrefix(const CMboxList& list, const cdstring& offset)
{
	// Now add current items
	size_t offset_length = offset.length();
	for(CMboxList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (!(*iter)->IsDirectory())
		{
			cdstring theName = (*iter)->GetName();
			if (offset_length && ::strncmp(offset.c_str(), theName.c_str(), offset_length))
				return false;
		}
	}
	
	return true;
}

// Add entire protocol to menu
void CCopyToMenu::AddProtocolToMenu(CMboxProtocol* proto, MenuHandle menuH, short& pos, bool acct_name, bool single)
{
	// Count number in list
	unsigned long num_mbox = proto->CountHierarchy();
	bool need_separator = false;

	// Add INBOX if present
	if (proto->GetINBOX())
	{
		cdstring theName = (acct_name ? proto->GetINBOX()->GetAccountName() : proto->GetINBOX()->GetName());
		::AppendItemToMenu(menuH, pos++, theName);
		need_separator = true;
	}

	const CHierarchies& hiers = proto->GetHierarchies();

	// Check to see whether flat wd is required
	cdstring name_offset;
	if (single && proto->FlatWD() && hiers.at(1)->IsRootName(hiers.at(1)->GetDirDelim()))
	{
		name_offset = hiers.at(1)->GetRootName();

		// Check to see whether subscribed is all from same root
		if (TestPrefix(*hiers.front(), name_offset))
			sStripSubscribePrefix = true;
		else
		{
			name_offset = cdstring::null_str;
			sStripSubscribePrefix = false;
		}
	}

	// Always add subscribed
	AppendListToMenu(*hiers.front(), menuH, pos, acct_name, name_offset);
	if (hiers.front()->size())
		need_separator = true;

	// Limit number in list
	if (num_mbox + pos > cMaxMailbox)
	{
		// Add 'Too Many Mailboxes' item (disabled)
		LStr255 str(STRx_Standards, str_TooManyForMenu);
		if (need_separator)
			::AppendMenu(menuH,"\p(-");
		::AppendMenu(menuH, str);
		pos += 2;
		
		// Special - if any default copy to is on this server then add it as a special case
		size_t name_offset_length = name_offset.length();
		const CIdentityList& ids = CPreferences::sPrefs->mIdentities.GetValue();
		cdstrvect added;
		for(CIdentityList::const_iterator iter = ids.begin(); iter != ids.end(); iter++)
		{
			cdstring copyto = (*iter).GetCopyTo(true);
			if (!(*iter).GetCopyToNone(true) && !(*iter).GetCopyToChoose(true))
			{
				// Must not be duplicate
				cdstrvect::const_iterator found = std::find(added.begin(), added.end(), copyto);
				if (found != added.end())
					continue;

				// Check for account match
				cdstring acctname = proto->GetAccountName();
				acctname += cMailAccountSeparator;
				if (!::strncmp(acctname, copyto, acctname.length()))
				{
					// Punt over account prefix if not required
					cdstring use_name;
					if (acct_name)
						use_name = copyto;
					else
						use_name = copyto.c_str() + acctname.length();

					if (name_offset_length && (::strncmp(name_offset.c_str(), use_name.c_str(), name_offset_length) == 0))
						::AppendItemToMenu(menuH, pos++, use_name.c_str() + name_offset_length);
					else
						::AppendItemToMenu(menuH, pos++, use_name.c_str());
					added.push_back(copyto);
				}
			}
		}
	}
	else
	{
		// Add remainder (with separators)
		for(CHierarchies::const_iterator iter = hiers.begin() + 1; iter != hiers.end(); iter++)
		{
			// Only bother if something present
			if ((*iter)->size())
			{
				// Do first separator only if its needed
				if (need_separator)
				{
					::AppendMenu(menuH,"\p(-");
					pos++;
				}
				else
					need_separator = true;
				AppendListToMenu(**iter, menuH, pos, acct_name, name_offset);
			}
		}
	}
}

// Append entire list to menu
void CCopyToMenu::AppendListToMenu(const CMboxList& list, MenuHandle menuH, short& pos, bool acct_name, const cdstring& offset)
{
	// Now add current items
	size_t offset_length = offset.length();
	for(CMboxList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (!(*iter)->IsDirectory())
			AppendMbox(*iter, menuH, pos, acct_name, offset, offset_length);
	}
}

// Append entire list to menu
void CCopyToMenu::AppendListToMenu(const CMboxRefList& list, bool dynamic, MenuHandle menuH, short& pos, bool acct_name, const cdstring& offset, bool reverse)
{
	// Now add current items
	size_t offset_length = offset.length();
	for(int i = 0 ; i < list.size(); i++)
	{
		CMboxRefList::const_iterator iter1 = reverse ? list.end() - (i + 1) : list.begin() + i;

		// Look for wildcard items and resolve all of them
		if ((*iter1)->IsWildcard())
		{
			// Resolve to mboxes - maybe wildcard so get list
			CMboxList match;
			static_cast<CWildcardMboxRef*>(*iter1)->ResolveMbox(match, dynamic);

			// Add each mailbox in resolved list
			for(CMboxList::iterator iter2 = match.begin(); iter2 != match.end(); iter2++)
			{
				// Do not add directories
				if (!(*iter2)->IsDirectory())
					AppendMbox(*iter2, menuH, pos, acct_name, offset, offset_length);
			}
		}

		// Do not add directories
		else if (!(*iter1)->IsDirectory())
			AppendMbox(*iter1, menuH, pos, acct_name, offset, offset_length);
	}
}

void CCopyToMenu::AppendMbox(const CTreeNode* mbox, MenuHandle menuH, short& pos, bool acct_name, const cdstring& offset, size_t offset_length)
{
	cdstring theName = mbox->GetAccountName(acct_name);
	if (offset_length && (::strncmp(offset.c_str(), theName.c_str(), offset_length) == 0))
		::AppendItemToMenu(menuH, pos++, theName.c_str() + offset_length);
	else
		::AppendItemToMenu(menuH, pos++, theName.c_str());
}

// Find mailbox from menu
bool CCopyToMenu::GetMbox(short menu_id, short item_num, CMbox*& found)
{
	// Look for browse
	if (((menu_id == MENU_CopyTo) || (menu_id == MENU_AppendTo)) && (item_num == cCopyToChoose))
	{
		found = NULL;
		bool ignore = false;
		return CBrowseMailboxDialog::PoseDialog(false, false, found, ignore);
	}

	// Check for favourite
	bool favourite = (((menu_id == MENU_CopyTo) && sUseCopyToCabinet) ||
						((menu_id == MENU_AppendTo) && sUseAppendToCabinet));
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	bool multi = favourite ? (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1) : false;

	// Determine server index & get menu handle
	unsigned long server_index = 0;
	MenuHandle menuH = (menu_id == MENU_CopyTo) ? sCopyToMenu->GetMacMenuH() : sAppendToMenu->GetMacMenuH();
	if (!favourite && (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1))
	{
		if (menu_id < cSubAppendToMenuFirst)
		{
			server_index = menu_id - cSubCopyToMenuFirst;
			menuH = sCopyToMenuList.at(server_index)->GetMacMenuH();
		}
		else
		{
			server_index = menu_id - cSubAppendToMenuFirst;
			menuH = sAppendToMenuList.at(server_index)->GetMacMenuH();
		}
	}

	// May need to add back in WD prefix
	cdstring mbox_name;
	if ((protos.size() == 1) && protos.front()->FlatWD() &&
		(favourite ? sStripFavouritePrefix : sStripSubscribePrefix))
		mbox_name = protos.front()->GetHierarchies().at(1)->GetRootName();

	// Get name
	cdstring menuitem = ::GetMenuItemTextUTF8(menuH, item_num);

	// Process any prefix - not for INBOX
	if (menuitem.compare(cINBOX, true) == 0)
		mbox_name = menuitem;
	else
		mbox_name += menuitem;


	if (!multi)
		mbox_name = protos.at(server_index)->GetAccountName() + cMailAccountSeparator + mbox_name;

	// Create mbox ref
	CMboxRef ref(mbox_name, '.');
	found = ref.ResolveMbox(true);

	return found != NULL;
}

bool CCopyToMenu::GetPopupMboxSend(bool copy_to, short item_num, CMbox*& found, bool& set_as_default)
{
	return GetPopupMbox(copy_to, item_num, true, true, found, set_as_default);
}

bool CCopyToMenu::GetPopupMbox(bool copy_to, short item_num, CMbox*& found, bool do_choice)
{
	bool ignore = false;
	return GetPopupMbox(copy_to, item_num, do_choice, false, found, ignore);
}

bool CCopyToMenu::GetPopupMbox(bool copy_to, short item_num, bool do_choice, bool sending, CMbox*& found, bool& set_as_default)
{
	// Return false only if a choice was requested but cancelled

	// Special for first items
	if (item_num == (copy_to ? cPopupCopyNone : cPopupAppendNone))
	{
		found = (CMbox*) -1;
		return true;
	}
	else if (item_num == (copy_to ? cPopupCopyChoose : cPopupAppendChoose))
	{
		found = NULL;
		if (do_choice)
			CBrowseMailboxDialog::PoseDialog(false, sending, found, set_as_default);

		return (found != NULL) || !do_choice;
	}

	MenuHandle menuH = copy_to ? sCopyToPopupMenu : sAppendToPopupMenu;
	unsigned long server_index = 0;
	bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);
	bool use_cabinet = (copy_to && sUseCopyToCabinet) || (!copy_to && sUseAppendToCabinet);
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();

	// May need to add back in WD prefix
	cdstring mbox_name;
	if ((protos.size() == 1) && protos.front()->FlatWD() &&
		(use_cabinet ? sStripFavouritePrefix : sStripSubscribePrefix))
		mbox_name = protos.front()->GetHierarchies().at(1)->GetRootName();

	// Get name
	cdstring menuitem = ::GetMenuItemTextUTF8(menuH, item_num);

	// Process any prefix - not for INBOX
	if (menuitem.compare(cINBOX, true) == 0)
		mbox_name = menuitem;
	else
		mbox_name += menuitem;

	// If not cabinet, need to take server breaks into account
	if (!multi)
		mbox_name = protos.front()->GetAccountName() + cMailAccountSeparator + mbox_name;

	// Create mbox ref
	CMboxRef ref(mbox_name, '.');
	found = ref.ResolveMbox(true);

	return true;
}

bool CCopyToMenu::GetPopupMboxName(bool copy_to, short item_num, cdstring& found, bool do_choice)
{
	CMbox* mbox = NULL;
	if (GetPopupMbox(copy_to, item_num, mbox, do_choice))
	{
		if (mbox && (mbox != (CMbox*) -1))
		{
			//bool multi = (CMailAccountManager::sMailAccountManager->GetProtocolList().size() > 1);
			bool multi = true;

			found = (multi ? mbox->GetAccountName() : mbox->GetName());
		}
		else if (mbox)
			found = cdstring::null_str;
		else
			found = "\1";

		return true;
	}
	else
		return false;
}

// Find position of mailbox in menu
SInt16 CCopyToMenu::FindPopupMboxPos(bool copy_to, const char* name)
{
	// Check for 'Choose' special
	if ((*name == 1) && (*(name+1) == 0))
		return 2;

	// Loop over all items
	short max_items = ::CountMenuItems(copy_to ? sCopyToPopupMenu : sAppendToPopupMenu);
	for(short i = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX; i <= max_items; i++)
	{
		// Check for disabled menu
		Str255 text;
		::GetMenuItemText(copy_to ? sCopyToPopupMenu : sAppendToPopupMenu, i, text);
		if ((text[0] == 1) && (text[1] == '-'))
			continue;

		cdstring mbox_name;
		if (GetPopupMboxName(copy_to, i, mbox_name) && (mbox_name == name))
			return i;
	}

	return 0;
}

// Set the menu items from the various lists
void CCopyToMenu::ResetAll(LMenu* main_menu, MenuHandle popup_menu, ulvector& breaks, bool copy_to)
{
	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(main_menu->GetMacMenuH());
	for(short i=1; i<=num_menu; i++)
		::DeleteMenuItem(main_menu->GetMacMenuH(),1);

	// Remove any existing items from popup menu
	num_menu = ::CountMenuItems(popup_menu);
	for(short i = 1; i <= num_menu; i++)
		::DeleteMenuItem(popup_menu, 1);
	breaks.clear();

	short pos_main = 1;
	short pos_popup = 1;

	// Add option for choice in main
	LStr255 str1(STRx_Standards, str_MailboxAllowChoice);
	str1 += "\p;(-";
	::AppendMenu(main_menu->GetMacMenuH(), str1);
	pos_main += 2;

	// Add option for none in popup if append menu
	if (copy_to)
	{
		LStr255 str2(STRx_Standards, str_MailboxAllowChoice);
		::AppendMenu(popup_menu, str2);
		pos_popup += 1;
	}
	else
	{
		LStr255 str2(STRx_Standards, str_MailboxNone);
		LStr255 str3(STRx_Standards, str_MailboxChooseLater);
		str2 += "\p;" + str3;
		::AppendMenu(popup_menu, str2);
		pos_popup += 2;
	}

	// Look for single or multiple accounts
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	if (CMailAccountManager::sMailAccountManager->GetProtocolCount() == 1)
	{
		// Single account - add to main menu
		AddProtocolToMenu(protos.front(), main_menu->GetMacMenuH(), pos_main, false, true);
		::AppendMenu(popup_menu,"\p(-");
		pos_popup++;
		AddProtocolToMenu(protos.front(), popup_menu, pos_popup, false, true);
	}
	else if (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1)
	{
		// Start sub-menu
		ResIDT sub_id = copy_to ? cSubCopyToMenuFirst : cSubAppendToMenuFirst;

		// Add each protocol to menu as sub-menu
		for(CMboxProtocolList::const_iterator iter = protos.begin(); (sub_id < cMaxSubMenu) && (iter != protos.end()); iter++, sub_id++)
		{
			LMenu* menu = new LMenu(sub_id, "\p");
			if (copy_to)
				sCopyToMenuList.push_back(menu);
			else
				sAppendToMenuList.push_back(menu);
			LMenuBar::GetCurrentMenuBar()->InstallMenu(menu, hierMenu);

			// Add protocol to menu
			short pos = 1;
			AddProtocolToMenu(*iter, menu->GetMacMenuH(), pos, false);

			// Add this as sub-menu to main
			::AppendItemToMenu(main_menu->GetMacMenuH(), pos_main, (*iter)->GetAccountName());
			::SetItemCmd(main_menu->GetMacMenuH(), pos_main, hMenuCmd);
			::SetItemMark(main_menu->GetMacMenuH(), pos_main, sub_id);
			pos_main++;

			// Add this as sub-menu to popup
			::AppendMenu(popup_menu,"\p(-");
			pos_popup++;
			breaks.push_back(pos_popup);
			AddProtocolToMenu(*iter, popup_menu, pos_popup, true);
		}
	}
}


// Set the menu items from the various lists
void CCopyToMenu::ResetFavourite(LMenu* main_menu, MenuHandle popup_menu, ulvector& breaks, const CMboxRefList& items, const CMboxRefList& mrus, bool copy_to)
{
	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(main_menu->GetMacMenuH());
	for(short i=1; i<=num_menu; i++)
		::DeleteMenuItem(main_menu->GetMacMenuH(),1);

	// Remove any existing items from popup menu
	num_menu = ::CountMenuItems(popup_menu);
	for(short i = 1; i <= num_menu; i++)
		::DeleteMenuItem(popup_menu, 1);
	breaks.clear();

	short pos_main = 1;
	short pos_popup = 1;

	// Add option for choice in main
	LStr255 str1(STRx_Standards, str_MailboxAllowChoice);
	str1 += "\p;(-";
	::AppendMenu(main_menu->GetMacMenuH(), str1);
	pos_main += 2;

	// Add option for none in popup if append menu
	if (copy_to)
	{
		LStr255 str2(STRx_Standards, str_MailboxAllowChoice);
		str2 += "\p;(-";
		::AppendMenu(popup_menu, str2);
		pos_popup += 2;
	}
	else
	{
		LStr255 str2(STRx_Standards, str_MailboxNone);
		LStr255 str3(STRx_Standards, str_MailboxChooseLater);
		str2 += "\p;" + str3 + "\p;(-";
		::AppendMenu(popup_menu, str2);
		pos_popup += 3;
	}

	bool acct_name = (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1);

	// Check to see whether flat wd is required
	cdstring name_offset;
	sStripFavouritePrefix = false;
	if (!acct_name)
	{
		const CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocolList().front();
		const CHierarchies& hiers = proto->GetHierarchies();
		if (proto->FlatWD() && hiers.at(1)->IsRootName(hiers.at(1)->GetDirDelim()))
		{
			name_offset = hiers.at(1)->GetRootName();

			// Check to see whether subscribed is all from same root
			if (TestPrefix(*hiers.front(), name_offset))
				sStripFavouritePrefix = true;
			else
				name_offset = cdstring::null_str;
		}
	}

	// Now add current items
	AppendListToMenu(items, true, main_menu->GetMacMenuH(), pos_main, acct_name, name_offset);
	AppendListToMenu(items, false, popup_menu, pos_popup, acct_name, name_offset);

	// Add MRU break
	::AppendMenu(main_menu->GetMacMenuH(),"\p(-");
	pos_main++;
	::AppendMenu(popup_menu,"\p(-");
	pos_popup++;

	// Add MRUs (in reverse)
	AppendListToMenu(mrus, true, main_menu->GetMacMenuH(), pos_main, acct_name, name_offset, true);
	AppendListToMenu(mrus, false, popup_menu, pos_popup, acct_name, name_offset, true);
}
