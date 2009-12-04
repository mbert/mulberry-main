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

#include "CXStringResources.h"

// __________________________________________________________________________________________________
// C L A S S __ C T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSimpleTitleTable::CSimpleTitleTable()
{
}

// Constructor from stream
CSimpleTitleTable::CSimpleTitleTable(LStream *inStream)
	: CTitleTable(inStream)
{
}

// Default destructor
CSimpleTitleTable::~CSimpleTitleTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSimpleTitleTable::FinishCreateSelf(void)
{
	// Do inherited
	SetTitleInfo(false, false, 0, 0, 0);
	CTitleTable::FinishCreateSelf();

	// Make it fit to the superview
	InsertCols(1, 1, nil, 0, false);
	InsertRows(1, 1, nil, 0, false);
	AdaptToNewSurroundings();
}

// Get menu
void CSimpleTitleTable::InitColumnChanger(void)
{
	mColumnChanger = NULL;
}

// Handle choice
void CSimpleTitleTable::MenuChoice(short col, bool sort_col, short menu_item)
{
}

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
