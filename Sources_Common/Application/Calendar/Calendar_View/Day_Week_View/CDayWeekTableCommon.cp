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

#include "CDayEvent.h"
#include "CDayWeekTitleTable.h"
#include "CDayWeekView.h"
#include "CPreferences.h"

#include "CICalendarComponentExpanded.h"

#include <algorithm>

const uint32_t cAllDayRow = 1;
const uint32_t cFirstTimedRow = 2;
const uint32_t cEventOffset = 16;
const uint32_t cEventMargin = 4;

void CDayWeekTable::Idle()
{
	// Get current time relative to current timezone
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&mTimezone);
	
	// Check with previous time
	CheckEventsNow();

	// Check now marker
	if (CheckNow())
	{
		mLastIdleTime = now;
		ResetToday();
		RefreshNow();
	}

	// Check today cell
	if (!now.CompareDate(mLastIdleTime))
	{
		mLastIdleTime = now;
		ResetToday();
		FRAMEWORK_REFRESH_WINDOW(this)
		if (mTitles)
			FRAMEWORK_REFRESH_WINDOW(mTitles)
	}
	
}

void CDayWeekTable::ResetToday()
{
	// Today cell has changed
	
	// Determine today cell
	STableCell cell(1, 1);
	mNowCell.col = 0;
	for(TableIndexT col = 2; col <= mCols; col++)
	{
		cell.SetCell(1, col);
		if (GetCellStartDate(cell).CompareDate(mLastIdleTime))
		{
			mNowCell.col = col;
			break;
		}
	}
}

const iCal::CICalendarDateTime& CDayWeekTable::GetCellStartDate(const STableCell& cell) const
{
	return  mCellDates[(cell.col >= 2) ? cell.col - 2 : 0];
}

const iCal::CICalendarDateTime& CDayWeekTable::GetCellEndDate(const STableCell& cell) const
{
	return mCellDates[(cell.col >= 1) ? cell.col - 2 + 1 : 0];
}

const iCal::CICalendarDateTime& CDayWeekTable::GetTimedStartDate(const STableCell& cell) const
{
	return mTimedStartDates[(cell.col >= 2) ? cell.col - 2 : 0];
}

const iCal::CICalendarDateTime& CDayWeekTable::GetTimedEndDate(const STableCell& cell) const
{
	return mTimedEndDates[(cell.col >= 2) ? cell.col - 2 : 0];
}

void CDayWeekTable::ResetTable(iCal::CICalendarDateTime& date, int type, CDayWeekViewTimeRange::ERanges range)
{
	// Remove all events
	InitEvents(0, 0);

	// Always set date to start at 00:00:00
	mDate = date;
	mDate.SetHHMMSS(0, 0, 0);
	mTimezone = mDate.GetTimezone();
	
	// Change type if needed
	TableIndexT new_cols;
	switch(type)
	{
	case CDayWeekView::eDay:
	default:
		new_cols = 2;
		break;
	case CDayWeekView::eWorkWeek:
		new_cols = 6;
		break;
	case CDayWeekView::eWeek:
		new_cols = 8;
		break;
	}

	if (mCols > new_cols)
		RemoveCols(mCols - new_cols, 1, false);
	else if (mCols < new_cols)
		InsertCols(new_cols - mCols, 1);
	
	// Determine appropriate row range
	iCal::CICalendarDateTime dtstart = CPreferences::sPrefs->mDayWeekTimeRanges.GetValue().GetStart(range);
	iCal::CICalendarDateTime dtend = CPreferences::sPrefs->mDayWeekTimeRanges.GetValue().GetEnd(range);
	
	// Round up/down
	dtstart.SetMinutes(0);
	dtstart.SetSeconds(0);
	
	if ((dtend.GetMinutes() != 0) || (dtend.GetSeconds() != 0))
	{
		dtend.SetMinutes(0);
		dtend.SetSeconds(0);
		dtend.OffsetHours(1);
	}
	
	mStartHour = dtstart.GetHours();
	mEndHour = dtend.GetHours();
	if (mEndHour <= mStartHour)
		mEndHour = 24;

	// Adjust rows
	TableIndexT new_rows = (mEndHour - mStartHour) * 2 + 1;
	if (mRows > new_rows)
		RemoveRows(mRows - new_rows, 1, false);
	else if (mRows < new_rows)
		InsertRows(new_rows - mRows, 1);

	// Make sure cell dates are cached
	CacheCellDates();

	// Determine today cell
	STableCell cell(1, 1);
	mNowCell.col = 0;
	for(cell.col = 2; cell.col <= mCols; cell.col++)
	{
		if (GetCellStartDate(cell) == iCal::CICalendarDateTime::GetToday())
		{
			mNowCell.col = cell.col;
			break;
		}
	}

	// Redo events
	InitEvents(mRows, mCols - 1);

	// Force frame size recalc
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	AdaptToNewSurroundings();

#elif __dest_os == __win32_os

	AdjustSize();

#elif __dest_os == __linux_os

	ApertureResized(0, 0);

#endif

	// Do this after adding events so that top row offset
	// is correctly setup
	CheckNow();
}

void CDayWeekTable::CacheCellDates()
{
	// Clear existing
	mCellDates.clear();
	mTimedStartDates.clear();
	mTimedEndDates.clear();
	
	mCellDates.reserve(mCols);
	mTimedStartDates.reserve(mCols);
	mTimedEndDates.reserve(mCols);
	
	for(TableIndexT col = 2; col <= mCols; col++)
	{
		iCal::CICalendarDateTime cell_date(mDate);
		cell_date.SetDateOnly(false);
		if (col > 2)
			cell_date.OffsetDay(col - 2);
		mCellDates.push_back(cell_date);
		
		cell_date.OffsetHours(mStartHour);
		mTimedStartDates.push_back(cell_date);
		
		cell_date.OffsetHours(mEndHour - mStartHour);
		mTimedEndDates.push_back(cell_date);
	}
	
	// Add the last one (used only for all day events)
	iCal::CICalendarDateTime cell_date(mCellDates.back());
	cell_date.OffsetDay(1);
	mCellDates.push_back(cell_date);
}

void CDayWeekTable::AddItems(iCal::CICalendarExpandedComponents& vevents, iCal::CICalendarComponentList& vfreebusys)
{
	// Add all day event first, then others
	for(iCal::CICalendarExpandedComponents::iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		if ((**iter).GetInstanceStart().IsDateOnly())
			AddAllDayEvent(*iter);
	}
	
	// Look at total number of slots and resize all day row to show all of them
	uint32_t slots_used = mAllDayEvents[0].size();
	if (slots_used == 0)
		slots_used = 1;
	UInt16 old_height = GetRowHeight(cAllDayRow);
	UInt16 new_height = slots_used * cEventOffset + 2 * cEventMargin;
	if (old_height != new_height)
	{
		SetRowHeight(new_height, cAllDayRow, cAllDayRow);
		RescaleHeight();
	}

	for(iCal::CICalendarExpandedComponents::iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		if (!(**iter).GetInstanceStart().IsDateOnly())
			AddTimedEvent(*iter);
	}
	
	for(iCal::CICalendarComponentList::iterator iter = vfreebusys.begin(); iter != vfreebusys.end(); iter++)
	{
		AddTimedFreeBusy(*iter);
	}
	
	ColumnateEvents();
	CheckNow();
}

void CDayWeekTable::InitEvents(TableIndexT rows, TableIndexT cols)
{
	// Clear all cached selected events
	ClearSelectedEvents();

	// Delete all all day events
	for(CDayEventMatrixRow::iterator iter1 = mAllDayEvents.begin(); iter1 != mAllDayEvents.end(); iter1++)
	{
		for(CDayEventSlotList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			// Delete it its a real event
			if (((*iter2).first != NULL) && (*iter2).second)
				delete (*iter2).first;
		}
	}
	mAllDayEvents.clear();
	
	// Initialise new matrices
	if ((rows != 0) && (cols != 0))
	{
		CDayEventSlotList elist;
		mAllDayEvents.insert(mAllDayEvents.begin(), cols, elist);
	}
	
	// Delete all timed events
	for(CDayEventColumn::iterator iter1 = mTimedEvents.begin(); iter1 != mTimedEvents.end(); iter1++)
	{
		for(CDayEventList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			// Delete it its a real event
			delete *iter2;
		}
	}
	mTimedEvents.clear();
	
	// Initialise new matrices
	if ((rows != 0) && (cols != 0))
	{
		CDayEventList elist;
		mTimedEvents.insert(mTimedEvents.begin(), cols, elist);
	}
}

void CDayWeekTable::RepositionEvents()
{
	// Resposition all day events
	STableCell cell(cAllDayRow, 2);
	for(CDayEventMatrixRow::iterator iter1 = mAllDayEvents.begin(); iter1 != mAllDayEvents.end(); iter1++, cell.col++)
	{
		size_t offset = 0;
		for(CDayEventSlotList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++, offset++)
		{
			// Only for real events
			if (((*iter2).first != NULL) && (*iter2).second)
				PositionAllDayEvent((*iter2).first, cell, offset);
		}
	}
	
	// Reposition timed events
	cell.SetCell(cFirstTimedRow, 2);
	for(CDayEventColumn::iterator iter1 = mTimedEvents.begin(); iter1 != mTimedEvents.end(); iter1++, cell.col++)
	{
		int64_t top_secs = GetTimedStartDate(cell).GetPosixTime();

		for(CDayEventList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			PositionTimedEvent(*iter2, cell, top_secs);
		}
	}
}

void CDayWeekTable::ClearEvents()
{
	// Delete all all day events
	for(CDayEventMatrixRow::iterator iter1 = mAllDayEvents.begin(); iter1 != mAllDayEvents.end(); iter1++)
	{
		for(CDayEventSlotList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			// Delete it its a real event
			if (((*iter2).first != NULL) && (*iter2).second)
				delete (*iter2).first;
		}
	}
	mAllDayEvents.clear();
	
	// Delete all timed events
	for(CDayEventColumn::iterator iter1 = mTimedEvents.begin(); iter1 != mTimedEvents.end(); iter1++)
	{
		for(CDayEventList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			// Delete it its a real event
			delete *iter2;
		}
	}
	mTimedEvents.clear();
}

void CDayWeekTable::SelectEvent(bool next)
{
	// Find the last selected event if it exists
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
		// Outer loop over all day events first
		unsigned long column = 0;
		for(CDayEventMatrixRow::iterator iter1 = mAllDayEvents.begin(); !done && (iter1 != mAllDayEvents.end()); iter1++, column++)
		{
			for(CDayEventSlotList::iterator iter2 = (*iter1).begin(); !done && (iter2 != (*iter1).end()); iter2++)
			{
				// Only consider valid items (and the first one in a link series)
				if (((*iter2).first != NULL) && (*iter2).second && ((*iter2).first->GetPreviousLink() == NULL))
				{
					bool got_match = ((*iter2).first == last_selected);
					
					// Look for next match
					if (next)
					{
						if (!found)
							found = got_match;
						else
						{
							ClickEvent((*iter2).first, false);
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
						previous = (*iter2).first;
					}
				}
			}
			
			// Do inner loop over current column
			if (!done)
			{
				CDayEventColumn::iterator iter3 = mTimedEvents.begin() + column;
				for(CDayEventList::iterator iter4 = (*iter3).begin(); !done && (iter4 != (*iter3).end()); iter4++)
				{
					// Only consider valid items (and the first one in a link series)
					if ((*iter4)->GetPreviousLink() == NULL)
					{
						bool got_match = (*iter4 == last_selected);
						
						// Look for next match
						if (next)
						{
							if (!found)
								found = got_match;
							else
							{
								ClickEvent(*iter4, false);
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
							previous = *iter4;
						}
					}
				}
			}
		}
		
		// After first loop always mark as found so that the first is selected
		found = true;
	}
}

// Return true if now has changed
bool CDayWeekTable::CheckNow()
{
	// Get current time
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&mTimezone);
	float secs = (now.GetHours() * 60 + now.GetMinutes()) * 60 + now.GetSeconds();
	float top_secs = mStartHour * 60 * 60;
	float bot_secs = mEndHour * 60 * 60;
	uint32_t old_marker = mNowMarker;
	if (secs < top_secs)
		mNowMarker = 0xFFFFFFFF;
	else if (secs > bot_secs)
		mNowMarker = 0xFFFFFFFE;
	else
	{
		float scale = (secs - top_secs) / (bot_secs - top_secs);
		uint32_t columnHeight = GetRowHeight(cFirstTimedRow) * (mRows - 1);
		STableCell timed_cell(cFirstTimedRow, 1);
		SInt32	cellLeft, cellTop, cellRight, cellBottom;
		GetImageCellBounds(timed_cell, cellLeft, cellTop, cellRight, cellBottom);
		mNowMarker = (uint32_t) (cellTop + columnHeight * scale);
		
		mNowCell.row = (TableIndexT) (cFirstTimedRow + (mRows - 1) * scale);
	}
	
	return mNowMarker != old_marker; 
}

void CDayWeekTable::CheckEventsNow()
{
	// Tell each timed event to check itself
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&mTimezone);
	for(CDayEventColumn::iterator iter1 = mTimedEvents.begin(); iter1 != mTimedEvents.end(); iter1++)
	{
		for(CDayEventList::iterator iter2 = (*iter1).begin(); iter2 != (*iter1).end(); iter2++)
		{
			(*iter2)->CheckNow(now);
		}
	}
}
