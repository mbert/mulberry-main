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


// Source for CMailboxPopup class


#include "CMailboxPopup.h"

#include "CMbox.h"
#include "CMailAccountManager.h"

#include <algorithm>

const short cMaxMailbox = 300;
const short	cMboxStartPos = 3;


// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P O P U P
// __________________________________________________________________________________________________

CMailboxPopup::CMailboxPopupList CMailboxPopup::sMailboxPopupMenus;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxPopup::CMailboxPopup(JXContainer* enclosure,
							 const HSizingOption hSizing,
							 const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h,
							 bool copy_to)
	: HPopupMenu("", enclosure, hSizing, vSizing, x, y, w, h), mCopyTo(copy_to)
{
	CommonConstruct(enclosure, hSizing, vSizing, x, y, w, h);
}

CMailboxPopup::CMailboxPopup(bool copy_to,
							 JXContainer* enclosure,
							 const HSizingOption hSizing,
							 const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h)
	: HPopupMenu("", enclosure, hSizing, vSizing, x, y, w, h), mCopyTo(copy_to)
{
	CommonConstruct(enclosure, hSizing, vSizing, x, y, w, h);
}

void CMailboxPopup::CommonConstruct(JXContainer* enclosure,
									 const HSizingOption hSizing,
									 const VSizingOption vSizing,
									 const JCoordinate x, const JCoordinate y,
									 const JCoordinate w, const JCoordinate h)
{
	// Add to list
	sMailboxPopupMenus.push_back(this);
	mMissingMailbox = false;
	
	mCachedMbox = NULL;

	// Let JX know this is a popup menu
	SetValue(1);
	SetUpdateAction(kDisableNone);

	// Force reset of menus - will only be done if dirty
	CCopyToMenu::ResetMenuList(NULL, NULL);
	SetMenuItems(CCopyToMenu::GetPopupMenu(mCopyTo).c_str());
	mNeedsSync = false;
}

// Default destructor
CMailboxPopup::~CMailboxPopup()
{
	// Remove from list
	CMailboxPopupList::iterator found = std::find(sMailboxPopupMenus.begin(), sMailboxPopupMenus.end(), this);
	if (found != sMailboxPopupMenus.end())
		sMailboxPopupMenus.erase(found);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set to default item
void CMailboxPopup::SetDefault()
{
	mChosenName = cdstring::null_str;

	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList(NULL, NULL);

	// Sync with menu
	SyncMenu();
}

// Update menu to mailbox list
void CMailboxPopup::ResetMenuList()
{
	// Iterate over all menus and sync
	for(CMailboxPopupList::iterator iter = sMailboxPopupMenus.begin(); iter != sMailboxPopupMenus.end(); iter++)
		(*iter)->SetNeedsSync();
}

// Mailbox list change
void CMailboxPopup::ChangeMenuList(void)
{
	// Iterate over all menus and sync
	for(CMailboxPopupList::iterator iter = sMailboxPopupMenus.begin(); iter != sMailboxPopupMenus.end(); iter++)
		(*iter)->mCachedMbox = NULL;
}

// Delete mark after popup
void CMailboxPopup::SetValue(JIndex value)
{
	// Reset cached mbox immediately before processing change in value
	mCachedMbox = NULL;

	HPopupMenu::SetValue(value);
	
	CCopyToMenu::GetPopupMboxName(this, mCopyTo, value, mChosenName, false);
}


// Set the menu items from the various lists
void CMailboxPopup::SyncMenu()
{
	// Only if required
	if (!NeedsSync())
		return;

	// Update our menu items to match CCopyToMenu
	SetMenuItems(CCopyToMenu::GetPopupMenu(mCopyTo).c_str());

	// Only bother if there's a name
	if (!mChosenName.empty())
	{
		short set_item = mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendChoose;// Must set to Choose if no sync

		if (mChosenName != "\1")
		{
			short num_menu =GetItemCount();
			for(short i = cMboxStartPos; i <= num_menu; i++)
			{
				// Compare with current text value
				cdstring mbox_name;
				if (CCopyToMenu::GetPopupMboxName(this, mCopyTo, i, mbox_name) 
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
void CMailboxPopup::SetSelectedMbox(const cdstring& mbox_name, bool none, bool choose)
{
	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList(NULL, NULL);

	// Default to Choose if UseCopyTo is on but mailbox name is empty,
	// otherwise use None
	short set_item = none ? (mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendNone) :
						(mCopyTo ? CCopyToMenu::cPopupCopyChoose : CCopyToMenu::cPopupAppendChoose);

	// Check for 'None'
	if (!none && !choose && !mbox_name.empty())
	{

		// Fetch mbox at
		set_item = CCopyToMenu::FindPopupMboxPos(this, mCopyTo, mbox_name.c_str());
		
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
					set_item = CCopyToMenu::FindPopupMboxPos(this, mCopyTo, mbox_name.c_str());
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

	// Always broadcast this change
	BroadcastSelection(mValue, kJFalse);
}

// Get name of selected mailbox
bool CMailboxPopup::GetSelectedMbox(CMbox*& found, bool do_choice)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMbox(this, mCopyTo, GetValue(), found, do_choice))
			return false;
		else
			mCachedMbox = found;
	}
	else
		found = mCachedMbox;
	return true;
}

// Get name of selected mailbox
bool CMailboxPopup::GetSelectedMboxSend(CMbox*& found, bool& set_as_default)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMboxSend(this, mCopyTo, GetValue(), found, set_as_default))
			return false;

		// Cache the mailbox if it was not done by Choose...
		else if (mValue != 2)
			mCachedMbox = found;
	}
	else
		found = mCachedMbox;
	return true;
}

// Get name of selected mailbox
bool CMailboxPopup::GetSelectedMboxName(cdstring& found, bool do_choice)
{
	return CCopyToMenu::GetPopupMboxName(this, mCopyTo, GetValue(), found, do_choice);
}

void CMailboxPopup::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == (JBroadcaster*)(this) && message.Is(JXMenu::kNeedsUpdate))
	{
		CCopyToMenu::ResetMenuList(NULL, NULL);
		SyncMenu();
	    CheckItem(mValue);
	}
	else
		HPopupMenu::Receive(sender, message);
}

