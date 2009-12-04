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

#include "CDateTimeZoneSelect.h"

#include "CChooseDateDialog.h"
#include "CDateTimeControl.h"
#include "CTimezonePopup.h"

#include "CICalendarManager.h"

#include "JXMultiImageButton.h"

#include <cassert>

// ---------------------------------------------------------------------------
//	CDateTimeZoneSelect														  [public]
/**
	Default constructor */

CDateTimeZoneSelect::CDateTimeZoneSelect(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mAllDay = false;
}


// ---------------------------------------------------------------------------
//	~CDateTimeZoneSelect														  [public]
/**
	Destructor */

CDateTimeZoneSelect::~CDateTimeZoneSelect()
{
}

#pragma mark -

CDateTimeZoneSelect* CDateTimeZoneSelect::CreateInside(JXContainer* parent)
{
	CDateTimeZoneSelect* result = new CDateTimeZoneSelect(parent, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 100, 25);
	result->OnCreate();
	result->FitToEnclosure();
	
	return result;
}

void CDateTimeZoneSelect::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mDate =
        new CDateTimeControl(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 100,20);
    assert( mDate != NULL );

    mDateBtn =
        new JXMultiImageButton(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,5, 20,20);
    assert( mDateBtn != NULL );

    mTime =
        new CDateTimeControl(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 140,5, 105,20);
    assert( mTime != NULL );

    mZone =
        new CTimezonePopup("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,5, 140,20);
    assert( mZone != NULL );

// end JXLayout1

	mDate->OnCreate(CDateTimeControl::eDate);
	mDateBtn->SetImage(IDI_CALENDAR);
	mTime->OnCreate(CDateTimeControl::eTimeSecs);
	mZone->OnCreate();

	ListenTo(mDateBtn);
}

// Handle OK button
void CDateTimeZoneSelect::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mDateBtn)
			OnDateBtn();
	}
}

void CDateTimeZoneSelect::SetAllDay(bool all_day)
{
	mAllDay = all_day;
	mTime->SetVisible(!all_day);
	mZone->SetVisible(!all_day);
}

void CDateTimeZoneSelect::SetDateTimeZone(const iCal::CICalendarDateTime& dt, bool all_day)
{
	SetAllDay(all_day);
	
	mDate->SetDate(dt.GetYear(), dt.GetMonth(), dt.GetDay());
	mTime->SetTime(dt.GetHours(), dt.GetMinutes(), dt.GetSeconds());
	
	if (all_day)
		mZone->SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	else
		mZone->SetTimezone(dt.GetTimezone());
}

void CDateTimeZoneSelect::GetDateTimeZone(iCal::CICalendarDateTime& dt, bool all_day)
{
	dt.SetDateOnly(all_day);

	// Get date
	{
		unsigned long year;
		unsigned long month;
		unsigned long day;
		mDate->GetDate(year, month, day);
		
		dt.SetYear(year);
		dt.SetMonth(month);
		dt.SetDay(day);
	}
	
	// Get time & zone if required
	if (!all_day)
	{
		unsigned long hour;
		unsigned long minute;
		unsigned long second;
		mTime->GetTime(hour, minute, second);
		
		dt.SetHours(hour);
		dt.SetMinutes(minute);
		dt.SetSeconds(second);
		
		// Get timezone
		mZone->GetTimezone(dt.GetTimezone());
	}
}

void CDateTimeZoneSelect::SetReadOnly(bool read_only)
{
	mDate->SetActive(!read_only);
	mDateBtn->SetActive(!read_only);
	mTime->SetActive(!read_only);
	mZone->SetActive(!read_only);
}

void CDateTimeZoneSelect::OnDateBtn()
{
	// Get current date from controls
	iCal::CICalendarDateTime dt;
	GetDateTimeZone(dt, mAllDay);
	
	// Now do date chooser dialog
	if (CChooseDateDialog::PoseDialog(dt))
	{
		// Set the new date/time
		SetDateTimeZone(dt, mAllDay);
	}
}
