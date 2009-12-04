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

#include "CSummaryView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CSummaryTable.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"

#include "CCalendarStoreManager.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

void CSummaryView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
	case CTableDrag::eBroadcast_Deactivate:
	case CTableDrag::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		Broadcast_Message(msg, param);
		break;

	case CSummaryTable::eBroadcast_DblClkToDay:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToDay, param);
		break;
	case CCalendarEventBase::eBroadcast_EditEvent:
		// Reset all events
		ResetDate();
		break;
	default:
		CCalendarViewBase::ListenTo_Message(msg, param);
		break;
	}
}

iCal::CICalendarDateTime CSummaryView::GetSelectedDate() const
{
	// Is anything selected in the table
	STableCell cell = static_cast<CSummaryTable*>(GetTable())->GetFirstSelectedCell();
	if (cell.IsNullCell())
		return mDate;
	else
		return static_cast<CSummaryTable*>(GetTable())->GetCellStartDate(cell);
}

cdstring CSummaryView::GetTitle() const
{
	return mRangeText;
}

void CSummaryView::ResetDate()
{
	// Only if we have a celendar
	if (mCalendarView == NULL)
		return;
	
	// Date must be at start of day
	mDate.SetHHMMSS(0, 0, 0);

	static_cast<CSummaryTable*>(GetTable())->ResetTable(mDate);

	// Get date range for event lookup
	iCal::CICalendarDateTime dtstart(mDate);
	dtstart.SetDateOnly(false);
	iCal::CICalendarDateTime dtend(mDate);
	dtend.SetDateOnly(false);
	
	// Offset start/stop based on range
	switch(mSummaryRange)
	{
	case NCalendarView::eToday:
		// Just the current day
		dtend.OffsetDay(1);
		break;
	case NCalendarView::eDays:
		// Seven days either side of the current day
		dtstart.OffsetDay(-7);
		dtend.OffsetDay(8);
		break;
	case NCalendarView::eThisWeek:
		// This week - step back to beginning
		if (dtstart.GetDayOfWeek() != CPreferences::sPrefs->mWeekStartDay.GetValue())
			dtstart.OffsetDay((CPreferences::sPrefs->mWeekStartDay.GetValue() - dtstart.GetDayOfWeek() - 7) % 7);
		dtend = dtstart;
		dtend.OffsetDay(7);
		break;
	case NCalendarView::eWeeks:
		// This week - step back to beginning
		if (dtstart.GetDayOfWeek() != CPreferences::sPrefs->mWeekStartDay.GetValue())
			dtstart.OffsetDay((CPreferences::sPrefs->mWeekStartDay.GetValue() - dtstart.GetDayOfWeek() - 7) % 7);
		dtend = dtstart;
		dtend.OffsetDay(7);
		
		// Two weeks either side
		dtstart.OffsetDay(-2 * 7);
		dtend.OffsetDay(2 * 7);
		break;
	case NCalendarView::eThisMonth:
		// This month - step back to beginning
		dtstart.SetDay(1);
		dtend = dtstart;
		dtend.OffsetMonth(1);
		break;
	case NCalendarView::eMonths:
		// This month - step back to beginning
		dtstart.SetDay(1);
		dtend = dtstart;
		dtend.OffsetMonth(1);
		
		// Two months either side
		dtstart.OffsetMonth(-2);
		dtend.OffsetMonth(2);
		break;
	case NCalendarView::eThisYear:
		// This year - step back to beginning
		dtstart.SetDay(1);
		dtstart.SetMonth(1);
		dtend = dtstart;
		dtend.OffsetYear(1);
		break;
	case NCalendarView::eYears:
		// This year - step back to beginning
		dtstart.SetDay(1);
		dtstart.SetMonth(1);
		dtend = dtstart;
		dtend.OffsetYear(1);
		
		// Two years either side
		dtstart.OffsetYear(-2);
		dtend.OffsetYear(2);
		break;
	}
	
	// Get events in the range
	iCal::CICalendarExpandedComponents vevents;
	iCal::CICalendarPeriod period(dtstart, dtend);
	
	// Look for single calendar
	if (IsSingleCalendar())
	{
		// Get events
		if (GetCalendar() != NULL)
			GetCalendar()->GetVEvents(period, vevents, false);
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
			(*iter)->GetVEvents(period, vevents, false);
		}
	}
	
	// Add events to table
	static_cast<CSummaryTable*>(GetTable())->AddEvents(vevents);
	
	// Set range string
	dtstart.SetDateOnly(true);
	dtend.SetDateOnly(true);
	dtend.OffsetDay(-1);
	
	mRangeText = dtstart.GetLocaleDate(iCal::CICalendarDateTime::eFullDate);
	mRangeText += " - ";
	mRangeText += dtend.GetLocaleDate(iCal::CICalendarDateTime::eFullDate);

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CSummaryView::OnToday()
{
	mDate.SetToday();
	
	// Now offset to start of week if in week view

	ResetDate();
}

void CSummaryView::OnNewEvent()
{
	if (mCalendarView == NULL)
		return;

	CNewEventDialog::StartNew(mDate, IsSingleCalendar() ? GetCalendar() : NULL);
}


