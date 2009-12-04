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

#include "CToDoView.h"

#include "CCalendarView.h"
#include "CNewToDoDialog.h"
#include "CToDoItem.h"
#include "CToDoTable.h"

#include "CCalendarStoreManager.h"

#include <algorithm>

void CToDoView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
	case CTableDrag::eBroadcast_Deactivate:
	case CTableDrag::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		Broadcast_Message(msg, param);
		break;

	case CToDoItem::eBroadcast_EditToDo:
		// Reset all to dos
		ResetDate();
		break;

	default:
		CCalendarViewBase::ListenTo_Message(msg, param);
		break;
	}
}

iCal::CICalendarDateTime CToDoView::GetSelectedDate() const
{
	// Is anything selected in the table
	return iCal::CICalendarDateTime::GetToday(NULL);
}

cdstring CToDoView::GetTitle() const
{
	return cdstring::null_str;
}

void CToDoView::ResetDate()
{
	// Only if we have a celendar
	if (mCalendarView == NULL)
		return;

	// Strictly speaking there is no date to set for To Do items - instead we use
	// the Show popup to determine which items are visible
	
	// Get to dos in the range
	iCal::CICalendarExpandedComponents vtodos;
	bool only_due = false;
	bool all_dates = false;
	iCal::CICalendarDateTime dt;
	dt.SetToday();
	switch(mShowValue)
	{
	case eShowAllDue:
		only_due = true;
		all_dates = true;
		break;
	case eShowAll:
		only_due = false;
		all_dates = true;
		break;
	case eShowDueToday:
		only_due = true;
		all_dates = false;
		break;
	case eShowDueThisWeek:
		only_due = true;
		all_dates = false;
		dt.OffsetDay(6 - dt.GetDayOfWeek());
		break;
	case eShowOverdue:
		only_due = true;
		all_dates = false;
		dt.OffsetDay(-1);
		break;
	}

	// Look for single calendar
	if (IsSingleCalendar())
	{
		// Get to dos
		if (GetCalendar() != NULL)
			GetCalendar()->GetVToDos(only_due, all_dates, dt, vtodos);
	}

	// Only do visible calendars
	else if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			// Listen to the calendar in case it gets destroyed
			(*iter)->Add_Listener(this);
			
			// Get to dos
			(*iter)->GetVToDos(only_due, all_dates, dt, vtodos);
		}
	}
	std::sort(vtodos.begin(), vtodos.end(), iCal::CICalendarVToDo::sort_for_display);
	
	// Add events to table
	static_cast<CToDoTable*>(GetTable())->AddToDos(vtodos);
	
	FRAMEWORK_REFRESH_WINDOW(this);
}

void CToDoView::OnNewToDo()
{
	CNewToDoDialog::StartNew(IsSingleCalendar() ? GetCalendar() : NULL);
}
