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


// CPrefsCalendar.cpp : implementation file
//

#include "CPrefsCalendar.h"

#include "CDateTimeControl.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include "JXSecondaryRadioGroup.h"
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include "JXTextPushButton.h"

#include <cassert>

/////////////////////////////////////////////////////////////////////////////
// CPrefsCalendar dialog

CPrefsCalendar::CPrefsCalendar( JXContainer* enclosure,
														const HSizingOption hSizing, 
														const VSizingOption vSizing,
														const JCoordinate x, const JCoordinate y,
														const JCoordinate w, const JCoordinate h) 
	:CPrefsPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
}

/////////////////////////////////////////////////////////////////////////////
// CPrefsCalendar message handlers

void CPrefsCalendar::OnCreate()
{
// begin JXLayout1

    JXStaticText* obj1 =
        new JXStaticText("Start Week on:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,30, 95,20);
    assert( obj1 != NULL );

    mWeekStartBtn =
        new HPopupMenu("",this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 125,25, 195,25);
    assert( mWeekStartBtn != NULL );

    JXRadioGroup* obj2 =
        new JXRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,70, 360,50);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Work Days are:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,60, 100,20);
    assert( obj3 != NULL );

    mWorkDay0 =
        new JXTextPushButton("Sun", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,15, 50,25);
    assert( mWorkDay0 != NULL );

    mWorkDay1 =
        new JXTextPushButton("Mon", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,15, 50,25);
    assert( mWorkDay1 != NULL );

    mWorkDay2 =
        new JXTextPushButton("Tue", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 105,15, 50,25);
    assert( mWorkDay2 != NULL );

    mWorkDay3 =
        new JXTextPushButton("Wed", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 155,15, 50,25);
    assert( mWorkDay3 != NULL );

    mWorkDay4 =
        new JXTextPushButton("Thu", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 205,15, 50,25);
    assert( mWorkDay4 != NULL );

    mWorkDay5 =
        new JXTextPushButton("Fri", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 255,15, 50,25);
    assert( mWorkDay5 != NULL );

    mWorkDay6 =
        new JXTextPushButton("Sat", obj2,
                    JXWidget::kHElastic, JXWidget::kVElastic, 305,15, 50,25);
    assert( mWorkDay6 != NULL );

    JXSecondaryRadioGroup* obj4 =
        new JXSecondaryRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,140, 360,170);
    assert( obj4 != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Preset Hour Ranges for Day/Week Displays:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 25,130, 265,20);
    assert( obj5 != NULL );

    JXStaticText* obj6 =
        new JXStaticText("Start on", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 140,10, 60,20);
    assert( obj6 != NULL );

    JXStaticText* obj7 =
        new JXStaticText("End at", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,10, 60,20);
    assert( obj7 != NULL );

    JXStaticText* obj8 =
        new JXStaticText("Morning:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,35, 70,20);
    assert( obj8 != NULL );

    JXStaticText* obj9 =
        new JXStaticText("Afternoon:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,60, 70,20);
    assert( obj9 != NULL );

    JXStaticText* obj10 =
        new JXStaticText("Evening:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,85, 70,20);
    assert( obj10 != NULL );

    JXStaticText* obj11 =
        new JXStaticText("Awake Period:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,110, 90,20);
    assert( obj11 != NULL );

    JXStaticText* obj12 =
        new JXStaticText("Work Period:", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,135, 90,20);
    assert( obj12 != NULL );

    mHandleICS =
        new JXTextCheckbox("Handle text/calendar Attachments Internally", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,336, 300,20);
    assert( mHandleICS != NULL );

    mAutomaticIMIP =
        new JXTextCheckbox("Automatically Send Email Invitations and RSVPs", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,355, 310,20);
    assert( mAutomaticIMIP != NULL );

    mDisplayTime =
        new JXTextCheckbox("Display Times in Event Labels", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,316, 300,20);
    assert( mDisplayTime != NULL );

    JXStaticText* obj13 =
        new JXStaticText("Calendar Preferences", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 140,20);
    assert( obj13 != NULL );
    const JFontStyle obj13_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj13->SetFontStyle(obj13_style);

    mDT1 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,33, 90,24);
    assert( mDT1 != NULL );

    mDT2 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,33, 90,24);
    assert( mDT2 != NULL );

    mDT3 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,58, 90,24);
    assert( mDT3 != NULL );

    mDT4 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,58, 90,24);
    assert( mDT4 != NULL );

    mDT5 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,83, 90,24);
    assert( mDT5 != NULL );

    mDT6 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,83, 90,24);
    assert( mDT6 != NULL );

    mDT7 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,108, 90,24);
    assert( mDT7 != NULL );

    mDT8 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,108, 90,24);
    assert( mDT8 != NULL );

    mDT9 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,133, 90,24);
    assert( mDT9 != NULL );

    mDT10 =
        new CDateTimeControl(obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 235,133, 90,24);
    assert( mDT10 != NULL );

    mAutomaticEDST =
        new JXTextCheckbox("Automatically Update Timezones for 2007 Changes", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 20,375, 320,20);
    assert( mAutomaticEDST != NULL );

// end JXLayout1

	mWorkDay[0] = mWorkDay0;
	mWorkDay[1] = mWorkDay1;
	mWorkDay[2] = mWorkDay2;
	mWorkDay[3] = mWorkDay3;
	mWorkDay[4] = mWorkDay4;
	mWorkDay[5] = mWorkDay5;
	mWorkDay[6] = mWorkDay6;

	// Init the popup - noting missing plugins
	mWeekStartBtn->SetMenuItems("Sunday %r | Monday %r | Tuesday %r | Wednesday %r | Thursday %r | Friday %r | Saturday %r");
	mWeekStartBtn->SetUpdateAction(JXMenu::kDisableNone);
	mWeekStartBtn->SetToPopupChoice(kTrue, 1);
	
	mRangeStarts[0] = mDT1;
	mRangeStarts[1] = mDT3;
	mRangeStarts[2] = mDT5;
	mRangeStarts[3] = mDT7;
	mRangeStarts[4] = mDT9;
	
	mRangeEnds[0] = mDT2;
	mRangeEnds[1] = mDT4;
	mRangeEnds[2] = mDT6;
	mRangeEnds[3] = mDT8;
	mRangeEnds[4] = mDT10;
	
	for(int i = 0; i < 5; i++)
	{
		mRangeStarts[i]->OnCreate(CDateTimeControl::eTimeNoSecs);
		mRangeEnds[i]->OnCreate(CDateTimeControl::eTimeNoSecs);
	}
}

// Set up params for DDX
void CPrefsCalendar::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mWeekStartBtn->SetValue(mCopyPrefs->mWeekStartDay.GetValue() + 1);

	for(int i = 0; i < 7; i++)
		mWorkDay[i]->SetState(JBoolean((mCopyPrefs->mWorkDayMask.GetValue() & (1L << i)) != 0));

	for(int i = 0; i < 5; i++)
	{
		SetHours(mRangeStarts[i], mCopyPrefs->mDayWeekTimeRanges.GetValue().GetStart(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours());
		SetHours(mRangeEnds[i], mCopyPrefs->mDayWeekTimeRanges.GetValue().GetEnd(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours());
	}

	mHandleICS->SetState(JBoolean(prefs->mHandleICS.GetValue()));
	mAutomaticIMIP->SetState(JBoolean(prefs->mAutomaticIMIP.GetValue()));
	mAutomaticEDST->SetState(JBoolean(prefs->mAutomaticEDST.GetValue()));
	mDisplayTime->SetState(JBoolean(prefs->mDisplayTime.GetValue()));
}

// Get params from DDX
void CPrefsCalendar::UpdatePrefs(CPreferences* prefs)
{
	// Get all controls
	prefs->mWeekStartDay.SetValue(static_cast<iCal::CICalendarDateTime::EDayOfWeek>(mWeekStartBtn->GetValue() - 1));
	
	long mask = 0L;
	for(int i = 0; i < 7; i++)
	{
		if (mWorkDay[i]->IsChecked())
			mask |= (1L << i);
	}
	prefs->mWorkDayMask.SetValue(mask);

	for(int i = 0; i < 5; i++)
	{
		// Check for change to existing
		long start = GetHours(mRangeStarts[i]);
		long end = GetHours(mRangeEnds[i]);
		if ((mCopyPrefs->mDayWeekTimeRanges.GetValue().GetStart(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours() != start) ||
			(mCopyPrefs->mDayWeekTimeRanges.GetValue().GetEnd(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours() != end))
		{
			mCopyPrefs->mDayWeekTimeRanges.SetDirty();
			
			iCal::CICalendarDateTime dtstart(2000, 1, 1, start, 0, 0);
			iCal::CICalendarDateTime dtend(2000, 1, 1, end, 0, 0);
			mCopyPrefs->mDayWeekTimeRanges.Value().SetRange(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i), dtstart, dtend);
		}
	}

	prefs->mHandleICS.SetValue(mHandleICS->IsChecked());
	prefs->mAutomaticIMIP.SetValue(mAutomaticIMIP->IsChecked());
	prefs->mAutomaticEDST.SetValue(mAutomaticEDST->IsChecked());
	prefs->mDisplayTime.SetValue(mDisplayTime->IsChecked());
}

void CPrefsCalendar::SetHours(CDateTimeControl* ctrl, unsigned long hours)
{
	ctrl->SetTime(hours, 0, 0);
}

unsigned long CPrefsCalendar::GetHours(CDateTimeControl* ctrl)
{
	unsigned long hours;
	unsigned long mins;
	unsigned long secs;
	ctrl->GetTime(hours, mins, secs);
	return hours;
}
