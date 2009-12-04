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

#include "CDayWeekTable.h"

#include "CCommands.h"
#include "CDayEvent.h"
#include "CDayWeekTitleTable.h"
#include "CGUtils.h"
#include "CPreferences.h"
#include "CTableColumnSelector.h"
#include "CXStringResources.h"

#include "CICalendarComponentExpanded.h"
#include "CICalendarLocale.h"
#include "CICalendarVFreeBusy.h"

#include <LTableMultiGeometry.h>

#include <algorithm>

const uint32_t cRowHeight = 24;
const uint32_t cTimeColumnWidth = 56;
const uint32_t cDayColumnWidth = 128;
const uint32_t cNumberRows = 1 + 48;	// One for all-day, 48 for half-hour intervals
const uint32_t cAllDayRow = 1;
const uint32_t cFirstTimedRow = 2;
const uint32_t cEventOffset = 16;
const uint32_t cEventMargin = 4;

const float line_gray = 0.75;
const float line_gray_lighter = 0.875;

// ---------------------------------------------------------------------------
//	CDayWeekTable														  [public]
/**
	Default constructor */

CDayWeekTable::CDayWeekTable(LStream *inStream) :
	CCalendarEventTableBase(inStream)
{
	mTitles = NULL;

	mNowCell.SetCell(0, 0);
	mScaleRows = 0;
	mStartHour = 0;
	mEndHour = 24;

	mTableGeometry = new LTableMultiGeometry(this, 128, 24);
	
	// Single selector
	mTableSelector = new CTableSingleColumnSelector(this);
}


// ---------------------------------------------------------------------------
//	~CDayWeekTable														  [public]
/**
	Destructor */

CDayWeekTable::~CDayWeekTable()
{
	ClearEvents();
}

#pragma mark -

void
CDayWeekTable::FinishCreateSelf()
{
	// Call inherited
	CCalendarEventTableBase::FinishCreateSelf();

	InsertCols(2, 1, NULL);
	InsertRows(cNumberRows, 1, NULL);

	SetRowHeight(cRowHeight, cAllDayRow, cAllDayRow);
	SetRowHeight(cRowHeight / (mScaleRows == 0 ? 1 : mScaleRows), cFirstTimedRow, mRows);

	SetColWidth(cTimeColumnWidth, 1, 1);
	SetColWidth(cDayColumnWidth, 2, 2);

	SPoint32 pt = { cDayColumnWidth, GetRowHeight(cFirstTimedRow) };
	SetScrollUnit(pt);

	AdaptToNewSurroundings();
}

bool CDayWeekTable::ObeyContextCommand(CommandT inCommand, const EventRecord& event)
{
	// Look for New Event command and use the click point to determine event start
	if (inCommand == cmd_NewEvent)
	{
		// Convert event point to a local point
		Point localPt = event.where;
		GlobalToPortPoint(localPt);
		PortToLocalPoint(localPt);

		// See if click is on a valid cell
		SPoint32 imagePt;
		LocalToImagePoint(localPt, imagePt);
		STableCell hitCell;
		if (GetCellHitBy(imagePt, hitCell) && (hitCell.col > 1))
		{
			// Use current position for new event time and create new event
			iCal::CICalendarDateTime dt(GetClickDateTime(localPt));
			Broadcast_Message(eBroadcast_DblClkToDay, &dt);
			
			// By-pass normal context event handling
			return true;
		}
	}

	return false;
}

iCal::CICalendarDateTime CDayWeekTable::GetClickDateTime(const Point& localPt) const
{
	SPoint32 imagePt;
	LocalToImagePoint(localPt, imagePt);
	STableCell hitCell;
	if (GetCellHitBy(imagePt, hitCell))
	{
		if (hitCell.row == cAllDayRow)
		{
			iCal::CICalendarDateTime dt(GetCellStartDate(hitCell));
			dt.SetDateOnly(true);
			return dt;
		}
		else
		{
			// Get top/bottom image points of timed rows
			STableCell timed_cell(cFirstTimedRow, 1);
			SInt32	cellLeft, cellTop, cellRight, cellBottom;
			GetImageCellBounds(timed_cell, cellLeft, cellTop, cellRight, cellBottom);
			cellBottom = cellTop + GetRowHeight(cFirstTimedRow) * (mRows - 1);

			// Get top/bottom seconds range
			uint32_t top_secs = mStartHour * 60 * 60;
			uint32_t bot_secs = mEndHour * 60 * 60;

			uint32_t click_secs = top_secs + (bot_secs - top_secs) * (imagePt.v - cellTop) / (cellBottom - cellTop);

			// Round to nearest 15 minute (900 second) interval
			click_secs = ((click_secs + 450) / 900) * 900;

			// Adjust actual date/time to match clicked time
			iCal::CICalendarDateTime dt(GetCellStartDate(hitCell));
			dt.SetHHMMSS(0, 0, 0);
			dt.OffsetSeconds(click_secs);
			
			return dt;
		}
	}
	else
	{
		iCal::CICalendarDateTime dt;
		return dt;
	}
}

void CDayWeekTable::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
{
	// Broadcast double-click if in day area
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect && (inCell.col > 1))
	{
		iCal::CICalendarDateTime dt(GetClickDateTime(inMouseDown.whereLocal));
		Broadcast_Message(eBroadcast_DblClkToDay, &dt);
	}
	else
		CCalendarEventTableBase::ClickCell(inCell, inMouseDown);
}

void CDayWeekTable::DrawCell(const STableCell &inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	// Draw frame components by hand (no aliasing for lines)
	::CGContextSetLineWidth(inContext, 1.0);
	::CGContextSetShouldAntialias(inContext, false);
	
	// Do selection/today background
	if (CellIsSelected(inCell) && (inCell.col > 1) && mDrawSelection)
	{
		::CGContextSetGrayFillColor(inContext, 0.85, 1.0);
		::CGContextFillRect(inContext, adjustedRect);
	}
	else if (IsTodayColumn(inCell))
	{
		::CGContextSetRGBFillColor(inContext, 0.95, 0.95, 1.0, 1.0);
		::CGContextFillRect(inContext, adjustedRect);
	}

	// Left-side always
	::CGContextBeginPath(inContext);
	::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y);
	::CGContextAddLineToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height);
	::CGContextSetGrayStrokeColor(inContext, line_gray, 1.0);
	::CGContextStrokePath(inContext);
	//::CGContextClosePath(inContext);

	// Right-side only for last column
	if (inCell.col == mCols)
	{
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextSetGrayStrokeColor(inContext, line_gray, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}

	// Top-side always (lighter line above half-hour row)
	// No lighter line if row height <= 18
	if ((GetRowHeight(cFirstTimedRow) > 18) || (inCell.row % 2 != 1))
	{
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y);
		::CGContextSetGrayStrokeColor(inContext, (inCell.row % 2 == 1) && (inCell.row != 1) ? line_gray_lighter : line_gray, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}

	// Bottom-side always
	if ((GetRowHeight(cFirstTimedRow) > 18) || (inCell.row % 2 != 0))
	{
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, adjustedRect.origin.x, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextAddLineToPoint(inContext, adjustedRect.origin.x + adjustedRect.size.width, adjustedRect.origin.y + adjustedRect.size.height);
		::CGContextSetGrayStrokeColor(inContext, (inCell.row % 2 == 1) && (inCell.row != 1) ? line_gray_lighter : line_gray, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}
	
	// Turn on aliasing for text
	::CGContextSetShouldAntialias(inContext, true);

	adjustedRect.origin.x += 2.0;
	adjustedRect.size.width -= 4.0;
	
	if (inCell.col == 1)
	{
		PPx::CFString txt;
		bool draw_txt = false;
		if (inCell.row == cAllDayRow)
		{
			txt = CFSTR("All Day");
			draw_txt = true;
		}
		else if (inCell.row % 2 == 0)
		{
			uint32_t hour = (inCell.row - cFirstTimedRow) / 2 + mStartHour;

			// Check for am/pm display here
			if (!iCal::CICalendarLocale::Use24HourTime())
			{
				bool am = (hour < 12);
				if (!am)
				{
					hour -= 12;
				}
				if (hour == 0)
					hour = 12;
				txt.AssignNumericValue<uint32_t>(hour);
				txt.Append(am ? CFSTR(":00 am") : CFSTR(":00 pm"));
			}
			else
			{
				txt.AssignNumericValue<uint32_t>(hour);
				txt.Append(CFSTR(":00"));
			}

			adjustedRect.size.height = 14.0;
			draw_txt = true;
		}
		
		// Draw text
		if (draw_txt)
		{
			Rect box;
			CGUtils::HIToQDRect(adjustedRect, box);
			::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
			::DrawThemeTextBox(txt, kThemeSmallSystemFont, kThemeStateActive, false, &box, teJustRight, inContext);
		}
	}
	
	if (IsNowRow(inCell))
	{
		// Get local point offset
		STableCell todayCell(cFirstTimedRow, mNowCell.col);
		SInt32	cellLeft, cellTop, cellRight, cellBottom;
		GetImageCellBounds(todayCell, cellLeft, cellTop, cellRight, cellBottom);
		
		SPoint32 left_image_pt = { cellLeft, mNowMarker };
		Point local_pt;
		ImageToLocalPoint(left_image_pt, local_pt);
		HIPoint hi_pt;
		CGUtils::QDToHIPoint(local_pt, hi_pt);
		
		// Draw frame components by hand (no aliasing for lines)
		::CGContextSetShouldAntialias(inContext, false);
	
		if ((inCell.col == 1) || IsTodayColumn(inCell))
		{
			// Draw arrows at left/right side

			// Left arrow
			::CGContextBeginPath(inContext);
			::CGContextMoveToPoint(inContext, inLocalRect.origin.x + 1, hi_pt.y - 2);
			::CGContextAddLineToPoint(inContext, inLocalRect.origin.x + 1, hi_pt.y + 2);
			::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextSetRGBFillColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextClosePath(inContext);
			::CGContextStrokePath(inContext);
			::CGContextBeginPath(inContext);
			::CGContextMoveToPoint(inContext, inLocalRect.origin.x + 2, hi_pt.y - 1);
			::CGContextAddLineToPoint(inContext, inLocalRect.origin.x + 2, hi_pt.y + 1);
			::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextSetRGBFillColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextClosePath(inContext);
			::CGContextStrokePath(inContext);

			// Right arrow
			::CGContextBeginPath(inContext);
			::CGContextMoveToPoint(inContext, inLocalRect.origin.x + inLocalRect.size.width - 1, hi_pt.y + 2);
			::CGContextAddLineToPoint(inContext, inLocalRect.origin.x + inLocalRect.size.width - 1, hi_pt.y - 2);
			::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextSetRGBFillColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextClosePath(inContext);
			::CGContextDrawPath(inContext, kCGPathFillStroke);
			::CGContextBeginPath(inContext);
			::CGContextMoveToPoint(inContext, inLocalRect.origin.x + inLocalRect.size.width - 2, hi_pt.y + 1);
			::CGContextAddLineToPoint(inContext, inLocalRect.origin.x + inLocalRect.size.width - 2, hi_pt.y - 1);
			::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextSetRGBFillColor(inContext, 0.95, 0.0, 0.0, 1.0);
			::CGContextClosePath(inContext);
			::CGContextDrawPath(inContext, kCGPathFillStroke);
		}
		
		// Draw solid line for today column or time column
		if (!IsTodayColumn(inCell) && (inCell.col != 1))
		{
			// Dashed line
			float dashes[2] = { 3.0, 3.0 };
			::CGContextSetLineDash(inContext, ((long)inLocalRect.origin.x) % 6, dashes, 2);
		}

		// Draw line
		::CGContextBeginPath(inContext);
		::CGContextMoveToPoint(inContext, inLocalRect.origin.x, hi_pt.y);
		::CGContextAddLineToPoint(inContext, inLocalRect.origin.x + inLocalRect.size.width, hi_pt.y);
		::CGContextSetRGBStrokeColor(inContext, 0.95, 0.0, 0.0, 1.0);
		::CGContextStrokePath(inContext);
		//::CGContextClosePath(inContext);
	}
}

void CDayWeekTable::RefreshNow()
{
	// Redraw - double-buffering removes any flashing
	FRAMEWORK_REFRESH_WINDOW(this)
}

void CDayWeekTable::AdaptToNewSurroundings()
{
	// Allow frame adapter to adjust size
	CCalendarEventTableBase::AdaptToNewSurroundings();

	// Look for change in image size
	UInt32 old_width, old_height;
	mTableGeometry->GetTableDimensions(old_width, old_height);

	// The current frame is the frame set by the scroller and represents the visible area we have available
	// We ignore the super frame size as that does not account for scrollbars etc
	SDimension16 my_frame;
	GetFrameSize(my_frame);

	// Determine new column sizes
	SInt32 col_size = (mCols <= 1) ? 64L : std::max((SInt32) ((my_frame.width - cTimeColumnWidth) / (mCols - 1)), 64L);
	
	SetColWidth(col_size, 2, mCols);

	// If auto-fit rows, change row height
	RescaleHeight();

	SPoint32 pt = {col_size, 2 * GetRowHeight(cFirstTimedRow)};
	SetScrollUnit(pt);

	UInt32 new_width, new_height;
	mTableGeometry->GetTableDimensions(new_width, new_height);

	if ((new_width != old_width) || (new_height != old_height))
	{
		RepositionEvents();
		CheckNow();
	}
	
	if (mTitles)
		mTitles->TableChanged();
}

void CDayWeekTable::RescaleHeight()
{
	// If auto-fit rows, change row height
	if (mScaleRows == 0)
	{
		SDimension16 my_frame;
		GetFrameSize(my_frame);
	
		SInt32 row_size = (mRows <= 1) ? 8L : std::max((SInt32) ((my_frame.height - GetRowHeight(cAllDayRow)) / (mRows - 1)), 8L);
		SetRowHeight(row_size, cFirstTimedRow, mRows);
	}
}

// Keep titles in sync
void CDayWeekTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	if (mTitles)
		mTitles->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

void CDayWeekTable::SelectionChanged()
{
	// Must refresh titles
	mTitles->Refresh();
}

void CDayWeekTable::ScaleRows(uint32_t scale)
{
	if (scale != mScaleRows)
	{
		mScaleRows = scale;
		if (mScaleRows > 0)
			SetRowHeight(cRowHeight / mScaleRows, cFirstTimedRow, mRows);
		else
		{
			RescaleHeight();
		}
		RepositionEvents();
		CheckNow();
	}
}

// Add an event to the month calendar view.
// This is complicated as we must ensure that events do not overlap in a row
// as some events can span multiple columns. To handle this we will insert entire rows
// worth of event objects in the mAllDayEvents[0...mCols - 2] arrays, but use NULL for ones that have not
// yet been assigned. When a new event needs to be added we scan the array looking for a suitable index
// into it such that all entries spanned by the new event are NULL - or else we have to create a new
// row of all NULL's.
//
// Maybe there is an easier way to do this?
///
void CDayWeekTable::AddAllDayEvent(iCal::CICalendarComponentExpandedShared& vevent)
{
	iCal::CICalendarDateTime dtstart(vevent->GetInstanceStart());
	iCal::CICalendarDateTime dtend(vevent->GetInstanceEnd());
	
	// Iterate over each cell and see if event should be in it
	STableCell cell(cAllDayRow, 2);
	TableIndexT col_start = 0;
	TableIndexT col_end = 0;
	bool starts_in_first_column = false;
	bool ends_in_last_column = false;
	for(cell.col = 2; cell.col <= mCols; cell.col++)
	{
		const iCal::CICalendarDateTime& cell_start = GetCellStartDate(cell);
		const iCal::CICalendarDateTime& cell_end = GetCellEndDate(cell);

		// First check that event actually spans this day
		if ((dtend <= cell_start) || (dtstart >= cell_end))
			continue;
		
		// See if this is the first one
		if (col_start == 0)
		{
			col_end = col_start = cell.col;
			starts_in_first_column = (vevent->GetInstanceStart() >= cell_start);
			ends_in_last_column = (vevent->GetInstanceEnd() <= cell_end);
		}
		else
		{
			col_end = cell.col;
			ends_in_last_column = (vevent->GetInstanceEnd() <= cell_end);
		}
	}
	
	// Add an event if required
	if (col_start != 0)
	{
		// Determine vertical offset for this event so that it does not clash with other events
		// in the cells that it spans in this row
		size_t offset = 0;
		
		// NB the mAllDayEvents[col - 2] vectors MUST have the same size in each row to make this easy
		
		// Find an entire slot that is empty for the span of this event
		size_t slot = 0;
		for(CDayEventSlotList::const_iterator iter = mAllDayEvents[col_start - 2].begin(); iter != mAllDayEvents[col_start - 2].end(); iter++, slot++)
		{
			// Look for an empty row in the first column of the event
			if ((*iter).first == NULL)
			{
				// Now scan across other cells and see if empty
				bool all_empty = true;
				for(TableIndexT col_ctr = col_start; col_ctr <= col_end; col_ctr++)
				{
					all_empty &= (mAllDayEvents[col_ctr - 2][slot].first == NULL);
				}
				
				// Exit main loop if we have found an entire empty row portion for the event
				if (all_empty)
					break;
			}
		}
		
		// If we get here without finding an empty portion, we must add new slots for all
		// columns in this row
		if (slot >= mAllDayEvents[col_start - 2].size())
		{
			for(TableIndexT col_ctr = 2; col_ctr <= mCols; col_ctr++)
				mAllDayEvents[col_ctr - 2].push_back(CDayEventSlotList::value_type(NULL, false));
		}
		
		// Determine visual position in table cell
		STableCell start_cell(cell.row, col_start);
		STableCell end_cell(cell.row, col_end);

		Rect start_cellFrameQD;
		HIRect start_cellFrame;
		GetLocalCellRectAlways(start_cell, start_cellFrameQD);
		CGUtils::QDToHIRect(start_cellFrameQD, start_cellFrame);
		HIRect actual_cellFrame = start_cellFrame;

		Rect end_cellFrameQD;
		HIRect end_cellFrame;
		GetLocalCellRectAlways(end_cell, end_cellFrameQD);
		CGUtils::QDToHIRect(end_cellFrameQD, end_cellFrame);

		start_cellFrame.size.width = end_cellFrame.origin.x - start_cellFrame.origin.x + end_cellFrame.size.width;

		start_cellFrame.origin.y += cEventOffset * slot + cEventMargin;
		start_cellFrame.size.height = cEventOffset;

		// Create new month event
		CDayEvent* event = CDayEvent::Create(this, start_cellFrame);
		event->Add_Listener(this);
		
		// For non-all day event that starts in this row, prepend its time to the title
		cdstring summary;
		summary = vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
	
		// Now set event details
		event->SetDetails(vevent, this, summary.c_str(), true, starts_in_first_column, ends_in_last_column, true);
		event->SetColumnSpan(col_end - col_start + 1);

		// Modify all columns - first is the real one, others are pseudo
		for(TableIndexT col_ctr = col_start; col_ctr <= col_end; col_ctr++)
			mAllDayEvents[col_ctr - 2][slot] = std::make_pair(event, col_ctr == col_start);
		
		// Now show it
		event->SetVisible(true);
	}
}

// Add an event to the month calendar view.
// This is complicated as we must ensure that events do not overlap in a row
// as some events can span multiple columns. To handle this we will insert entire rows
// worth of event objects in the mAllDayEvents[0...mCols - 2] arrays, but use NULL for ones that have not
// yet been assigned. When a new event needs to be added we scan the array looking for a suitable index
// into it such that all entries spanned by the new event are NULL - or else we have to create a new
// row of all NULL's.
//
// Maybe there is an easier way to do this?
///
void CDayWeekTable::AddTimedEvent(iCal::CICalendarComponentExpandedShared& vevent)
{
	iCal::CICalendarDateTime dtstart(vevent->GetInstanceStart());
	iCal::CICalendarDateTime dtend(vevent->GetInstanceEnd());
	bool all_day = vevent->GetInstanceStart().IsDateOnly();
	CDayEvent* prev_event = NULL;
	
	// Iterate over each cell and see if event should be in it
	STableCell cell(cFirstTimedRow, 0);
	bool starts_in_first_column = false;
	bool ends_in_last_column = false;
	for(cell.col = 2; cell.col <= mCols; cell.col++)
	{
		const iCal::CICalendarDateTime& cell_start = GetTimedStartDate(cell);
		const iCal::CICalendarDateTime& cell_end = GetTimedEndDate(cell);

		// First check that event actually spans this day
		if ((dtend <= cell_start) || (dtstart >= cell_end))
			continue;
		
		starts_in_first_column = (vevent->GetInstanceStart() >= cell_start);
		ends_in_last_column = (vevent->GetInstanceEnd() <= cell_end);

		Rect start_cellFrameQD;
		HIRect start_cellFrame;
		GetLocalCellRectAlways(cell, start_cellFrameQD);
		CGUtils::QDToHIRect(start_cellFrameQD, start_cellFrame);

		// Create new day event
		CDayEvent* event = CDayEvent::Create(this, start_cellFrame);
		event->Add_Listener(this);
		
		// For non-all day event that starts in this row, prepend its time to the title
		cdstring summary;
		if (starts_in_first_column && CPreferences::sPrefs->mDisplayTime.GetValue())
		{
			summary = vevent->GetInstanceStart().GetAdjustedTime(mTimezone).GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
			summary += " ";
			summary += vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
		}
		else
			summary = vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
	
		// Now set event details
		event->SetDetails(vevent, this, summary.c_str(), false, starts_in_first_column, ends_in_last_column, false);
		event->SetColumnSpan(1);

		// Now link to previous one
		if (prev_event)
		{
			event->SetPreviousLink(prev_event);
			prev_event->SetNextLink(event);
		}
		prev_event = event;

		// Add to list
		mTimedEvents[cell.col - 2].push_back(event);
		
		// Now show it
		PositionTimedEvent(event, cell, cell_start.GetPosixTime());
		event->SetVisible(true);
	}
}

void CDayWeekTable::AddTimedFreeBusy(iCal::CICalendarComponent* vfreebusy)
{
	// Get the actual VFREEBUSY item
	iCal::CICalendarVFreeBusy* fb = dynamic_cast<iCal::CICalendarVFreeBusy*>(vfreebusy);
	if ((fb == NULL) || (fb->GetBusyTime() == NULL))
		return;
	
	// Now look at each period
	CDayEvent* prev_event = NULL;
	for(iCal::CICalendarFreeBusyList::const_iterator iter1 = fb->GetBusyTime()->begin(); iter1 != fb->GetBusyTime()->end(); iter1++)
	{
		// Must be busy item
		if ((*iter1).GetType() == iCal::CICalendarFreeBusy::eFree)
			continue;

		iCal::CICalendarDateTime dtstart((*iter1).GetPeriod().GetStart());
		iCal::CICalendarDateTime dtend((*iter1).GetPeriod().GetEnd());

		// Iterate over each cell and see if event should be in it
		STableCell cell(cFirstTimedRow, 0);
		bool starts_in_first_column = false;
		bool ends_in_last_column = false;
		for(cell.col = 2; cell.col <= mCols; cell.col++)
		{
			const iCal::CICalendarDateTime& cell_start = GetTimedStartDate(cell);
			const iCal::CICalendarDateTime& cell_end = GetTimedEndDate(cell);

			// First check that event actually spans this day
			if ((dtend <= cell_start) || (dtstart >= cell_end))
				continue;
			
			starts_in_first_column = ((*iter1).GetPeriod().GetStart() >= cell_start);
			ends_in_last_column = ((*iter1).GetPeriod().GetEnd() <= cell_end);

			Rect start_cellFrameQD;
			HIRect start_cellFrame;
			GetLocalCellRectAlways(cell, start_cellFrameQD);
			CGUtils::QDToHIRect(start_cellFrameQD, start_cellFrame);

			// Create new day event
			CDayEvent* event = CDayEvent::Create(this, start_cellFrame);
			event->Add_Listener(this);
			
			// For non-all day event that starts in this row, prepend its time to the title
			cdstring summary;
			if (starts_in_first_column && CPreferences::sPrefs->mDisplayTime.GetValue())
			{
				summary = (*iter1).GetPeriod().GetStart().GetAdjustedTime(mTimezone).GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
				summary += " ";
				summary += rsrc::GetString("CDayWeekTable::Busy");
			}
			else
				summary = rsrc::GetString("CDayWeekTable::Busy");
		
			// Now set event details
			event->SetDetails(fb, (*iter1).GetPeriod(), this, summary.c_str(), false, starts_in_first_column, ends_in_last_column, false);
			event->SetColumnSpan(1);

			// Now link to previous one
			if (prev_event)
			{
				event->SetPreviousLink(prev_event);
				prev_event->SetNextLink(event);
			}
			prev_event = event;

			// Add to list
			mTimedEvents[cell.col - 2].push_back(event);
			
			// Now show it
			PositionTimedEvent(event, cell, cell_start.GetPosixTime());
			event->SetVisible(true);
		}
	}
}

void CDayWeekTable::PositionAllDayEvent(CDayEvent* event, const STableCell& cell, size_t offset)
{
	if ((event == NULL) || !IsValidCell(cell))
		return;

	Rect cellFrameQD;
	HIRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrameQD);
	CGUtils::QDToHIRect(cellFrameQD, cellFrame);
	cellFrame.origin.y += cEventOffset * offset + cEventMargin;
	cellFrame.size.height = cEventOffset;
	cellFrame.size.width *= event->GetColumnSpan();
	
	event->PlaceInSuperFrameAt(cellFrame.origin.x, cellFrame.origin.y, false);
	event->ResizeFrameTo(cellFrame.size.width, cellFrame.size.height, true);
}

void CDayWeekTable::PositionTimedEvent(CDayEvent* event, const STableCell& cell, int64_t top_secs)
{
	if ((event == NULL) || !IsValidCell(cell))
		return;
	
	int64_t start_secs = event->GetInstancePeriod().GetStart().GetPosixTime();
	int64_t end_secs = event->GetInstancePeriod().GetEnd().GetPosixTime();
	
	int64_t hours_in_column = (mRows - 1LL) / 2LL;
	int64_t bottom_secs = top_secs + hours_in_column * 60LL * 60LL;

	Rect cellFrameQD;
	HIRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrameQD);
	CGUtils::QDToHIRect(cellFrameQD, cellFrame);
	float columnHeight = GetRowHeight(cFirstTimedRow) * (mRows - 1);
	
	// Now set the top of the event's frame
	HIRect eventFrame = cellFrame;
	if (start_secs > top_secs)
	{
		eventFrame.origin.y += (start_secs - top_secs) * columnHeight / (hours_in_column * 60.0 * 60.0);
	}
	else
		start_secs = top_secs;
	
	// Now set bottom of the event's frame
	if (end_secs > bottom_secs)
	{
		eventFrame.size.height = columnHeight - (eventFrame.origin.y - cellFrame.origin.y);
	}
	else
	{
		eventFrame.size.height = (end_secs - top_secs) * columnHeight / (hours_in_column * 60.0 * 60.0) - (eventFrame.origin.y - cellFrame.origin.y);
	}
	eventFrame.origin.y = ::roundf(eventFrame.origin.y - 1);
	eventFrame.size.height++;

	// Now adjust for overlaps
	if (event->GetColumnTotal() != 1)
	{
		// Offset by relative offset
		eventFrame.origin.x += eventFrame.size.width * event->GetRelativeOffset();

		// Width from relative width
		eventFrame.size.width *= event->GetRelativeWidth();
	}

	event->PlaceInSuperFrameAt(eventFrame.origin.x, eventFrame.origin.y, false);
	event->ResizeFrameTo(eventFrame.size.width, eventFrame.size.height, true);
}

// Resolve overlapping events
		struct SEventInfo
		{
			CDayEvent*	mEvent;
			bool		mStarts;
			uint32_t	mColumn;
			int64_t		mPosix;
			uint32_t	mTotalCol;
			
			SEventInfo(CDayEvent* event, bool starts, uint32_t column, int64_t posix)
			{
				mEvent = event; mStarts = starts; mColumn = column; mPosix = posix; mTotalCol = 0;
			}
			SEventInfo(const SEventInfo& copy)
			{
				mEvent = copy.mEvent; mStarts = copy.mStarts; mColumn = copy.mColumn; mPosix = copy.mPosix; mTotalCol = copy.mTotalCol;
			}
			bool operator<(const SEventInfo& comp) const
			{
				if (mPosix != comp.mPosix) return mPosix < comp.mPosix;
				else if (mStarts ^ comp.mStarts) return !mStarts;
				else return mColumn < comp.mColumn;
			}
			
		};

void CDayWeekTable::ColumnateEvents()
{
	// Look for overlaps in each column
	for(TableIndexT col = 2; col <= mCols; col++)
	{
		CDayEventList& list = mTimedEvents[col - 2];
		
		std::vector<SEventInfo> data;
		std::vector<SEventInfo*> column_usage;
		column_usage.push_back(NULL);

		// Add each start/end position
		uint32_t col = 0;
		for(CDayEventList::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			data.push_back(SEventInfo(*iter, true, col++, (*iter)->GetInstancePeriod().GetStart().GetPosixTime()));
			data.push_back(SEventInfo(*iter, false, col++, (*iter)->GetInstancePeriod().GetEnd().GetPosixTime()));
		}
		
		// Sort by posix time
		std::sort(data.begin(), data.end());
		
		// We now have a sorted array of start/end periods
		
		// Create the column data
		uint32_t total_col = 0;
		CDayEvent* last_start = NULL;
		uint32_t last_start_col = 0;
		for(std::vector<SEventInfo>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			// Look for start/end
			if ((*iter).mStarts)
			{
				// Add to the next available column
				std::vector<SEventInfo*>::iterator found = std::find(column_usage.begin(), column_usage.end(), static_cast<SEventInfo*>(NULL));
				if (found == column_usage.end())
				{
					column_usage.push_back(&(*iter));
					(*iter).mColumn = column_usage.size() - 1;
					(*iter).mEvent->SetColumnOffset((*iter).mColumn);
				}
				else
				{
					*found = &(*iter);
					(*iter).mColumn = found - column_usage.begin();
					(*iter).mEvent->SetColumnOffset((*iter).mColumn);
				}
				(*iter).mEvent->SetSpanToEdge(false);
				(*iter).mTotalCol = ++total_col;
				
				last_start = (*iter).mEvent;
				last_start_col = (*iter).mColumn;
			}
			else
			{
				// See if this was the previous started event
				if ((*iter).mEvent == last_start)
				{
					// May be able to span columns
					bool cols_free_to_right = true;
					uint32_t col_index = (*iter).mEvent->GetColumnOffset();
					for(col_index++; col_index < column_usage.size(); col_index++)
					{
						if (column_usage[col_index] != NULL)
						{
							cols_free_to_right = false;
							break;
						}
					}
					
					// Now mark the event as spanning
					(*iter).mEvent->SetSpanToEdge(cols_free_to_right);
				}
				
				// Reset last start if the one ending is to the right of it
				else if ((*iter).mEvent->GetColumnOffset() > last_start_col)
				{
					last_start = NULL;
					last_start_col = 0;
				}

				// Remove from column
				column_usage[(*iter).mEvent->GetColumnOffset()] = NULL;
				(*iter).mColumn = (*iter).mEvent->GetColumnOffset();
				(*iter).mTotalCol = --total_col;
			}
		}
		
		// We now have the sequenced start/end array so calculate the total columnn width
		// for each non-zero sequence
		std::vector<SEventInfo>::iterator start = data.begin();
		uint32_t max_column = 1;
		for(std::vector<SEventInfo>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			if ((*iter).mStarts)
			{
				max_column = std::max(max_column, (*iter).mTotalCol);
			}
			else if ((*iter).mTotalCol == 0)
			{
				// Go back over the last set changing total col to the max value
				for(std::vector<SEventInfo>::iterator iter2 = start; iter2 != iter; iter2++)
					(*iter2).mTotalCol = max_column;
				
				// Reset start iterator
				start = iter;
				max_column = 1;
			}
		}
		
		// Now we have the array of data all set for the actual layout
		for(std::vector<SEventInfo>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			// Layout items when they start and more than one per column
			if ((*iter).mStarts && ((*iter).mTotalCol != 1))
			{
				// We only change the width and horizontal offset
				CDayEvent* event = (*iter).mEvent;
				event->SetColumnOffset((*iter).mColumn);
				event->SetColumnTotal((*iter).mTotalCol);

				Rect rect;
				event->CalcLocalFrameRect(rect);
				HIRect frame;
				CGUtils::QDToHIRect(rect, frame);

				float column_width = frame.size.width;

				// Cache relative value which get used when repositioning events
				event->SetRelativeWidth(1.0 / event->GetColumnTotal());
				event->SetRelativeOffset(((float)event->GetColumnOffset()) / ((float)event->GetColumnTotal()));

				// Adjust frame 
				frame.origin.x += column_width * event->GetRelativeOffset();
				frame.size.width = column_width * event->GetRelativeWidth();

				event->PlaceInSuperFrameAt(frame.origin.x, frame.origin.y, false);
				event->ResizeFrameTo(frame.size.width, frame.size.height, true);

				// Look for a spanning item and adjust all the others with the same start/end
				if (event->GetSpanToEdge())
				{
					// Find all previous ones with the same start/end
					std::vector<SEventInfo>::iterator start_spanners = iter;
					std::vector<SEventInfo>::iterator end_spanners = iter;
					uint32_t spanning_items = 1;
					for(std::vector<SEventInfo>::iterator iter2 = start_spanners - 1; iter2 != data.begin(); iter2--, start_spanners--)
					{
						if ((*iter2).mStarts)
						{
							CDayEvent* event2 = (*iter2).mEvent;
							if ((event2->GetInstancePeriod().GetStart() != event->GetInstancePeriod().GetStart()) ||
								(event2->GetInstancePeriod().GetEnd() != event->GetInstancePeriod().GetEnd()))
							{
								break;
							}
							spanning_items++;
						}
					}
					
					// Now adjust widths of all spanning items to account for free space
					uint32_t free_columns = event->GetColumnTotal() - event->GetColumnOffset() - 1;
					float rel_move_by = 0.0;
					float rel_adjust_width = ((float)free_columns) / ((float)spanning_items) / ((float)event->GetColumnTotal());
					for(std::vector<SEventInfo>::iterator iter2 = start_spanners; iter2 != end_spanners + 1; iter2++)
					{
						if ((*iter2).mStarts)
						{
							CDayEvent* event2 = (*iter2).mEvent;
							event2->SetRelativeWidth(event2->GetRelativeWidth() + rel_adjust_width);
							event2->SetRelativeOffset(event2->GetRelativeOffset() + rel_move_by);

							// Adjust frame
							Rect rect2;
							event2->CalcLocalFrameRect(rect2);
							HIRect frame2;
							CGUtils::QDToHIRect(rect2, frame2);

							frame2.origin.x += column_width * rel_move_by;
							frame2.size.width += column_width * rel_adjust_width;

							event2->PlaceInSuperFrameAt(frame2.origin.x, frame2.origin.y, false);
							event2->ResizeFrameTo(frame2.size.width, frame2.size.height, true);

							// Bump up move for next item
							rel_move_by += rel_adjust_width;
						}
					}
				}
			}
		}
	}
}
