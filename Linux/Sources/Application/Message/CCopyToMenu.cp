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

#include <algorithm>
#include <typeinfo>

const unsigned short cMaxMailbox = 300;

// __________________________________________________________________________________________________
// C L A S S __ C W I N D O W S M E N U
// __________________________________________________________________________________________________

// Statics
CCopyToMenu* CCopyToMenu::sMailboxMainMenu = NULL;

cdstring CCopyToMenu::sCopyToPopupMenu;
cdstring CCopyToMenu::sAppendToPopupMenu;

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
			ResetFavourite(NULL, sCopyToPopupMenu, *list,
							 CMailAccountManager::sMailAccountManager->GetMRUCopyTo(), true);
			CMailboxPopup::ResetMenuList();
			CMailboxToolbarPopup::ResetMenuList();
		}
		break;

	case CMailAccountManager::eBroadcast_MRUAppendToChange:
		if (sUseAppendToCabinet)
		{
			CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteAppendTo);
			ResetFavourite(NULL, sAppendToPopupMenu, *list,
							 CMailAccountManager::sMailAccountManager->GetMRUAppendTo(), false);
			CMailboxPopup::ResetMenuList();
			CMailboxToolbarPopup::ResetMenuList();
		}
		break;

	case CMailAccountManager::eBroadcast_NewMailAccount:
	case CMailAccountManager::eBroadcast_RemoveMailAccount:
		DirtyCopyMenuList();
		DirtyAppendMenuList();
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
			DirtyCopyMenuList();
		if (!sUseAppendToCabinet)
			DirtyAppendMenuList();

		// Must tell popups of change too
		CMailboxPopup::ChangeMenuList();
		CMailboxToolbarPopup::ChangeMenuList();
	}
	else if (typeid(*static_cast<CTreeNodeList*>(list)) == typeid(CMboxRefList))
	{
		// Only reset if favourites
		if (sUseCopyToCabinet &&
			CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteCopyTo)
			DirtyCopyMenuList();
		if (sUseAppendToCabinet &&
			CMailAccountManager::sMailAccountManager->GetFavouriteType(static_cast<CMboxRefList*>(list)) == CMailAccountManager::eFavouriteAppendTo)
			DirtyAppendMenuList();
	}
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

void CCopyToMenu::SetMenuOptions(bool use_copyto_cabinet, bool use_appendto_cabinet)
{
	// Look for change in state
	if ((sUseCopyToCabinet != use_copyto_cabinet) ||
		(sUseAppendToCabinet != use_appendto_cabinet))
		DirtyMenuList();

	sUseCopyToCabinet = use_copyto_cabinet;
	sUseAppendToCabinet = use_appendto_cabinet;
}

void CCopyToMenu::DirtyMenuList(void)
{
	DirtyCopyMenuList();
	DirtyAppendMenuList();
}

void CCopyToMenu::DirtyCopyMenuList(void)
{
	sCopyToReset = true;

	// Broadcast reset to all listening main menus
	if (sMailboxMainMenu)
		sMailboxMainMenu->Broadcast_Message(eBroadcast_ResetMenuCopy, NULL);
}

void CCopyToMenu::DirtyAppendMenuList(void)
{
	sAppendToReset = true;

	// Broadcast reset to all listening main menus
	if (sMailboxMainMenu)
		sMailboxMainMenu->Broadcast_Message(eBroadcast_ResetMenuAppend, NULL);
}

// Set the menu items from the various lists
void CCopyToMenu::ResetMenuList(JXTextMenu* copyto, JXTextMenu* appendto)
{
	// Try and cast to our type of menu
	CCopyToMain* mycopyto = dynamic_cast<CCopyToMain*>(copyto);
	CCopyToMain* myappendto = dynamic_cast<CCopyToMain*>(appendto);

	// Only allow reset of our kind of menu
	if (mycopyto && !mycopyto->RequiresReset(true))
		mycopyto = NULL;
	if (myappendto && !myappendto->RequiresReset(false))
		myappendto = NULL;

	// Only do if required
	if (!sCopyToPopupMenu.empty() && !sAppendToPopupMenu.empty() &&
		!sCopyToReset && !sAppendToReset && !mycopyto && !myappendto)
		return;

	// Can only do this is accounts already setup
	if (CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->GetProtocolCount())
	{
		if (sCopyToPopupMenu.empty() || sCopyToReset || mycopyto)
		{
			if (sUseCopyToCabinet)
			{
				CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteCopyTo);
				ResetFavourite(mycopyto, sCopyToPopupMenu, 
								 *list, CMailAccountManager::sMailAccountManager->GetMRUCopyTo(), true);
				
			}
			else
				ResetAll(mycopyto, sCopyToPopupMenu, true);

			// Mark main menu item as reset
			if (mycopyto)
				mycopyto->Reset();
		}

		if (sAppendToPopupMenu.empty() || sAppendToReset || myappendto)
		{
			if (sUseAppendToCabinet)
			{
				CMboxRefList* list = CMailAccountManager::sMailAccountManager->GetFavourites().at(CMailAccountManager::eFavouriteAppendTo);
				ResetFavourite(myappendto, sAppendToPopupMenu, 
								*list, CMailAccountManager::sMailAccountManager->GetMRUAppendTo(), false);
			}
			else
				ResetAll(myappendto, sAppendToPopupMenu, false);

			// Mark main menu item as reset
			if (myappendto)
				myappendto->Reset();
		}

		// Always resync popups after change
		CMailboxPopup::ResetMenuList();
		CMailboxToolbarPopup::ResetMenuList();
	}
	
	// Always do this last as resolve may force reset
	sCopyToReset = false;
	sAppendToReset = false;
}

// Add entire protocol to menu
void CCopyToMenu::AddProtocolToMenu(CMboxProtocol* proto, cdstring& menu, bool is_popup, int& pos, bool acct_name, bool single)
{
	// Count number in list
	unsigned long num_mbox = proto->CountHierarchy();
	bool need_separator = false;

	// Add INBOX if present
	if (proto->GetINBOX())
	{
		cdstring theName = (acct_name ? proto->GetINBOX()->GetAccountName() : proto->GetINBOX()->GetName());
		if (!menu.empty())
			menu += "| ";
		menu += theName.c_str();
		if (is_popup)
			menu += " %r";
		pos++;
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
	AppendListToMenu(*hiers.front(), menu, is_popup, pos, acct_name, name_offset);
	if (hiers.front()->size())
		need_separator = true;

	// Limit number in list
	if (num_mbox + pos > cMaxMailbox)
	{
		// Add 'Too Many Mailboxes' item (disabled)
		cdstring str;
		str.FromResource(IDE_TooManyForMenu);
		if (!menu.empty())
			menu += (need_separator ? " %l |" : " |");
		menu += str;
		menu += " %d";
		
		// Special - if any default copy to is on this server then add it as a special case
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
					menu += " |";

					// Check whether account prefix is required and strip if not
					if (acct_name)
						menu += copyto;
					else
						menu += &copyto[acctname.length()];
					
					if (is_popup)
						menu += " %r";
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
					menu += " %l";
				else
					need_separator = true;
				AppendListToMenu(**iter, menu, is_popup, pos, acct_name, name_offset);
			}
		}
	}
}

// Append entire list to menu
void CCopyToMenu::AppendListToMenu(const CMboxList& list, cdstring& menu, bool is_popup, int& pos, bool acct_name, const cdstring& noffset)
{
	// Now add current items
	size_t offset_length = noffset.length();
	for(CMboxList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		if (!(*iter)->IsDirectory())
		{
			cdstring theName = (acct_name ? ((CMbox*) *iter)->GetAccountName() : ((CMbox*) *iter)->GetName());
			if (!menu.empty())
				menu +="| ";
			if (offset_length && (::strncmp(noffset.c_str(), theName.c_str(), offset_length) == 0))
				menu += theName.c_str() + offset_length;
			else
				menu += theName.c_str();
			if (is_popup)
				menu += " %r";
			pos++;
		}
	}
}

// Append entire list to menu
void CCopyToMenu::AppendListToMenu(const CMboxRefList& list, bool dynamic, cdstring& menu, bool is_popup, int& pos,bool acct_name, const cdstring& noffset, bool reverse)
{
	// Now add current items
	size_t offset_length = noffset.length();
	for(unsigned int i = 0 ; i < list.size(); i++)
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
				{
					cdstring theName = (acct_name ? (*iter2)->GetAccountName() : (*iter2)->GetName());
					if (!menu.empty())
						menu += "| ";
					if (offset_length && (::strncmp(noffset.c_str(), theName.c_str(), offset_length) == 0))
						menu += theName.c_str() + offset_length;
					else
						menu += theName.c_str();
					if (is_popup)
						menu += " %r";
				}
			}
		}

		// Do not add directories
		else if (!(*iter1)->IsDirectory())
		{
			cdstring theName = (*iter1)->GetAccountName(acct_name);
			if (!menu.empty())
				menu += "| ";
			if (offset_length && (::strncmp(noffset.c_str(), theName.c_str(), offset_length) == 0))
				menu += theName.c_str() + offset_length;
			else
				menu += theName.c_str();
			if (is_popup)
				menu += " %r";
		}
	}
}

// Find mailbox from menu
bool CCopyToMenu::GetMbox(JXTextMenu* menu, bool copy_to, int item_num, CMbox*& found)
{
	// Look for browse in top-level menus
	if ((dynamic_cast<CCopyToMain*>(menu) != NULL) && (item_num == cCopyToChoose))
	{
		found = NULL;
		bool ignore = false;
		return CBrowseMailboxDialog::PoseDialog(false, false, found, ignore);
	}

	// Check for favourite
	bool favourite = ((copy_to && sUseCopyToCabinet) ||
						(!copy_to && sUseAppendToCabinet));
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	bool multi = favourite ? (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1) : false;

	// Determine server index
	cdstring server_title;
	if (!favourite && (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1))
		server_title = menu->GetTitleText();
	else
		server_title = protos.front()->GetAccountName();

	// May need to add back in WD prefix
	cdstring mbox_name;
	if ((protos.size() == 1) && protos.front()->FlatWD())
		mbox_name = protos.front()->GetHierarchies().at(1)->GetRootName();

	// Get name
	mbox_name += menu->GetItemText(item_num);
	
	if (!multi)
		mbox_name = server_title + cMailAccountSeparator + mbox_name;

	// Create mbox ref
	CMboxRef ref(mbox_name, '.');
	found = ref.ResolveMbox(true);
	
	return found != NULL;
}

bool CCopyToMenu::GetPopupMboxSend(JXTextMenu* theMenu, bool copy_to, short item_num, CMbox*& found, bool& set_as_default)
{
	return GetPopupMbox(theMenu, copy_to, item_num, true, true, found, set_as_default);
}

bool CCopyToMenu::GetPopupMbox(JXTextMenu* theMenu, bool copy_to, short item_num, CMbox*& found, bool do_choice)
{
	bool ignore = false;
	return GetPopupMbox(theMenu, copy_to, item_num, do_choice, false, found, ignore);
}

bool CCopyToMenu::GetPopupMbox(JXTextMenu* theMenu, bool copy_to, short item_num, bool do_choice, bool sending, CMbox*& found, bool& set_as_default)
{
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
	cdstring theTxt;
	theTxt = theMenu->GetItemText(item_num);
	
	// Process any prefix - not for INBOX
	if (theTxt == cINBOX)
		mbox_name = theTxt;
	else
		mbox_name += theTxt;

	// If not cabinet, need to take server breaks into account
	if (!multi)
		mbox_name = protos.front()->GetAccountName() + cMailAccountSeparator + mbox_name;

	// Create mbox ref
	CMboxRef ref(mbox_name, '.');
	found = ref.ResolveMbox(true);
		
	return found != NULL;
}

bool CCopyToMenu::GetPopupMboxName(JXTextMenu* theMenu, 
									 bool copy_to, int item_num, cdstring& found, bool do_choice)
{
	CMbox* mbox = nil;
	if (GetPopupMbox(theMenu, copy_to, item_num, mbox, do_choice))
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
SInt16 CCopyToMenu::FindPopupMboxPos(JXTextMenu* theMenu, bool copy_to, const char* name)
{
	// Check for 'Choose' special
	if ((*name == 1) && (*(name+1) == 0))
		return (copy_to ? cPopupCopyChoose : cPopupAppendChoose);

	int max_items = theMenu->GetItemCount();
	for(int i = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX; i <= max_items; i++)
	{
		cdstring found;
		if (GetPopupMboxName(theMenu, copy_to, i, found) && (found == name))
			return i;
	}

	// Nothing found
	return 0;
}


// Set the menu items from the various lists
void CCopyToMenu::ResetAll(JXTextMenu* main_menu, cdstring& popup_menu, bool copy_to)
{
	// Remove any existing items from main menu
	if (main_menu)
		main_menu->RemoveAllItems();
	cdstring main_menu_str = "Choose... %l";

	// Remove any existing items from popup menu
	popup_menu = copy_to ? "Choose Later %r %l" : "None %r | Choose Later %r %l";
	int pos_main = cCopyToSingleINBOX;
	int pos_popup = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX;

	// Only do the reset if mail accounts sets up
	if (!CMailAccountManager::sMailAccountManager)
	{
		if (main_menu)
			main_menu->SetMenuItems(main_menu_str);
		return;
	}

	// Look for single or multiple accounts
	CMboxProtocolList& protos = CMailAccountManager::sMailAccountManager->GetProtocolList();
	if (CMailAccountManager::sMailAccountManager->GetProtocolCount() == 1)
	{
		// Single account - add to main menu
		if (main_menu)
		{
			AddProtocolToMenu(protos.front(), main_menu_str, false, pos_main, false, true);
			main_menu->SetMenuItems(main_menu_str);
		}
		AddProtocolToMenu(protos.front(), popup_menu, true, pos_popup, false, true);
	}
	else if (CMailAccountManager::sMailAccountManager->GetProtocolCount() > 1)
	{
		bool first = true;
		int cmd_pos = pos_main;

		if (main_menu)
			main_menu->SetMenuItems(main_menu_str);	

		// Add each protocol to menu as sub-menu
		for(CMboxProtocolList::const_iterator iter = protos.begin(); iter != protos.end(); iter++)
		{
			// Add this as sub-menu to main
			cdstring theName = (*iter)->GetAccountName();
			if (main_menu)
			{
				main_menu->AppendItem(theName);
				JXTextMenu* submenu = new CCopyToSub(main_menu, pos_main++, const_cast<JXContainer*>(main_menu->GetEnclosure()));

				// Add server name for later retrieval in GetMbox
				submenu->SetTitleText(theName);

				// Sub-menus are always enabled. This removes the need to add
				// menu update handlers in any window that uses these menus. The
				// parent menu will have its items enabled appropriately.
				submenu->SetUpdateAction(JXMenu::kDisableNone);

				// Add protocol to menu
				cdstring sub_menu_str;
				AddProtocolToMenu(*iter, sub_menu_str, false, cmd_pos, false);
				submenu->SetMenuItems(sub_menu_str);
			}

			// Add this as sub-section to popup
			if (!first)
				popup_menu += " %l";
			else
				first = false;

			AddProtocolToMenu(*iter, popup_menu, true, pos_popup, true);
		}
	}
	
	// Force reset of popups
	CMailboxPopup::ResetMenuList();
	CMailboxToolbarPopup::ResetMenuList();
}


// Set the menu items from the various lists
void CCopyToMenu::ResetFavourite(JXTextMenu* main_menu, cdstring& popup_menu,
									const CMboxRefList& items, const CMboxRefList& mrus, bool copy_to)
{
	// Remove any existing items from main menu
	if (main_menu)
		main_menu->RemoveAllItems();
	cdstring main_menu_str = "Choose... %l";

	// Remove any existing items from popup menu
	popup_menu = copy_to ? "Choose Later %r %l" : "None %r | Choose Later %r %l";

	int pos_main = cCopyToSingleINBOX;
	int pos_popup = copy_to ? cPopupCopySingleINBOX : cPopupAppendSingleINBOX;

	// Only do the reset if mail accounts sets up
	if (!CMailAccountManager::sMailAccountManager)
	{
		if (main_menu)
			main_menu->SetMenuItems(main_menu_str);
		return;
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
	if (main_menu)
		AppendListToMenu(items,true, main_menu_str, false, pos_main, acct_name, name_offset);
	AppendListToMenu(items, false, popup_menu, true, pos_popup, acct_name, name_offset);
	
	// Add MRU break
	main_menu_str += " %l";
	popup_menu += " %l";

	// Add MRUs (in reverse)
	if (main_menu)
	{
		AppendListToMenu(mrus, true, main_menu_str, false, pos_main, acct_name, name_offset, true);
		main_menu->SetMenuItems(main_menu_str);		
	}
	AppendListToMenu(mrus, false, popup_menu, true, pos_popup, acct_name, name_offset, true);
}

//----------
//
//

CCopyToMenu::CCopyToMain::CCopyToMain(bool copyto, JXMenu* owner, const JIndex itemIndex, JXContainer* enclosure)
	: CMenu(owner, itemIndex, enclosure)
{
	mCopyTo = copyto;
	mRequiresReset = true;

	if (CCopyToMenu::sMailboxMainMenu)
		CCopyToMenu::sMailboxMainMenu->Add_Listener(this);
}

// Respond to reset
void CCopyToMenu::CCopyToMain::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case eBroadcast_ResetMenuCopy:
		if (mCopyTo)
			mRequiresReset = true;
		break;
	case eBroadcast_ResetMenuAppend:
		if (!mCopyTo)
			mRequiresReset = true;
		break;
	default:;
	}
}
