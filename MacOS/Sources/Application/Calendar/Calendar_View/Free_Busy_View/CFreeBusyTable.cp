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

// ===========================================================================
//	CFreeBusyTable.cp			PowerPlantX 1.0			©2003 Metrowerks Corp.
// ===========================================================================

#include "CFreeBusyTable.h"

#include "CCommands.h"
#include "CFreeBusyTitleTable.h"
#include "CGUtils.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include <LTableMultiGeometry.h>

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

CFreeBusyTable::CFreeBusyTable(LStream *inStream) :
	CCalendarEventTableBase(inStream)
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
CFreeBusyTable::FinishCreateSelf()
{
	// Call inherited
	CCalendarEventTableBase::FinishCreateSelf();

	InsertCols(1, 1, NULL);
	InsertRows(1, 1, NULL);

	SetRowHeight(cRowHeight, 1, 1);
	SetColWidth(cNameColumnWidth, 1, 1);

	SPoint32 pt = { cHourColumnWidth, cRowHeight };
	SetScrollUnit(pt);

	AdaptToNewSurroundings();
}

void CFreeBusyTable::DrawCellRange(const STableCell& inTopLeftCell, const STableCell& inBottomRightCell)
{
	STableCell	cell;
	
	for ( cell.row = inTopLeftCell.row;
		  cell.row <= inBottomRightCell.row;
		  cell.row++ )
	{
		// Draw entire row
		Rect	rowRect;
		GetLocalCellRectAlways(STableCell(cell.row, 2), rowRect);

		Rect	cellRect;
		GetLocalCellRectAlways(STableCell(cell.row, mCols), cellRect);
		rowRect.right = cellRect.right;

		DrawRow(cell.row, rowRect);
	
		for ( cell.col = inTopLeftCell.col;
			  cell.col <= inBottomRightCell.col;
			  cell.col++ )
		{
			  
			Rect	cellRect;
			GetLocalCellRect(cell, cellRect);
			DrawCell(cell, cellRect);
		}
	}
}

void CFreeBusyTable::DrawRow(TableIndexT row, const Rect& inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;
	adjustedRect.origin.y += 3;
	adjustedRect.size.height -= 6;
	float total_width = adjustedRect.size.width;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);
	
	const SFreeBusyInfo& info = mItems.at(row - 1);
	HIRect itemRect = adjustedRect;
	for(std::vector<SFreeBusyInfo::SFreeBusyPeriod>::const_iterator iter = info.mPeriods.begin(); iter != info.mPeriods.end(); iter++)
	{
		// Adjust for current width
		itemRect.size.width = total_width * ((float) (*iter).second / (float)mColumnSeconds);
		itemRect.origin.x += 1;
		itemRect.size.width -= 2;

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
		CGUtils::LightenColours(red, green, blue);

		// Draw it
		::CGContextBeginPath(inContext);
		::CGContextAddRect(inContext, itemRect);
		::CGContextClosePath(inContext);
		::CGContextSetRGBFillColor(inContext, red, green, blue, 1.0);
		::CGContextSetRGBStrokeColor(inContext, red * 0.6, green * 0.6, blue * 0.6, 1.0);
		::CGContextDrawPath(inContext, kCGPathFillStroke);

		// Now adjust for next one
		itemRect.origin.x += itemRect.size.width + 1;
		itemRect.size.width = adjustedRect.origin.x + adjustedRect.size.width - itemRect.origin.x;
	}

}

void CFreeBusyTable::DrawCell(const STableCell &inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	// Draw frame components by hand (no aliasing for lines)
	::CGContextSetShouldAntialias(inContext, false);
	
	// Left-side always
	::CGContextBeginPath(inContext);
	::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y);
	::CGContextAddLineToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height);
	::CGContextSetGrayStrokeColor(inContext, (inCell.col > 2) ? 0.75 : 0.5, 1.0);
	::CGContextStrokePath(inContext);
	//::CGContextClosePath(inContext);

	// Right-side only for last column
	if (inCell.col == mCols)
	{
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}

	// Top-side always (lighter line above half-hour row)
	::CGContextBeginPath(inContext);
	::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y);
	::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y);
	::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
	::CGContextStrokePath(inContext);
	//::CGContextClosePath(inContext);

	// Bottom-side for last row
	if (inCell.row == mRows)
	{
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}
	
	// Turn on aliasing for text
	::CGContextSetShouldAntialias(inContext, true);

	adjustedRect.origin.x += 2.0;
	adjustedRect.size.width -= 4.0;
	
	if (inCell.col == 1)
	{
		cdstring name = mItems.at(inCell.row - 1).mName;
		MyCFString txt(name);
		
		// Want to vertically center the text
		Point bounds;
		::GetThemeTextDimensions(txt, kThemeSmallSystemFont, kThemeStateActive, false, &bounds, NULL);
		
		// Center rect in circle
		adjustedRect.origin.y += (adjustedRect.size.height - bounds.v)/ 2.0;
		adjustedRect.size.height = bounds.v;

		Rect box;
		CGUtils::HIToQDRect(adjustedRect, box);
		::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
		::DrawThemeTextBox(txt, kThemeSmallSystemFont, kThemeStateActive, false, &box, teJustRight, inContext);
	}
}

void CFreeBusyTable::RefreshNow()
{
	// Redraw - double-buffering removes any flashing
	FRAMEWORK_REFRESH_WINDOW(this)
}

void CFreeBusyTable::AdaptToNewSurroundings()
{
	// Allow frame adapter to adjust size
	CCalendarEventTableBase::AdaptToNewSurroundings();

	// Look for change in image size
	UInt32 old_width, old_height;
	mTableGeometry->GetTableDimensions(old_width, old_height);

	// If auto-fit rows, change row height
	RescaleWidth();

	SPoint32 pt = {GetColWidth(cFirstTimedColumn), cRowHeight};
	SetScrollUnit(pt);

	if (mTitles)
		mTitles->TableChanged();
}

void CFreeBusyTable::RescaleWidth()
{
	// If auto-fit rows, change row height
	if (mScaleColumns == 0)
	{
		SDimension16 my_frame;
		GetFrameSize(my_frame);
	
		if (mCols > 1)
		{
			SInt32 col_size = std::max((SInt32) ((my_frame.width - GetColWidth(cNameColumn)) / (mCols - 1)), 8L);
			SetColWidth(col_size, cFirstTimedColumn, mCols);
		}
	}
}

// Keep titles in sync
void CFreeBusyTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
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

