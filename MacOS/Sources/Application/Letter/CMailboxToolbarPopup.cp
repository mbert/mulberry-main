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
#include "CToolbarButtonImp.h"

const short cMaxMailbox = 300;
const SInt16	cMboxStartPos = 3;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X P O P U P
// __________________________________________________________________________________________________

LArray CMailboxToolbarPopup::sMailboxToolbarPopupMenus(sizeof(CMailboxToolbarPopup*));

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CMailboxToolbarPopup::CMailboxToolbarPopup(LStream *inStream)
		: CToolbarButton(inStream)
{
	mCopyTo = true;
	
	// Make sure chosen value is checked off
	mValueMenu = true;

	// Add to list
	CMailboxToolbarPopup* _this = this;
	sMailboxToolbarPopupMenus.InsertItemsAt(1, LArray::index_Last, &_this);
	mMissingMailbox = false;

	mCachedMbox = NULL;

	// Make it a click-and-ppup control
	static_cast<CToolbarButtonImp*>(mControlImp)->SetClickAndPopup(true);
}

// Constructor
CMailboxToolbarPopup::CMailboxToolbarPopup(
		const SPaneInfo	&inPaneInfo,
		MessageT		inValueMessage,
		SInt16			inBevelProc,
		ResIDT			inMenuID,
		SInt16			inMenuPlacement,
		SInt16			inContentType,
		SInt16			inContentResID,
		ResIDT			inTextTraits,
		ConstStringPtr	inTitle,
		SInt16			inInitialValue,
		SInt16			inTitlePlacement,
		SInt16			inTitleAlignment,
		SInt16			inTitleOffset,
		SInt16			inGraphicAlignment,
		Point			inGraphicOffset,
		Boolean			inCenterPopupGlyph)
	: CToolbarButton(inPaneInfo, inValueMessage, inBevelProc, inMenuID, inMenuPlacement, inContentType, inContentResID, inTextTraits,
					inTitle, inInitialValue, inTitlePlacement, inTitleAlignment, inTitleOffset, inGraphicAlignment, inGraphicOffset, inCenterPopupGlyph)
{
	mCopyTo = true;

	// Make sure chosen value is checked off
	mValueMenu = true;

	// Add to list
	CMailboxToolbarPopup* _this = this;
	sMailboxToolbarPopupMenus.InsertItemsAt(1, LArray::index_Last, &_this);
	mMissingMailbox = false;

	mCachedMbox = NULL;

	// Make it a click-and-ppup control
	static_cast<CToolbarButtonImp*>(mControlImp)->SetClickAndPopup(true);
}

// Default destructor
CMailboxToolbarPopup::~CMailboxToolbarPopup()
{
	// Remove from list
	CMailboxToolbarPopup* _this = this;
	sMailboxToolbarPopupMenus.Remove(&_this);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Set to default item
void CMailboxToolbarPopup::FinishCreateSelf()
{
	CToolbarButton::FinishCreateSelf();

	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	// Always start with Choose Later
	SetMenuValue(1, true);

	SetDefault();
}

void CMailboxToolbarPopup::SetCopyTo(bool copy_to)
{
	mCopyTo = copy_to;
	SetMacMenuH(CCopyToMenu::GetPopupMenuHandle(mCopyTo), false);
}

// Set to default item
void CMailboxToolbarPopup::SetDefault()
{
	mChosenName = cdstring::null_str;

	// Sync with menu
	SyncMenu();
}

// Delete mark after popup
void CMailboxToolbarPopup::SetMenuValue(SInt32 inValue, bool silent)
{
	// Reset cached mbox immediately before processing change in value
	mCachedMbox = NULL;

	// Do this before setting the value as the menu item list may change
	// as a result of the copy causing the MRU list to change
	if (inValue > 0)
		CCopyToMenu::GetPopupMboxName(mCopyTo, inValue, mChosenName, false);
	
	// Set menu control value
	CToolbarButton::SetMenuValue(inValue, silent);
}

Boolean CMailboxToolbarPopup::TrackHotSpot(SInt16 inHotSpot, Point inPoint, SInt16 inModifiers)
{
	// Always keep limits in sync with changes to menu
	SetMenuMinMax();

	// Must force reset (if required) before handling popup
	CCopyToMenu::ResetMenuList();

	// Do inherited
	return LBevelButton::TrackHotSpot(inHotSpot, inPoint, inModifiers);
}

// Update menu to mailbox list
void CMailboxToolbarPopup::ResetMenuList(void)
{
	// Iterate over all menus and sync
	LArrayIterator iterator(sMailboxToolbarPopupMenus, LArrayIterator::from_Start);

	CMailboxToolbarPopup* aPopup;
	while(iterator.Next(&aPopup))
		aPopup->SyncMenu();
}

// Mailbox list changed
void CMailboxToolbarPopup::ChangeMenuList(void)
{
	// Iterate over all menus and sync
	LArrayIterator iterator(sMailboxToolbarPopupMenus, LArrayIterator::from_Start);

	CMailboxToolbarPopup* aPopup;
	while(iterator.Next(&aPopup))
		aPopup->mCachedMbox = NULL;
}

// Sync control to menu
void CMailboxToolbarPopup::SyncMenu(void)
{
	// Set limits from menu
	SetMenuMinMax();

	// Only bother if there's a name
	if (!mChosenName.empty())
	{
		short set_item = 1;	// Must set to Choose if no sync

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
		SetMenuValue(set_item, true);
	}
	else
		// Set to choose
		SetMenuValue(1, true);

	Refresh();
}

// Set name of selected mailbox
void CMailboxToolbarPopup::SetSelectedMbox(const cdstring& mbox_name, bool choose)
{
	// Must make sure menu is up to date before sync'ing
	CCopyToMenu::ResetMenuList();

	// Default to Choose
	short set_item = 1;

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
		SetMenuValue(set_item, true);

		mMissingMailbox = false;
		mMissingName = cdstring::null_str;
	}
	else
	{
		// Always set to Choose if previous is now missing
		SetMenuValue(1, true);

		mMissingMailbox = true;
		mMissingName = mbox_name;
	}
}

// Get name of selected mailbox
bool CMailboxToolbarPopup::GetSelectedMbox(CMbox*& found, bool do_choice)
{
	if (!mCachedMbox)
	{
		if (!CCopyToMenu::GetPopupMbox(mCopyTo, mMenuChoice, found, do_choice))
			return false;
		
		// Cache the mailbox if it was not done by Choose...
		else if (mMenuChoice != 1)
			mCachedMbox = found;
	}
	else
		found = mCachedMbox;
	return true;
}

// Get name of selected mailbox
bool CMailboxToolbarPopup::GetSelectedMboxName(cdstring& found, bool do_choice)
{
	return CCopyToMenu::GetPopupMboxName(mCopyTo, mMenuChoice, found, do_choice);
}