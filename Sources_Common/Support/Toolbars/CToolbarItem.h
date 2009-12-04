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


// Header for CToolbarItem class

#ifndef __CTOOLBARITEM__MULBERRY__
#define __CTOOLBARITEM__MULBERRY__

//typedef	CommandT CCommandID;

#include "cdstring.h"
#include "ptrvector.h"

// Classes
class CToolbarItem
{
public:
	class CToolbarItemInfo
	{
	public:
		CToolbarItemInfo(const CToolbarItem* item, const cdstring& extra = cdstring::null_str)
		{
			mItem = item;
			mExtraInfo = extra;
		}
		CToolbarItemInfo(const CToolbarItemInfo& copy)
			{ _copy(copy); }
		~CToolbarItemInfo() {}

		CToolbarItemInfo& operator=(const CToolbarItemInfo& copy)					// Assignment with same type
			{ if (this != &copy) _copy(copy); return *this; }
		
		const CToolbarItem* GetItem() const
			{ return mItem; }

		const cdstring& GetExtraInfo() const
			{ return mExtraInfo; }
		void SetExtraInfo(const cdstring& extra)
			{ mExtraInfo = extra; }

	private:
		const CToolbarItem*	mItem;
		cdstring			mExtraInfo;

		void _copy(const CToolbarItemInfo& copy)
		{ mItem = copy.mItem;
		  mExtraInfo = copy.mExtraInfo; }
	};

	typedef ptrvector<CToolbarItem> CToolbarItems;
	typedef std::vector<CToolbarItemInfo> CToolbarPtrItems;

	enum EItemType
	{
		eNone = 0,
		eSeparator,
		eSpace,
		eExpandSpace,
		ePushButton,
		eToggleButton,
		ePopupButton,
		ePopupPushButton,
		ePopupToggleButton,
		ePopupMenu,
		eStaticText,
		eEditText,
		eStaticIcon,
		
		// Special-case items
		eCabinetButton,
		eCopyToButton,
		eSelectButton,
		eSMTPAccountPopup
	};

	struct SToolbarItem
	{
		EItemType		mType;
		unsigned long	mTitleID;
		bool			mDynamicTitle;
		unsigned long	mIconID;
		bool			mToggleIcon;
		unsigned long	mCommand;
		unsigned long	mAltCommand;
		unsigned long	mMenuCommand;
		unsigned long	mMenuAltCommand;
		unsigned long	mPopupMenu;
	};

	CToolbarItem(const SToolbarItem& details)
		: mType(details.mType), mTitleID(details.mTitleID), mDynamicTitle(details.mDynamicTitle), mIconID(details.mIconID), mToggleIcon(details.mToggleIcon),
			mCommand(details.mCommand), mAltCommand(details.mAltCommand), mMenuCommand(details.mMenuCommand), mMenuAltCommand(details.mMenuAltCommand), mPopupMenu(details.mPopupMenu) {}
	CToolbarItem(const CToolbarItem& copy)
		{ _copy(copy); }
	~CToolbarItem() {}

	CToolbarItem& operator=(const CToolbarItem& copy)					// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }
	
	EItemType GetType() const
		{ return mType; }
	unsigned long GetTitleID() const
		{ return mTitleID; }
	bool IsDynamicTitle() const
		{ return mDynamicTitle; }
	unsigned long GetIconID() const
		{ return mIconID; }
	bool IsToggleIcon() const
		{ return mToggleIcon; }
	unsigned long GetCommand() const
		{ return mCommand; }
	unsigned long GetAltCommand() const
		{ return mAltCommand; }
	unsigned long GetMenuCommand() const
		{ return mMenuCommand; }
	unsigned long GetMenuAltCommand() const
		{ return mMenuAltCommand; }
	unsigned long GetPopupMenu() const
		{ return mPopupMenu; }

protected:
	EItemType		mType;
	unsigned long	mTitleID;
	bool			mDynamicTitle;
	unsigned long	mIconID;
	bool			mToggleIcon;
	unsigned long	mCommand;
	unsigned long	mAltCommand;
	unsigned long	mMenuCommand;
	unsigned long	mMenuAltCommand;
	unsigned long	mPopupMenu;

private:	
	void _copy(const CToolbarItem& copy)
	{ mType = copy.mType;
	  mTitleID = copy.mTitleID;
	  mDynamicTitle = copy.mDynamicTitle;
	  mIconID = copy.mIconID;
	  mToggleIcon = copy.mToggleIcon;
	  mCommand = copy.mCommand;
	  mAltCommand = copy.mAltCommand;
	  mMenuCommand = copy.mMenuCommand;
	  mMenuAltCommand = copy.mMenuAltCommand;
	  mPopupMenu = copy.mPopupMenu; }
};

#endif
