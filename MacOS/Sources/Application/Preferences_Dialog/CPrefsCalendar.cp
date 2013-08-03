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


// Source for CPrefsCalendar class

#include "CPrefsCalendar.h"

#include "CPreferences.h"

#include <LBevelButton.h>
#include <LCheckbox.h>
#include <LClock.h>
#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C P R E F S A D D R E S S
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPrefsCalendar::CPrefsCalendar()
{
}

// Constructor from stream
CPrefsCalendar::CPrefsCalendar(LStream *inStream)
		: CPrefsPanel(inStream)
{
}

// Default destructor
CPrefsCalendar::~CPrefsCalendar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CPrefsCalendar::FinishCreateSelf(void)
{
	// Do inherited
	CPrefsPanel::FinishCreateSelf();

	mWeekStart = (LPopupButton*) FindPaneByID(paneid_CalendarWeekStart);
	for(int i = 0; i < 7; i++)
		mWorkDay[i] = (LBevelButton*) FindPaneByID(paneid_CalendarWorkSunday + i);

	for(int i = 0; i < 5; i++)
	{
		mRangeStarts[i] = (LClock*) FindPaneByID(paneid_CalendarMorningStart + i);
		mRangeEnds[i] = (LClock*) FindPaneByID(paneid_CalendarMorningEnd + i);
	}

	mDisplayTime = (LCheckBox*) FindPaneByID(paneid_CalendarDisplayTime);
	mHandleICS = (LCheckBox*) FindPaneByID(paneid_CalendarHandleICS);
	mAutomaticIMIP = (LCheckBox*) FindPaneByID(paneid_AutomaticIMIP);
	mAutomaticEDST = (LCheckBox*) FindPaneByID(paneid_AutomaticEDST);
	mShowUID = (LCheckBox*) FindPaneByID(paneid_ShowUID);

	// Find super commander in view chain
	LView* cparent = GetSuperView();
	while((cparent != NULL) && (dynamic_cast<LDialogBox*>(cparent) == NULL))
		cparent = cparent->GetSuperView();
	LDialogBox* dlog = dynamic_cast<LDialogBox*>(cparent);

	mClockHandler.InstallMLTEEventHandler(GetWindowEventTarget(dlog->GetMacWindow()));
}

// Toggle display of IC items
void CPrefsCalendar::ToggleICDisplay(void)
{
}

// Set prefs
void CPrefsCalendar::SetPrefs(CPreferences* copyPrefs)
{
	// Save ref to prefs
	mCopyPrefs = copyPrefs;

	// Set all controls
	mWeekStart->SetValue(copyPrefs->mWeekStartDay.GetValue() + 1);
	
	for(int i = 0; i < 7; i++)
		mWorkDay[i]->SetValue((copyPrefs->mWorkDayMask.GetValue() & (1L << i)) != 0);

	for(int i = 0; i < 5; i++)
	{
		SetHours(mRangeStarts[i], copyPrefs->mDayWeekTimeRanges.GetValue().GetStart(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours());
		SetHours(mRangeEnds[i], copyPrefs->mDayWeekTimeRanges.GetValue().GetEnd(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours());
	}

	mDisplayTime->SetValue(copyPrefs->mDisplayTime.GetValue());
	mHandleICS->SetValue(copyPrefs->mHandleICS.GetValue());
	mAutomaticIMIP->SetValue(copyPrefs->mAutomaticIMIP.GetValue());
	mAutomaticEDST->SetValue(copyPrefs->mAutomaticEDST.GetValue());
	mShowUID->SetValue(copyPrefs->mShowUID.GetValue());
}

// Force update of prefs
void CPrefsCalendar::UpdatePrefs(void)
{
	// Get all controls
	mCopyPrefs->mWeekStartDay.SetValue(static_cast<iCal::CICalendarDateTime::EDayOfWeek>(mWeekStart->GetValue() - 1));
	
	long mask = 0L;
	for(int i = 0; i < 7; i++)
	{
		if (mWorkDay[i]->GetValue())
			mask |= (1L << i);
	}
	mCopyPrefs->mWorkDayMask.SetValue(mask);

	for(int i = 0; i < 5; i++)
	{
		// Check for change to existing
		uint32_t start = GetHours(mRangeStarts[i]);
		uint32_t end = GetHours(mRangeEnds[i]);
		if ((mCopyPrefs->mDayWeekTimeRanges.GetValue().GetStart(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours() != start) ||
			(mCopyPrefs->mDayWeekTimeRanges.GetValue().GetEnd(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours() != end))
		{
			mCopyPrefs->mDayWeekTimeRanges.SetDirty();
			
			iCal::CICalendarDateTime dtstart(2000, 1, 1, start, 0, 0);
			iCal::CICalendarDateTime dtend(2000, 1, 1, end, 0, 0);
			mCopyPrefs->mDayWeekTimeRanges.Value().SetRange(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i), dtstart, dtend);
		}
	}

	mCopyPrefs->mDisplayTime.SetValue(mDisplayTime->GetValue());
	mCopyPrefs->mHandleICS.SetValue(mHandleICS->GetValue());
	mCopyPrefs->mAutomaticIMIP.SetValue(mAutomaticIMIP->GetValue());
	mCopyPrefs->mAutomaticEDST.SetValue(mAutomaticEDST->GetValue());
	mCopyPrefs->mShowUID.SetValue(mShowUID->GetValue());
}

void CPrefsCalendar::SetHours(LClock* ctrl, uint32_t hours)
{
	LongDateRec dtr;
	::memset(&dtr, 0, sizeof(LongDateRec));
	dtr.ld.year = 2000;
	dtr.ld.month = 1;
	dtr.ld.day = 1;
	dtr.ld.hour = hours;
	dtr.ld.minute = 0;
	dtr.ld.second = 0;
	
	ctrl->SetLongDate(dtr);
}

uint32_t CPrefsCalendar::GetHours(LClock* ctrl)
{
	LongDateRec dtr;
	ctrl->GetLongDate(dtr);
	return dtr.ld.hour;
}
