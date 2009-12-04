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

// ===========================================================================
//	CFreeBusyTable.cp			PowerPlantX 1.0			©2003 Metrowerks Corp.
// ===========================================================================

#include "CFreeBusyTable.h"

#include "CFreeBusyTitleTable.h"
#include "CFreeBusyView.h"
#include "CPreferences.h"

const uint32_t cNameColumn = 1;
const uint32_t cFirstTimedColumn = 2;

const iCal::CICalendarDateTime& CFreeBusyTable::GetTimedStartDate(const TableIndexT& col) const
{
	return mColumnDates[(col >= cFirstTimedColumn) ? col - cFirstTimedColumn : 0];
}

const iCal::CICalendarDateTime& CFreeBusyTable::GetTimedEndDate(const TableIndexT& col) const
{
	return mColumnDates[(col >= cFirstTimedColumn) ? col - cFirstTimedColumn + 1 : 0];
}

void CFreeBusyTable::ResetTable(iCal::CICalendarDateTime& date, CDayWeekViewTimeRange::ERanges range)
{
	// Remove all events
	mItems.clear();
	
	SFreeBusyInfo defaultItem;
	mItems.push_back(defaultItem);

	// Always set date to start at 00:00:00
	mDate = date;
	mDate.SetHHMMSS(0, 0, 0);
	mTimezone = mDate.GetTimezone();
	
	// Change type if needed
	TableIndexT new_rows = 1;
	if (mRows > new_rows)
		RemoveRows(mRows - new_rows, 1, false);
	else if (mRows < new_rows)
		InsertRows(new_rows - mRows, 1);
	
	// Determine appropriate column range
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

	// Adjust columns
	TableIndexT new_cols = (mEndHour - mStartHour) + 1;
	if (mCols > new_cols)
		RemoveCols(mCols - new_cols, 2, false);
	else if (mCols < new_cols)
		InsertCols(new_cols - mCols, 2);

	// Make sure cell dates are cached
	CacheCellDates();

	// Force frame size recalc
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	AdaptToNewSurroundings();

#elif __dest_os == __win32_os

	AdjustSize();

#elif __dest_os == __linux_os

	ApertureResized(0, 0);

#endif
}

void CFreeBusyTable::CacheCellDates()
{
	// Clear existing
	mColumnDates.clear();
	
	mColumnDates.reserve(mCols);
	
	for(TableIndexT col = cFirstTimedColumn; col <= mCols; col++)
	{
		iCal::CICalendarDateTime cell_date(mDate);
		cell_date.SetDateOnly(false);
		cell_date.OffsetHours(mStartHour + col - cFirstTimedColumn);
		mColumnDates.push_back(cell_date);
	}
	
	// Add the last one (used only for all day events)
	iCal::CICalendarDateTime cell_date(mColumnDates.back());
	cell_date.OffsetHours(1);
	mColumnDates.push_back(cell_date);
	
	mColumnSeconds = (mEndHour - mStartHour) * 60 * 60;
}

void CFreeBusyTable::AddItems(const calstore::CCalendarStoreFreeBusyList& list)
{
	for(calstore::CCalendarStoreFreeBusyList::const_iterator iter = list.begin(); iter != list.end(); iter++)
	{
		AddItem(*iter);
	}
}

void CFreeBusyTable::AddItem(const calstore::CCalendarStoreFreeBusy& item)
{
	// Create new info item
	SFreeBusyInfo info;
	info.mName = item.GetName();
	uint32_t last_offset = 0;
	uint32_t last_duration = mColumnSeconds;
	info.mPeriods.push_back(SFreeBusyInfo::SFreeBusyPeriod(iCal::CICalendarFreeBusy::eFree, last_duration));
	
	// Look at each period and determine intersection with table period
	const iCal::CICalendarDateTime& row_start = GetTimedStartDate(cFirstTimedColumn);
	const iCal::CICalendarDateTime& row_end = GetTimedEndDate(mCols);
	int64_t start_secs = row_start.GetPosixTime();
	int64_t end_secs = row_end.GetPosixTime();
	for(iCal::CICalendarFreeBusyList::const_iterator iter1 = item.GetPeriods().begin(); iter1 != item.GetPeriods().end(); iter1++)
	{
		// Must be busy item
		if ((*iter1).GetType() == iCal::CICalendarFreeBusy::eFree)
			continue;

		// Get start/end of this period
		iCal::CICalendarDateTime dtstart((*iter1).GetPeriod().GetStart());
		iCal::CICalendarDateTime dtend((*iter1).GetPeriod().GetEnd());

		// First check that event actually spans this day
		if ((dtend <= row_start) || (dtstart >= row_end))
			continue;
		
		// Determine start offset
		int64_t dtstart_secs = dtstart.GetPosixTime();
		int64_t dtend_secs = dtend.GetPosixTime();
		
		int64_t start_offset = (dtstart_secs > start_secs) ? dtstart_secs - start_secs : 0;
		int64_t end_offset = (dtend_secs < end_secs) ? dtend_secs - start_secs : mColumnSeconds;
		uint32_t duration = end_offset - start_offset;
		
		// Now add to info
		
		// First change or delete current info
		if (start_offset > last_offset)
		{
			info.mPeriods.back().second = start_offset - last_offset;
		}
		else
			info.mPeriods.pop_back();
		
		// Add new info as busy time
		info.mPeriods.push_back(SFreeBusyInfo::SFreeBusyPeriod((*iter1).GetType(), duration));
		
		// Add trailing free time
		if (start_offset + duration < last_offset + last_duration)
			info.mPeriods.push_back(SFreeBusyInfo::SFreeBusyPeriod(iCal::CICalendarFreeBusy::eFree, last_offset + last_duration - (start_offset + duration)));
		
		// Adjust last value
		last_offset = end_offset;
		last_duration = mColumnSeconds - last_offset;
	}
	
	// Add to table
	mItems.push_back(info);
	InsertRows(1, mRows);
}
