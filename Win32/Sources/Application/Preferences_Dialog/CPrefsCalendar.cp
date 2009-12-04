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

#include "CPreferences.h"
#include "CMulberryCommon.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsCalendar dialog

IMPLEMENT_DYNCREATE(CPrefsCalendar, CPrefsPanel)

CPrefsCalendar::CPrefsCalendar() : CPrefsPanel(CPrefsCalendar::IDD), mWeekStartBtn(true)
{
	//{{AFX_DATA_INIT(CPrefsCalendar)
	//}}AFX_DATA_INIT
	
	mInitialised = false;
}

CPrefsCalendar::~CPrefsCalendar()
{
}

void CPrefsCalendar::DoDataExchange(CDataExchange* pDX)
{
	CPrefsPanel::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPrefsCalendar)
	DDX_Check(pDX, IDC_PREFSCALENDAR_DISPLAYTIMES, mDisplayTime);
	DDX_Check(pDX, IDC_PREFSCALENDAR_ATTACHMENTS, mHandleICS);
	DDX_Check(pDX, IDC_PREFSCALENDAR_AUTOMATICIMIP, mAutomaticIMIP);
	DDX_Check(pDX, IDC_PREFSCALENDAR_AUTOMATICEDST, mAutomaticEDST);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPrefsCalendar, CPrefsPanel)
	//{{AFX_MSG_MAP(CPrefsCalendar)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPrefsCalendar message handlers

void CPrefsCalendar::InitControls(void)
{
	// Subclass buttons
	mWeekStartBtn.SubclassDlgItem(IDC_PREFSCALENDAR_STARTWEEK, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mWeekStartBtn.SetMenu(IDR_POPUP_DAYS);
	
	for(int i = 0; i < 7; i++)
		mWorkDay[i].SubclassDlgItem(IDC_PREFSCALENDAR_SUN + i, this);

	for(int i = 0; i < 5; i++)
	{
		mRangeStarts[i].SubclassDlgItem(IDC_PREFSCALENDAR_MORNINGSTART + i, this);
		mRangeEnds[i].SubclassDlgItem(IDC_PREFSCALENDAR_MORNINGEND + i, this);
	}
}

void CPrefsCalendar::SetControls(void)
{
	// set initial control states
	mWeekStartBtn.SetValue(mCopyPrefs->mWeekStartDay.GetValue() + IDM_SUN);

	for(int i = 0; i < 7; i++)
		mWorkDay[i].SetPushed((mCopyPrefs->mWorkDayMask.GetValue() & (1L << i)) != 0);

	for(int i = 0; i < 5; i++)
	{
		SetHours(mRangeStarts[i], mCopyPrefs->mDayWeekTimeRanges.GetValue().GetStart(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours());
		SetHours(mRangeEnds[i], mCopyPrefs->mDayWeekTimeRanges.GetValue().GetEnd(static_cast<CDayWeekViewTimeRange::ERanges>(CDayWeekViewTimeRange::eMorning + i)).GetHours());
	}
	
	mInitialised = true;
}

// Set up params for DDX
void CPrefsCalendar::SetPrefs(CPreferences* prefs)
{
	mCopyPrefs = prefs;

	mDisplayTime = mCopyPrefs->mDisplayTime.GetValue();
	mHandleICS = mCopyPrefs->mHandleICS.GetValue();
	mAutomaticIMIP = mCopyPrefs->mAutomaticIMIP.GetValue();
	mAutomaticEDST = mCopyPrefs->mAutomaticEDST.GetValue();
}

// Get params from DDX
void CPrefsCalendar::UpdatePrefs(CPreferences* prefs)
{
	if (!mInitialised)
		return;

	// Get all controls
	mCopyPrefs->mWeekStartDay.SetValue(static_cast<iCal::CICalendarDateTime::EDayOfWeek>(mWeekStartBtn.GetValue() - IDM_SUN));
	
	long mask = 0L;
	for(int i = 0; i < 7; i++)
	{
		if (mWorkDay[i].IsPushed())
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

	mCopyPrefs->mDisplayTime.SetValue(mDisplayTime);
	mCopyPrefs->mHandleICS.SetValue(mHandleICS);
	mCopyPrefs->mAutomaticIMIP.SetValue(mAutomaticIMIP);
	mCopyPrefs->mAutomaticEDST.SetValue(mAutomaticEDST);
}

void CPrefsCalendar::SetHours(CDateControl& ctrl, uint32_t hours)
{
	SYSTEMTIME sys_time;
	sys_time.wYear = 2000;
	sys_time.wMonth = 1;
	sys_time.wDayOfWeek = 0;
	sys_time.wDay = 1;
	sys_time.wHour = hours;
	sys_time.wMinute = 0;
	sys_time.wSecond = 0;
	sys_time.wMilliseconds = 0;

	BOOL temp = DateTime_SetSystemtime(ctrl, GDT_VALID, &sys_time);
}

uint32_t CPrefsCalendar::GetHours(CDateControl& ctrl)
{
	SYSTEMTIME sys_time;
	BOOL temp = DateTime_GetSystemtime(ctrl, &sys_time);

	return sys_time.wHour;
}
