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

#include "CCalendarUtils.h"
#include "CDayEvent.h"
#include "CDayWeekTitleTable.h"
#include "CDayWeekView.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableColumnSelector.h"
#include "CXStringResources.h"

#include "CICalendarComponentExpanded.h"
#include "CICalendarLocale.h"

#include <WIN_LTableMultiGeometry.h>

#include <algorithm>

const uint32_t cRowHeight = 24;
const uint32_t cTimeColumnWidth = 56;
const uint32_t cDayColumnWidth = 128;
const uint32_t cNumberRows = 1 + 48;	// One for all-day, 48 for half-hour intervals
const uint32_t cAllDayRow = 1;
const uint32_t cFirstTimedRow = 2;
const uint32_t cEventOffset = 16;
const uint32_t cEventMargin = 4;

BEGIN_MESSAGE_MAP(CDayWeekTable, CCalendarEventTableBase)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CDayWeekTable														  [public]
/**
	Default constructor */

CDayWeekTable::CDayWeekTable()
{
	mTitles = NULL;

	mNowCell.SetCell(0, 0);
	mScaleRows = 0;
	mStartHour = 0;
	mEndHour = 24;

	mTableGeometry = new LTableMultiGeometry(this, 128, 24);
	
	// Single selector
	mTableSelector = new CTableSingleColumnSelector(this);
	
	mCurrentNowMarker = 0xFFFFFFFF;
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

int CDayWeekTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarEventTableBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	InsertCols(2, 1);
	InsertRows(cNumberRows, 1);

	SetRowHeight(cRowHeight, cAllDayRow, cAllDayRow);
	SetRowHeight(cRowHeight / (mScaleRows == 0 ? 1 : mScaleRows), cFirstTimedRow, mRows);

	SetColWidth(cTimeColumnWidth, 1, 1);
	SetColWidth(cDayColumnWidth, 2, 2);

	AdjustSize();
	
	return 0;
}

// Resize columns
void CDayWeekTable::OnSize(UINT nType, int cx, int cy)
{
	CCalendarEventTableBase::OnSize(nType, cx, cy);

	AdjustSize();
}

iCal::CICalendarDateTime CDayWeekTable::GetClickDateTime(const CPoint& localPt) const
{
	CPoint imagePt;
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

			uint32_t click_secs = top_secs + (bot_secs - top_secs) * (imagePt.y - cellTop) / (cellBottom - cellTop);

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

void CDayWeekTable::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CWnd::OnLButtonDblClk(nFlags, point);

	STableCell	hitCell;
	CPoint		imagePt;

	LocalToImagePoint(point, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		// Broadcast double-click if in day area
		if (hitCell.col > 1)
		{
			iCal::CICalendarDateTime dt(GetClickDateTime(point));
			Broadcast_Message(eBroadcast_DblClkToDay, &dt);
		}
	}
}

void CDayWeekTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState		save(pDC);
	CRect			adjustedRect = inLocalRect;

	// Do selection/today background
	if (CellIsSelected(inCell) && (inCell.col > 1) && mDrawSelection)
	{
		pDC->SetBkColor(CCalendarUtils::GetWinColor(0.85, 0.85, 0.85));
		pDC->ExtTextOut(adjustedRect.left, adjustedRect.top, ETO_OPAQUE, adjustedRect, _T(""), 0, NULL);
	}
	else if (IsTodayColumn(inCell))
	{
		pDC->SetBkColor(CCalendarUtils::GetWinColor(0.95, 0.95, 1.0));
		pDC->ExtTextOut(adjustedRect.left, adjustedRect.top, ETO_OPAQUE, adjustedRect, _T(""), 0, NULL);
	}

	// Left-side always
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinColor(0.5, 0.5, 0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.left, adjustedRect.top);
		pDC->LineTo(adjustedRect.left, adjustedRect.bottom);
		pDC->SelectObject(old_pen);
	}

	// Right-side only for last column
	if (inCell.col == mCols)
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinColor(0.5, 0.5, 0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.right, adjustedRect.top);
		pDC->LineTo(adjustedRect.right, adjustedRect.bottom);
		pDC->SelectObject(old_pen);
	}

	// Top-side always (lighter line above half-hour row)
	// No lighter line if row height <= 18
	if ((GetRowHeight(cFirstTimedRow) > 18) || (inCell.row % 2 != 1))
	{
		CPen temp(PS_SOLID, 1, (inCell.row % 2 == 1) && (inCell.row != 1) ? CCalendarUtils::GetWinColor(0.75, 0.75, 0.75) : CCalendarUtils::GetWinColor(0.5, 0.5, 0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.left, adjustedRect.top);
		pDC->LineTo(adjustedRect.right, adjustedRect.top);
		pDC->SelectObject(old_pen);
	}

	// Bottom-side always
	if ((GetRowHeight(cFirstTimedRow) > 18) || (inCell.row % 2 != 0))
	{
		CPen temp(PS_SOLID, 1, CCalendarUtils::GetWinColor(0.5, 0.5, 0.5));
		CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
		pDC->MoveTo(adjustedRect.left, adjustedRect.bottom);
		pDC->LineTo(adjustedRect.right, adjustedRect.bottom);
		pDC->SelectObject(old_pen);
	}
	
	adjustedRect.left += 2;
	adjustedRect.right -= 2;
	
	if (inCell.col == 1)
	{
		cdstring txt;
		bool draw_txt = false;
		if (inCell.row == cAllDayRow)
		{
			txt = rsrc::GetString("CDayWeekTable::All Day");
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
				txt = (long) hour;
				txt += am ? rsrc::GetString("CDayWeekTable::00 am") : rsrc::GetString("CDayWeekTable::00 pm");
			}
			else
			{
				txt = (long) hour;
				txt += rsrc::GetString("CDayWeekTable::00");
			}

			adjustedRect.bottom = adjustedRect.top + 14;
			draw_txt = true;

			// Determine appropriate font size
			pDC->SelectObject(CFontCache::GetListFontScaled(adjustedRect.Height() - 2));

		}
		
		// Draw text
		if (draw_txt)
		{
			pDC->SetTextColor(CDrawUtils::sBlackColor);
			CRect clipRect(adjustedRect);
			clipRect.DeflateRect(0, 1);
			::DrawClippedStringUTF8(pDC, txt, CPoint(adjustedRect.left, adjustedRect.top), clipRect, eDrawString_Right);
		}
	}

	if (IsNowRow(inCell))
	{
		CBrush temp_brush(CCalendarUtils::GetWinColor(0.95, 0.0, 0.0));
		CBrush* old_brush = pDC->SelectObject(&temp_brush);
		CPen temp_pen(PS_SOLID, 1, CCalendarUtils::GetWinColor(0.95, 0.0, 0.0));
		LOGBRUSH lb;
		temp_brush.GetLogBrush(&lb);
		CPen temp_pen_dash;
		temp_pen_dash.CreatePen(PS_DASH, 1, &lb);
		CPen* old_pen = pDC->SelectObject(&temp_pen);
		pDC->SetBkColor(CCalendarUtils::GetWinColor(0.95, 0.0, 0.0));

		// Get local point offset
		STableCell todayCell(cFirstTimedRow, mNowCell.col);
		SInt32	cellLeft, cellTop, cellRight, cellBottom;
		GetImageCellBounds(todayCell, cellLeft, cellTop, cellRight, cellBottom);
		
		CPoint left_image_pt(cellLeft, mNowMarker);
		CPoint local_pt;
		ImageToLocalPoint(left_image_pt, local_pt);
		
		if ((inCell.col == 1) || IsTodayColumn(inCell))
		{
			// Draw arrows at left/right side

			// Left arrow
			pDC->BeginPath();
			pDC->MoveTo(inLocalRect.left, local_pt.y - 2);
			pDC->LineTo(inLocalRect.left, local_pt.y + 2);
			pDC->LineTo(inLocalRect.left + 2, local_pt.y);
			pDC->LineTo(inLocalRect.left, local_pt.y - 2);
			pDC->EndPath();
			pDC->StrokeAndFillPath();

			// Right arrow
			pDC->BeginPath();
			pDC->MoveTo(inLocalRect.right - 1, local_pt.y - 2);
			pDC->LineTo(inLocalRect.right - 1, local_pt.y + 2);
			pDC->LineTo(inLocalRect.right - 3, local_pt.y);
			pDC->LineTo(inLocalRect.right - 1, local_pt.y - 2);
			pDC->EndPath();
			pDC->StrokeAndFillPath();
		}
		
		// Draw solid line for today column or time column
		if (!IsTodayColumn(inCell) && (inCell.col != 1))
		{
			// Dashed line
			pDC->SelectObject(&temp_pen_dash);
		}

		// Draw line
		pDC->BeginPath();
		pDC->MoveTo(inLocalRect.left, local_pt.y);
		pDC->LineTo(inLocalRect.right, local_pt.y);
		pDC->EndPath();
		pDC->StrokePath();

		pDC->SelectObject(old_pen);
		pDC->SelectObject(old_brush);
	}
}

void CDayWeekTable::RefreshNow()
{
	// Get local point of now marker
	if (mCurrentNowMarker < 0xFFFFFFFE)
	{
		// Determine region around now
		CRect client;
		GetClientRect(client);
		
		CPoint left_image_pt(0, mCurrentNowMarker);
		CPoint local_pt;
		ImageToLocalPoint(left_image_pt, local_pt);
	
		client.top = local_pt.y - 5;
		client.bottom = local_pt.y + 5;

		RedrawWindow(client, NULL, RDW_INVALIDATE);
	}
	
	// Get local point of now marker
	if (mNowMarker < 0xFFFFFFFE)
	{
		// Determine region around now
		CRect client;
		GetClientRect(client);
		
		CPoint left_image_pt(0, mNowMarker);
		CPoint local_pt;
		ImageToLocalPoint(left_image_pt, local_pt);
	
		client.top = local_pt.y - 5;
		client.bottom = local_pt.y + 5;

		RedrawWindow(client, NULL, RDW_INVALIDATE);
	}
	
	mCurrentNowMarker = mNowMarker;
}

// Display and track context menu
void CDayWeekTable::HandleContextMenu(CWnd*, CPoint point)
{
	// Must update the selection state before doing the popup
	// The selection updating has been deferred at this point
	DoSelectionChanged();

	CMenu menu;
	VERIFY(menu.LoadMenu(mContextMenuID));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this);

	if (popup_result == IDM_CALENDAR_NEWEVENT)
	{
		CPoint localPt(point);
		ScreenToClient(&localPt);

		// See if click is on a valid cell
		CPoint imagePt;
		LocalToImagePoint(localPt, imagePt);
		STableCell hitCell;
		if (GetCellHitBy(imagePt, hitCell) && (hitCell.col > 1))
		{
			// Use current position for new event time and create new event
			iCal::CICalendarDateTime dt(GetClickDateTime(localPt));
			Broadcast_Message(eBroadcast_DblClkToDay, &dt);
			
			// By-pass normal context event handling
			return;
		}
	}
	
	// Send message in normal way
	SendMessage(WM_COMMAND, popup_result);
}

void CDayWeekTable::AdjustSize()
{
	if ((mRows != 0) && (mCols != 0))
	{
		StDeferTableAdjustment _defer(this);

		// Look for change in image size
		CPoint imageSize;
		GetImageSize(imageSize);

		// The current frame is the frame set by the scroller and represents the visible area we have available
		// We ignore the super frame size as that does not account for scrollbars etc
		CRect my_frame;
		GetClientRect(my_frame);

		// Determine new column sizes
		int32_t col_size = max((int32_t) ((my_frame.Width() - cTimeColumnWidth) / (mCols - 1)), 64L);
		
		SetColWidth(col_size, 2, mCols);

		// If auto-fit rows, change row height
		RescaleHeight();

		CPoint newSize;
		GetImageSize(newSize);

		if ((newSize.x != imageSize.x) || (newSize.y != imageSize.y))
		{
			RepositionEvents();
			CheckNow();
		}
		
		if (mTitles)
			mTitles->TableChanged();
	}
}

void CDayWeekTable::RescaleHeight()
{
	// If auto-fit rows, change row height
	if (mScaleRows == 0)
	{
		CRect my_frame;
		GetClientRect(my_frame);
	
		int32_t row_size = max((int32_t) ((my_frame.Height() - GetRowHeight(cAllDayRow)) / (mRows - 1)), 8L);
		SetRowHeight(row_size, cFirstTimedRow, mRows);
	}
}

void CDayWeekTable::SelectionChanged()
{
	// Must refresh titles
	mTitles->RedrawWindow();
}

void CDayWeekTable::ScaleRows(uint32_t scale)
{
	if (scale != mScaleRows)
	{
		mScaleRows = scale;
		if (mScaleRows > 0)
		{
			SetRowHeight(cRowHeight / mScaleRows, cFirstTimedRow, mRows);
		}
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
		
		// NB the mAllDayEvents[col - 1] vectors MUST have the same size in each row to make this easy
		
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

		CRect start_cellFrame;
		GetLocalCellRectAlways(start_cell, start_cellFrame);
		CRect actual_cellFrame = start_cellFrame;

		CRect end_cellFrame;
		GetLocalCellRectAlways(end_cell, end_cellFrame);

		start_cellFrame.right = end_cellFrame.right;

		start_cellFrame.top += cEventOffset * slot + cEventMargin;
		start_cellFrame.bottom = start_cellFrame.top +cEventOffset;

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
			mAllDayEvents[col_ctr - 2][slot] = make_pair(event, col_ctr == col_start);
		
		// Now show it
		event->ShowWindow(SW_SHOW);
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

		CRect start_cellFrame;
		GetLocalCellRectAlways(cell, start_cellFrame);

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
		event->ShowWindow(SW_SHOW);
	}
}

void CDayWeekTable::PositionAllDayEvent(CDayEvent* event, const STableCell& cell, size_t offset)
{
	if ((event == NULL) || !IsValidCell(cell))
		return;

	CRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrame);
	cellFrame.top += cEventOffset * offset + cEventMargin;
	cellFrame.bottom = cellFrame.top + cEventOffset;
	cellFrame.right = cellFrame.left + cellFrame.Width() * event->GetColumnSpan();
	
	::MoveWindowTo(event, cellFrame.left, cellFrame.top, false);
	::ResizeWindowTo(event, cellFrame.Width(), cellFrame.Height(), true);
}

void CDayWeekTable::PositionTimedEvent(CDayEvent* event, const STableCell& cell, int64_t top_secs)
{
	if ((event == NULL) || !IsValidCell(cell))
		return;
	
	const iCal::CICalendarComponentExpandedShared& vevent = event->GetVEvent();
	int64_t start_secs = vevent->GetInstanceStart().GetPosixTime();
	int64_t end_secs = vevent->GetInstanceEnd().GetPosixTime();
	
	int64_t hours_in_column = (mRows - 1LL) / 2LL;
	int64_t bottom_secs = top_secs + hours_in_column * 60LL * 60LL;

	CRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrame);
	float columnHeight = GetRowHeight(cFirstTimedRow) * (mRows - 1);
	
	// Now set the top of the event's frame
	CRect eventFrame = cellFrame;
	if (start_secs > top_secs)
	{
		eventFrame.top += (start_secs - top_secs) * columnHeight / (hours_in_column * 60LL * 60LL);
	}
	else
		start_secs = top_secs;
	
	// Now set bottom of the event's frame
	if (end_secs > bottom_secs)
	{
		eventFrame.bottom = cellFrame.top + columnHeight;
	}
	else
	{
		eventFrame.bottom = eventFrame.top + (end_secs - start_secs) * columnHeight / (hours_in_column * 60LL * 60LL);
	}
	eventFrame.bottom++;

	// Now adjust for overlaps
	if (event->GetColumnTotal() != 1)
	{
		int32_t orig_width = eventFrame.Width();

		// Offset by relative offset
		eventFrame.left += orig_width * event->GetRelativeOffset();

		// Width from relative width
		eventFrame.right = eventFrame.left + orig_width * event->GetRelativeWidth();
	}

	::MoveWindowTo(event, eventFrame.left, eventFrame.top, false);
	::ResizeWindowTo(event, eventFrame.Width(), eventFrame.Height(), true);
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
	for(TableIndexT column = 2; column <= mCols; column++)
	{
		CDayEventList& list = mTimedEvents[column - 2];
		
		std::vector<SEventInfo> data;
		std::vector<SEventInfo*> column_usage;
		column_usage.push_back(NULL);

		// Add each start/end position
		uint32_t col = 0;
		for(CDayEventList::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			data.push_back(SEventInfo(*iter, true, col++, (*iter)->GetVEvent()->GetInstanceStart().GetPosixTime()));
			data.push_back(SEventInfo(*iter, false, col++, (*iter)->GetVEvent()->GetInstanceEnd().GetPosixTime()));
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
		
		// We now have the sequenced start/end array so calculate the total column width
		// for each non-zero sequence
		std::vector<SEventInfo>::iterator start = data.begin();
		uint32_t max_column = 1;
		for(std::vector<SEventInfo>::iterator iter = data.begin(); iter != data.end(); iter++)
		{
			if ((*iter).mStarts)
			{
				max_column = max(max_column, (*iter).mTotalCol);
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

				CRect frame;
				event->GetWindowRect(frame);
				ScreenToClient(frame);

				float column_width = frame.Width();

				// Cache relative value which get used when repositioning events
				event->SetRelativeWidth(1.0 / event->GetColumnTotal());
				event->SetRelativeOffset(((float)event->GetColumnOffset()) / ((float)event->GetColumnTotal()));

				// Adjust frame 
				frame.left += column_width * event->GetRelativeOffset();
				frame.right = frame.left + column_width * event->GetRelativeWidth();

				::MoveWindowTo(event, frame.left, frame.top, false);
				::ResizeWindowTo(event, frame.Width(), frame.Height(), true);

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
							if ((event2->GetVEvent()->GetInstanceStart() != event->GetVEvent()->GetInstanceStart()) ||
								(event2->GetVEvent()->GetInstanceEnd() != event->GetVEvent()->GetInstanceEnd()))
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
							CRect frame2;
							event2->GetWindowRect(frame2);
							ScreenToClient(frame2);

							frame2.left += column_width * rel_move_by;
							frame2.right += column_width * rel_move_by + column_width * rel_adjust_width;

							::MoveWindowTo(event2, frame2.left, frame2.top, false);
							::ResizeWindowTo(event2, frame2.Width(), frame2.Height(), true);

							// Bump up move for next item
							rel_move_by += rel_adjust_width;
						}
					}
				}
			}
		}
	}
}
