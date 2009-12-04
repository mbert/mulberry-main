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

#include "CDisplayItem.h"

#include "char_stream.h"

extern const char* cSpace;
extern const char* cValueBoolTrue;
extern const char* cValueBoolFalse;

#pragma mark ____________________________CDisplayItem

// Assignment with same type
void CDisplayItem::_copy(const CDisplayItem& copy)
{
	mName = copy.mName;
	mHierarchic = copy.mHierarchic;
	mExpanded = copy.mExpanded;
}

// Write prefs
cdstring CDisplayItem::GetInfo(void) const
{
	cdstring all;
	cdstring temp;
	temp = mName;
	temp.quote();
	temp.ConvertFromOS();
	all += temp;
	all += cSpace;

	all += (mHierarchic ? cValueBoolTrue : cValueBoolFalse);
	all += cSpace;

	all += (mExpanded ? cValueBoolTrue : cValueBoolFalse);

	return all;
}

// Read prefs
bool CDisplayItem::SetInfo(char_stream& txt, NumVersion vers_prefs)
{
	txt.get(mName, true);
	txt.get(mHierarchic);
	txt.get(mExpanded);

	return true;
}
