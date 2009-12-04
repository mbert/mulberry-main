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

#include "CDayWeekTable.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"

#include "cdustring.h"

// ---------------------------------------------------------------------------
//	CDayWeekTitleTable														  [public]
/**
	Default constructor */

CDayWeekTitleTable::CDayWeekTitleTable()
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
}

void CDayWeekTitleTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	// Broadcast double-click
	STableCell cell(2, inCell.col);
	iCal::CICalendarDateTime dt(mTable->GetCellStartDate(cell));
	Broadcast_Message(eBroadcast_DblClick, &dt);
}

// Draw a row
void CDayWeekTitleTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Look for selected column
	bool selected = false;
	if (mTable != NULL)
	{
		STableCell selCell = mTable->GetFirstSelectedCell();
		selected = !selCell.IsNullCell() && (selCell.col > 0) && (inCell.col == selCell.col);
	}

	// Draw frame
	pDC->FillRect(&inLocalRect, selected ? &CDrawUtils::sDkGrayBrush : &CDrawUtils::sGrayBrush);
	if (!selected)
	{
		pDC->SelectObject(CDrawUtils::sWhitePen);
		pDC->MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
		pDC->LineTo(inLocalRect.left, inLocalRect.top);
		pDC->LineTo(inLocalRect.right, inLocalRect.top);
		pDC->SelectObject(CDrawUtils::sDkGrayPen);
		pDC->MoveTo(inLocalRect.right - 2, inLocalRect.top + 1);
		pDC->LineTo(inLocalRect.right - 2, inLocalRect.bottom - 2);
		pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 2);
		pDC->SelectObject(CDrawUtils::sBlackPen);
		pDC->MoveTo(inLocalRect.right - 1, inLocalRect.top);
		pDC->LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
		pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 1);
		pDC->SetTextColor(CDrawUtils::sBtnTextColor);
	}
	pDC->SetBkColor(selected ? CDrawUtils::sDkGrayColor : CDrawUtils::sGrayColor);

	// cache suitable titles (make sure font is set first)
	pDC->SelectObject(CFontCache::GetListFont());
	if (mTitles.empty())
		GenerateTitles(pDC);
	
	cdstring txt(mTitles.at(inCell.col - 1));
	if ((mTable != NULL) && mTable->IsTodayColumn(inCell))
		pDC->SetTextColor(RGB(0x00,0x00,0xFF));
	else
		pDC->SetTextColor(selected ? CDrawUtils::sWhiteColor : CDrawUtils::sBtnTextColor);
		
	::DrawClippedStringUTF8(pDC, txt, CPoint(inLocalRect.left + 2, inLocalRect.top + 1), inLocalRect, eDrawString_Center);
}

void CDayWeekTitleTable::GenerateTitles(CDC* pDC)
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
		cdustring trunc(mTitles.back());
		SIZE size;
		::GetTextExtentPoint32W(*pDC, trunc, trunc.length(), &size);
		int str_width = size.cx;
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
			cdustring trunc(mTitles.back());
			SIZE size;
			::GetTextExtentPoint32W(*pDC, trunc, trunc.length(), &size);
			int str_width = size.cx;
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
