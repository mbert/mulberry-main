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

#include "CSummaryTitleTable.h"

#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CSummaryTable.h"
#include "CXStringResources.h"

BEGIN_MESSAGE_MAP(CSummaryTitleTable, CSimpleTitleTable)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CSummaryTitleTable														  [public]
/**
	Default constructor */

CSummaryTitleTable::CSummaryTitleTable()
{
}


// ---------------------------------------------------------------------------
//	~CSummaryTitleTable														  [public]
/**
	Destructor */

CSummaryTitleTable::~CSummaryTitleTable()
{
}

#pragma mark -

int CSummaryTitleTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSimpleTitleTable::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Setup titles
	LoadTitles("UI::Titles::CalendarSummary", 6);
	
	return 0;
}

void CSummaryTitleTable::TableChanged()
{
	// Only if we have a table
	if (mTable == NULL)
		return;

	// Sync column count
	TableIndexT rows;
	TableIndexT cols;
	mTable->GetTableSize(rows, cols);
	if (mCols != cols)
	{
		// Add/remove required amount
		if (mCols > cols)
			RemoveCols(mCols - cols, 2, false);
		else
			InsertCols(cols - mCols, mCols, NULL);
	}
	
	// Sync column widths
	for(TableIndexT col = 1; col <= mCols; col++)
		SetColWidth(mTable->GetColWidth(col), col, col);
}
