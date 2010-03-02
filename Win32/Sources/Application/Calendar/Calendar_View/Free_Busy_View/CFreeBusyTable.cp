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

#include "CFreeBusyTable.h"

#include "CCalendarUtils.h"
#include "CFreeBusyTitleTable.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include <WIN_LTableMultiGeometry.h>

#include <algorithm>

const uint32_t cRowHeight = 24;
const uint32_t cNameColumnWidth = 128;
const uint32_t cHourColumnWidth = 96;
const uint32_t cNameColumn = 1;
const uint32_t cFirstTimedColumn = 2;

BEGIN_MESSAGE_MAP(CFreeBusyTable, CCalendarEventTableBase)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CFreeBusyTable														  [public]
/**
	Default constructor */

CFreeBusyTable::CFreeBusyTable()
{
	mTitles = NULL;

	mScaleColumns = 0;
	mStartHour = 0;
	mEndHour = 24;

	mTableGeometry = new LTableMultiGeometry(this, 128, 24);
}


// ---------------------------------------------------------------------------
//	~CFreeBusyTable														  [public]
/**
	Destructor */

CFreeBusyTable::~CFreeBusyTable()
{
}

#pragma mark -

int CFreeBusyTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarEventTableBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	InsertCols(1, 1);
	InsertRows(1, 1);

	SetRowHeight(cRowHeight, 1, 1);
	SetColWidth(cNameColumnWidth, 1, 1);

	AdjustSize();
	
	return 0;
}

// Resize columns
void CFreeBusyTable::OnSize(UINT nType, int cx, int cy)
{
	CCalendarEventTableBase::OnSize(nType, cx, cy);

	AdjustSize();
}

void CFreeBusyTable::DrawCellRange(CDC* pDC,
							const STableCell&	inTopLeftCell,
							const STableCell&	inBottomRightCell)
{
	STableCell	cell;

	for ( cell.row = inTopLeftCell.row;
		  cell.row <= inBottomRightCell.row;
		  cell.row++ )
	{
		// Draw entire row
		CRect	rowRect;
		GetLocalCellRectAlways(STableCell(cell.row, 2), rowRect);

		CRect	cellRect;
		GetLocalCellRectAlways(STableCell(cell.row, mCols), cellRect);
		rowRect.right = cellRect.right;

		DrawRow(pDC, cell.row, rowRect);

		for ( cell.col = inTopLeftCell.col;
			  cell.col <= inBottomRightCell.col;
			  cell.col++ )
		{

			CRect cellRect;
			GetLocalCellRectAlways(cell, cellRect);
			DrawCell(pDC, cell, cellRect);
		}
	}
}

void CFreeBusyTable::DrawRow(CDC* pDC, TableIndexT row, const CRect& inLocalRect)
{
	StDCState		save(pDC);
	CRect			adjustedRect = inLocalRect;

	adjustedRect.DeflateRect(0, 3);
	float total_width = adjustedRect.Width();

	const SFreeBusyInfo& info = mItems.at(row - 1);
	CRect itemRect = adjustedRect;
	for(std::vector<SFreeBusyInfo::SFreeBusyPeriod>::const_iterator iter = info.mPeriods.begin(); iter != info.mPeriods.end(); iter++)
	{
		// Adjust for current width
		itemRect.right = itemRect.left + total_width * ((float) (*iter).second / (float)mColumnSeconds);
		itemRect.DeflateRect(1, 0);

		// Red for busy, green for free, blue for tentative, grey for unavailable
		double red = 0.0;
		double green = 0.0;
		double blue = 0.0;
		switch((*iter).first)
		{
		case iCal::CICalendarFreeBusy::eFree:
			green = 1.0;
			break;
		case iCal::CICalendarFreeBusy::eBusyTentative:
			blue = 1.0;
			break;
		case iCal::CICalendarFreeBusy::eBusyUnavailable:
			red = 0.25;
			green = 0.25;
			blue = 0.25;
			break;
		case iCal::CICalendarFreeBusy::eBusy:
			red = 1.0;
			break;
		}
		CCalendarUtils::LightenColours(red, green, blue);

		// Draw it
		CBrush temp_brush(CCalendarUtils::GetWinColor(red, green, blue));
		CBrush* old_brush = (CBrush*) pDC->SelectObject(&temp_brush);
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinColor(red * 0.6, green * 0.6, blue * 0.6));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->Rectangle(itemRect);
		pDC->SelectObject(old_pen);
		pDC->SelectObject(old_brush);

		// Now adjust for next one
		itemRect.left += itemRect.Width() + 1;
		itemRect.right = itemRect.left + adjustedRect.right - itemRect.left;
	}

}

void CFreeBusyTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState		save(pDC);
	CRect			adjustedRect = inLocalRect;

	// Left-side always
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinGreyColor((inCell.col > 2) ? 0.75 : 0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.left, adjustedRect.top);
		pDC->LineTo(adjustedRect.left, adjustedRect.bottom);
		pDC->SelectObject(old_pen);
	}

	// Right-side only for last column
	if (inCell.col == mCols)
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinGreyColor(0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.right, adjustedRect.top);
		pDC->LineTo(adjustedRect.right, adjustedRect.bottom);
		pDC->SelectObject(old_pen);
	}

	// Top-side always
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinGreyColor(0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.left, adjustedRect.top);
		pDC->LineTo(adjustedRect.right, adjustedRect.top);
		pDC->SelectObject(old_pen);
	}

	// Bottom-side for last row
	if (inCell.row == mRows)
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinGreyColor(0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.left, adjustedRect.bottom);
		pDC->LineTo(adjustedRect.right, adjustedRect.bottom);
		pDC->SelectObject(old_pen);
	}
	
	adjustedRect.DeflateRect(2, 0);
	
	if (inCell.col == 1)
	{
		cdstring name = mItems.at(inCell.row - 1).mName;
		pDC->SetTextColor(CDrawUtils::sBlackColor);
		pDC->SelectObject(CFontCache::GetListFont());
		::DrawClippedStringUTF8(pDC, name, CPoint(adjustedRect.left, adjustedRect.top + mTextOrigin), adjustedRect, eDrawString_Right);
	}
}

void CFreeBusyTable::AdjustSize()
{
	if ((mRows != 0) && (mCols != 0))
	{
		StDeferTableAdjustment _defer(this);

		// If auto-fit rows, change row height
		RescaleWidth();

		if (mTitles)
			mTitles->TableChanged();
	}
}

void CFreeBusyTable::RescaleWidth()
{
	// If auto-fit rows, change row height
	if (mScaleColumns == 0)
	{
		CRect my_frame;
		GetClientRect(my_frame);
	
		if (mCols > 1)
		{
			SInt32 col_size = std::max((SInt32) ((my_frame.Width() - GetColWidth(cNameColumn)) / (mCols - 1)), (SInt32) 8L);
			SetColWidth(col_size, cFirstTimedColumn, mCols);
		}
	}
}

void CFreeBusyTable::ScaleColumns(uint32_t scale)
{
	if (scale != mScaleColumns)
	{
		mScaleColumns = scale;
		if (mScaleColumns > 0)
			SetColWidth(cHourColumnWidth / mScaleColumns, cFirstTimedColumn, mCols);
		else
		{
			RescaleWidth();
		}
	}
}
