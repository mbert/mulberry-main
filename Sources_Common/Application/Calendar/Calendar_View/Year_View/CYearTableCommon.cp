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

#include "CYearTable.h"

#include "CCalendarViewBase.h"
#include "CMonthIndicator.h"

void CYearTable::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CMonthIndicator::eBroadcast_DblClkMonth:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToMonth, param);
		break;
	case CMonthIndicator::eBroadcast_DblClkDay:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToDay, param);
		break;
	case CMonthIndicator::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		DoIndicatorSelectionChanged(static_cast<CMonthIndicator*>(param));
		break;
	}
}

void CYearTable::Idle()
{
	// Get current time relative to current timezone
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&GetCalendarView()->GetDate().GetTimezone());
	
	// Check with previous time
	if (!now.CompareDate(mLastIdleTime))
	{
		mLastIdleTime = now;
		FRAMEWORK_REFRESH_WINDOW(this)
	}
}

void CYearTable::ResetTable(int32_t year, NCalendarView::EYearLayout layout)
{
	// Check for different layout
	if (layout != mLayout)
	{
		StDeferTableAdjustment _defer(this);

		mLayout = layout;
		TableIndexT new_rows;
		TableIndexT new_cols;
		
		switch(mLayout) 
		{
		case NCalendarView::e1x12:
			new_rows = 1;
			new_cols = 12;
			break;
		case NCalendarView::e2x6:
			new_rows = 2;
			new_cols = 6;
			break;
		case NCalendarView::e3x4:
			new_rows = 3;
			new_cols = 4;
			break;
		case NCalendarView::e4x3:
		default:
			new_rows = 4;
			new_cols = 3;
			break;
		case NCalendarView::e6x2:
			new_rows = 6;
			new_cols = 2;
			break;
		case NCalendarView::e12x1:
			new_rows = 12;
			new_cols = 1;
			break;
		}
		
		// Now adjust rows and columns
		if (mRows > new_rows)
			RemoveRows(mRows - new_rows, 1, false);
		else if (mRows < new_rows)
			InsertRows(new_rows - mRows, 1);
		if (mCols > new_cols)
			RemoveCols(mCols - new_cols, 1, false);
		else if (mCols < new_cols)
			InsertCols(new_cols - mCols, 1);
	}

	// Change year/month in each indicator
	if (mYear != year)
	{
		mYear = year;
		for(uint32_t i = 0; i < 12; i++)
		{
			mMonths[i]->ResetTable(iCal::CICalendarDateTime(year, i + 1, 1));
		}
	}
	
	// Reset frame/rows to new indicators
	ResetFrame();

	// Force redraw
	FRAMEWORK_REFRESH_WINDOW(this)
}

bool CYearTable::GetSelectedDate(iCal::CICalendarDateTime& date) const
{
	if (mLastSelected != NULL)
	{
		// Is anything selected in the table
		STableCell cell = mLastSelected->GetFirstSelectedCell();
		if (!cell.IsNullCell())
		{
			date = mLastSelected->GetCellDate(cell);
			return true;
		}
	}

	return false;
}

CMonthIndicator* CYearTable::GetIndicator(const STableCell& inCell)
{
	return mMonths[(inCell.row - 1) * mCols + (inCell.col - 1)];
}
