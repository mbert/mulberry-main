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

#include "CCalendarUtils.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include "cdustring.h"

#include <WIN_LTableMonoGeometry.h>
#include <WIN_LTableSingleSelector.h>

// ---------------------------------------------------------------------------
//	CMonthIndicator														  [public]
/**
	Default constructor */

CMonthIndicator::CMonthIndicator()
{
	mHasScrollbars = false;

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

CMonthIndicator* CMonthIndicator::Create(CWnd* parent, const CRect& frame)
{
	CMonthIndicator* indicator = new CMonthIndicator;
	indicator->CTable::Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP, frame, parent, IDC_STATIC);
	indicator->ResetFont(CFontCache::GetListFont());

	return indicator;
}

#pragma mark -

iCal::CICalendarDateTime CMonthIndicator::GetCellDate(const STableCell& cell) const
{
	return iCal::CICalendarDateTime(mData[cell.row - 2][cell.col - 1]);
}

bool CMonthIndicator::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Return => edit
	STableCell selCell = GetFirstSelectedCell();
	switch(nChar)
	{
	case VK_LEFT:
		if (!selCell.IsNullCell())
		{
			selCell.col--;
		}
		break;
	case VK_RIGHT:
		if (!selCell.IsNullCell())
		{
			selCell.col++;
		}
		break;
	case VK_UP:
		if (!selCell.IsNullCell())
		{
			selCell.row--;
		}
		break;
	case VK_DOWN:
		if (!selCell.IsNullCell())
		{
			selCell.row++;
		}
		break;
	default:
		// Do default key press processing
		return CTable::HandleKeyDown(nChar, nRepCnt, nFlags);
	}
	
	// Make cell valid
	if (selCell.col >= mCols)
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

void CMonthIndicator::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	// Broadcast double-click if in month area
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

void CMonthIndicator::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	// Ignore if data not already set
	if (mData.size() == 0)
		return;
	
	if (mPrinting)
		DrawCellPrint(pDC, inCell, inLocalRect);
	else
		DrawCellScreen(pDC, inCell, inLocalRect);
}

void CMonthIndicator::DrawCellScreen(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	CRect adjustedRect = inLocalRect;
	adjustedRect.DeflateRect(1, 1);

	StDCState		save(pDC);

	bool today_cell = mTodayCell == inCell;

	// Determine appropriate font size
	pDC->SelectObject(CFontCache::GetListFontScaled(adjustedRect.Height() - 4));

	// First row is day names
	cdstring temp;
	bool draw_text = false;
	float text_grey = 0.0;
	EDrawStringAlignment just;
	if (inCell.row == 1)
	{
		// Draw a circle around the number if not priting
		pDC->BeginPath();
		pDC->Arc(adjustedRect.left, adjustedRect.top, adjustedRect.right, adjustedRect.bottom, adjustedRect.left, adjustedRect.top, adjustedRect.left, adjustedRect.top);
		pDC->EndPath();
		CBrush temp_brush(CCalendarUtils::GetWinColor(0.75, 0.5, 0.25));
		CBrush* old_brush = pDC->SelectObject(&temp_brush);
		pDC->FillPath();
		pDC->SelectObject(old_brush);
		pDC->SetBkColor(CCalendarUtils::GetWinColor(0.75, 0.5, 0.25));

		uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
		temp = iCal::CICalendarLocale::GetDay(day_of_week, iCal::CICalendarLocale::eAbbreviated);

		// Get a suitable offset for drawing the day letter in the circle
		cdustring utf16(temp);
		SIZE bounds;
		::GetTextExtentPoint32W(*pDC, utf16, utf16.length(), &bounds);

		// Center rect in circle
		adjustedRect.top += (adjustedRect.Height() - bounds.cy) / 2;
		adjustedRect.bottom = adjustedRect.top + bounds.cy;

		draw_text = true;
		text_grey = 1.0;
		just = eDrawString_Center;
	}
	else
	{
		int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[inCell.row - 2][inCell.col - 1]);
		if (mday > 0)
		{
			uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
			bool work_day = (CPreferences::sPrefs->mWorkDayMask.GetValue() & (1L << day_of_week)) != 0;
			float grey_level = !work_day ? 0.9 : 0.95;
			pDC->SetBkColor(today_cell ? CCalendarUtils::GetWinColor(0.85, 0.85, 1.0) : CCalendarUtils::GetWinColor(grey_level, grey_level, grey_level));
			pDC->ExtTextOut(adjustedRect.left, adjustedRect.top, ETO_OPAQUE, adjustedRect, _T(""), 0, NULL);

			// Do selection after background
			if (CellIsSelected(inCell))
				DrawCellSelection(pDC, inCell);

			temp = mday;
			draw_text = true;
			just = eDrawString_Right;

			// Get a suitable offset for drawing the day numbers in the rect
			cdustring utf16(temp);
			SIZE bounds;
			::GetTextExtentPoint32W(*pDC, utf16, utf16.length(), &bounds);

			// Position text in center
			adjustedRect.top += (adjustedRect.Height() - bounds.cy) / 2;
			adjustedRect.bottom = adjustedRect.top + bounds.cy;
			adjustedRect.right += 2;
		}
	}

	if (draw_text)
	{
		pDC->SetTextColor(CCalendarUtils::GetWinColor(text_grey, text_grey, text_grey));
		::DrawClippedStringUTF8(pDC, temp, CPoint(adjustedRect.left, adjustedRect.top), adjustedRect, just);
	}
}

void CMonthIndicator::DrawCellPrint(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
#ifdef _TODO
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
		std::uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
		temp = cdstring(iCal::CICalendarLocale::GetDay(day_of_week, iCal::CICalendarLocale::eAbbreviated));
		draw_text = true;
		just = teJustCenter;
	}
	else
	{
		int32_t mday = iCal::CICalendarUtils::UnpackDateDay(mData[inCell.row - 2][inCell.col - 1]);
		if (mday > 0)
		{
			std::uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
			bool work_day = (CPreferences::sPrefs->mWorkDayMask.GetValue() & (1L << day_of_week)) != 0;
			float grey_level = !work_day ? 0.9 : 0.95;
			if (today_cell)
				::CGContextSetRGBFillColor(inContext, 0.85, 0.85, 1.0, 1.0);
			else
				::CGContextSetGrayFillColor(inContext, grey_level, 1.0);
			::CGContextFillRect(inContext, adjustedRect);

			// Do selection after background
			if (CellIsSelected(inCell))
				DrawCellSelection(pDC, inCell);

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
#endif
}

bool CMonthIndicator::DrawCellSelection(CDC* pDC, const STableCell& inCell)
{
	bool selected = CellIsSelected(inCell);
	bool active = (mHasFocus || mIsDropTarget);
	
	if (selected && mDrawSelection && active)
	{
		StDCState save(pDC);

		CRect cellRect;
		GetLocalCellRect(inCell, cellRect);
		CBrush temp_brush(pDC->GetBkColor());
		CBrush* old_brush = pDC->SelectObject(&temp_brush);
		CDrawUtils::DrawFocusFrame(pDC, cellRect);
		pDC->SelectObject(old_brush);
		return true;
	}
	
	return false;
}

void CMonthIndicator::ResetTable(const iCal::CICalendarDateTime date, uint32_t width, bool printing)
{
	// Only bother if date (YY/MM) is different or data table is empty
	if ((mDate.GetYear() == date.GetYear()) && (mDate.GetMonth() == date.GetMonth()) && (mData.size() > 0))
		return;
	
	StDeferTableAdjustment _defer(this);

	// Cache date
	mDate = date;
	mPrinting = printing;

	// Now generate new month date table and get today cell
	pair<int32_t, int32_t> today_index;
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
	{
		// Get current size
		::ResizeWindowTo(this, GetColWidth(1) * mCols, GetRowHeight(1) * mRows, false);
	}
	else
	{
		// Resize to make total width less than the one requested, and keep cells square
		SetColWidth(width / mCols, 1, mCols);
		SetRowHeight(width / mCols, 1, mRows);
	}
}

