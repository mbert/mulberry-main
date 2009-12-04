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

#include "CSummaryTable.h"

#include "CCalendarViewBase.h"
#include "CPreferences.h"
#include "CSummaryEvent.h"
#include "CSummaryTitleTable.h"

#include <algorithm>

void CSummaryTable::DoSelectionChanged(void)
{
	// Fake click
	STableCell cell = GetFirstSelectedCell();
	DoSingleClick(cell, CKeyModifiers(0));
}

// Single click on a message
void CSummaryTable::DoSingleClick(const STableCell& cell, const CKeyModifiers& mods)
{
	// Only if single selection
	iCal::CICalendarComponentExpandedShared event;
	if (IsSingleSelection())
		event = mEvents[cell.row - 1]->GetVEvent();

	// Preview this one
	if (GetCalendarView() != NULL)
		GetCalendarView()->PreviewComponent(event);
	
}

// Double click on a message
void CSummaryTable::DoDoubleClick(const STableCell& cell, const CKeyModifiers& mods)
{
	iCal::CICalendarDateTime dt(GetCellStartDate(cell));
	Broadcast_Message(eBroadcast_DblClkToDay, &dt);
}

void CSummaryTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	// Get text tooltip for cell
	const CSummaryEvent* event = mEvents[cell.row - 1];
	txt = event->mTooltip;
}

void CSummaryTable::ResetColumns()
{
	TableIndexT new_cols = mColumnInfo.size();
	if (mCols > new_cols)
		RemoveCols(mCols - new_cols, 1, false);
	else if (mCols < new_cols)
		InsertCols(new_cols - mCols, 1);
	
	for(TableIndexT col = 1; col <= mCols; col++)
		SetColWidth(mColumnInfo[col - 1].second, col, col);

	if (mTitles)
		mTitles->TableChanged();
}

const iCal::CICalendarDateTime& CSummaryTable::GetCellStartDate(const STableCell& cell) const
{
	return mEvents[cell.row - 1]->mSortDT;
}

void CSummaryTable::ResetTable(iCal::CICalendarDateTime& date)
{
	// Remove all events
	mEvents.clear();
	RemoveRows(mRows, 1, false);

	// Always set date to start at 00:00:00
	mDate = date;
	mDate.SetHHMMSS(0, 0, 0);
	mTimezone = mDate.GetTimezone();

	// Force redraw
	FRAMEWORK_REFRESH_WINDOW(this);
}

void CSummaryTable::AddEvents(iCal::CICalendarExpandedComponents& vevents)
{
	// Now
	iCal::CICalendarDateTime today;
	today.SetNow();
	today.SetDateOnly(true);
	
	// Yesterday
	iCal::CICalendarDateTime yesterday(today);
	yesterday.OffsetDay(-1);
	
	// Tomorrow
	iCal::CICalendarDateTime tomorrow(today);
	tomorrow.OffsetDay(1);
	
	// This Week Start
	iCal::CICalendarDateTime thisweekstart(today);
	if (thisweekstart.GetDayOfWeek() != CPreferences::sPrefs->mWeekStartDay.GetValue())
		thisweekstart.OffsetDay((CPreferences::sPrefs->mWeekStartDay.GetValue() - thisweekstart.GetDayOfWeek() - 7) % 7);

	// Next Week Start
	iCal::CICalendarDateTime nextweekstart(thisweekstart);
	nextweekstart.OffsetDay(7);

	// Two Week Start
	iCal::CICalendarDateTime twoweekstart(nextweekstart);
	twoweekstart.OffsetDay(7);

	// Add events
	CSummaryEvent::EType previous_type = CSummaryEvent::ePast;
	for(iCal::CICalendarExpandedComponents::iterator iter = vevents.begin(); iter != vevents.end(); iter++)
	{
		// Determine type
		CSummaryEvent::EType type = CSummaryEvent::ePast;
		if ((*iter)->GetInstanceStart() >= twoweekstart)
			type = CSummaryEvent::eFuture;
		else if ((*iter)->GetInstanceStart() >= nextweekstart)
			type = CSummaryEvent::eNextWeek;
		else if ((*iter)->GetInstanceStart() > tomorrow)
			type = CSummaryEvent::eRemainderOfWeek;
		else if ((*iter)->GetInstanceStart() == tomorrow)
			type = CSummaryEvent::eTomorrow;
		else if ((*iter)->GetInstanceStart() == today)
			type = CSummaryEvent::eToday;
		else if ((*iter)->GetInstanceStart() == yesterday)
			type = CSummaryEvent::eYesterday;
		else if ((*iter)->GetInstanceStart() >= thisweekstart)
			type = CSummaryEvent::eEarlierThisWeek;
		else
			type = CSummaryEvent::ePast;
		
		// Look for change in type
		if ((iter == vevents.begin()) || (type != previous_type))
		{
			// Always insert today
			if (((iter == vevents.begin()) || (previous_type < CSummaryEvent::eToday)) && (type > CSummaryEvent::eToday))
				mEvents.push_back(new CSummaryEvent(CSummaryEvent::eToday, 0UL));
			
			// Insert new type row and update
			mEvents.push_back(new CSummaryEvent(type, 0UL));
			previous_type = type;
		}

		// Determine day start/end
		bool start = false;
		bool end = false;
		if (iter == vevents.begin())
			start = true;
		else
			start = !(*iter)->GetInstanceStart().CompareDate((*(iter - 1))->GetInstanceStart());
			
		if (iter == vevents.end() - 1)
			end = true;
		else
			end = !(*iter)->GetInstanceStart().CompareDate((*(iter + 1))->GetInstanceStart());
			
		mEvents.push_back(new CSummaryEvent(type, *iter, start, end));
	}

	// Adjust rows
	TableIndexT new_rows = mEvents.size();
	if (mRows > new_rows)
		RemoveRows(mRows - new_rows, 1, false);
	else if (mRows < new_rows)
		InsertRows(new_rows - mRows, 1);
}

void CSummaryTable::SelectEvent(bool next)
{
	NudgeSelection(next ? 1 : -1, false);
}
