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


// CFavouriteItem : class to implement a favourite item

#ifndef __CFAVOURITEITEM__MULBERRY__
#define __CFAVOURITEITEM__MULBERRY__

#include "CDisplayItem.h"
#include "cdstring.h"
#include "prefsvector.h"

class CFavouriteItem : public CDisplayItem
{
public:
	CFavouriteItem()
		{ mVisible = true;}
	CFavouriteItem(const CFavouriteItem& copy)
		: CDisplayItem(copy), mItems(copy.mItems)
		{ mVisible = copy.mVisible; }
	CFavouriteItem(const char* name)
		: CDisplayItem(name)
		{ mVisible = true; }
	virtual ~CFavouriteItem() {}

	CFavouriteItem& operator=(const CFavouriteItem& copy);					// Assignment with same type
	
	bool IsVisible(void) const
		{ return mVisible; }
	void SetVisible(bool visible)
		{ mVisible = visible; }

	const cdstrpairvect& GetItems() const
		{ return mItems; }
	cdstrpairvect& GetItems()
		{ return mItems; }

	// Read/write prefs
	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	cdstrpairvect	mItems;
	bool			mVisible;
};

typedef prefsvector<CFavouriteItem> CFavouriteItemList;

#endif
