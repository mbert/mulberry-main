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

#include "CMonthIndicator.h"

#include "CGUtils.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include <LTableMonoGeometry.h>
#include <LTableSingleSelector.h>
#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CMonthIndicator														  [public]
/**
	Default constructor */

CMonthIndicator::CMonthIndicator(const SPaneInfo &inPaneInfo, const SViewInfo &inViewInfo) :
	CTableDrag(inPaneInfo, inViewInfo)
{
	mTodayCell.SetCell(0, 0);
	
	// Single geometry (all rows/columns are the same size)
	mTableGeometry = new LTableMonoGeometry(this, 20, 20);
	
	// Single selector
	mTableSelector = new LTableSingleSelector(this);
	
	mPrinting = false;
}

CMonthIndicator::CMonthIndicator(LStream* inStream) :
	CTableDrag(inStream)
{
	mTodayCell.SetCell(0, 0);
	
	// Single geometry (all rows/columns are the same size)
	mTableGeometry = new LTableMonoGeometry(this, 20, 20);
	
	// Single selector
	mTableSelector = new LTableSingleSelector(this);
	
	mPrinting = false;
}

// ---------------------------------------------------------------------------
//	~CMonthIndicator														  [public]
/**
	Destructor */

CMonthIndicator::~CMonthIndicator()
{
}

CMonthIndicator* CMonthIndicator::Create(LView* parent, const HIRect& frame)
{
	SPaneInfo pane;
	pane.visible = true;
	pane.enabled = true;
	pane.userCon = 0L;
	pane.superView = parent;
	pane.bindings.left = true;
	pane.bindings.right = false;
	pane.bindings.top = true;
	pane.bindings.bottom = false;
	pane.paneID = 0;
	pane.width = frame.size.width;
	pane.height = frame.size.height;
	pane.left = frame.origin.x;
	pane.top = frame.origin.y;
	SViewInfo view;
	view.imageSize.width = frame.size.width;
	view.imageSize.height = frame.size.height;
	view.scrollPos.h = 0;
	view.scrollPos.v = 0;
	view.scrollUnit.h = 0;
	view.scrollUnit.v = 0;
	view.reconcileOverhang = 0;
	CMonthIndicator* result = new CMonthIndicator(pane, view);
	try
	{
		result->FinishCreate();
	}
	catch (...)
	{									// FinishCreate failed. View is
		delete result;					//   in an inconsistent state.
		throw;							//   Delete it and rethrow.
	}

	return result;
}

#pragma mark -

iCal::CICalendarDateTime CMonthIndicator::GetCellDate(const STableCell& cell) const
{
	return iCal::CICalendarDateTime(mData[cell.row - 2][cell.col - 1]);
}

Boolean CMonthIndicator::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Return => edit
	STableCell selCell = GetFirstSelectedCell();
	char key_press = (inKeyEvent.message & charCodeMask);
	switch(key_press)
	{
	case char_LeftArrow:
		if (!selCell.IsNullCell())
		{
			selCell.col--;
		}
		break;
	case char_RightArrow:
		if (!selCell.IsNullCell())
		{
			selCell.col++;
		}
		break;
	case char_UpArrow:
		if (!selCell.IsNullCell())
		{
			selCell.row--;
		}
		break;
	case char_DownArrow:
		if (!selCell.IsNullCell())
		{
			selCell.row++;
		}
		break;
	default:
		// Do default key press processing
		return CTableDrag::HandleKeyPress(inKeyEvent);
	}
	
	// Make cell valid
	if (selCell.col > mCols)
	{
		selCell.col = 1;
		selCell.row++;
	}
	if (selCell.col < 1)
	{
		selCell.col = mCols;
		selCell.row--;
	}
	if ((selCell.row < 2) || (selCell.row > mRows))
		selCell.SetCell(0, 0);
	else
	{
		// Must be valid day
		int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[selCell.row - 2][selCell.col - 1]);
		if (mday < 0)
			selCell.SetCell(0, 0);
	}
	
	if (selCell.IsNullCell())
		UnselectAllCells();
	else
		SelectCell(selCell);
	return true;
}

// Click
void CMonthIndicator::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);

	CTableDrag::ClickSelf(inMouseDown);
}

void CMonthIndicator::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
{
	// Broadcast double-click if in month area
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
	{
		int32_t msg = eBroadcast_DblClkMonth;
		iCal::CICalendarDateTime dt(mDate.GetYear(), mDate.GetMonth(), 1);
		
		// Look for valid month day
		if (inCell.row > 1)
		{
			int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[inCell.row - 2][inCell.col - 1]);
			if (mday > 0)
			{
				dt.SetDay(mday);
				msg = eBroadcast_DblClkDay;
			}
		}

		Broadcast_Message(msg, &dt);
	}
	else
		CTableDrag::ClickCell(inCell, inMouseDown);
}

void CMonthIndicator::SelectDate(const iCal::CICalendarDateTime date)
{
	UnselectAllCells();

	// The year and month must match
	if ((date.GetYear() != mDate.GetYear()) ||
		(date.GetMonth() != mDate.GetMonth()))
		return;

	// Now look for cell with matching day
	STableCell cell(1, 1);
	for(cell.row = 2; cell.row <= mRows; cell.row++)
	{
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[cell.row - 2][cell.col - 1]);
			if (mday == date.GetDay())
			{
				SelectCell(cell);
				return;
			}
		}
	}
}

void CMonthIndicator::SelectionChanged()
{
	STableCell sel_cell = GetFirstSelectedCell();
	CMonthIndicator* broadcast = NULL;

	// Look for valid month day
	if (sel_cell.row > 1)
	{
		int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[sel_cell.row - 2][sel_cell.col - 1]);
		if (mday > 0)
		{
			broadcast = this;
		}
	}
	
	Broadcast_Message(eBroadcast_SelectionChanged, broadcast);
}

void CMonthIndicator::DrawCell(const STableCell &inCell, const Rect &inLocalQDRect)
{
	// Ignore if data not already set
	if (mData.size() == 0)
		return;
	
	if (mPrinting)
		DrawCellPrint(inCell, inLocalQDRect);
	else
		DrawCellScreen(inCell, inLocalQDRect);
}

void CMonthIndicator::DrawCellScreen(const STableCell &inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;
	adjustedRect.origin.x += 1.0;
	adjustedRect.origin.y += 1.0;
	adjustedRect.size.width -= 2.0;
	adjustedRect.size.height -= 2.0;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	bool today_cell = mTodayCell == inCell;

	// First row is day names
	MyCFString temp;
	bool draw_text = false;
	float text_grey = 0.0;
	SInt16 just;
	if (inCell.row == 1)
	{
		// Draw a circle around the number if not printing
		::CGContextBeginPath(inContext);
		::CGContextAddArc(inContext, adjustedRect.origin.x + adjustedRect.size.width / 2.0, adjustedRect.origin.y + adjustedRect.size.width / 2.0, adjustedRect.size.width / 2.0, 0.0, 2.0 * pi, 0);
		::CGContextClosePath(inContext);
		::CGContextSetRGBFillColor(inContext, 0.75, 0.5, 0.25, 1.0);
		::CGContextFillPath(inContext);

		uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
		temp = MyCFString(iCal::CICalendarLocale::GetDay(day_of_week, iCal::CICalendarLocale::eAbbreviated), kCFStringEncodingUTF8);

		// Get a suitable offset for drawing the day letter in the circle
		Point bounds;
		::GetThemeTextDimensions(temp, kThemeSmallSystemFont, kThemeStateActive, false, &bounds, NULL);

		// Center rect in circle
		adjustedRect.origin.y += (adjustedRect.size.height - bounds.v)/ 2.0;
		adjustedRect.size.height = bounds.v;

		draw_text = true;
		text_grey = 1.0;
		just = teJustCenter;
	}
	else
	{
		int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[inCell.row - 2][inCell.col - 1]);
		if (mday > 0)
		{
			uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
			bool work_day = (CPreferences::sPrefs->mWorkDayMask.GetValue() & (1L << day_of_week)) != 0;
			float grey_level = !work_day ? 0.9 : 0.95;
			if (today_cell)
				::CGContextSetRGBFillColor(inContext, 0.85, 0.85, 1.0, 1.0);
			else
				::CGContextSetGrayFillColor(inContext, grey_level, 1.0);
			::CGContextFillRect(inContext, adjustedRect);

			// Do selection after background
			if (CellIsSelected(inCell))
				DrawCellSelection(inCell, inContext);

			temp.AssignNumericValue<int32_t>(mday);
			draw_text = true;
			just = teJustRight;

			// Get a suitable offset for drawing the day numbers in the rect
			Point bounds;
			::GetThemeTextDimensions(temp, kThemeSmallSystemFont, kThemeStateActive, false, &bounds, NULL);

			// Position text in center
			adjustedRect.origin.y += (adjustedRect.size.height - bounds.v)/ 2.0;
			adjustedRect.size.height = bounds.v;
			adjustedRect.size.width -= (adjustedRect.size.width - ((mday < 10) ? 2 : 1) * bounds.h) / 2;
		}
	}

	if (draw_text)
	{
		Rect box;
		CGUtils::HIToQDRect(adjustedRect, box);
		::CGContextSetGrayFillColor(inContext, text_grey, 1.0);
		::DrawThemeTextBox(temp, kThemeSmallSystemFont, kThemeStateActive, false, &box, just, inContext);
	}
}

void CMonthIndicator::DrawCellPrint(const STableCell &inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;
	adjustedRect.origin.x += 1.0;
	adjustedRect.origin.y += 1.0;
	adjustedRect.size.width -= 2.0;
	adjustedRect.size.height -= 2.0;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	bool today_cell = mTodayCell == inCell;

	// First row is day names
	cdstring temp;
	bool draw_text = false;
	SInt16 just;
	if (inCell.row == 1)
	{
		uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
		temp = cdstring(iCal::CICalendarLocale::GetDay(day_of_week, iCal::CICalendarLocale::eAbbreviated));
		draw_text = true;
		just = teJustCenter;
	}
	else
	{
		int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[inCell.row - 2][inCell.col - 1]);
		if (mday > 0)
		{
			uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
			bool work_day = (CPreferences::sPrefs->mWorkDayMask.GetValue() & (1L << day_of_week)) != 0;
			float grey_level = !work_day ? 0.9 : 0.95;
			if (today_cell)
				::CGContextSetRGBFillColor(inContext, 0.85, 0.85, 1.0, 1.0);
			else
				::CGContextSetGrayFillColor(inContext, grey_level, 1.0);
			::CGContextFillRect(inContext, adjustedRect);

			// Do selection after background
			if (CellIsSelected(inCell))
				DrawCellSelection(inCell, inContext);

			temp = cdstring(mday);
			draw_text = true;
			just = teJustRight;
		}
	}

	if (draw_text)
	{
		// Adjust the transform so the text doesn't draw upside down
		CGAffineTransform transform = CGAffineTransformIdentity;
		transform = ::CGAffineTransformScale(transform, 1, -1);
		::CGContextSetTextMatrix(inContext, transform); 

		Rect box;
		CGUtils::HIToQDRect(adjustedRect, box);
		::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
 		
		// Set font
		::CGContextSelectFont(inContext, "Arial", adjustedRect.size.width / 1.5, kCGEncodingMacRoman);

		// Get current location
		HIPoint old_pt = ::CGContextGetTextPosition(inContext);
		
		// Do invisible draw for text size
 		::CGContextSetTextDrawingMode(inContext, kCGTextInvisible);
		::CGContextShowText(inContext, temp.c_str(), temp.length());
 		::CGContextSetTextDrawingMode(inContext, kCGTextFill);

		// Get width of text
		HIPoint new_pt = ::CGContextGetTextPosition(inContext);
		uint32_t twidth = new_pt.x - old_pt.x;

		HIPoint tpt;
		tpt.x = adjustedRect.origin.x;
		tpt.y = adjustedRect.origin.y + adjustedRect.size.height * 5.0 / 6.0;
		
		if (just == teJustRight)
			tpt.x += adjustedRect.size.width - twidth - 2.0;
		else if (just == teJustCenter)
			tpt.x += (adjustedRect.size.width - twidth) / 2.0;

		::CGContextShowTextAtPoint(inContext, tpt.x, tpt.y, temp.c_str(), temp.length());
	}
}

bool CMonthIndicator::DrawCellSelection(const STableCell& inCell, CGContextRef inContext)
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

void CMonthIndicator::ResetTable(const iCal::CICalendarDateTime date, uint32_t width, bool printing)
{
	// Only bother if date (YY/MM) is different or data table is empty
	if ((mDate.GetYear() == date.GetYear()) && (mDate.GetMonth() == date.GetMonth()) && (mData.size() > 0))
		return;

	// Cache date
	mDate = date;
	mPrinting = printing;

	// Now generate new month date table and get today cell
	std::pair<int32_t, int32_t> today_index;
	iCal::CICalendarUtils::GetMonthTable(mDate.GetMonth(), mDate.GetYear(), CPreferences::sPrefs->mWeekStartDay.GetValue(), mData, today_index);
	mTodayCell.SetCell(today_index.first + 2, today_index.second + 1);

	// Make sure columns are present
	if (mCols == 0)
		InsertCols(7, 0, NULL);
	
	// Remove previous selection
	UnselectAllCells();
		
	// Adjust existing rows
	if (mRows > mData.size() + 1)
		RemoveRows(mRows - mData.size() - 1, 1, true);
	else if (mRows < mData.size() + 1)
		InsertRows(mData.size() + 1 - mRows, mRows, NULL);

	// Adjust frame to total row height
	if (width == 0)
		ResizeFrameTo(GetColWidth(1) * mCols, GetRowHeight(1) * mRows, false);
	else
	{
		// Resize to make total width less than the one requested, and keep cells square
		SetColWidth(width / mCols, 1, mCols);
		SetRowHeight(width / mCols, 1, mRows);
	}
	
	Refresh();
}

