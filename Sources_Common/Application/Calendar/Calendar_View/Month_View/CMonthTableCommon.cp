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

#include "CMonthEvent.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarComponentExpanded.h"
#include "CICalendarLocale.h"

void CMonthTable::Idle()
{
	// Get current time relative to current timezone
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&mTimezone);
	
	// Check with previous time
	CheckEventsNow();
	if (!now.CompareDate(mLastIdleTime))
	{
		mLastIdleTime = now;
		ResetToday();
		FRAMEWORK_REFRESH_WINDOW(this)
	}
}

void CMonthTable::ResetToday()
{
	// Today cell has changed
	
	// Check start/end cells as shortcut
	STableCell cell1(1, 1);
	STableCell cell2(mRows, mCols);
	if ((GetCellStartDate(cell1) > mLastIdleTime) || (GetCellStartDate(cell2) < mLastIdleTime))
	{
		mTodayCell.SetCell(0, 0);
		return;
	}
	
	// Scan each cell in the table and see if its the today cell
	for(TableIndexT row = 1; row <= mRows; row++)
	{
		for(TableIndexT col = 1; col <= mCols; col++)
		{
			cell1.SetCell(row, col);
			if (GetCellStartDate(cell1).CompareDate(mLastIdleTime))
			{
				mTodayCell.SetCell(row, col);
				break;
			}
		}
	}
}

const iCal::CICalendarDateTime& CMonthTable::GetCellStartDate(const STableCell& cell) const
{
	return mCellDates[(cell.row - 1) * mCols + (cell.col - 1)];
}

const iCal::CICalendarDateTime& CMonthTable::GetCellEndDate(const STableCell& cell) const
{
	return mCellDates[(cell.row - 1) * mCols + (cell.col - 1) + 1];
}

void CMonthTable::SelectEvent(bool next)
{
	// Find the last selected event if it exists (skip back to the first in a link series)
	const CCalendarEventBase* last_selected = NULL;
	if (mSelectedEvents.size() != 0)
	{
		last_selected = mSelectedEvents.back();
		while(last_selected->GetPreviousLink() != NULL)
			last_selected = last_selected->GetPreviousLink();
	}

	// Iterate up to the last one found, then select the next one
	bool done = false;
	bool found = (last_selected == NULL);
	CCalendarEventBase* previous = NULL;
	
	// Do the loop twice so that we cycle back from last to first
	int count = 2;
	while(count-- && !done)
	{
		for(CMonthEventMatrix::iterator iter1 = mEvents.begin(); !done && (iter1 != mEvents.end()); iter1++)
		{
			for(CMonthEventMatrixRow::iterator iter2 = (*iter1).begin(); !done && (iter2 != (*iter1).end()); iter2++)
			{
				for(CMonthEventSlotList::iterator iter3 = (*iter2).begin(); !done && (iter3 != (*iter2).end()); iter3++)
				{
					// Only consider valid items (and the first one in a link series)
					if (((*iter3).first != NULL) && (*iter3).second && ((*iter3).first->GetPreviousLink() == NULL))
					{
						bool got_match = ((*iter3).first == last_selected);
						
						// Look for next match
						if (next)
						{
							if (!found)
								found = got_match;
							else
							{
								ClickEvent((*iter3).first, false);
								done = true;
							}
						}

						// Look for previous match
						else
						{
							if (got_match && (previous != NULL))
							{
								ClickEvent(previous, false);
								done = true;
							}
							
							// Always cache previous one
							previous = (*iter3).first;
						}
					}
				}
			}
		}
	}
}

void CMonthTable::SelectDay()
{
	// Trigger day view of selected cell
	STableCell inCell = GetFirstSelectedCell();
	if (IsValidCell(inCell))
	{
		iCal::CICalendarDateTime dt(GetCellStartDate(inCell));
		Broadcast_Message(eBroadcast_DblClkToDay, &dt);
	}
}

void CMonthTable::ResetTable(const iCal::CICalendarUtils::CICalendarTable& table, const std::pair<int32_t, int32_t>& today_index, const iCal::CICalendarTimezone& timezone)
{
	// Remove all events
	InitEvents(0, 0);

	mData = table;
	mTodayCell.SetCell(today_index.first + 1, today_index.second + 1);
	mTimezone = timezone;

	// Make sure columns are present
	if (mCols == 0)
	{
		bool more = false;
		InsertCols(7, 0, &more, sizeof(bool));
	}
	
	// Remove previous selection
	UnselectAllCells();
		
	// Adjust existing rows
	if (mRows > table.size())
		RemoveRows(mRows - table.size(), 1, false);
	else if (mRows < table.size())
	{
		bool more = false;
		InsertRows(table.size() - mRows, mRows, &more, sizeof(bool));
	}

	// Make sure cell dates are cached
	CacheCellDates();

#if __dest_os == __mac_os || __dest_os == __mac_os_x

	AdaptToNewSurroundings();

#elif __dest_os == __win32_os

	AdjustSize();

#elif __dest_os == __linux_os

	ApertureResized(0, 0);

#endif

	// Redo events
	InitEvents(mRows, mCols);

	// Force redraw
	FRAMEWORK_REFRESH_WINDOW(this);
}

void CMonthTable::CacheCellDates()
{
	// Clear existing
	mCellDates.clear();
	
	mCellDates.reserve(mRows * mCols + 1);
	
	for(TableIndexT row = 1; row <= mRows; row++)
	{
		for(TableIndexT col = 1; col <= mCols; col++)
		{
			iCal::CICalendarDateTime cell_date(mData[row - 1][col - 1]);
			cell_date.SetDateOnly(false);
			cell_date.SetTimezone(mTimezone);
			mCellDates.push_back(cell_date);
		}
	}
	
	// Add the last one
	iCal::CICalendarDateTime cell_date(mCellDates.back());
	cell_date.OffsetDay(1);
	mCellDates.push_back(cell_date);
}

void CMonthTable::AddEvents(iCal::CICalendarExpandedComponents& vevents)
{
	// Add all day event first, then others
	for(iCal::CICalendarExpandedComponents::iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		if ((**iter).GetInstanceStart().IsDateOnly())
			AddEvent(*iter);
	}
	for(iCal::CICalendarExpandedComponents::iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		if (!(**iter).GetInstanceStart().IsDateOnly())
			AddEvent(*iter);
	}
}

void CMonthTable::InitEvents(TableIndexT rows, TableIndexT cols)
{
	// Clear all cached selected events
	ClearSelectedEvents();

	// Delete all events
	for(CMonthEventMatrix::iterator iter1 = mEvents.begin(); iter1 != mEvents.end(); iter1++)
	{
		for(CMonthEventMatrixRow::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			for(CMonthEventSlotList::iterator iter3 = (*iter2).begin(); iter3 != (*iter2).end(); iter3++)
			{
				// Delete it its a real event
				if (((*iter3).first != NULL) && (*iter3).second)
					delete (*iter3).first;
			}
		}
	}
	mEvents.clear();
	
	// Initialise new matrices
	if ((rows != 0) && (cols != 0))
	{
		CMonthEventSlotList elist;
		CMonthEventMatrixRow erow;
		erow.insert(erow.begin(), cols, elist);
		mEvents.insert(mEvents.begin(), rows, erow);
	}
	
	// Clear all cell data
	bool more = false;
	STableCell cell(1, 1);
	for(cell.row = 1; cell.row <= mRows; cell.row++)
		for(cell.col = 1; cell.col <= mCols; cell.col++)
			SetCellData(cell, &more, sizeof(bool));
}

void CMonthTable::RepositionEvents()
{
	// Clear all cell data
	bool more = false;
	STableCell cell(1, 1);
	for(cell.row = 1; cell.row <= mRows; cell.row++)
		for(cell.col = 1; cell.col <= mCols; cell.col++)
			SetCellData(cell, &more, sizeof(bool));

	cell.SetCell(1, 1);
	for(CMonthEventMatrix::iterator iter1 = mEvents.begin(); iter1 != mEvents.end(); iter1++, cell.row++)
	{
		cell.col = 1;
		for(CMonthEventMatrixRow::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++, cell.col++)
		{
			size_t offset = 0;
			for(CMonthEventSlotList::iterator iter3 = (*iter2).begin(); iter3 != (*iter2).end(); iter3++, offset++)
			{
				// Only for real events
				if (((*iter3).first != NULL) && (*iter3).second)
					PositionEvent((*iter3).first, cell, offset);
			}
		}
	}
}

void CMonthTable::ClearEvents()
{
	// Delete all events
	for(CMonthEventMatrix::iterator iter1 = mEvents.begin(); iter1 != mEvents.end(); iter1++)
	{
		for(CMonthEventMatrixRow::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			for(CMonthEventSlotList::iterator iter3 = (*iter2).begin(); iter3 != (*iter2).end(); iter3++)
			{
				// Delete it its a real event
				if (((*iter3).first != NULL) && (*iter3).second)
					delete (*iter3).first;
			}
		}
	}
	mEvents.clear();
}

void CMonthTable::CheckEventsNow()
{
	// Tell each timed event to check itself
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&mTimezone);
	for(CMonthEventMatrix::iterator iter1 = mEvents.begin(); iter1 != mEvents.end(); iter1++)
	{
		for(CMonthEventMatrixRow::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			for(CMonthEventSlotList::iterator iter3 = (*iter2).begin(); iter3 != (*iter2).end(); iter3++)
			{
				// Only for real events
				if (((*iter3).first != NULL) && (*iter3).second)
					(*iter3).first->CheckNow(now);
			}
		}
	}
}
