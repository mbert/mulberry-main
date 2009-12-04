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

#include "CYearView.h"

#include "CYearTable.h"

#include "CCalendarStoreManager.h"

void CYearView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
	case CTableDrag::eBroadcast_Deactivate:
	case CTableDrag::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		Broadcast_Message(msg, param);
		break;

	case CYearTable::eBroadcast_DblClkToMonth:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToMonth, param);
		break;
	case CYearTable::eBroadcast_DblClkToDay:
		// Rebroadcast to parent
		Broadcast_Message(eBroadcast_DblClkToDay, param);
		break;
	}
}

iCal::CICalendarDateTime CYearView::GetSelectedDate() const
{
	// Is anything selected in the table
	iCal::CICalendarDateTime date;
	if (!static_cast<CYearTable*>(GetTable())->GetSelectedDate(date))
		date = mDate;
	return date;
}

cdstring CYearView::GetTitle() const
{
	cdstring year((unsigned long) mDate.GetYear());
	return year;
}

void CYearView::ResetDate()
{
	// Set static captions
	SetCaptions();
	
#if 0
	iCal::CICalendarDateTime dtstart(mDate.GetYear(), 1, 1);
	dtstart.SetDateOnly(false);
	iCal::CICalendarDateTime dtend(dtstart);
	dtend.OffsetYear(1);
	
	// Get events in the range
	ulvector event_days;
	iCal::CICalendarPeriod period(dtstart, dtend);

	// Only do visible calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			// Listen to the calendar in case it gets destroyed
			(*iter)->Add_Listener(this);
			
			// Get events
			iCal::CICalendarExpandedComponents vevents;
			(*iter)->GetVEvents(period, vevents);
		}
	}
#endif

	// Reset table
	static_cast<CYearTable*>(GetTable())->ResetTable(mDate.GetYear(), mLayoutValue);

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CYearView::OnPreviousYear()
{
	mDate.OffsetYear(-1);
	ResetDate();
}

void CYearView::OnNextYear()
{
	mDate.OffsetYear(1);
	ResetDate();
}

void CYearView::OnThisYear()
{
	mDate.SetToday();
	ResetDate();
}

void CYearView::OnNewEvent()
{
}
