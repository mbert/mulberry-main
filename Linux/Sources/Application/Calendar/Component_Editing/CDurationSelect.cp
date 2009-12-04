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

#include "CDurationSelect.h"

#include "CNumberEdit.h"

#include "TPopupMenu.h"

#include "JXMultiImageButton.h"
#include <JXFlatRect.h>
#include <JXStaticText.h>

#include <cassert>

// ---------------------------------------------------------------------------
//	CDurationSelect														  [public]
/**
	Default constructor */

CDurationSelect::CDurationSelect(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h)
{
	mAllDay = false;
}


// ---------------------------------------------------------------------------
//	~CDurationSelect														  [public]
/**
	Destructor */

CDurationSelect::~CDurationSelect()
{
}

#pragma mark -

CDurationSelect* CDurationSelect::CreateInside(JXContainer* parent)
{
	CDurationSelect* result = new CDurationSelect(parent, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 100, 25);
	result->OnCreate();
	result->FitToEnclosure();
	
	return result;
}

void CDurationSelect::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mDurationNumber =
        new CNumberEdit(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 50,20);
    assert( mDurationNumber != NULL );

    mDurationPeriod =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 62,5, 85,20);
    assert( mDurationPeriod != NULL );

    mDurationTime =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,0, 250,30);
    assert( mDurationTime != NULL );

    JXStaticText* obj1 =
        new JXStaticText("and", mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,7, 30,14);
    assert( obj1 != NULL );

    mDurationHours =
        new CNumberEdit(mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,5, 45,20);
    assert( mDurationHours != NULL );

    JXStaticText* obj2 =
        new JXStaticText(":", mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,7, 10,14);
    assert( obj2 != NULL );

    mDurationMinutes =
        new CNumberEdit(mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 80,5, 45,20);
    assert( mDurationMinutes != NULL );

    JXStaticText* obj3 =
        new JXStaticText(":", mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,7, 10,14);
    assert( obj3 != NULL );

    mDurationSeconds =
        new CNumberEdit(mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,5, 45,20);
    assert( mDurationSeconds != NULL );

    JXStaticText* obj4 =
        new JXStaticText("HH:MM:SS", mDurationTime,
                    JXWidget::kHElastic, JXWidget::kVElastic, 180,7, 70,14);
    assert( obj4 != NULL );

// end JXLayout1

	mDurationNumber->OnCreate(0, 0x7FFFFFF);
	mDurationPeriod->SetMenuItems("Days %r | Months %r");
	mDurationPeriod->SetValue(eDurationPeriod_Days);
	mDurationHours->OnCreate(0, 23, 2);
	mDurationMinutes->OnCreate(0, 59, 2);
	mDurationSeconds->OnCreate(0, 59, 2);

	ListenTo(mDurationPeriod);
}

// Respond to clicks in the icon buttons
void CDurationSelect::Receive(JBroadcaster* sender, const Message& message)
{
	if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mDurationPeriod)
		{
			OnDurationPopup(is->GetIndex());
			return;
		}
	}
}

void CDurationSelect::OnDurationPopup(JIndex value)
{
	switch(value)
	{
	case eDurationPeriod_Days:
		mDurationTime->SetVisible(!mAllDay);
		break;
	case eDurationPeriod_Weeks:
	default:
		mDurationTime->SetVisible(false);
		break;
	}
}

void CDurationSelect::SetAllDay(bool all_day)
{
	mAllDay = all_day;
	mDurationTime->SetVisible(!all_day);
	
	// When going to all day, make sure duration is not zero
	if (all_day)
	{
		long number = mDurationNumber->GetNumberValue();
		if (number == 0)
			mDurationNumber->SetNumberValue(1);
	}
}

void CDurationSelect::SetDuration(const iCal::CICalendarDuration& du, bool all_day)
{
	SetAllDay(all_day);
	
	if (du.GetWeeks() > 0)
	{
		mDurationPeriod->SetValue(eDurationPeriod_Weeks);
		mDurationNumber->SetNumberValue(du.GetWeeks());
	}
	else
	{
		mDurationPeriod->SetValue(eDurationPeriod_Days);
		mDurationNumber->SetNumberValue(du.GetDays());
		mDurationHours->SetNumberValue(du.GetHours());
		mDurationMinutes->SetNumberValue(du.GetMinutes());
		mDurationSeconds->SetNumberValue(du.GetSeconds());
	}
}

void CDurationSelect::GetDuration(iCal::CICalendarDuration& du, bool all_day)
{
	int64_t seconds = 0;
	if (mDurationPeriod->GetValue() == eDurationPeriod_Weeks)
	{
		seconds = mDurationNumber->GetNumberValue() * 7LL * 24LL * 60LL * 60LL;
		du.SetDuration(seconds);
	}
	else
	{
		seconds = mDurationNumber->GetNumberValue() * 24LL * 60LL * 60LL;
		if (!all_day)
		{
			seconds += mDurationHours->GetNumberValue() * 60LL * 60LL;
			seconds += mDurationMinutes->GetNumberValue() * 60LL;
			seconds += mDurationSeconds->GetNumberValue();
		}
	}
	du.SetDuration(seconds);
}

void CDurationSelect::SetReadOnly(bool read_only)
{
	mDurationNumber->SetReadOnly(read_only);
	mDurationPeriod->SetActive(!read_only);
	mDurationHours->SetReadOnly(read_only);
	mDurationMinutes->SetReadOnly(read_only);
	mDurationSeconds->SetReadOnly(read_only);
}
