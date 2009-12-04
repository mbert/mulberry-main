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

#include "CNewEventTiming.h"

#include "CDateTimeZoneSelect.h"
#include "CDurationSelect.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarComponentExpanded.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// ---------------------------------------------------------------------------
//	CNewEventTiming														  [public]
/**
	Default constructor */

CNewEventTiming::CNewEventTiming(LStream *inStream) :
	CNewTimingPanel(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CNewEventTiming														  [public]
/**
	Destructor */

CNewEventTiming::~CNewEventTiming()
{
}

#pragma mark -

void CNewEventTiming::FinishCreateSelf()
{
	// Get UI items
	mAllDay = dynamic_cast<LCheckBox*>(FindPaneByID(eAllDay_ID));

	mStartDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eStartDateTimeZone_ID)));

	mUseEnd = dynamic_cast<LRadioButton*>(FindPaneByID(eEnds_ID));
	mUseDuration = dynamic_cast<LRadioButton*>(FindPaneByID(eDuration_ID));

	mEndDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eEndDateTimeZone_ID)));

	mDuration = CDurationSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eDurationSelect_ID)));
	mDuration->SetEnabled(false);

	// Listen to some of them
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
	
	// Init controls
	DoAllDay(false);
	DoEnds(false);
}

// Respond to clicks in the icon buttons
void CNewEventTiming::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eAllDay_ID:
		DoAllDay(mAllDay->GetValue() == 1);
		break;
	case eEnds_ID:
		if (*(long*)ioParam)
			DoEnds(false);
		break;
	case eDuration_ID:
		if (*(long*)ioParam)
			DoEnds(true);
		break;
	}
}

void CNewEventTiming::SetReadOnly(bool read_only)
{
	mAllDay->SetEnabled(!read_only);

	mStartDateTimeZone->SetReadOnly(read_only);

	mUseEnd->SetEnabled(!read_only);
	mUseDuration->SetEnabled(!read_only);

	mEndDateTimeZone->SetReadOnly(read_only);

	mDuration->SetReadOnly(read_only);
}

bool CNewEventTiming::GetAllDay() const
{
	return mAllDay->GetValue() == 1;
}

void CNewEventTiming::GetTimezone(iCal::CICalendarTimezone& tz) const
{
	iCal::CICalendarDateTime dt;
	mStartDateTimeZone->GetDateTimeZone(dt, true);
	tz = dt.GetTimezone();
}

void CNewEventTiming::DoAllDay(bool set)
{
	mStartDateTimeZone->SetAllDay(set);
	mEndDateTimeZone->SetAllDay(set);
	mDuration->SetAllDay(set);
	
	// Make duration the default ending type when turning on all day
	if (set)
		mUseDuration->SetValue(1);
}

void CNewEventTiming::DoEnds(bool use_duration)
{
	mEndDateTimeZone->SetEnabled(!use_duration);
	mDuration->SetEnabled(use_duration);
	
	if (use_duration)
		SyncDuration();
	else
		SyncEnd();
}

void CNewEventTiming::SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded)
{
	iCal::CICalendarDateTime event_start;
	iCal::CICalendarDateTime event_end;
	
	if (expanded != NULL)
	{
		event_start = expanded->GetInstanceStart();
		event_end = expanded->GetInstanceEnd();
	}
	else
	{
		event_start = vevent.GetStart();
		event_end = vevent.GetEnd();
	}

	// Set the relevant fields
	mAllDay->SetValue(event_start.IsDateOnly() ? 1 : 0);
	
	// Set start date-time
	mStartDateTimeZone->SetDateTimeZone(event_start, event_start.IsDateOnly());
	
	// Set ending type
	if (vevent.UseDuration())
		mUseDuration->SetValue(1);
	else
		mUseEnd->SetValue(1);
	
	// Set end date-time
	if (event_start.IsDateOnly())
	{
		// Switch to inclusive end as that is what user will expect

		// Offset end day by one as its inclusive for all day events
		iCal::CICalendarDateTime temp(event_end);
		temp.OffsetDay(-1);
		mEndDateTimeZone->SetDateTimeZone(temp, event_start.IsDateOnly());
	}
	else
		// Set non-inclusive end
		mEndDateTimeZone->SetDateTimeZone(event_end, event_start.IsDateOnly());
	
	// Set duration
	iCal::CICalendarDuration duration = event_end - event_start;
	mDuration->SetDuration(duration, mAllDay->GetValue() == 1);
}

void CNewEventTiming::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do timing
	bool all_day = (mAllDay->GetValue() == 1);
	if (mUseDuration->GetValue() == 1)
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);
		
		iCal::CICalendarDuration duration;
		mDuration->GetDuration(duration, all_day);
		
		vevent.EditTiming(dtstart, duration);
	}
	else
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);

		iCal::CICalendarDateTime dtend;
		mEndDateTimeZone->GetDateTimeZone(dtend, all_day);
		
		// If all day event, the end display is the inclusive end, but iCal uses non-inclusive
		// so we must adjust by one day
		if (all_day)
			dtend.OffsetDay(1);

		vevent.EditTiming(dtstart, dtend);
	}
}

void CNewEventTiming::GetPeriod(iCal::CICalendarPeriod& period)
{
	// Do timing
	bool all_day = (mAllDay->GetValue() == 1);
	if (mUseDuration->GetValue() == 1)
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);
		
		iCal::CICalendarDuration duration;
		mDuration->GetDuration(duration, all_day);
		
		period = iCal::CICalendarPeriod(dtstart, duration);
	}
	else
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);
		
		iCal::CICalendarDateTime dtend;
		mEndDateTimeZone->GetDateTimeZone(dtend, all_day);
		
		// If all day event, the end display is the inclusive end, but iCal uses non-inclusive
		// so we must adjust by one day
		if (all_day)
			dtend.OffsetDay(1);
		
		period = iCal::CICalendarPeriod(dtstart, dtend);
	}
}

void CNewEventTiming::SyncEnd()
{
	bool all_day = (mAllDay->GetValue() == 1);

	// Get start
	iCal::CICalendarDateTime dtstart;
	mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);

	// Get duration	
	iCal::CICalendarDuration duration;
	mDuration->GetDuration(duration, all_day);
	
	// Get end
	iCal::CICalendarDateTime dtend = dtstart + duration;
	
	// Set end
	if (all_day)
	{
		// Switch to inclusive end as that is way user will expect
		dtend.OffsetDay(-1);
	}
	mEndDateTimeZone->SetDateTimeZone(dtend, all_day);
}

void CNewEventTiming::SyncDuration()
{
	bool all_day = (mAllDay->GetValue() == 1);

	// Get start
	iCal::CICalendarDateTime dtstart;
	mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);

	// Get end	
	iCal::CICalendarDateTime dtend;
	mEndDateTimeZone->GetDateTimeZone(dtend, all_day);

	// If all day event, the end display is the inclusive end, but iCal uses non-inclusive
	// so we must adjust by one day
	if (all_day)
		dtend.OffsetDay(1);
	
	// Get duration
	iCal::CICalendarDuration duration = dtend - dtstart;
	
	// Cannot be less than or equal to zero
	if (!duration.GetForward() || (duration.GetTotalSeconds() == 0))
		duration.SetDuration(60LL * 60LL);
	
	// Set duration
	mDuration->SetDuration(duration, all_day);
}
