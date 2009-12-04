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

#include "CMonthView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CMonthTable.h"
#include "CPreferences.h"
#include "CNewEventDialog.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

void CMonthView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
	case CTableDrag::eBroadcast_Deactivate:
	case CTableDrag::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		Broadcast_Message(msg, param);
		break;

	case CMonthTable::eBroadcast_DblClkToDay:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToDay, param);
		break;
	case CMonthTable::eBroadcast_CreateNew:
		// New event
		OnNewEvent();
		break;
	case CCalendarEventTableBase::eBroadcast_CalendarChanged:
		// Reset all events
		ResetDate();
		break;
	default:
		CCalendarViewBase::ListenTo_Message(msg, param);
		break;
	}
}

iCal::CICalendarDateTime CMonthView::GetSelectedDate() const
{
	// Is anything selected in the table
	STableCell cell = static_cast<CMonthTable*>(GetTable())->GetFirstSelectedCell();
	if (cell.IsNullCell())
		return mDate;
	else
		return static_cast<CMonthTable*>(GetTable())->GetCellStartDate(cell);
}

cdstring CMonthView::GetTitle() const
{
	cdstring result = mDate.GetMonthText(false);
	result += " ";
	result += cdstring((unsigned long) mDate.GetYear());
	return result;
}

void CMonthView::ResetDate()
{
	// Only if we have a celendar
	if (mCalendarView == NULL)
		return;

	// Date must be at start of day
	mDate.SetHHMMSS(0, 0, 0);

	// Set static captions
	SetCaptions();
	
	// Now generate new month date table
	iCal::CICalendarUtils::CICalendarTable monthDays;
	std::pair<int32_t, int32_t> today_index;

	iCal::CICalendarUtils::GetMonthTable(mDate.GetMonth(), mDate.GetYear(), CPreferences::sPrefs->mWeekStartDay.GetValue(), monthDays, today_index);
	
	// Reset table
	static_cast<CMonthTable*>(GetTable())->ResetTable(monthDays, today_index, mDate.GetTimezone());

	// Get date range for event lookup
	int32_t packed_start = monthDays[0][0];
	int32_t packed_end = monthDays[monthDays.size() - 1][6];

	iCal::CICalendarDateTime dtstart(packed_start);
	dtstart.SetDateOnly(false);
	dtstart.SetTimezone(mDate.GetTimezone());
	iCal::CICalendarDateTime dtend(packed_end);
	dtend.SetDateOnly(false);
	dtend.SetTimezone(mDate.GetTimezone());
	dtend.OffsetDay(1);
	
	// Get events in the range
	iCal::CICalendarExpandedComponents vevents;
	iCal::CICalendarPeriod period(dtstart, dtend);

	// Look for single calendar
	if (IsSingleCalendar())
	{
		// Get events
		if (GetCalendar() != NULL)
			GetCalendar()->GetVEvents(period, vevents);
	}

	// Only do visible calendars
	else if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			// Listen to the calendar in case it gets destroyed
			(*iter)->Add_Listener(this);
			
			// Get events
			(*iter)->GetVEvents(period, vevents);
		}
	}

	// Add events to table
	static_cast<CMonthTable*>(GetTable())->AddEvents(vevents);

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CMonthView::OnPreviousMonth()
{
	mDate.OffsetMonth(-1);
	ResetDate();
}

void CMonthView::OnNextMonth()
{
	mDate.OffsetMonth(1);
	ResetDate();
}

void CMonthView::OnPreviousYear()
{
	mDate.OffsetYear(-1);
	ResetDate();
}

void CMonthView::OnNextYear()
{
	mDate.OffsetYear(1);
	ResetDate();
}

void CMonthView::OnToday()
{
	mDate.SetToday();
	ResetDate();
}

void CMonthView::OnNewEvent()
{
	if (mCalendarView == NULL)
		return;

	// Set it to the start date of the selected cell
	iCal::CICalendarDateTime dtstart;
	STableCell selected = static_cast<CMonthTable*>(GetTable())->GetFirstSelectedCell();
	if (selected.IsNullCell() || static_cast<CMonthTable*>(GetTable())->IsTodayCell(selected))
	{
		// Just use today's date-time
		dtstart = iCal::CICalendarDateTime::GetNow();
		dtstart.SetTimezone(mDate.GetTimezone());
		
		// Set to start at next hour after now
		dtstart.SetMinutes(0);
		dtstart.SetSeconds(0);
		dtstart.OffsetHours(1);
	}
	else
	{
		dtstart = static_cast<CMonthTable*>(GetTable())->GetCellStartDate(selected);
		dtstart.SetTimezone(mDate.GetTimezone());
		dtstart.SetDateOnly(false);
		dtstart.SetHHMMSS(12, 0, 0);
	}

	CNewEventDialog::StartNew(dtstart, IsSingleCalendar() ? GetCalendar() : NULL);
}
