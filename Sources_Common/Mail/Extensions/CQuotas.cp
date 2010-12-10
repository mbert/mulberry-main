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


// Quota support

#include "CQuotas.h"

#include "CStringUtils.h"

#include <cstdlib>
#include <memory>

#pragma mark ____________________________CQuotaItem

// CQuotaItem: contains specific quota item

// Construct from text
CQuotaItem::CQuotaItem(const char* txt1, const char* txt2, const char* txt3)
{
	if (txt1)
		mItem = txt1;

	if (txt2)
		mCurrent = ::atol(txt2);
	else
		mCurrent = 0;

	if (txt3)
		mMax = ::atol(txt3);
	else
		mMax = 0;
}


// Copy construct
CQuotaItem::CQuotaItem(const CQuotaItem& copy)
{
	mItem = copy.GetItem();
	mCurrent = copy.GetCurrent();
	mMax = copy.GetMax();
}

// Assignment with same type
CQuotaItem& CQuotaItem::operator=(const CQuotaItem& copy)
{
	if (this != &copy)
	{
		mItem = copy.GetItem();
		mCurrent = copy.GetCurrent();
		mMax = copy.GetMax();
	}

	return *this;
}

#pragma mark ____________________________CQuotaRoot

// CQuotaRoot: contains list quota items

// Copy construct
CQuotaRoot::CQuotaRoot(const CQuotaRoot& copy)
{
	mName = copy.GetName();
	mItems = copy.GetItems();
}

// Assignment with same type
CQuotaRoot& CQuotaRoot::operator=(const CQuotaRoot& copy)
{
	if (this != &copy)
	{
		mName = copy.GetName();
		mItems = copy.GetItems();
	}

	return *this;
}

// Parse list of items from server
void CQuotaRoot::ParseList(const char* txt)
{
	// Dump existing items
	mItems.clear();
	if (txt == NULL)
		return;

	// Duplicate for parsing
	std::auto_ptr<char> dup(::strdup(txt));
	char* p = dup.get();

	while(p && *p)
	{
		// Parse three strings
		char* item = ::strgetquotestr(&p);
		if (!item) break;
		char* current = ::strgetquotestr(&p);
		if (!current) break;
		char* max = ::strgetquotestr(&p);
		if (!current) break;

		// Add to list
		mItems.push_back(CQuotaItem(item, current, max));
	}
}
