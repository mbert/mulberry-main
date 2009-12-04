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

#include "CDayWeekTitleTable.h"

#include "CCalendarUtils.h"
#include "CDayWeekTable.h"
#include "CDrawUtils.h"
#include "CMulberryCommon.h"

#include "StPenState.h"

#include <JPainter.h>
#include <JXColormap.h>

// ---------------------------------------------------------------------------
//	CDayWeekTitleTable														  [public]
/**
	Default constructor */

CDayWeekTitleTable::CDayWeekTitleTable(JXScrollbarSet* scrollbarSet,
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
//	~CDayWeekTitleTable														  [public]
/**
	Destructor */

CDayWeekTitleTable::~CDayWeekTitleTable()
{
}

#pragma mark -

void CDayWeekTitleTable::OnCreate()
{
	CSimpleTitleTable::OnCreate();

	//SetRowHeight(GetApertureHeight(), 1, 1);

	ApertureResized(0, 0);
}

void CDayWeekTitleTable::TableChanged()
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

void CDayWeekTitleTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	// Broadcast double-click
	STableCell cell(2, inCell.col);
	iCal::CICalendarDateTime dt(mTable->GetCellStartDate(cell));
	Broadcast_Message(eBroadcast_DblClick, &dt);
}

void CDayWeekTitleTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect &cellRect)
{
	StPenState save(pDC);

	// Look for selected column
	bool selected = false;
	if (mTable != NULL)
	{
		STableCell selCell = mTable->GetFirstSelectedCell();
		selected = !selCell.IsNullCell() && (selCell.col > 0) && (inCell.col == selCell.col);
	}

	// Draw frame
	JRect actual_rect = cellRect;
	actual_rect.bottom--;
	if (selected)
	{
		CDrawUtils::DrawBackground(*pDC, actual_rect, true, true);
	}
	else
	{
		CDrawUtils::DrawSimpleBorder(*pDC, cellRect, false, true);
	}

	// cache suitable titles (make sure font is set first)
	if (mTitles.empty())
		GenerateTitles(pDC);
	
	cdstring txt(mTitles.at(inCell.col - 1));
	JFontStyle text_style = pDC->GetFontStyle();
	if ((mTable != NULL) && mTable->IsTodayColumn(inCell))
		text_style.color = pDC->GetColormap()->GetBlueColor();
	else
		text_style.color = selected ? pDC->GetColormap()->GetWhiteColor() : pDC->GetColormap()->GetBlackColor();
	pDC->SetFontStyle(text_style);

	::DrawClippedStringUTF8(pDC, txt, JPoint(cellRect.left + 2, cellRect.top + 1), cellRect, eDrawString_Center);
}

void CDayWeekTitleTable::GenerateTitles(JPainter* pDC)
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
		const iCal::CICalendarDateTime& dt = mTable->GetCellStartDate(cell);
		
		mTitles.push_back(dt.GetLocaleDate(iCal::CICalendarDateTime::eFullDateNoYear));
		
		// See if too large for column width
		int width = mTable->GetColWidth(cell.col);
		width -= 6;
		
		// Attempt to truncate text 
		cdstring trunc(mTitles.back());
		int size = pDC->GetStringWidth(trunc.c_str());
		int str_width = size;
		if (str_width >= width)
		{
			mTitles.clear();
			break;
		}
	}
	
	// Try again with next shortest type if none were added
	if (mTitles.empty())
	{
		STableCell cell(2, 2);
		for(cell.col = 2; cell.col <= mCols; cell.col++)
		{
			const iCal::CICalendarDateTime& dt = mTable->GetCellStartDate(cell);
			
			mTitles.push_back(dt.GetLocaleDate(iCal::CICalendarDateTime::eAbbrevDateNoYear));
			
			// See if too large for column width
			int width = mTable->GetColWidth(cell.col);
			width -= 6;
			
			// Attempt to truncate text 
			cdstring trunc(mTitles.back());
			int size = pDC->GetStringWidth(trunc.c_str());
			int str_width = size;
			if (str_width >= width)
			{
				mTitles.clear();
				break;
			}
		}
	}
	
	// Try again with next shortest type if none were added - these we must always use as they are the shortest
	if (mTitles.empty())
	{
		STableCell cell(2, 2);
		for(cell.col = 2; cell.col <= mCols; cell.col++)
		{
			const iCal::CICalendarDateTime& dt = mTable->GetCellStartDate(cell);
			
			mTitles.push_back(dt.GetLocaleDate(iCal::CICalendarDateTime::eNumericDateNoYear));
		}
	}
	
	// Always add year as the first item
	cdstring temp((long)mTable->GetCellStartDate(STableCell(2, 2)).GetYear());
	mTitles.insert(mTitles.begin(), temp);
}
