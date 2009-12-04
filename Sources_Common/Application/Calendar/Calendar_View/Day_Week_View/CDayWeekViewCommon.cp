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

#include "CDayWeekView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CDayWeekTable.h"
#include "CDayWeekTitleTable.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

void CDayWeekView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
	case CTableDrag::eBroadcast_Deactivate:
	case CTableDrag::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		Broadcast_Message(msg, param);
		break;

	case CDayWeekTitleTable::eBroadcast_DblClick:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToDay, param);
		break;
	case CDayWeekTable::eBroadcast_DblClkToDay:
		{
			// Create new event with selected start
			iCal::CICalendarDateTime* dt = static_cast<iCal::CICalendarDateTime*>(param);
			OnNewEvent(*dt);
		}
		break;
	case CDayWeekTable::eBroadcast_CreateNew:
		// New event
		OnNewEvent();
		break;
	case CCalendarEventBase::eBroadcast_EditEvent:
		// Reset all events
		ResetDate();
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

iCal::CICalendarDateTime CDayWeekView::GetSelectedDate() const
{
	// Is anything selected in the table
	STableCell cell = static_cast<CDayWeekTable*>(GetTable())->GetFirstSelectedCell();
	if (cell.IsNullCell() || (cell.col == 1))
		return mDate;
	else
		return static_cast<CDayWeekTable*>(GetTable())->GetCellStartDate(cell);
}

cdstring CDayWeekView::GetTitle() const
{
	cdstring result = mDate.GetLocaleDate(iCal::CICalendarDateTime::eFullDate);
	switch(mType)
	{
	case eDay:
	default:
		// Nothing to do;
		break;
	case eWorkWeek:
	case eWeek:
		{
			iCal::CICalendarDateTime end_date(mDate);
			end_date.OffsetDay((mType == eWorkWeek) ? 4 : 6);
			result += " - ";
			result += end_date.GetLocaleDate(iCal::CICalendarDateTime::eFullDate);
		}
		break;
	}
	return result;
}

void CDayWeekView::ResetDate()
{
	// Only if we have a celendar
	if (mCalendarView == NULL)
		return;
	
	// Date must be at start of day
	mDate.SetHHMMSS(0, 0, 0);

	// Adjust date based on type
	int32_t num_days = 1;
	switch(mType)
	{
	case eDay:
	default:
		// Nothing to do;
		break;
	case eWorkWeek:
		// Adjust date to the Monday at or before mDate
		if (mDate.GetDayOfWeek() != iCal::CICalendarDateTime::eMonday)
			mDate.OffsetDay(iCal::CICalendarDateTime::eMonday - mDate.GetDayOfWeek());
		num_days = 5;
		break;
	case eWeek:
		// Adjust date to the Sunday at or before mDate
		if (mDate.GetDayOfWeek() != CPreferences::sPrefs->mWeekStartDay.GetValue())
			mDate.OffsetDay((CPreferences::sPrefs->mWeekStartDay.GetValue() - mDate.GetDayOfWeek() - 7) % 7);
		num_days = 7;
		break;
	}

	static_cast<CDayWeekTable*>(GetTable())->ResetTable(mDate, mType, mDayWeekRange);

	// Get date range for event lookup
	iCal::CICalendarDateTime dtstart(mDate);
	dtstart.SetDateOnly(false);
	iCal::CICalendarDateTime dtend(mDate);
	dtend.SetDateOnly(false);
	dtend.OffsetDay(num_days);
	
	// Get events/free-busy in the range
	iCal::CICalendarExpandedComponents vevents;
	iCal::CICalendarComponentList vfreebusy;
	iCal::CICalendarPeriod period(dtstart, dtend);
	
	// Look for single calendar
	if (IsSingleCalendar())
	{
		// Get events and free-busy
		if (GetCalendar() != NULL)
		{
			GetCalendar()->GetVEvents(period, vevents);
			GetCalendar()->GetVFreeBusy(period, vfreebusy);
		}
	}

	// Only do visible calendars
	else if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			// Listen to the calendar in case it gets destroyed
			(*iter)->Add_Listener(this);
			
			// Get events and free-busy
			(*iter)->GetVEvents(period, vevents);
			(*iter)->GetVFreeBusy(period, vfreebusy);
		}
	}
	
	// Add events and free-busy to table
	static_cast<CDayWeekTable*>(GetTable())->AddItems(vevents, vfreebusy);

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CDayWeekView::OnPrevWeek()
{
	mDate.OffsetDay(-7);
	ResetDate();
}

void CDayWeekView::OnNextWeek()
{
	mDate.OffsetDay(7);
	ResetDate();
}

void CDayWeekView::OnPrevDay()
{
	mDate.OffsetDay(mType == eDay ? -1 : -7);
	ResetDate();
}

void CDayWeekView::OnNextDay()
{
	mDate.OffsetDay(mType == eDay ? 1 : 7);
	ResetDate();
}

void CDayWeekView::OnToday()
{
	mDate.SetToday();
	
	// Now offset to start of week if in week view

	ResetDate();
}

void CDayWeekView::OnNewEvent()
{
	if (mCalendarView == NULL)
		return;

	// Set it to the start date of the selected cell
	iCal::CICalendarDateTime dtstart;
	STableCell selected = static_cast<CDayWeekTable*>(GetTable())->GetFirstSelectedCell();
	if (selected.IsNullCell() || (selected.col == 1))
	{
		// Use the date from the current view
		selected.SetCell(1, 2);
	}

	if (static_cast<CDayWeekTable*>(GetTable())->IsTodayColumn(selected))
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
		dtstart = static_cast<CDayWeekTable*>(GetTable())->GetCellStartDate(selected);
		dtstart.SetTimezone(mDate.GetTimezone());
		dtstart.SetDateOnly(false);
		dtstart.SetHHMMSS(12, 0, 0);
	}
	
	CNewEventDialog::StartNew(dtstart, IsSingleCalendar() ? GetCalendar() : NULL);
}

void CDayWeekView::OnNewEvent(const iCal::CICalendarDateTime& dt)
{
	if (mCalendarView == NULL)
		return;
	
	CNewEventDialog::StartNew(dt, IsSingleCalendar() ? GetCalendar() : NULL);
}

