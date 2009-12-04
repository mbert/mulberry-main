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

#include "CMulberryCommon.h"

// ---------------------------------------------------------------------------
//	CDurationSelect														  [public]
/**
	Default constructor */

CDurationSelect::CDurationSelect() :
	CTabPanel(IDD_CALENDAR_DURATIONSELECT),
	mPeriod(true)
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

BEGIN_MESSAGE_MAP(CDurationSelect, CTabPanel)
	//{{AFX_MSG_MAP(CDurationSelect)
	ON_WM_ENABLE()

	ON_COMMAND(IDC_CALENDAR_DURATIONSELECT_PERIOD, OnDurationPopup)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDurationSelect::OnInitDialog()
{
	CTabPanel::OnInitDialog();

		// Get UI items
	mNumber.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_NUMBER, this);
	mNumberSpin.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_NUMBERSPIN, this);
	mNumberSpin.SetRange(0, 1000);
	mPeriod.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_PERIOD, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mPeriod.SetMenu(IDR_DURATION_PERIOD_POPUP);
	mPeriod.SetValue(IDM_DURATION_PERIOD_DAYS);

	mCaption1.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_CAPTION1, this);

	mHours.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_HOURS, this);
	CRect wr;
	mHours.GetWindowRect(wr);
	mHoursSpin.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_HOURSSPIN, this);
	mHoursSpin.SetRange(0, 23);
	mHours.GetWindowRect(wr);

	mCaption2.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_CAPTION2, this);

	mMinutes.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_MINS, this);
	mMinutesSpin.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_MINSSPIN, this);
	mMinutesSpin.SetRange(0, 59);

	mCaption3.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_CAPTION3, this);

	mSeconds.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_SECS, this);
	mSecondsSpin.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_SECSSPIN, this);
	mSecondsSpin.SetRange(0, 59);

	mCaption4.SubclassDlgItem(IDC_CALENDAR_DURATIONSELECT_CAPTION4, this);

	return true;
}

void CDurationSelect::OnEnable(BOOL bEnable)
{
	mNumber.EnableWindow(bEnable);
	mNumberSpin.EnableWindow(bEnable);
	mPeriod.EnableWindow(bEnable);

	mHours.EnableWindow(bEnable);
	mHoursSpin.EnableWindow(bEnable);

	mMinutes.EnableWindow(bEnable);
	mMinutesSpin.EnableWindow(bEnable);

	mSeconds.EnableWindow(bEnable);
	mSecondsSpin.EnableWindow(bEnable);
}

void CDurationSelect::OnDurationPopup()
{
	if (mPeriod.GetValue() == IDM_DURATION_PERIOD_DAYS)
	{
		ShowTime(!mAllDay);
	}
	else if (mPeriod.GetValue() == IDM_DUARTION_PERIOD_WEEKS)
	{
		ShowTime(false);
	}
}

void CDurationSelect::SetAllDay(bool all_day)
{
	mAllDay = all_day;
	ShowTime(!all_day);
	
	// When going to all day, make sure duration is not zero
	if (all_day)
	{
		SInt32 number = mNumber.GetValue();
		if (number == 0)
			mNumber.SetValue(1);
	}
}

void CDurationSelect::SetDuration(const iCal::CICalendarDuration& du, bool all_day)
{
	SetAllDay(all_day);
	
	if (du.GetWeeks() > 0)
	{
		mPeriod.SetValue(IDM_DUARTION_PERIOD_WEEKS);
		mNumber.SetValue(du.GetWeeks());
	}
	else
	{
		mPeriod.SetValue(IDM_DURATION_PERIOD_DAYS);
		mNumber.SetValue(du.GetDays());
		mHours.SetValue(du.GetHours());
		mMinutes.SetValue(du.GetMinutes());
		mSeconds.SetValue(du.GetSeconds());
	}
}

void CDurationSelect::GetDuration(iCal::CICalendarDuration& du, bool all_day)
{
	int64_t seconds = 0;
	if (mPeriod.GetValue() == IDM_DUARTION_PERIOD_WEEKS)
	{
		seconds = mNumber.GetValue() * 7LL * 24LL * 60LL * 60LL;
		du.SetDuration(seconds);
	}
	else
	{
		seconds = mNumber.GetValue() * 24LL * 60LL * 60LL;
		if (!all_day)
		{
			seconds += mHours.GetValue() * 60LL * 60LL;
			seconds += mMinutes.GetValue() * 60LL;
			seconds += mSeconds.GetValue();
		}
	}
	du.SetDuration(seconds);
}

void CDurationSelect::SetReadOnly(bool read_only)
{
	mNumber.SetReadOnly(read_only);
	mNumberSpin.EnableWindow(!read_only);
	mPeriod.EnableWindow(!read_only);
	mHours.SetReadOnly(read_only);
	mHoursSpin.EnableWindow(!read_only);
	mMinutes.SetReadOnly(read_only);
	mMinutesSpin.EnableWindow(!read_only);
	mSeconds.SetReadOnly(read_only);
	mSecondsSpin.EnableWindow(!read_only);
}

void CDurationSelect::ShowTime(bool show)
{
	mCaption1.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mHours.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mHoursSpin.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mCaption2.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mMinutes.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mMinutesSpin.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mCaption3.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mSeconds.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mSecondsSpin.ShowWindow(show ? SW_SHOW : SW_HIDE);
	mCaption4.ShowWindow(show ? SW_SHOW : SW_HIDE);
}
