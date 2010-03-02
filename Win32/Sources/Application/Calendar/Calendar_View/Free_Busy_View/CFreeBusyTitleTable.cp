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

#include "CFreeBusyTable.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"

#include "CICalendarLocale.h"

#include "cdustring.h"

// ---------------------------------------------------------------------------
//	CFreeBusyTitleTable														  [public]
/**
	Default constructor */

CFreeBusyTitleTable::CFreeBusyTitleTable()
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
}

// Draw a row
void CFreeBusyTitleTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Draw frame
	pDC->FillRect(&inLocalRect, &CDrawUtils::sGrayBrush);
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
	pDC->SetBkColor(CDrawUtils::sGrayColor);

	// cache suitable titles (make sure font is set first)
	pDC->SelectObject(CFontCache::GetListFont());
	if (mTitles.empty())
		GenerateTitles(pDC);
	
	cdstring txt(mTitles.at(inCell.col - 1));
	if (mTable != NULL)
		pDC->SetTextColor(RGB(0x00,0x00,0xFF));
	else
		pDC->SetTextColor(CDrawUtils::sBtnTextColor);
		
	::DrawClippedStringUTF8(pDC, txt, CPoint(inLocalRect.left + 2, inLocalRect.top + 1), inLocalRect, eDrawString_Left);
}

void CFreeBusyTitleTable::GenerateTitles(CDC* pDC)
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
