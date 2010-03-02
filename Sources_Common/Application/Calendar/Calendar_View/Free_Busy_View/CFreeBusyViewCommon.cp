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
//	PPxStaticText.cp			PowerPlantX 1.0			©2003 Metrowerks Corp.
// ===========================================================================

#include "CFreeBusyView.h"

#include "CCalendarEventBase.h"
#include "CCalendarView.h"
#include "CFreeBusyTable.h"
#include "CFreeBusyTitleTable.h"
#include "CNewEventDialog.h"
#include "CPreferences.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"

#include "CICalendarManager.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

void CFreeBusyView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CTableDrag::eBroadcast_Activate:
	case CTableDrag::eBroadcast_Deactivate:
	case CTableDrag::eBroadcast_SelectionChanged:
		// Rebroadcast to parent
		Broadcast_Message(msg, param);
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

iCal::CICalendarDateTime CFreeBusyView::GetSelectedDate() const
{
	// Is anything selected in the table
	return mDate;
}

cdstring CFreeBusyView::GetTitle() const
{
	cdstring result = mDate.GetLocaleDate(iCal::CICalendarDateTime::eFullDate);
	return result;
}

void CFreeBusyView::ResetDate()
{
	// Only if we have a celendar
	if (mCalendarView == NULL)
		return;
	
	// Date must be at start of day
	mDate.SetHHMMSS(0, 0, 0);

	// Adjust date based on type
	int32_t num_days = 1;

	static_cast<CFreeBusyTable*>(GetTable())->ResetTable(mDate, mFreeBusyRange);

	// Get date range for event lookup
	iCal::CICalendarDateTime dtstart(mDate);
	dtstart.SetDateOnly(false);
	dtstart.AdjustToUTC();
	iCal::CICalendarDateTime dtend(dtstart);
	dtend.OffsetDay(num_days);
	
	// Get events/free-busy in the range
	iCal::CICalendarPeriod period(dtstart, dtend);

	const iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mCalendarRef);
	const CIdentity* identity = CPreferences::sPrefs->mIdentities.GetValue().GetIdentity(mIdentityRef);
	if (cal && identity)
	{
		iCal::CITIPScheduleResultsList results;
		iCal::CITIPProcessor::FreeBusyRequest(&mOrganizer, &mAttendees, cal, identity, period, results);
		
		calstore::CCalendarStoreFreeBusyList selected;
		for(iCal::CITIPScheduleResultsList::const_iterator iter = results.begin(); iter != results.end(); iter++)
		{
			cdstring name = (*iter).GetAttendee();
			if (name.empty())
				name = (*iter).GetRecipient();
			calstore::CCalendarStoreFreeBusy items(name);
			items.SetPeriods((*iter).GetPeriods());
			selected.push_back(items);
		}
		mDetails = selected;
	}

	dynamic_cast<CFreeBusyTable*>(GetTable())->AddItems(mDetails);
	FRAMEWORK_REFRESH_WINDOW(GetTable())

	// Broadcast change to listeners
	Broadcast_Message(eBroadcast_ViewChanged, this);
}

void CFreeBusyView::SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date)
{
	// Change date to be today
	mCalendarRef = calref;
	mIdentityRef = id;
	mOrganizer = organizer;
	mAttendees = attendees;
	SetDate(date);
}

void CFreeBusyView::OnPrevDay()
{
	mDate.OffsetDay(-1);
	ResetDate();
}

void CFreeBusyView::OnNextDay()
{
	mDate.OffsetDay(1);
	ResetDate();
}

void CFreeBusyView::OnToday()
{
	mDate.SetToday();
	
	// Now offset to start of week if in week view

	ResetDate();
}

void CFreeBusyView::OnNewEvent()
{
}

void CFreeBusyView::OnNewEvent(const iCal::CICalendarDateTime& dt)
{
}

