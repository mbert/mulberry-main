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

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// ---------------------------------------------------------------------------
//	CNewToDoTiming														  [public]
/**
	Default constructor */

CNewToDoTiming::CNewToDoTiming(LStream *inStream) :
	CNewTimingPanel(inStream)
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

void CNewToDoTiming::FinishCreateSelf()
{
	// Get UI items
	mAllDay = dynamic_cast<LCheckBox*>(FindPaneByID(eAllDay_ID));

	mNoDue = dynamic_cast<LRadioButton*>(FindPaneByID(eNoDue_ID));
	mDueBy = dynamic_cast<LRadioButton*>(FindPaneByID(eDueBy_ID));
	mStarts = dynamic_cast<LRadioButton*>(FindPaneByID(eStarts_ID));

	mDueDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eEndDateTimeZone_ID)));
	mDueDateTimeZone->SetEnabled(false);
	mStartDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eStartDateTimeZone_ID)));
	mStartDateTimeZone->SetEnabled(false);
	mDuration = CDurationSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eDurationSelect_ID)));
	mDuration->SetEnabled(false);

	// Listen to some of them
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

// Respond to clicks in the icon buttons
void CNewToDoTiming::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eAllDay_ID:
		DoAllDay(mAllDay->GetValue() == 1);
		break;
	case eNoDue_ID:
		if (*(long*)ioParam)
			DoDueGroup(eDue_NoDue);
		break;
	case eDueBy_ID:
		if (*(long*)ioParam)
			DoDueGroup(eDue_DueBy);
		break;
	case eStarts_ID:
		if (*(long*)ioParam)
			DoDueGroup(eDue_Start);
		break;
	}
}

void CNewToDoTiming::SetReadOnly(bool read_only)
{
	mAllDay->SetEnabled(!read_only);

	mNoDue->SetEnabled(!read_only);
	mDueBy->SetEnabled(!read_only);
	mStarts->SetEnabled(!read_only);

	mDueDateTimeZone->SetReadOnly(read_only);
	mStartDateTimeZone->SetReadOnly(read_only);
	mDuration->SetReadOnly(read_only);
}

bool CNewToDoTiming::GetAllDay() const
{
	return mAllDay->GetValue() == 1;
}

void CNewToDoTiming::GetTimezone(iCal::CICalendarTimezone& tz) const
{
	iCal::CICalendarDateTime dt;
	if (mStarts->GetValue() == 1)
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
		mDueDateTimeZone->SetEnabled(false);
		mStartDateTimeZone->SetEnabled(false);
		mDuration->SetEnabled(false);
		break;
	case eDue_DueBy:
		mDueDateTimeZone->SetEnabled(true);
		mStartDateTimeZone->SetEnabled(false);
		mDuration->SetEnabled(false);
		break;
	case eDue_Start:
		mDueDateTimeZone->SetEnabled(false);
		mStartDateTimeZone->SetEnabled(true);
		mDuration->SetEnabled(true);
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
	mAllDay->SetValue(all_day ? 1 : 0);
	
	if (vtodo.HasEnd())
	{
		mDueBy->SetValue(1);
		mDueDateTimeZone->SetDateTimeZone(vtodo.GetEnd(), all_day);
	}
	else if (vtodo.HasStart())
	{
		mStarts->SetValue(1);
		mStartDateTimeZone->SetDateTimeZone(vtodo.GetStart(), all_day);

		// Set duration
		iCal::CICalendarDuration duration = vtodo.GetEnd() - vtodo.GetStart();
		mDuration->SetDuration(duration, mAllDay->GetValue() == 1);
	}
}

void CNewToDoTiming::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do timing
	bool all_day = (mAllDay->GetValue() == 1);

	if (mNoDue->GetValue() == 1)
	{
		vtodo.EditTiming();
	}
	else if (mDueBy->GetValue() == 1)
	{
		iCal::CICalendarDateTime due;
		mDueDateTimeZone->GetDateTimeZone(due, all_day);
		vtodo.EditTiming(due);
	}
	else if (mStarts->GetValue() == 1)
	{
		iCal::CICalendarDateTime dtstart;
		mStartDateTimeZone->GetDateTimeZone(dtstart, all_day);
		
		iCal::CICalendarDuration duration;
		mDuration->GetDuration(duration, all_day);
		
		vtodo.EditTiming(dtstart, duration);
	}
}
