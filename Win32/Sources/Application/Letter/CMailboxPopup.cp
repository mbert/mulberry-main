/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CIMAPClient.h"
#include "CMbox.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerWindow.h"

const short cMaxMailbox = 300;
const short	cMboxStartPos = 3;

IMPLEMENT_DYNCREATE(CMailboxPopup, CPopupButton)

BEGIN_MESSAGE_MAP(CMailboxPopup, CPopupButton)
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P O P U P
// __________________________________________________________________________________________________

CMailboxPopup::CMailboxPopupList CMailboxPopup::sMailboxPopupMenus;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxPopup::CMailboxPopup()
{
	mCopyTo = true;

	// Add to list
	sMailboxPopupMenus.push_back(this);
	mMissingMailbox = false;
	
	mCachedMbox = NULL;
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
	CCopyToMenu::ResetMenuList();

	// Sync with menu
	SyncMenu();
}

// Update menu to mailbox list
void CMailboxPopup::ResetMenuList()
{
	// Iterate over all menus and sync
	for(CMailboxPopupList::iterator iter = sMailboxPopupMenus.begin(); iter != sMailboxPopupMenus.end(); iter++)
		(*iter)->SyncMenu();
}

// Mailbox list change
void CMailboxPopup::ChangeMenuList()
{
	// Iterate over all menus and sync
	for(CMailboxPopupList::iterator iter = sMailboxPopupMenus.begin(); iter != sMailboxPopupMenus.end(); iter++)
		(*iter)->mCachedMbox = NULL;
}

// Delete mark after popup
void CMailboxPopup::SetValue(UINT value)
{
	// Reset cached mbox immediately before processing change in value
	mCachedMbox = NULL;

	CPopupButton::SetValue (value);
	
	CCopyToMenu::GetPopupMboxName(mCopyTo, value - (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone), mChosenName, false);
}

void CMailboxPopup::SetupCurrentMenuItem(bool check)
{
	// Force reset of menus - will only be done if dirty
	CCopyToMenu::ResetMenuList();

	// Do inherited
	CPopupButton::SetupCurrentMenuItem(check);
}

// Set the menu items from the various lists
void CMailboxPopup::SyncMenu()
{
	CMenu* pPopup = GetPopupMenu();

	// Only bother if there's a name
	if (!mChosenName.empty())
	{
		short set_item = 1;	// Must set to Choose if no sync

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
		SetValue(set_item + (mCopyTo ? IDM_CopyToPopupNone : IDM_AppendToPopupNone));
	}
	else
		// Set to none if already None
		SetValue(mCopyTo ? IDM_CopyToPopupNone : IDM_AppendToPopupNone);
}

// Set name of selected mailbox
void CMailboxPopup::SetSelectedMbox(const cdstring& mbox_name, bool none, bool choose)
{
	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	// Default to Choose if UseCopyTo is on but mailbox name is empty,
	// otherwise use None
	short set_item = none ? 0 : 1;

	// Check for 'None'
	if (!none && !choose && !mbox_name.empty())
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
		SetValue(set_item + (mCopyTo ? IDM_CopyToPopupNone : IDM_AppendToPopupNone));

		mMissingMailbox = false;
		mMissingName = cdstring::null_str;
	}
	else
	{
		// Always set to Choose if previous is now missing
		SetValue((mCopyTo ? IDM_CopyToPopupNone : IDM_AppendToPopupNone) + 1);

		mMissingMailbox = true;
		mMissingName = mbox_name;
	}
}

// Get name of selected mailbox
bool CMailboxPopup::GetSelectedMbox(CMbox*& found, bool do_choice)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMbox(mCopyTo, mValue - (mCopyTo ? IDM_CopyToPopupNone : IDM_AppendToPopupNone), found, do_choice))
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
bool CMailboxPopup::GetSelectedMboxSend(CMbox*& found, bool& set_as_default)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMboxSend(mCopyTo, mValue - (mCopyTo ? IDM_CopyToPopupNone : IDM_AppendToPopupNone), found, set_as_default))
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
bool CMailboxPopup::GetSelectedMboxName(cdstring& found, bool do_choice)
{
	return CCopyToMenu::GetPopupMboxName(mCopyTo, mValue - (mCopyTo ? IDM_CopyToPopupChoose : IDM_AppendToPopupNone), found, do_choice);
}
