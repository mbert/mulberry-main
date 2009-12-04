/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CFreeBusyTitleTable.h"

#include "CCalendarUtils.h"
#include "CFreeBusyTable.h"
#include "CDrawUtils.h"
#include "CMulberryCommon.h"

#include "CICalendarLocale.h"

#include "StPenState.h"

#include <JPainter.h>
#include <JXColormap.h>

// ---------------------------------------------------------------------------
//	CFreeBusyTitleTable														  [public]
/**
	Default constructor */

CFreeBusyTitleTable::CFreeBusyTitleTable(JXScrollbarSet* scrollbarSet,
				     JXContainer* enclosure,
				     const HSizingOption hSizing, 
				     const VSizingOption vSizing,
				     const JCoordinate x, const JCoordinate y,
				     const JCoordinate w, const JCoordinate h)
	: CSimpleTitleTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mTable = NULL;
}


// ---------------------------------------------------------------------------
//	~CFreeBusyTitleTable														  [public]
/**
	Destructor */

CFreeBusyTitleTable::~CFreeBusyTitleTable()
{
}

#pragma mark -

void CFreeBusyTitleTable::OnCreate()
{
	CSimpleTitleTable::OnCreate();

	//SetRowHeight(GetApertureHeight(), 1, 1);

	ApertureResized(0, 0);
}

void CFreeBusyTitleTable::TableChanged()
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
	SetColWidth(mTable->GetColWidth(1), 1, 1);
	SetColWidth(mTable->GetColWidth(2), 2, mCols);

	// Reset titles
	mTitles.clear();
	Refresh();
}

void CFreeBusyTitleTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect &cellRect)
{
	StPenState save(pDC);

	// Draw frame
	JRect actual_rect = cellRect;
	actual_rect.bottom--;
	CDrawUtils::DrawSimpleBorder(*pDC, cellRect, false, true);

	// cache suitable titles (make sure font is set first)
	if (mTitles.empty())
		GenerateTitles(pDC);
	
	cdstring txt(mTitles.at(inCell.col - 1));
	JFontStyle text_style = pDC->GetFontStyle();
	text_style.color = pDC->GetColormap()->GetBlackColor();
	pDC->SetFontStyle(text_style);

	::DrawClippedStringUTF8(pDC, txt, JPoint(cellRect.left + 2, cellRect.top + 1), cellRect, eDrawString_Left);
}

void CFreeBusyTitleTable::GenerateTitles(JPainter* pDC)
{
	// Clear existing titles
	mTitles.clear();
	
	// Determine format for title
	
	// Try the following formats (but localised) to see what fits the column width:
	// <<full day name>>, <<full month>> <<day number>>
	// <<short day name>>, <<short month>> <<day number>>
	// <<month number>>/<<day number>>
	// 
	
	// Fill each string
	mTitles.clear();
	STableCell cell(2, 2);
	for(cell.col = 2; cell.col <= mCols; cell.col++)
	{
		iCal::CICalendarDateTime dt = mTable->GetTimedStartDate(cell.col).GetAdjustedTime();
		
		mTitles.push_back(dt.GetTime(false, !iCal::CICalendarLocale::Use24HourTime()));
	}
	
	// Always add empty first item
	cdstring temp;
	mTitles.insert(mTitles.begin(), temp);
}
