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

#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

#include <cassert>

#pragma mark -

void CNewEventTiming::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mAllDay =
        new JXTextCheckbox("All Day Event", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,0, 120,20);
    assert( mAllDay != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Starts on:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,32, 70,20);
    assert( obj1 != NULL );

    mStartDateTimeZone =
        new CDateTimeZoneSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,25, 400,30);
    assert( mStartDateTimeZone != NULL );

    mUseGroup =
        new JXRadioGroup(this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 10,55, 80,60);
    assert( mUseGroup != NULL );
    mUseGroup->SetBorderWidth(0);

    mUseEnd =
        new JXTextRadioButton(1, "Ends on:", mUseGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,5, 80,20);
    assert( mUseEnd != NULL );

    mEndDateTimeZone =
        new CDateTimeZoneSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,55, 400,30);
    assert( mEndDateTimeZone != NULL );

    mUseDuration =
        new JXTextRadioButton(2, "Duration:", mUseGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,35, 80,20);
    assert( mUseDuration != NULL );

    mDuration =
        new CDurationSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,85, 400,30);
    assert( mDuration != NULL );

// end JXLayout1

	mStartDateTimeZone->OnCreate();
	mEndDateTimeZone->OnCreate();
	mDuration->OnCreate();
	mDuration->SetActive(false);

	// Init controls
	DoAllDay(false);
	DoEnds(false);
	
	ListenTo(mAllDay);
	ListenTo(mUseGroup);
}

// Respond to clicks in the icon buttons
void CNewEventTiming::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mAllDay)
		{
			DoAllDay(mAllDay->IsChecked());
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mUseGroup)
		{
			DoEnds(index == eEnd_Duration);
			return;
		}
	}
}

void CNewEventTiming::SetReadOnly(bool read_only)
{
	mAllDay->SetActive(!read_only);

	mStartDateTimeZone->SetReadOnly(read_only);

	mUseEnd->SetActive(!read_only);
	mUseDuration->SetActive(!read_only);

	mEndDateTimeZone->SetReadOnly(read_only);

	mDuration->SetReadOnly(read_only);
}

bool CNewEventTiming::GetAllDay() const
{
	return mAllDay->IsChecked();
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
		mUseGroup->SelectItem(eEnd_Duration);
}

void CNewEventTiming::DoEnds(bool use_duration)
{
	mEndDateTimeZone->SetActive(!use_duration);
	mDuration->SetActive(use_duration);
	
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
	mAllDay->SetState(event_start.IsDateOnly() ? 1 : 0);
	
	// Set start date-time
	mStartDateTimeZone->SetDateTimeZone(event_start, event_start.IsDateOnly());
	
	// Set ending type
	if (vevent.UseDuration())
		mUseGroup->SelectItem(eEnd_Duration);
	else
		mUseGroup->SelectItem(eEnd_End);
	
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
	mDuration->SetDuration(duration, mAllDay->IsChecked());
}

void CNewEventTiming::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do timing
	bool all_day = mAllDay->IsChecked();
	if (mUseGroup->GetSelectedItem() == eEnd_Duration)
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
	bool all_day = mAllDay->IsChecked();
	if (mUseGroup->GetSelectedItem() == eEnd_Duration)
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
	bool all_day = mAllDay->IsChecked();

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
	bool all_day = mAllDay->IsChecked();

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
