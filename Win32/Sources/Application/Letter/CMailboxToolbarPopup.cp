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

#include "CIMAPClient.h"
#include "CMbox.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerWindow.h"

const short cMaxMailbox = 300;
const short	cMboxStartPos = 2;

IMPLEMENT_DYNCREATE(CMailboxToolbarPopup, CToolbarPopupButton)

BEGIN_MESSAGE_MAP(CMailboxToolbarPopup, CToolbarPopupButton)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P O P U P
// __________________________________________________________________________________________________

CMailboxToolbarPopup::CMailboxToolbarPopupList CMailboxToolbarPopup::sMailboxToolbarPopupMenus;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxToolbarPopup::CMailboxToolbarPopup()
{
	mCopyTo = true;

	// Add to list
	sMailboxToolbarPopupMenus.push_back(this);
	mMissingMailbox = false;
	
	mCachedMbox = NULL;
	
	mPopupSetValue = true;

	// Always start with Choose Later
	SetValue(mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupChoose);
}

// Default destructor
CMailboxToolbarPopup::~CMailboxToolbarPopup()
{
	// Remove from list
	CMailboxToolbarPopupList::iterator found = ::find(sMailboxToolbarPopupMenus.begin(), sMailboxToolbarPopupMenus.end(), this);
	
	if (found != sMailboxToolbarPopupMenus.end())
		sMailboxToolbarPopupMenus.erase(found);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set to default item
void CMailboxToolbarPopup::SetDefault()
{
	mChosenName = cdstring::null_str;

	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	// Always start with Choose Later
	SetValue(mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupChoose);

	// Sync with menu
	SyncMenu();
}

// Update menu to mailbox list
void CMailboxToolbarPopup::ResetMenuList()
{
	// Iterate over all menus and sync
	for(CMailboxToolbarPopupList::iterator iter = sMailboxToolbarPopupMenus.begin(); iter != sMailboxToolbarPopupMenus.end(); iter++)
		(*iter)->SyncMenu();
}

// Mailbox list change
void CMailboxToolbarPopup::ChangeMenuList()
{
	// Iterate over all menus and sync
	for(CMailboxToolbarPopupList::iterator iter = sMailboxToolbarPopupMenus.begin(); iter != sMailboxToolbarPopupMenus.end(); iter++)
		(*iter)->mCachedMbox = NULL;
}

// Delete mark after popup
void CMailboxToolbarPopup::SetValue(UINT value)
{
	// Reset cached mbox immediately before processing change in value
	mCachedMbox = NULL;

	CToolbarPopupButton::SetValue(value);
	
	CCopyToMenu::GetPopupMboxName(mCopyTo, value - (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone), mChosenName, false);
}

void CMailboxToolbarPopup::SetupCurrentMenuItem(bool check)
{
	// Force reset of menus - will only be done if dirty
	CCopyToMenu::ResetMenuList();

	// Do inherited
	CToolbarPopupButton::SetupCurrentMenuItem(check);
}

// Set the menu items from the various lists
void CMailboxToolbarPopup::SyncMenu()
{
	CMenu* pPopup = GetPopupMenu();

	// Only bother if there's a name
	if (!mChosenName.empty())
	{
		short set_item = 0;	// Must set to Choose if no sync

		if (mChosenName != "\1")
		{
			short num_menu = pPopup->GetMenuItemCount();
			for(short i = cMboxStartPos; i <= num_menu; i++)
			{
				// Compare with current text value
				cdstring mbox_name;
				if (CCopyToMenu::GetPopupMboxName(mCopyTo, i, mbox_name) && (mChosenName == mbox_name))
				{
					set_item = i;
					break;
				}
			}
		}

		// Now set new value
		SetValue(set_item + (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone));
	}
	else
		// Set to none if already None
		SetValue(mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone);
}

// Set name of selected mailbox
void CMailboxToolbarPopup::SetSelectedMbox(const cdstring& mbox_name, bool choose)
{
	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	// Default to Choose
	short set_item = 0;

	// Check for 'None'
	if (!choose && !mbox_name.empty())
	{

		// Fetch mbox at
		set_item = CCopyToMenu::FindPopupMboxPos(mCopyTo, mbox_name.c_str());
		
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
					set_item = CCopyToMenu::FindPopupMboxPos(mCopyTo, mbox_name.c_str());
				}
			}
			
			// Extra flag of failure
			if (!set_item)
				set_item = -1;
		}
	}

	// Set menu to new value
	if (set_item >= 0)
	{
		SetValue(set_item + (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone));

		mMissingMailbox = false;
		mMissingName = cdstring::null_str;
	}
	else
	{
		// Always set to Choose if previous is now missing
		SetValue((mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone) + 1);

		mMissingMailbox = true;
		mMissingName = mbox_name;
	}
}

// Get name of selected mailbox
bool CMailboxToolbarPopup::GetSelectedMbox(CMbox*& found, bool do_choice)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMbox(mCopyTo, mValue - (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone), found, do_choice))
			return false;

		// Cache the mailbox if it was not done by Choose...
		else if ((mValue != IDM_CopyToPopupChoose) && (mValue != IDM_AppendToPopupChoose))
			mCachedMbox = found;
	}
	else
		found = mCachedMbox;
	return true;
}

// Get name of selected mailbox
bool CMailboxToolbarPopup::GetSelectedMboxName(cdstring& found, bool do_choice)
{
	return CCopyToMenu::GetPopupMboxName(mCopyTo, mValue - (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone), found, do_choice);
}
