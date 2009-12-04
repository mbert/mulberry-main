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


// CDisplayItem : class to implement a favourite item

#ifndef __CDISPLAYITEM__MULBERRY__
#define __CDISPLAYITEM__MULBERRY__

#include "cdstring.h"
#include "prefsvector.h"

class CDisplayItem
{
public:
	CDisplayItem()
		{ mHierarchic = false; mExpanded = false; }
	CDisplayItem(const CDisplayItem& copy)
		{ _copy(copy); }
	CDisplayItem(const char* name)
		{ mName = name; mHierarchic = true; mExpanded = false; }
	virtual ~CDisplayItem() {}
	
	CDisplayItem& operator=(const CDisplayItem& copy)					// Assignment with same type
		{ if (this != &copy) _copy(copy); return *this; }

	int operator==(const CDisplayItem& comp) const						// Compare with same type
		{ return (mName == comp.mName) &&
					(mHierarchic == comp.mHierarchic) &&
					(mExpanded == comp.mExpanded); }

	const cdstring& GetName(void) const
		{ return mName; }
	void SetName(const char* name)
		{ mName = name; }
	
	bool IsHierarchic(void) const
		{ return mHierarchic; }
	void SetHierarchic(bool hier)
		{ mHierarchic = hier; }

	bool IsExpanded(void) const
		{ return mExpanded; }
	void SetExpanded(bool expanded)
		{ mExpanded = expanded; }

	// Read/write prefs
	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

protected:
	cdstring	mName;
	bool		mHierarchic;
	bool		mExpanded;
	
	void _copy(const CDisplayItem& copy);
};

typedef prefsvector<CDisplayItem> CDisplayItemList;

#endif
