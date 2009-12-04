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

// ---------------------------------------------------------------------------
//	CNewEventTiming														  [public]
/**
	Default constructor */

CNewEventTiming::CNewEventTiming() :
	CNewTimingPanel(IDD_CALENDAR_NEWEVENTTIMING)
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

BEGIN_MESSAGE_MAP(CNewEventTiming, CNewTimingPanel)
	ON_COMMAND(IDC_CALENDAR_NEWEVENTTIMING_ALLDAY, OnAllDay)
	ON_COMMAND(IDC_CALENDAR_NEWEVENTTIMING_ENDSON, OnEnds)
	ON_COMMAND(IDC_CALENDAR_NEWEVENTTIMING_DURATIONON, OnDuration)
END_MESSAGE_MAP()

BOOL CNewEventTiming::OnInitDialog()
{
	CNewTimingPanel::OnInitDialog();

	// Get UI items
	mAllDay.SubclassDlgItem(IDC_CALENDAR_NEWEVENTTIMING_ALLDAY, this);

	mStartDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEWEVENTTIMING_STARTS, this);
	mStartDateTimeZone = new CDateTimeZoneSelect;
	mStartDateTimeZoneItem.AddPanel(mStartDateTimeZone);
	mStartDateTimeZoneItem.SetPanel(0);

	mUseEnd.SubclassDlgItem(IDC_CALENDAR_NEWEVENTTIMING_ENDSON, this);
	mUseDuration.SubclassDlgItem(IDC_CALENDAR_NEWEVENTTIMING_DURATIONON, this);

	mEndDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEWEVENTTIMING_ENDS, this);
	mEndDateTimeZone = new CDateTimeZoneSelect;
	mEndDateTimeZoneItem.AddPanel(mEndDateTimeZone);
	mEndDateTimeZoneItem.SetPanel(0);

	mDurationItem.SubclassDlgItem(IDC_CALENDAR_NEWEVENTTIMING_DURATION, this);
	mDuration = new CDurationSelect;
	mDurationItem.AddPanel(mDuration);
	mDurationItem.SetPanel(0);
	mDuration->EnableWindow(false);
	
	// Init controls
	DoAllDay(false);
	DoEnds(false);
	
	return true;
}

void CNewEventTiming::SetReadOnly(bool read_only)
{
	mAllDay.EnableWindow(!read_only);

	mStartDateTimeZone->SetReadOnly(read_only);

	mUseEnd.EnableWindow(!read_only);
	mUseDuration.EnableWindow(!read_only);

	mEndDateTimeZone->SetReadOnly(read_only);

	mDuration->SetReadOnly(read_only);
}

void CNewEventTiming::OnAllDay()
{
	DoAllDay(mAllDay.GetCheck());
}

void CNewEventTiming::OnEnds()
{
	DoEnds(false);
}

void CNewEventTiming::OnDuration()
{
	DoEnds(true);
}

bool CNewEventTiming::GetAllDay() const
{
	return mAllDay.GetCheck() == 1;
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
	if (set && (mUseDuration.GetCheck() == 0))
	{
		mUseEnd.SetCheck(0);
		mUseDuration.SetCheck(1);
		DoEnds(true);
	}
}

void CNewEventTiming::DoEnds(bool use_duration)
{
	mEndDateTimeZone->EnableWindow(!use_duration);
	mDuration->EnableWindow(use_duration);
	
	if (use_duration)
		SyncDuration();
	else
		SyncEnd();
}

void CNewEventTiming::SetEvent(const iCal::CICalendarVEvent& vevent)
{
	// Set the relevant fields
	mAllDay.SetCheck(vevent.GetStart().IsDateOnly() ? 1 : 0);
	
	const iCal::CICalendarDateTime& event_start = vevent.GetStart();
	const iCal::CICalendarDateTime& event_end = vevent.GetEnd();

	// Set start date-time
	mStartDateTimeZone->SetDateTimeZone(event_start, vevent.GetStart().IsDateOnly());
	
	// Set ending type
	if (vevent.UseDuration())
	{
		mUseDuration.SetCheck(1);
		DoEnds(true);
	}
	else
	{
		mUseEnd.SetCheck(1);
		DoEnds(false);
	}
	
	// Set end date-time
	if (vevent.GetStart().IsDateOnly())
	{
		// Switch to inclusive end as that is whay user will expect

		// Offset end day by one as its inclusive for all day events
		iCal::CICalendarDateTime temp(event_end);
		temp.OffsetDay(-1);
		mEndDateTimeZone->SetDateTimeZone(temp, vevent.GetStart().IsDateOnly());
	}
	else
		// Set non-inclusive end
		mEndDateTimeZone->SetDateTimeZone(event_end, vevent.GetStart().IsDateOnly());
	
	// Set duration
	iCal::CICalendarDuration duration = event_end - event_start;
	mDuration->SetDuration(duration, mAllDay.GetCheck() == 1);
}

void CNewEventTiming::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do timing
	bool all_day = (mAllDay.GetCheck() == 1);
	if (mUseDuration.GetCheck() == 1)
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

void CNewEventTiming::SyncEnd()
{
	bool all_day = (mAllDay.GetCheck() == 1);

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
	bool all_day = (mAllDay.GetCheck() == 1);

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
