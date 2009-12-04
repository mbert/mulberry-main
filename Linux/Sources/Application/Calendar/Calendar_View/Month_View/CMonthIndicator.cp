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
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include "StPenState.h"

#include <UNX_LTableMonoGeometry.h>
#include <UNX_LTableSingleSelector.h>

#include <JXColormap.h>
#include <jASCIIConstants.h>

// ---------------------------------------------------------------------------
//	CMonthIndicator														  [public]
/**
	Default constructor */

CMonthIndicator::CMonthIndicator(JXScrollbarSet* scrollbarSet, 
				 	JXContainer* enclosure,
				 	const HSizingOption hSizing, 
				 	const VSizingOption vSizing,
				 	const JCoordinate x, const JCoordinate y,
				 	const JCoordinate w, const JCoordinate h) :
  	CTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	SetBorderWidth(0);

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

CMonthIndicator* CMonthIndicator::Create(JXContainer* enclosure, const JRect& frame)
{
	CMonthIndicator* result = new CMonthIndicator(NULL, enclosure, 
													kFixedLeft, kFixedTop,
													frame.left, frame.top, frame.width(), frame.height());
	try
	{
		result->OnCreate();
	}
	catch (...)
	{									// FinishCreate failed. View is
		delete result;					//   in an inconsistent state.
		throw;							//   Delete it and rethrow.
	}

	return result;
}

#pragma mark -

void CMonthIndicator::OnCreate()
{
	CTable::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);
}

iCal::CICalendarDateTime CMonthIndicator::GetCellDate(const STableCell& cell) const
{
	return iCal::CICalendarDateTime(mData[cell.row - 2][cell.col - 1]);
}

void CMonthIndicator::HandleKeyPress(const int key, const JXKeyModifiers& modifiers)
{
	// Return => edit
	STableCell selCell = GetFirstSelectedCell();
	switch(key)
	{
	case kJLeftArrow:
		if (!selCell.IsNullCell())
		{
			selCell.col--;
		}
		break;
	case kJRightArrow:
		if (!selCell.IsNullCell())
		{
			selCell.col++;
		}
		break;
	case kJUpArrow:
		if (!selCell.IsNullCell())
		{
			selCell.row--;
		}
		break;
	case kJDownArrow:
		if (!selCell.IsNullCell())
		{
			selCell.row++;
		}
		break;
	default:
		// Do default key press processing
		CTable::HandleKeyPress(key, modifiers);
		return;
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
	return;
}

void CMonthIndicator::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
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

void CMonthIndicator::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	// Ignore if data not already set
	if (mData.size() == 0)
		return;
	
	if (mPrinting)
		DrawCellPrint(pDC, inCell, inLocalRect);
	else
		DrawCellScreen(pDC, inCell, inLocalRect);
}

void CMonthIndicator::DrawCellScreen(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	JRect adjustedRect = inLocalRect;
	adjustedRect.Shrink(1, 1);

	StPenState	save(pDC);

	bool today_cell = mTodayCell == inCell;

	// First row is day names
	cdstring temp;
	bool draw_text = false;
	bool white_txt = false;
	EDrawStringAlignment just;
	if (inCell.row == 1)
	{
		// Draw a circle around the number if not printing
		JColorIndex cindex;
		GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(0.75, 0.5, 0.25), &cindex);
		pDC->SetPenColor(cindex);
		pDC->SetFilling(kTrue);
		pDC->Ellipse(adjustedRect);

		uint32_t day_of_week = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
		temp = iCal::CICalendarLocale::GetDay(day_of_week, iCal::CICalendarLocale::eAbbreviated);

		draw_text = true;
		white_txt = true;
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
			JColorIndex cindex;
			GetColormap()->JColormap::AllocateStaticColor(today_cell ? CCalendarUtils::GetRGBColor(0.85, 0.85, 1.0) : CCalendarUtils::GetRGBColor(grey_level, grey_level, grey_level), &cindex);
			pDC->SetPenColor(cindex);
			pDC->SetFilling(kTrue);
			pDC->Rect(adjustedRect);
			pDC->SetFilling(kFalse);

			// Do selection after background
			if (CellIsSelected(inCell))
				DrawCellSelection(pDC, inCell);

			temp = mday;
			draw_text = true;
			just = eDrawString_Right;
		}
	}

	if (draw_text)
	{
		// Determine appropriate font size
		pDC->SetFontSize(adjustedRect.height()  / 2);
		adjustedRect.Shrink(1, 1);

		JFontStyle text_style = pDC->GetFontStyle();
		text_style.color = white_txt ? pDC->GetColormap()->GetWhiteColor() : pDC->GetColormap()->GetBlackColor();
		pDC->SetFontStyle(text_style);
		
		// For digits, center two-digit numbers and align one-digit to the right of two-digits
		if (just == eDrawString_Right)
		{
			JSize width = pDC->GetStringWidth("30");
			adjustedRect.right -= (adjustedRect.width() - width) / 2;
		}
		pDC->String(adjustedRect, temp.c_str(), (just == eDrawString_Center) ? JPainter::kHAlignCenter : JPainter::kHAlignRight, JPainter::kVAlignCenter);
	}
}

void CMonthIndicator::DrawCellPrint(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
#ifdef _TODO
#endif
}

bool CMonthIndicator::DrawCellSelection(JPainter* pDC, const STableCell& inCell)
{
	bool selected = CellIsSelected(inCell);
	bool active = (HasFocus() || mIsDropTarget);
	
	if (selected && mDrawSelection && active)
	{
		StPenState	save(pDC);

		JRect cellRect;
		GetLocalCellRect(inCell, cellRect);
		cellRect.Shrink(1, 1);

		pDC->SetPenColor(GetColormap()->GetDefaultSelectionColor());
		pDC->SetLineWidth(3);
		pDC->RectInside(cellRect);
		return true;
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
		SetSize(GetColWidth(1) * mCols, GetRowHeight(1) * mRows);
	else
	{
		// Resize to make total width less than the one requested, and keep cells square
		SetColWidth(width / mCols, 1, mCols);
		SetRowHeight(width / mCols, 1, mRows);
	}
	
	Refresh();
}

