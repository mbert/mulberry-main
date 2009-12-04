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

#include "CMonthTable.h"

#include "CGUtils.h"
#include "CMonthEvent.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarComponentExpanded.h"
#include "CICalendarLocale.h"

#include <LTableArrayStorage.h>
#include <LTableMonoGeometry.h>
#include <LTableSingleSelector.h>
#include "MyCFString.h"

const uint32_t cEventOffset = 18;
const uint32_t cEventHeight = 16;

const float line_gray = 0.75;
const float line_gray_lighter = 0.875;

// ---------------------------------------------------------------------------
//	CMonthTable														  [public]
/**
	Default constructor */

CMonthTable::CMonthTable(LStream *inStream) :
	CCalendarEventTableBase(inStream)
{
	mTodayCell.SetCell(0, 0);
	
	// Single geometry (all rows/columns are the same size)
	mTableGeometry = new LTableMonoGeometry(this, 100, 70);
	
	// Single selector
	mTableSelector = new LTableSingleSelector(this);
	
	// Storage
	mTableStorage = new LTableArrayStorage(this, sizeof(bool));
}


// ---------------------------------------------------------------------------
//	~CMonthTable														  [public]
/**
	Destructor */

CMonthTable::~CMonthTable()
{
	ClearEvents();
}

#pragma mark -

void
CMonthTable::FinishCreateSelf()
{
	// Call inherited
	CCalendarEventTableBase::FinishCreateSelf();
	
	AdaptToNewSurroundings();
}

void CMonthTable::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
{
	// Broadcast double-click if in month area
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
	{
		iCal::CICalendarDateTime dt(GetCellStartDate(inCell));
		Broadcast_Message(eBroadcast_DblClkToDay, &dt);
	}
	else
		CCalendarEventTableBase::ClickCell(inCell, inMouseDown);
}

void CMonthTable::DrawCell(const STableCell &inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	bool today_cell = mTodayCell == inCell;

	int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[inCell.row - 1][inCell.col - 1]);
	if (mday < 0)
	{
		if (today_cell)
			::CGContextSetRGBFillColor(inContext, 0.75, 0.75, 0.9, 1.0);
		else
			::CGContextSetGrayFillColor(inContext, 0.9, 1.0);
		::CGContextFillRect(inContext, adjustedRect);
		mday = -mday;
	}
	else
	{
		if (today_cell)
			::CGContextSetRGBFillColor(inContext, 0.85, 0.85, 1.0, 1.0);
		else
			::CGContextSetGrayFillColor(inContext, 1.0, 1.0);
		::CGContextFillRect(inContext, adjustedRect);
	}

	// Draw frame (no aliasing for lines)
	::CGContextSetShouldAntialias(inContext, false);
	
	::CGContextSetGrayStrokeColor(inContext, line_gray, 1.0);
	::CGContextStrokeRect(inContext, adjustedRect);
	
	// Turn on aliasing for text
	::CGContextSetShouldAntialias(inContext, true);

	// Do selection after background
	if (CellIsSelected(inCell))
		DrawCellSelection(inCell, inContext);

	// get numeric day as string
	cdstring title(mday);

	// If this cell is the first day in a month, or the first cell in the table, then
	// also draw the month name on the right-side
	if ((mday == 1) || (inCell == STableCell(1, 1)))
	{
		int32_t month = iCal::CICalendarUtils::UnpackDateMonth(mData[inCell.row - 1][inCell.col - 1]);
		title += " ";
		title += iCal::CICalendarLocale::GetMonth(month, (inLocalRect.size.width < 80) ? iCal::CICalendarLocale::eShort : iCal::CICalendarLocale::eLong);
	}
	
	MyCFString temp(title, kCFStringEncodingUTF8);

	adjustedRect = ::CGRectInset(adjustedRect, 2.0, 0.0);

	adjustedRect.size.height = 16.0;
	Rect box;
	CGUtils::HIToQDRect(adjustedRect, box);
	::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
	::DrawThemeTextBox(temp, kThemeSystemFont, kThemeStateActive, false, &box, teJustLeft, inContext);
	
	// If more data than cell can display then provide indicator
	bool more;
	UInt32 dataSize = sizeof(bool);
	GetCellData(inCell, &more, dataSize);
	if (more)
	{
		MyCFString temp2(rsrc::GetString("::More"), kCFStringEncodingUTF8);
		::DrawThemeTextBox(temp2, kThemeSmallSystemFont, kThemeStateActive, false, &box, teJustRight, inContext);
	}
}

bool CMonthTable::DrawCellSelection(const STableCell& inCell, CGContextRef inContext)
{
	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = IsActive();
	
	Rect cellFrame;
	if (selected && mDrawSelection && GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		if (active)
		{
			HIRect inLocalRect;
			CGUtils::QDToHIRect(cellFrame, inLocalRect);

			CGUtils::CGContextSaver _cg(inContext);

			// Set theme selection colour
			RGBColor selectColor;
			::GetThemeBrushAsColor(kThemeBrushPrimaryHighlightColor, 32, true, &selectColor);
			::CGContextSetRGBStrokeColor(inContext, ((float)selectColor.red)/ 65535.0, ((float)selectColor.green)/ 65535.0, ((float)selectColor.blue)/ 65535.0, 1.0);

			// Draw thick border rect inside
			HIRect drawit = ::CGRectInset(inLocalRect, 1.0, 1.0);
			::CGContextStrokeRectWithWidth(inContext, drawit, 3.0);
			
			return true;
		}
	}
	
	return false;
}

void CMonthTable::AdaptToNewSurroundings()
{
	// Allow frame adapter to adjust size
	CCalendarEventTableBase::AdaptToNewSurroundings();

	SDimension16 my_frame;
	GetFrameSize(my_frame);

	// Determine new row and column sizes
	SInt32 row_size = (mRows == 0) ? 64L : std::max((SInt32) (my_frame.height/ mRows), 64L);
	SInt32 col_size = (mCols == 0) ? 64L : std::max((SInt32) (my_frame.width / mCols), 64L);
	
	SetRowHeight(row_size, 1, 1);
	SetColWidth(col_size, 1, 1);

	//ResizeFrameTo(GetColWidth(1) * mCols, GetRowHeight(1) * mRows, true);
	RepositionEvents();
}

// Add an event to the month calendar view.
// This is complicated as we must ensure that events do not overlap in a row
// as some events can span multiple columns. To handle this we will insert entire rows
// worth of event objects in the mEvents[row][0...6] arrays, but use NULL for ones that have not
// yet been assigned. When a new event needs to be added we scan the array looking for a suitable index
// into it such that all entries spanned by the new event are NULL - or else we have to create a new
// row of all NULL's.
//
// Maybe there is an easier way to do this?
///
void CMonthTable::AddEvent(iCal::CICalendarComponentExpandedShared& vevent)
{
	iCal::CICalendarDateTime dtstart(vevent->GetInstanceStart());
	iCal::CICalendarDateTime dtend(vevent->GetInstanceEnd());
	bool all_day = vevent->GetInstanceStart().IsDateOnly();
	CMonthEvent* prev_event = NULL;
	
	// Iterate over each cell and see if event should be in it
	for(TableIndexT row = 1; row <= mRows; row++)
	{
		TableIndexT col_start = 0;
		TableIndexT col_end = 0;
		bool starts_in_first_column = false;
		bool ends_in_last_column = false;

		// To quick check to see if event is anywhere in the row
		STableCell row_start_cell(row, 1);
		STableCell row_end_cell(row, mCols);
		const iCal::CICalendarDateTime& row_start = GetCellStartDate(row_start_cell);
		const iCal::CICalendarDateTime& row_end = GetCellEndDate(row_end_cell);

		// First check that event actually spans this range of days
		if ((dtend <= row_start) || (dtstart >= row_end))
			continue;
		
		// Now examine each cell and determine the span of columns for the event
		for(TableIndexT col = 1; col <= mCols; col++)
		{
			STableCell cell(row, col);

			const iCal::CICalendarDateTime& cell_start = GetCellStartDate(cell);
			const iCal::CICalendarDateTime& cell_end = GetCellEndDate(cell);

			// First check that event actually spans this day
			if ((dtend <= cell_start) || (dtstart >= cell_end))
				continue;
			
			// See if this is the first one
			if (col_start == 0)
			{
				col_end = col_start = col;
				starts_in_first_column = (vevent->GetInstanceStart() >= cell_start);
				ends_in_last_column = (vevent->GetInstanceEnd() <= cell_end);
				
				// Non-all day events only appear in one column, though if they go over they will
				// be drawn with the zig-zag indicator at the end
				if (!all_day)
					break;
			}
			else
			{
				col_end = col;
				ends_in_last_column = (vevent->GetInstanceEnd() <= cell_end);
			}
		}
		
		// Add an event if required
		if (col_start != 0)
		{
			// Determine vertical offset for this event so that it does not clash with other events
			// in the cells that it spans in this row
			size_t offset = 0;
			
			// NB the mEvents[row - 1][col - 1] vectors MUST have the same size in each row to make this easy
			
			// Find an entire slot that is empty for the span of this event
			size_t slot = 0;
			for(CMonthEventSlotList::const_iterator iter = mEvents[row - 1][col_start - 1].begin(); iter != mEvents[row - 1][col_start - 1].end(); iter++, slot++)
			{
				// Look for an empty row in the first column of the event
				if ((*iter).first == NULL)
				{
					// Now scan across other cells and see if empty
					bool all_empty = true;
					for(TableIndexT col_ctr = col_start; col_ctr <= col_end; col_ctr++)
					{
						all_empty &= (mEvents[row - 1][col_ctr - 1][slot].first == NULL);
					}
					
					// Exit main loop if we have found an entire empty row portion for the event
					if (all_empty)
						break;
				}
			}
			
			// If we get here without finding an empty portion, we must add new slots for all
			// columns in this row
			if (slot >= mEvents[row - 1][col_start - 1].size())
			{
				for(TableIndexT col_ctr = 1; col_ctr <= 7; col_ctr++)
					mEvents[row - 1][col_ctr - 1].push_back(CMonthEventSlotList::value_type(NULL, false));
			}
			
			// Determine visual poisition in table cell
			STableCell start_cell(row, col_start);
			STableCell end_cell(row, col_end);

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

			start_cellFrame.origin.y += cEventOffset + cEventHeight * slot;
			start_cellFrame.size.height = cEventHeight;

			// Create new month event
			CMonthEvent* event = CMonthEvent::Create(this, start_cellFrame);
			event->Add_Listener(this);
			
			// For non-all day event that starts in this row, prepend its time to the title
			cdstring summary;
			if (!all_day && starts_in_first_column && CPreferences::sPrefs->mDisplayTime.GetValue())
			{
				summary = vevent->GetInstanceStart().GetAdjustedTime(mTimezone).GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
				summary += " ";
				summary += vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
			}
			else
				summary = vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
		
			// Now set event details
			event->SetDetails(vevent, this, summary.c_str(), all_day, starts_in_first_column, ends_in_last_column, true);
			event->SetColumnSpan(col_end - col_start + 1);

			// Now link to previous one
			if (prev_event)
			{
				event->SetPreviousLink(prev_event);
				prev_event->SetNextLink(event);
			}
			prev_event = event;

			// Modify all columns - first is the real one, others are pseudo
			for(TableIndexT col_ctr = col_start; col_ctr <= col_end; col_ctr++)
				mEvents[row - 1][col_ctr - 1][slot] = std::make_pair(event, col_ctr == col_start);
			
			// Now show it if it fits entirely within the cell
			bool visible = (start_cellFrame.origin.y + start_cellFrame.size.height <= actual_cellFrame.origin.y + actual_cellFrame.size.height);
			event->SetVisible(visible);

			// Mark cell
			if (!visible)
			{
				bool more = true;
				SetCellData(start_cell, &more, sizeof(bool));
			}
		}
	}
}

void CMonthTable::PositionEvent(CMonthEvent* event, const STableCell& cell, size_t offset)
{
	if ((event == NULL) || !IsValidCell(cell))
		return;

	Rect cellFrameQD;
	HIRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrameQD);
	CGUtils::QDToHIRect(cellFrameQD, cellFrame);
	HIRect eventFrame = cellFrame;

	eventFrame.origin.y += cEventOffset + cEventHeight * offset;
	eventFrame.size.height = cEventHeight;
	eventFrame.size.width *= event->GetColumnSpan();
	
	event->PlaceInSuperFrameAt(eventFrame.origin.x, eventFrame.origin.y, false);
	event->ResizeFrameTo(eventFrame.size.width, eventFrame.size.height, true);
	
	// Show hide depending on whether it fits entirely within the cell
	bool visible = (eventFrame.origin.y + eventFrame.size.height <= cellFrame.origin.y + cellFrame.size.height);
	event->SetVisible(visible);
	
	// Mark cell
	if (!visible)
	{
		bool more = true;
		SetCellData(cell, &more, sizeof(bool));
	}
}
