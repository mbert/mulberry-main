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


// Search style data object

// Stores hierarchical search criteria that can be saved in prefs or applied as filter

#ifndef __CSEARCHSTYLE__MULBERRY__
#define __CSEARCHSTYLE__MULBERRY__

#include "cdstring.h"
#include "CSearchItem.h"

#include "prefsvector.h"

// CSearchStyle

class CSearchStyle
{
public:
	CSearchStyle()
		{ mItem = nil; }
	CSearchStyle(const cdstring& name)
		{ mName = name; mItem = nil; }
	CSearchStyle(const cdstring& name, CSearchItem* item)
		{ mName = name; mItem = item; }
	CSearchStyle(const CSearchStyle& copy);					// Copy construct
	~CSearchStyle()
		{ _tidy(); }

	CSearchStyle& operator=(const CSearchStyle& copy);				// Assignment with same type
	int operator==(const CSearchStyle& other) const;				// Compare with same type
	
	// Getters & Setters
	const cdstring& GetName() const
		{ return mName; }
	void SetName(const cdstring& name)
		{ mName = name; }
	
	const CSearchItem* GetSearchItem() const
		{ return mItem; }
	void SetSearchItem(CSearchItem* item)
		{ delete mItem; mItem = item; }

	// Serialize
	bool SetInfo(char_stream& info, NumVersion vers_prefs);			// Parse S-Expression element
	cdstring GetInfo(void) const;								// Create S_Expression element
	
protected:
	cdstring mName;			// Friendly name of style
	CSearchItem* mItem;		// Search item hierarchy for style

private:
	void _copy(const CSearchStyle& copy);
	void _tidy()
		{ delete mItem; mItem = NULL; }
};

class CSearchStyleList : public prefsptrvector<CSearchStyle>
{
public:
	CSearchStyleList() {}
	~CSearchStyleList() {}
	
	const CSearchStyle* FindStyle(const cdstring& name) const;		// Find named style
	long FindIndexOf(const cdstring& name) const;					// Find index of named style
};

#endif
