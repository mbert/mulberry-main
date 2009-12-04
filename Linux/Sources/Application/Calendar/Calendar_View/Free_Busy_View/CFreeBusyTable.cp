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
#include "CCommands.h"
#include "CFreeBusyTitleTable.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include "StPenState.h"

#include <UNX_LTableMultiGeometry.h>

#include <JPainter.h>
#include <JXColormap.h>
#include <JXTextMenu.h>
#include <JXScrollbar.h>
#include <JXScrollbarSet.h>

#include <algorithm>

const uint32_t cRowHeight = 24;
const uint32_t cNameColumnWidth = 128;
const uint32_t cHourColumnWidth = 96;
const uint32_t cNameColumn = 1;
const uint32_t cFirstTimedColumn = 2;

// ---------------------------------------------------------------------------
//	CFreeBusyTable														  [public]
/**
	Default constructor */

CFreeBusyTable::CFreeBusyTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CCalendarEventTableBase(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
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

void
CFreeBusyTable::OnCreate()
{
	// Call inherited
	CCalendarEventTableBase::OnCreate();

	InsertCols(1, 1, NULL);
	InsertRows(1, 1, NULL);

	SetRowHeight(cRowHeight, 1, 1);
	SetColWidth(cNameColumnWidth, 1, 1);

	itsScrollbarSet->GetHScrollbar()->SetStepSize(cHourColumnWidth);
	itsScrollbarSet->GetVScrollbar()->SetStepSize(cRowHeight);
	
	CreateContextMenu(CMainMenu::eContextCalendarEventTable);

	ApertureResized(0, 0);
}

void CFreeBusyTable::DrawCellRange(JPainter* pDC,
							const STableCell&	inTopLeftCell,
							const STableCell&	inBottomRightCell)
{
	STableCell	cell;

	for ( cell.row = inTopLeftCell.row;
		  cell.row <= inBottomRightCell.row;
		  cell.row++ )
	{
		// Draw entire row
		JRect	rowRect;
		GetLocalCellRect(STableCell(cell.row, 2), rowRect);

		JRect	cellRect;
		GetLocalCellRect(STableCell(cell.row, mCols), cellRect);

		rowRect.right = cellRect.right;

		DrawRow(pDC, cell.row, rowRect);

		for ( cell.col = inTopLeftCell.col;
			  cell.col <= inBottomRightCell.col;
			  cell.col++ )
		{

			JRect cellRect;
			GetLocalCellRect(cell, cellRect);
			DrawCell(pDC, cell, cellRect);
		}
	}
}

void CFreeBusyTable::DrawRow(JPainter* pDC, TableIndexT row, const JRect& inLocalRect)
{
	StPenState		save(pDC);
	JRect			adjustedRect = inLocalRect;

	adjustedRect.Shrink(0, 3);
	float total_width = adjustedRect.width();

	const SFreeBusyInfo& info = mItems.at(row - 1);
	JRect itemRect = adjustedRect;
	for(std::vector<SFreeBusyInfo::SFreeBusyPeriod>::const_iterator iter = info.mPeriods.begin(); iter != info.mPeriods.end(); iter++)
	{
		// Adjust for current width
		itemRect.right = itemRect.left + total_width * ((float) (*iter).second / (float)mColumnSeconds);
		itemRect.Shrink(1, 0);

		// Red for busy, green for free, blue for tentative, grey for unavailable
		float red = 0.0;
		float green = 0.0;
		float blue = 0.0;
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
		JColorIndex cindex;
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(red, green, blue), &cindex);
		pDC->SetPenColor(cindex);
		pDC->SetFilling(kTrue);
		pDC->Rect(itemRect);
		pDC->SetFilling(kFalse);
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(red * 0.6, green * 0.6, blue * 0.6), &cindex);
		pDC->SetPenColor(cindex);
		pDC->Rect(itemRect);

		// Now adjust for next one
		itemRect.left += itemRect.width() + 1;
		itemRect.right = itemRect.left + adjustedRect.right - itemRect.left;
	}

}

void CFreeBusyTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	StPenState		save(pDC);
	JRect			adjustedRect = inLocalRect;
	JColorIndex cindex;

	// Left-side always
	{
		JColorIndex old_cindex = pDC->GetPenColor();
		GetColormap()->JColormap::AllocateStaticColor(
				CCalendarUtils::GetGreyColor((inCell.col > 2) ? 0.75 : 0.5),
				&cindex);
		pDC->SetPenColor(cindex);
		pDC->Line(adjustedRect.left, adjustedRect.top, adjustedRect.left, adjustedRect.bottom);
		pDC->SetPenColor(old_cindex);
	}

	// Right-side only for last column
	if (inCell.col == mCols)
	{
		JColorIndex old_cindex = pDC->GetPenColor();
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetGreyColor(0.5), &cindex);
		pDC->SetPenColor(cindex);
		pDC->Line(adjustedRect.right, adjustedRect.top, adjustedRect.right, adjustedRect.bottom);
		pDC->SetPenColor(old_cindex);
	}

	// Top-side always (lighter line above half-hour row)
	{
		JColorIndex old_cindex = pDC->GetPenColor();
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetGreyColor(0.5), &cindex);
		pDC->SetPenColor(cindex);
		pDC->Line(adjustedRect.left, adjustedRect.top, adjustedRect.right, adjustedRect.top);
		pDC->SetPenColor(old_cindex);
	}

	// Bottom-side for last row
	if (inCell.row == mRows)
	{
		JColorIndex old_cindex = pDC->GetPenColor();
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetGreyColor(0.5), &cindex);
		pDC->SetPenColor(cindex);
		pDC->Line(adjustedRect.left, adjustedRect.bottom, adjustedRect.right, adjustedRect.bottom);
		pDC->SetPenColor(old_cindex);
	}
	
	adjustedRect.Shrink(2, 0);
	
	if (inCell.col == 1)
	{
		cdstring name = mItems.at(inCell.row - 1).mName;
		pDC->SetPenColor(GetColormap()->GetBlackColor());
		JRect clipRect(adjustedRect);
		clipRect.Shrink(0, 1);
		clipRect.right += 2;
		::DrawClippedStringUTF8(pDC, name, JPoint(adjustedRect.left, adjustedRect.top), clipRect, eDrawString_Right);
	}
}

void CFreeBusyTable::RefreshNow()
{
	// Redraw - double-buffering removes any flashing
	FRAMEWORK_REFRESH_WINDOW(this)
}

void CFreeBusyTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Allow frame adapter to adjust size
	CCalendarEventTableBase::ApertureResized(dw, dh);

	// Look for change in image size
	UInt32 old_width, old_height;
	mTableGeometry->GetTableDimensions(old_width, old_height);

	// If auto-fit rows, change row height
	RescaleWidth();

	itsScrollbarSet->GetHScrollbar()->SetStepSize(GetColWidth(cFirstTimedColumn));
	itsScrollbarSet->GetVScrollbar()->SetStepSize(cRowHeight);

	UInt32 new_width, new_height;
	mTableGeometry->GetTableDimensions(new_width, new_height);

	if (mTitles)
		mTitles->TableChanged();
}

void CFreeBusyTable::RescaleWidth()
{
	// If auto-fit rows, change row height
	if (mScaleColumns == 0)
	{
		JRect my_frame = GetFrameGlobal();
	
		if (mCols > 1)
		{
			SInt32 col_size = std::max((SInt32) ((my_frame.width() - GetColWidth(cNameColumn)) / (mCols - 1)), 8L);
			SetColWidth(col_size, cFirstTimedColumn, mCols);
		}
	}
}

// Keep titles in sync
void CFreeBusyTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	if (mTitles)
		mTitles->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
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
