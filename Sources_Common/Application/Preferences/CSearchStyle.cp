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

#include "CSearchStyle.h"

#include "char_stream.h"

#pragma mark ____________________________CSearchStyle

extern const char* cSpace;

// Copy construct
CSearchStyle::CSearchStyle(const CSearchStyle& copy)
{
	mItem = nil;
	_copy(copy);
}

// Assignment with same type
CSearchStyle& CSearchStyle::operator=(const CSearchStyle& copy)
{
	if (this != &copy)
	{
		_tidy();
		_copy(copy);
	}

	return *this;
}

// Compare with same type
int CSearchStyle::operator==(const CSearchStyle& other) const
{
	// Just compare names
	return mName == other.mName;
}

void CSearchStyle::_copy(const CSearchStyle& copy)
{
	mName = copy.mName;
	mItem = new CSearchItem(*copy.mItem);
}

// Parse S-Expression element
bool CSearchStyle::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	bool result = true;

	txt.get(mName, true);

	mItem = new CSearchItem;
	mItem->SetInfo(txt);

	return result;
}

// Create S_Expression element
cdstring CSearchStyle::GetInfo(void) const
{
	cdstring all;
	cdstring temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += mItem->GetInfo();

	return all;
}

#pragma mark ____________________________CSearchStyleList

// Find named style
const CSearchStyle* CSearchStyleList::FindStyle(const cdstring& name) const
{
	CSearchStyle temp(name);
	CSearchStyleList::const_iterator found = begin();
	for(; found != end(); found++)
	{
		if (**found == temp)
			break;
	}

	if (found != end())
		return *found;
	else
		return nil;
}

// Find index of named style
long CSearchStyleList::FindIndexOf(const cdstring& name) const
{
	CSearchStyle temp(name);
	CSearchStyleList::const_iterator found = begin();
	for(; found != end(); found++)
	{
		if (**found == temp)
			break;
	}

	if (found != end())
		return found - begin();
	else
		return -1;
}
