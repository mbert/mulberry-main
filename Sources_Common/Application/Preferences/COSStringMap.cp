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


// COSStringMap : class to implement a favourite item

#include "COSStringMap.h"

#include "char_stream.h"

extern const char* cOSKey;

#pragma mark ____________________________COSStringMap

const cdstring& COSStringMap::GetData() const
{
	// Look for OS key
	const_iterator found = find(cOSKey);
	if (found == end())
	{
		std::pair<iterator, bool> result = const_cast<COSStringMap*>(this)->insert(value_type(cOSKey, cdstring::null_str));
		found = result.first;
	}

	return (*found).second;
}

void COSStringMap::SetData(const cdstring& txt)
{
	// Try insert or replace
	std::pair<iterator, bool> found = insert(value_type(cOSKey, txt));
	if (!found.second)
		(*found.first).second = txt;
}

// Write items
cdstring COSStringMap::GetInfo() const
{
	cdstring sexpression;
	sexpression.CreateSExpression(*this);
	sexpression.ConvertFromOS();

	return sexpression;
}

// Read items
bool COSStringMap::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	cdstring::ParseSExpression(txt, *this, true);
	return true;
}
