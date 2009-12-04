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


// Source for CMailboxToolbarPopup class

#include "CMailboxToolbarPopup.h"

#include "CMailAccountManager.h"
#include "CMbox.h"

#include <algorithm>

const short cMaxMailbox = 300;
const short	cMboxStartPos = 3;

CMailboxToolbarPopup::CMailboxToolbarPopupList CMailboxToolbarPopup::sMailboxToolbarPopupMenus;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMailboxToolbarPopup::CMailboxToolbarPopup(bool copy_to, const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: CToolbarPopupButton(label, enclosure, hSizing, vSizing, x,y, w,h)
{
	mCopyTo = copy_to;

	// Add to list
	sMailboxToolbarPopupMenus.push_back(this);
	mMissingMailbox = false;
	
	mCachedMbox = NULL;

	// Let JX know this is a popup menu
	SetValue(1);
	GetPopupMenu()->SetUpdateAction(JXMenu::kDisableNone);

	// Force reset of menus - will only be done if dirty
	CCopyToMenu::ResetMenuList(NULL, NULL);
	GetPopupMenu()->SetMenuItems(CCopyToMenu::GetPopupMenu(mCopyTo).c_str());
	mNeedsSync = false;
}

// Default destructor
CMailboxToolbarPopup::~CMailboxToolbarPopup()
{
	// Remove from list
	CMailboxToolbarPopupList::iterator found = std::find(sMailboxToolbarPopupMenus.begin(), sMailboxToolbarPopupMenus.end(), this);
	if (found != sMailboxToolbarPopupMenus.end())
		sMailboxToolbarPopupMenus.erase(found);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set to default item
void CMailboxToolbarPopup::SetDefault()
{
	mChosenName = cdstring::null_str;

	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList(NULL, NULL);

	// Sync with menu
	SyncMenu();
}

// Update menu to mailbox list
void CMailboxToolbarPopup::ResetMenuList(void)
{
	// Iterate over all menus and sync
	for(CMailboxToolbarPopupList::iterator iter = sMailboxToolbarPopupMenus.begin(); iter != sMailboxToolbarPopupMenus.end(); iter++)
		(*iter)->SetNeedsSync();
}

// Mailbox list changed
void CMailboxToolbarPopup::ChangeMenuList(void)
{
	// Iterate over all menus and sync
	for(CMailboxToolbarPopupList::iterator iter = sMailboxToolbarPopupMenus.begin(); iter != sMailboxToolbarPopupMenus.end(); iter++)
		(*iter)->mCachedMbox = NULL;
}

// Delete mark after popup
void CMailboxToolbarPopup::SetValue(JIndex value)
{
	// Reset cached mbox immediately before processing change in value
	mCachedMbox = NULL;

	CToolbarPopupButton::SetValue(value);
	
	CCopyToMenu::GetPopupMboxName(GetPopupMenu(), mCopyTo, value, mChosenName, false);
}


// Sync control to menu
void CMailboxToolbarPopup::SyncMenu(void)
{
	// Only if required
	if (!NeedsSync())
		return;

	// Update our menu items to match CCopyToMenu
	GetPopupMenu()->SetMenuItems(CCopyToMenu::GetPopupMenu(mCopyTo).c_str());

	// Only bother if there's a name
	if (!mChosenName.empty())
	{
		short set_item = mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendChoose;// Must set to Choose if no sync

		if (mChosenName != "\1")
		{
			short num_menu = GetPopupMenu()->GetItemCount();
			for(short i = cMboxStartPos; i <= num_menu; i++)
			{
				// Compare with current text value
				cdstring mbox_name;
				if (CCopyToMenu::GetPopupMboxName(GetPopupMenu(), mCopyTo, i, mbox_name) 
						&& (mChosenName == mbox_name))
				{
					set_item = i;
					break;
				}
			}
		}

		// Now set new value
		SetValue(set_item);
	}
	else
		// Set to none if already None
		SetValue(mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendNone);
}

// Set name of selected mailbox
void CMailboxToolbarPopup::SetSelectedMbox(const cdstring& mbox_name, bool choose)
{
	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList(NULL, NULL);

	// Default to Choose,
	short set_item = mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendChoose;

	// Check for 'None'
	if (!choose && !mbox_name.empty())
	{

		// Fetch mbox at
		set_item = CCopyToMenu::FindPopupMboxPos(GetPopupMenu(), mCopyTo, mbox_name.c_str());
		
		// Try forced MRU
		if (set_item < 1)
		{
			if (mCopyTo && CCopyToMenu::sUseCopyToCabinet ||
				!mCopyTo && CCopyToMenu::sUseAppendToCabinet)
			{
				// Try to find mailbox
				CMbox* mbox = CMailAccountManager::sMailAccountManager->FindMboxAccount(mbox_name);

				if (mbox)
				{
					// Add to MRU list
					if (mCopyTo)
						CMailAccountManager::sMailAccountManager->AddMRUCopyTo(mbox);
					else
						CMailAccountManager::sMailAccountManager->AddMRUAppendTo(mbox);

					// Redo lookup
					set_item = CCopyToMenu::FindPopupMboxPos(GetPopupMenu(), mCopyTo, mbox_name.c_str());
				}
			}
		}
	}

	// Set menu to new value
	if (set_item > 0)
	{
		SetValue(set_item);

		mMissingMailbox = false;
		mMissingName = cdstring::null_str;
	}
	else
	{
		// Always set to Choose if previous is now missing
		SetValue(mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendChoose);

		mMissingMailbox = true;
		mMissingName = mbox_name;
	}
}

// Get name of selected mailbox
bool CMailboxToolbarPopup::GetSelectedMbox(CMbox*& found, bool do_choice)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMbox(GetPopupMenu(), mCopyTo, GetValue(), found, do_choice))
			return false;
		else
			mCachedMbox = found;
	}
	else
		found = mCachedMbox;
	return true;
}

// Get name of selected mailbox
bool CMailboxToolbarPopup::GetSelectedMboxName(cdstring& found, bool do_choice)
{
	return CCopyToMenu::GetPopupMboxName(GetPopupMenu(), mCopyTo, GetValue(), found, do_choice);
}

// Check items before doing popup
void CMailboxToolbarPopup::UpdateMenu()
{
	// Make sure everything is sync'd with actual mailbox list
	SyncMenu();
		
	// Do inherited
	CToolbarPopupButton::UpdateMenu();
}
