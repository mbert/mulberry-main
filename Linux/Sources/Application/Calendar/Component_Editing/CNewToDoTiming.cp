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

#include "CNewToDoTiming.h"

#include "CDateTimeZoneSelect.h"
#include "CDurationSelect.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarVToDo.h"

#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>

#include <cassert>

#pragma mark -

void CNewToDoTiming::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mAllDay =
        new JXTextCheckbox("Date Only - no Time", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,0, 140,20);
    assert( mAllDay != NULL );

    mDueGroup =
        new JXRadioGroup(this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 10,20, 100,80);
    assert( mDueGroup != NULL );
    mDueGroup->SetBorderWidth(0);

    mNoDue =
        new JXTextRadioButton(1, "No Due date", mDueGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 100,20);
    assert( mNoDue != NULL );

    mDueBy =
        new JXTextRadioButton(2, "Due by:", mDueGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,25, 90,20);
    assert( mDueBy != NULL );

    mDueDateTimeZone =
        new CDateTimeZoneSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,40, 400,30);
    assert( mDueDateTimeZone != NULL );

    mStarts =
        new JXTextRadioButton(3, "Starts on:", mDueGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,55, 90,20);
    assert( mStarts != NULL );

    mStartDateTimeZone =
        new CDateTimeZoneSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,70, 400,30);
    assert( mStartDateTimeZone != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Duration:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 31,107, 60,20);
    assert( obj1 != NULL );

    mDuration =
        new CDurationSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,100, 400,30);
    assert( mDuration != NULL );

// end JXLayout1

	mDueDateTimeZone->OnCreate();
	mDueDateTimeZone->SetActive(false);
	mStartDateTimeZone->OnCreate();
	mStartDateTimeZone->SetActive(false);
	mDuration->OnCreate();
	mDuration->SetActive(false);

	ListenTo(mAllDay);
	ListenTo(mDueGroup);
}

// Respond to clicks in the icon buttons
void CNewToDoTiming::Receive(JBroadcaster* sender, const Message& message)
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
		if (sender == mDueGroup)
		{
			DoDueGroup(index);
			return;
		}
	}
}

void CNewToDoTiming::SetReadOnly(bool read_only)
{
	mAllDay->SetActive(!read_only);

	mNoDue->SetActive(!read_only);
	mDueBy->SetActive(!read_only);
	mStarts->SetActive(!read_only);

	mDueDateTimeZone->SetReadOnly(read_only);
	mStartDateTimeZone->SetReadOnly(read_only);
	mDuration->SetReadOnly(read_only);
}

bool CNewToDoTiming::GetAllDay() const
{
	return mAllDay->IsChecked();
}

void CNewToDoTiming::GetTimezone(iCal::CICalendarTimezone& tz) const
{
	iCal::CICalendarDateTime dt;
	if (mDueGroup->GetSelectedItem() == eDue_Start)
		mStartDateTimeZone->GetDateTimeZone(dt, true);
	else
		mDueDateTimeZone->GetDateTimeZone(dt, true);
	tz = dt.GetTimezone();
}

void CNewToDoTiming::DoAllDay(bool set)
{
	mDueDateTimeZone->SetAllDay(set);
	mStartDateTimeZone->SetAllDay(set);
	mDuration->SetAllDay(set);
}

void CNewToDoTiming::DoDueGroup(JIndex group)
{
	switch(group)
	{
	case eDue_NoDue:
		mDueDateTimeZone->SetActive(false);
		mStartDateTimeZone->SetActive(false);
		mDuration->SetActive(false);
		break;
	case eDue_DueBy:
		mDueDateTimeZone->SetActive(true);
		mStartDateTimeZone->SetActive(false);
		mDuration->SetActive(false);
		break;
	case eDue_Start:
		mDueDateTimeZone->SetActive(false);
		mStartDateTimeZone->SetActive(true);
		mDuration->SetActive(true);
		break;
	}
}

void CNewToDoTiming::SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded)
{
	// Set the relevant fields
	bool all_day = true;
	if (vtodo.HasEnd())
		all_day = vtodo.GetEnd().IsDateOnly();
	else if (vtodo.HasStart())
		all_day = vtodo.GetStart().IsDateOnly();
	mAllDay->SetState(all_day ? 1 : 0);
	
	if (vtodo.HasEnd())
	{
		mDueGroup->SelectItem(eDue_DueBy);
		mDueDateTimeZone->SetDateTimeZone(vtodo.GetEnd(), all_day);
	}
	else if (vtodo.HasStart())
	{
		mDueGroup->SelectItem(eDue_Start);
		mStartDateTimeZone->SetDateTimeZone(vtodo.GetStart(), all_day);

		// Set duration
		iCal::CICalendarDuration duration = vtodo.GetEnd() - vtodo.GetStart();
		mDuration->SetDuration(duration, mAllDay->IsChecked());
	}
}

void CNewToDoTiming::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do timing
	bool all_day = mAllDay->IsChecked();

	if (mDueGroup->GetSelectedItem() == eDue_NoDue)
	{
		vtodo.EditTiming();
	}
	else if (mDueGroup->GetSelectedItem() == eDue_DueBy)
	{
		iCal::CICalendarDateTime due;
		mDueDateTimeZone->GetDateTimeZone(due, all_day);
		vtodo.EditTiming(due);
	}
	else if (mDueGroup->GetSelectedItem() == eDue_Start)
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);
		
		iCal::CICalendarDuration duration;
		mDuration->GetDuration(duration, all_day);
		
		vtodo.EditTiming(dtstart, duration);
	}
}
