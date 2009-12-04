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


// Source for CSimpleTitleTable class


#include "CSimpleTitleTable.h"

#include "CMulberryCommon.h"
#include "CXStringResources.h"

void CSimpleTitleTable::LoadTitles(const char* rsrcid, unsigned long count, bool clear_existing)
{
	if (clear_existing)
		mTitles.clear();

	for(unsigned long i = 0; i < count; i++)
	{
		mTitles.push_back(rsrc::GetIndexedString(rsrcid, i));
	}
}

void CSimpleTitleTable::AddTitle(const cdstring& title)
{
	mTitles.push_back(title);
}

void CSimpleTitleTable::ChangeTitle(TableIndexT col, const cdstring& title)
{
	if (col <= mTitles.size())
	{
		mTitles.at(col - 1) = title;
		RefreshCell(STableCell(1, col));
	}
}
