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

// ---------------------------------------------------------------------------
//	CNewToDoTiming														  [public]
/**
	Default constructor */

CNewToDoTiming::CNewToDoTiming() :
	CNewTimingPanel(IDD_CALENDAR_NEWTODOTIMING)
{
}


// ---------------------------------------------------------------------------
//	~CNewToDoTiming														  [public]
/**
	Destructor */

CNewToDoTiming::~CNewToDoTiming()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewToDoTiming, CNewTimingPanel)
	ON_COMMAND(IDC_CALENDAR_NEWTODOTIMING_ALLDAY, OnAllDay)
	ON_COMMAND(IDC_CALENDAR_NEWTODOTIMING_NODUEDATE, OnNoDue)
	ON_COMMAND(IDC_CALENDAR_NEWTODOTIMING_DUEBY, OnDueBy)
	ON_COMMAND(IDC_CALENDAR_NEWTODOTIMING_STARTSON, OnStarts)
END_MESSAGE_MAP()

BOOL CNewToDoTiming::OnInitDialog()
{
	CNewTimingPanel::OnInitDialog();

	// Get UI items
	mAllDay.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_ALLDAY, this);

	mNoDue.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_NODUEDATE, this);
	mDueBy.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_DUEBY, this);
	mStarts.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_STARTSON, this);

	mDueDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_DUE, this);
	mDueDateTimeZone = new CDateTimeZoneSelect;
	mDueDateTimeZoneItem.AddPanel(mDueDateTimeZone);
	mDueDateTimeZoneItem.SetPanel(0);
	mDueDateTimeZone->EnableWindow(false);

	mStartDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_STARTS, this);
	mStartDateTimeZone = new CDateTimeZoneSelect;
	mStartDateTimeZoneItem.AddPanel(mStartDateTimeZone);
	mStartDateTimeZoneItem.SetPanel(0);
	mStartDateTimeZone->EnableWindow(false);

	mDurationItem.SubclassDlgItem(IDC_CALENDAR_NEWTODOTIMING_DURATION, this);
	mDuration = new CDurationSelect;
	mDurationItem.AddPanel(mDuration);
	mDurationItem.SetPanel(0);
	mDuration->EnableWindow(false);
	
	// Init controls
	DoAllDay(false);
	DoDueGroup(eDue_NoDue);

	return true;
}

void CNewToDoTiming::SetReadOnly(bool read_only)
{
	mAllDay.EnableWindow(!read_only);

	mNoDue.EnableWindow(!read_only);
	mDueBy.EnableWindow(!read_only);
	mStarts.EnableWindow(!read_only);

	mDueDateTimeZone->SetReadOnly(read_only);
	mStartDateTimeZone->SetReadOnly(read_only);
	mDuration->SetReadOnly(read_only);
}

void CNewToDoTiming::OnAllDay()
{
	DoAllDay(mAllDay.GetCheck());
}

void CNewToDoTiming::OnNoDue()
{
	DoDueGroup(eDue_NoDue);
}

void CNewToDoTiming::OnDueBy()
{
	DoDueGroup(eDue_DueBy);
}

void CNewToDoTiming::OnStarts()
{
	DoDueGroup(eDue_Start);
}

bool CNewToDoTiming::GetAllDay() const
{
	return mAllDay.GetCheck() == 1;
}

void CNewToDoTiming::GetTimezone(iCal::CICalendarTimezone& tz) const
{
	iCal::CICalendarDateTime dt;
	if (mStarts.GetCheck() == 1)
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

void CNewToDoTiming::DoDueGroup(uint32_t group)
{
	switch(group)
	{
	case eDue_NoDue:
		mDueDateTimeZone->EnableWindow(false);
		mStartDateTimeZone->EnableWindow(false);
		mDuration->EnableWindow(false);
		break;
	case eDue_DueBy:
		mDueDateTimeZone->EnableWindow(true);
		mStartDateTimeZone->EnableWindow(false);
		mDuration->EnableWindow(false);
		break;
	case eDue_Start:
		mDueDateTimeZone->EnableWindow(false);
		mStartDateTimeZone->EnableWindow(true);
		mDuration->EnableWindow(true);
		break;
	}
}

void CNewToDoTiming::SetToDo(const iCal::CICalendarVToDo& vtodo)
{
	// Set the relevant fields
	bool all_day = true;
	if (vtodo.HasEnd())
		all_day = vtodo.GetEnd().IsDateOnly();
	else if (vtodo.HasStart())
		all_day = vtodo.GetStart().IsDateOnly();
	mAllDay.SetCheck(all_day ? 1 : 0);
	DoAllDay(all_day);
	
	if (vtodo.HasEnd())
	{
		mDueBy.SetCheck(1);
		DoDueGroup(eDue_DueBy);
		mDueDateTimeZone->SetDateTimeZone(vtodo.GetEnd(), all_day);
	}
	else if (vtodo.HasStart())
	{
		mStarts.SetCheck(1);
		DoDueGroup(eDue_Start);
		mStartDateTimeZone->SetDateTimeZone(vtodo.GetStart(), all_day);

		// Set duration
		iCal::CICalendarDuration duration = vtodo.GetEnd() - vtodo.GetStart();
		mDuration->SetDuration(duration, mAllDay.GetCheck() == 1);
	}
	else
	{
		mNoDue.SetCheck(1);
		DoDueGroup(eDue_NoDue);
	}
}

void CNewToDoTiming::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do timing
	bool all_day = (mAllDay.GetCheck() == 1);

	if (mNoDue.GetCheck() == 1)
	{
		vtodo.EditTiming();
	}
	else if (mDueBy.GetCheck() == 1)
	{
		iCal::CICalendarDateTime due;
		mDueDateTimeZone->GetDateTimeZone(due, all_day);
		vtodo.EditTiming(due);
	}
	else if (mStarts.GetCheck() == 1)
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);
		
		iCal::CICalendarDuration duration;
		mDuration->GetDuration(duration, all_day);
		
		vtodo.EditTiming(dtstart, duration);
	}
}
