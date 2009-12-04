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

#include "CSummaryTable.h"

#include "CGUtils.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSummaryEvent.h"
#include "CSummaryTitleTable.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"

#include <UGAColorRamp.h>

#include <algorithm>

const uint32_t cRowHeight = 18;
const uint32_t cLinkColumnWidth = 16;
const uint32_t cDateColumnWidth = 96;
const uint32_t cTimeColumnWidth = 72;
const uint32_t cSummaryColumnWidth = 196;
const uint32_t cStatusColumnWidth = 64;
const uint32_t cCalendarColumnWidth = 128;

const uint32_t cLinkHOffset = 8;
const uint32_t cLinkWidth = 4;
const uint32_t cLinkVOffset = 4;
const uint32_t cLinkHeight = 16;

const float body_transparency = 0.75;
const float line_transparency = 1.0;
const float text_transparency = 0.9;

// ---------------------------------------------------------------------------
//	CSummaryTable														  [public]
/**
	Default constructor */

CSummaryTable::CSummaryTable(LStream *inStream) :
	CCalendarTableBase(inStream)
{
	mTitles = NULL;

	mTableGeometry = new CTableRowGeometry(this, 128, cRowHeight);
	
	// Single selector
	mTableSelector = new CTableSingleRowSelector(this);

	SetRowSelect(true);

	mPrinting = false;
}


// ---------------------------------------------------------------------------
//	~CSummaryTable														  [public]
/**
	Destructor */

CSummaryTable::~CSummaryTable()
{
}

#pragma mark -

void CSummaryTable::FinishCreateSelf()
{
	// Call inherited
	CCalendarTableBase::FinishCreateSelf();

	InsertCols(6, 1, NULL);

	mColumnInfo.push_back(CColumnSpec(eLink, cLinkColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eDate, cDateColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eTime, cTimeColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eSummary, cSummaryColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eStatus, cStatusColumnWidth));
	mColumnInfo.push_back(CColumnSpec(eCalendar, cCalendarColumnWidth));

	//SPoint32 pt = {cDateColumnWidth, cRowHeight};
	//SetScrollUnit(pt);

	ResetColumns();

	AdaptToNewSurroundings();

	// Turn on tooltips
	EnableTooltips();

}

void CSummaryTable::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
{
	// Ignore clicks on non-events
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (!event->IsEvent())
	{
		return;
	}
	
	// Broadcast double-click if in month area
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoDoubleClick(inCell, CKeyModifiers(inMouseDown.macEvent.modifiers));
	else
		DoSingleClick(inCell, CKeyModifiers(inMouseDown.macEvent.modifiers));
}

void CSummaryTable::DrawCellRange(const STableCell& inTopLeftCell, const STableCell& inBottomRightCell)
{
	STableCell	cell;
	
	for ( cell.row = inTopLeftCell.row;
		  cell.row <= inBottomRightCell.row;
		  cell.row++ )
	{
		// See whether this is a row item
		const CSummaryEvent* event = mEvents[cell.row - 1];
		if (event->IsEvent())
		{
			for ( cell.col = inTopLeftCell.col;
				  cell.col <= inBottomRightCell.col;
				  cell.col++ )
			{
				  
				Rect	cellRect;
				GetLocalCellRect(cell, cellRect);
				DrawCell(cell, cellRect);
			}
		}
		else
		{
			// Draw entire row
			Rect	rowRect;
			GetLocalCellRectAlways(STableCell(cell.row, 1), rowRect);

			Rect	cellRect;
			GetLocalCellRectAlways(STableCell(cell.row, mCols), cellRect);
			rowRect.right = cellRect.right;

			DrawRow(cell.row, rowRect);
		}
	}
}

void CSummaryTable::DrawRow(TableIndexT row, const Rect& inLocalQDRect)
{
	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	const CSummaryEvent* event = mEvents[row - 1];

	if (!mPrinting)
	{
		ThemeButtonDrawInfo theme;
		theme.state = (IsActive() && IsEnabled()) ? kThemeStateActive : kThemeStateInactive;
		theme.value = kThemeButtonOff;
		theme.adornment = kThemeAdornmentNone;
		::DrawThemeButton(&inLocalQDRect, kThemeSmallBevelButton, &theme, NULL, NULL, NULL, NULL);
	}
	else
	{
		// Draw frame components by hand (no aliasing for lines)
		::CGContextSetShouldAntialias(inContext, false);
		
		::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
		::CGContextFillRect(inContext, adjustedRect);
		::CGContextSetGrayFillColor(inContext, 1.0, 1.0);
		
		// Turn on aliasing for text
		::CGContextSetShouldAntialias(inContext, true);
	}

	Rect box = inLocalQDRect;
	box.bottom -= mTextDescent;
	::DrawClippedStringUTF8(inContext, event->mSummary, box, eDrawString_Center);
}

void CSummaryTable::DrawCell(const STableCell &inCell, const Rect& inLocalQDRect)
{
	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	const CSummaryEvent* event = mEvents[inCell.row - 1];

	bool link = (mColumnInfo[inCell.col - 1].first == eLink);

	// Draw frame components by hand (no aliasing for lines)
	::CGContextSetShouldAntialias(inContext, false);
	
	// Do selection background
	float red = CGUtils::GetCGRed(event->mColour);
	float green = CGUtils::GetCGGreen(event->mColour);
	float blue = CGUtils::GetCGBlue(event->mColour);
	if (CellIsSelected(inCell) && mDrawSelection)
		;
	else
		CGUtils::LightenColours(red, green, blue);

	if (!link)
	{
		::CGContextSetRGBFillColor(inContext, red, green, blue, (CellIsSelected(inCell) && mDrawSelection) ? 1.0 : body_transparency);
		::CGContextFillRect(inContext, adjustedRect);
	}

	if (!link)
	{
		// Left-side always
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
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

		// Top-side always
		{
			::CGContextBeginPath(inContext);
			::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y);
			::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
			::CGContextStrokePath(inContext);
			//::CGContextClosePath(inContext);
		}

		// Bottom-side only for last row
		if (inCell.row == mRows)
		{
			::CGContextBeginPath(inContext);
			::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y + adjustedRect.size.height);
			::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
			::CGContextStrokePath(inContext);
			//::CGContextClosePath(inContext);
		}
	}
	
	// Get string data for column
	cdstring text;
	bool draw_txt = false;
	SInt16 just = teJustLeft;
	switch(mColumnInfo[inCell.col - 1].first)
	{
	case eLink:
	{
		::CGContextBeginPath(inContext);
		if (event->mDayStart)
		{
			::CGContextMoveToPoint(inContext, adjustedRect.origin.x + cLinkHOffset + cLinkWidth, adjustedRect.origin.y + cLinkVOffset);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + cLinkVOffset);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + adjustedRect.size.height / 2);
		}
		else
		{
			::CGContextMoveToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + adjustedRect.size.height / 2);
		}
		if (event->mDayEnd)
		{
			::CGContextMoveToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + adjustedRect.size.height / 2);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + adjustedRect.size.height - cLinkVOffset);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + cLinkHOffset + cLinkWidth, adjustedRect.origin.y + adjustedRect.size.height - cLinkVOffset);
		}
		else
		{
			::CGContextMoveToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + adjustedRect.size.height / 2);
			::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + cLinkHOffset, adjustedRect.origin.y + adjustedRect.size.height);
		}
		::CGContextSetGrayStrokeColor(inContext, 0.0, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
		
	}
		break;
	case eDate:
		text = event->mStartDate;
		draw_txt = true;
		just = teJustRight;
		break;
	case eTime:
		text = event->mStartTime;
		draw_txt = true;
		just = teJustRight;
		break;
	case eSummary:
		text = event->mSummary;
		draw_txt = true;
		break;
	case eStatus:
		break;
	case eCalendar:
		text = event->mCalendar;
		draw_txt = true;
		break;
	}
	
	// Turn on aliasing for text
	::CGContextSetShouldAntialias(inContext, true);

	::CGContextSetGrayFillColor(inContext, (CellIsSelected(inCell) && mDrawSelection) ? 1.0 : 0.0, 1.0);
	if (draw_txt)
	{
		Rect box = inLocalQDRect;
		box.bottom = box.bottom - mTextDescent - 2;
		::DrawClippedStringUTF8(inContext, text, box, eDrawString_Left);
	}

	// Strike out if status is cancelled
	if (!link && event->mCancelled)
	{
		// No aliasing for lines
		::CGContextSetShouldAntialias(inContext, false);
		
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height / 2);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y + adjustedRect.size.height / 2);
		::CGContextSetGrayStrokeColor(inContext, (CellIsSelected(inCell) && mDrawSelection) ? 1.0 : 0.0, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
		
		// Turn on aliasing for text
		::CGContextSetShouldAntialias(inContext, true);
	}
}

// Draw or undraw active hiliting for a Cell
void CSummaryTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	// Only if real event
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (event->IsEvent())
		CCalendarTableBase::HiliteCellActively(inCell, inHilite);
}

// Draw or undraw inactive hiliting for a Cell
void CSummaryTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	// Only if real event
	const CSummaryEvent* event = mEvents[inCell.row - 1];
	if (event->IsEvent())
		CCalendarTableBase::HiliteCellInactively(inCell, inHilite);
}

void CSummaryTable::AdaptToNewSurroundings()
{
	// Allow frame adapter to adjust size
	CCalendarTableBase::AdaptToNewSurroundings();
	
	// Change column info widths here
	
	// Columns changed
	ResetColumns();
}

// Keep titles in sync
void CSummaryTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	if (mTitles)
		mTitles->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}
