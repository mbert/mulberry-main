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

BEGIN_MESSAGE_MAP(CSimpleTitleTable, CTitleTable)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C S I M P L E T I T L E T A B L E
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S


// Default constructor
CSimpleTitleTable::CSimpleTitleTable()
{
	mMenuID = 0;
}

// Default destructor
CSimpleTitleTable::~CSimpleTitleTable()
{
}

int CSimpleTitleTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetTitleInfo(false, false, 0, 0, 0);

	if (CTitleTable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Make it fit to the superview
	InsertCols(1, 1, nil, 0, false);

	return 0;
}

void CSimpleTitleTable::LoadTitles(const char* rsrcid, UINT count, bool clear_existing)
{
	if (clear_existing)
		mTitles.clear();

	for(UINT i = 0; i < count; i++)
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
