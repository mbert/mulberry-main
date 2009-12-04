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

#include "CMailAccountManager.h"
#include "CMbox.h"

#include <LControlImp.h>

const short cMaxMailbox = 300;
const SInt16	cMboxStartPos = 4;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P O P U P
// __________________________________________________________________________________________________

LArray CMailboxPopup::sMailboxPopupMenus(sizeof(CMailboxPopup*));

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMailboxPopup::CMailboxPopup(LStream *inStream)
		: LPopupButton(inStream)
{
	mCopyTo = true;
	mPickPopup = true;

	// Add to list
	CMailboxPopup* _this = this;
	sMailboxPopupMenus.InsertItemsAt(1, LArray::index_Last, &_this);
	mMissingMailbox = false;

	mCachedMbox = NULL;
}

// Default destructor
CMailboxPopup::~CMailboxPopup()
{
	// Remove from list
	CMailboxPopup* _this = this;
	sMailboxPopupMenus.Remove(&_this);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set to default item
void CMailboxPopup::FinishCreateSelf()
{
	LPopupButton::FinishCreateSelf();

	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	SetDefault();
}

void CMailboxPopup::SetCopyTo(bool copy_to)
{
	mCopyTo = copy_to;
	SetMacMenuH(CCopyToMenu::GetPopupMenuHandle(mCopyTo), false);
}

// Set to default item
void CMailboxPopup::SetDefault()
{
	mChosenName = cdstring::null_str;

	// Sync with menu
	SyncMenu();
}

// Delete mark after popup
void CMailboxPopup::SetValue(SInt32 inValue)
{
	// Reset cached mbox immediately before processing change in value
	mCachedMbox = NULL;

	// Set menu control value
	LPopupButton::SetValue(inValue);

	CCopyToMenu::GetPopupMboxName(mCopyTo, inValue, mChosenName, false);
}

Boolean CMailboxPopup::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always keep limits in sync with changes to menu
	SetMenuMinMax();

	// Must force reset (if required) before handling popup
	CCopyToMenu::ResetMenuList();

	
	// Pick popup must not store value to allow the last selected item to be selected again
	if (mPickPopup)
	{
		// Force value off
		SetMinValue(0);
		mValue = 0;
		mControlImp->SetValue(0);
	}

	// Do inherited
	return LPopupButton::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}

// Update menu to mailbox list
void CMailboxPopup::ResetMenuList(void)
{
	// Iterate over all menus and sync
	LArrayIterator iterator(sMailboxPopupMenus, LArrayIterator::from_Start);

	CMailboxPopup* aPopup;
	while(iterator.Next(&aPopup))
		aPopup->SyncMenu();
}

// Mailbox list changed
void CMailboxPopup::ChangeMenuList(void)
{
	// Iterate over all menus and sync
	LArrayIterator iterator(sMailboxPopupMenus, LArrayIterator::from_Start);

	CMailboxPopup* aPopup;
	while(iterator.Next(&aPopup))
		aPopup->mCachedMbox = NULL;
}

// Sync control to menu
void CMailboxPopup::SyncMenu(void)
{
	// Set limits from menu
	SetMenuMinMax();

	// Only bother if there's a name
	if (!mChosenName.empty())
	{
		short set_item = 2;	// Must set to Choose if no sync

		if (mChosenName != "\1")
		{
			short num_menu = ::CountMenuItems(GetMacMenuH());
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
		SetValue(set_item);
	}
	else
		// Set to none if already None
		SetValue(1);

	Refresh();
}

// Set name of selected mailbox
void CMailboxPopup::SetSelectedMbox(const cdstring& mbox_name, bool none, bool choose)
{
	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	// Default to Choose if UseCopyTo is on but mailbox name is empty,
	// otherwise use None
	short set_item = none ? 1 : 2;

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

					// Set limits from menu
					SetMenuMinMax();

					// Redo lookup
					set_item = CCopyToMenu::FindPopupMboxPos(mCopyTo, mbox_name.c_str());
				}
			}
		}
	}

	if (set_item > 0)
	{
		SetValue(set_item);

		mMissingMailbox = false;
		mMissingName = cdstring::null_str;
	}
	else
	{
		// Always set to Choose if previous is now missing
		SetValue(2);

		mMissingMailbox = true;
		mMissingName = mbox_name;
	}
}

// Get name of selected mailbox
bool CMailboxPopup::GetSelectedMbox(CMbox*& found, bool do_choice)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMbox(mCopyTo, mValue, found, do_choice))
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
bool CMailboxPopup::GetSelectedMboxSend(CMbox*& found, bool& set_as_default)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMboxSend(mCopyTo, mValue, found, set_as_default))
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
	return CCopyToMenu::GetPopupMboxName(mCopyTo, mValue, found, do_choice);
}