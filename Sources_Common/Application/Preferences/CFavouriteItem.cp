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

#include "CFavouriteItem.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CFavouriteItem

// Assignment with same type
CFavouriteItem& CFavouriteItem::operator=(const CFavouriteItem& copy)
{
	if (this != &copy)
	{
		CDisplayItem::operator=(copy);
		mItems = copy.mItems;

		mVisible = copy.mVisible;
	}

	return *this;
}

// Write prefs
cdstring CFavouriteItem::GetInfo(void) const
{
	cdstring all;
	cdstring temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += (mHierarchic ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mExpanded ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mVisible ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	cdstring sexpression;
	sexpression.CreateSExpression(mItems);
	sexpression.ConvertFromOS();

	all += sexpression;

	return all;
}

// Read prefs
bool CFavouriteItem::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mName, true);
	txt.get(mHierarchic);
	txt.get(mExpanded);
	txt.get(mVisible);

	cdstring::ParseSExpression(txt, mItems, true);

	return true;
}
