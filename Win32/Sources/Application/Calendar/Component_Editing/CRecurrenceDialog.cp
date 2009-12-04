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

#include "CRecurrenceDialog.h"

#include "CDateTimeZoneSelect.h"
#include "CICalendar.h"
#include "CSDIFrame.h"

// ---------------------------------------------------------------------------
//	CRecurrenceDialog														  [public]
/**
	Default constructor */

CRecurrenceDialog::CRecurrenceDialog(CWnd* pParent /*=NULL*/) :
	CHelpDialog(CRecurrenceDialog::IDD, pParent),
	mOccursFreq(true), mByDayPopup(true), mBySetPosPopup(true)
{
}


// ---------------------------------------------------------------------------
//	~CRecurrenceDialog														  [public]
/**
	Destructor */

CRecurrenceDialog::~CRecurrenceDialog()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CRecurrenceDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CRecurrenceDialog)
	ON_COMMAND(IDC_RECURRENCE_FREQ, OnOccursFreq)
	ON_COMMAND(IDC_RECURRENCE_FOREVER, OnOccursForEver)
	ON_COMMAND(IDC_RECURRENCE_COUNT, OnOccursCount)
	ON_COMMAND(IDC_RECURRENCE_UNTIL, OnOccursUntil)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CRecurrenceDialog::InitControls()
{
	// Get UI items
	mOccursInterval.SubclassDlgItem(IDC_RECURRENCE_INTERVAL, this);
	mOccursInterval.SetValue(1);
	mOccursIntervalSpin.SubclassDlgItem(IDC_RECURRENCE_INTERVALSPIN, this);
	mOccursIntervalSpin.SetRange(1, 1000);
	mOccursFreq.SubclassDlgItem(IDC_RECURRENCE_FREQ, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mOccursFreq.SetMenu(IDR_CALENDAR_REPEAT_FREQ);
	mOccursFreq.SetValue(IDM_CALENDAR_REPEAT_FREQ_YEARS);

	mOccursForEver.SubclassDlgItem(IDC_RECURRENCE_FOREVER, this);
	mOccursCount.SubclassDlgItem(IDC_RECURRENCE_COUNT, this);
	mOccursUntil.SubclassDlgItem(IDC_RECURRENCE_UNTIL, this);
	mOccursForEver.SetCheck(1);

	mOccursCounter.SubclassDlgItem(IDC_RECURRENCE_COUNTER, this);
	mOccursCounter.SetValue(1);
	mOccursCounterSpin.SubclassDlgItem(IDC_RECURRENCE_COUNTERSPIN, this);
	mOccursCounterSpin.SetRange(1, 1000);

	mOccursDateTimeZoneItem.SubclassDlgItem(IDC_RECURRENCE_DATETZ, this);
	mOccursDateTimeZone = new CDateTimeZoneSelect;
	mOccursDateTimeZoneItem.AddPanel(mOccursDateTimeZone);
	mOccursDateTimeZoneItem.SetPanel(0);

	for(int i = 0; i < 12; i++)
		mByMonth[i].SubclassDlgItem(IDC_RECURRENCE_JAN + i, this);

	for(int i = 0; i < 31; i++)
		mByMonthDay[i].SubclassDlgItem(IDC_RECURRENCE_DAY1 + i, this);

	for(int i = 0; i < 7; i++)
		mByMonthDayLast[i].SubclassDlgItem(IDC_RECURRENCE_DAYM1 + i, this);

	for(int i = 0; i < 7; i++)
		mByDay[i].SubclassDlgItem(IDC_RECURRENCE_SUN + i, this);

	mByDayPopup.SubclassDlgItem(IDC_RECURRENCE_OCCURS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mByDayPopup.SetMenu(IDR_POPUP_BYDAY);
	mByDayPopup.SetValue(IDM_BYDAY_ANYTIME);
	mByDayCaption.SubclassDlgItem(IDC_RECURRENCE_OCCURSCAPTION, this);

	mBySetPosPopup.SubclassDlgItem(IDC_RECURRENCE_BYSETPOS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mBySetPosPopup.SetMenu(IDR_POPUP_BYSETPOS);
	mBySetPosPopup.SetValue(IDM_BYSETPOS_ALL);

	DoOccursGroup(eOccurs_ForEver);
}

void CRecurrenceDialog::OnOccursFreq()
{
	DoOccursFreq(mOccursFreq.GetValue() - IDM_CALENDAR_REPEAT_FREQ_YEARS);
}

void CRecurrenceDialog::OnOccursForEver()
{
	DoOccursGroup(eOccurs_ForEver);
}

void CRecurrenceDialog::OnOccursCount()
{
	DoOccursGroup(eOccurs_Count);
}

void CRecurrenceDialog::OnOccursUntil()
{
	DoOccursGroup(eOccurs_Until);
}

void CRecurrenceDialog::DoOccursFreq(UInt32 value)
{
	// Hide the relative day in month controls if not Yearly or Monthly
	switch(value)
	{
	case eOccurs_Yearly:
	case eOccurs_Monthly:
		mByDayPopup.ShowWindow(SW_SHOW);
		mByDayCaption.ShowWindow(SW_SHOW);
		break;
	default:
		mByDayPopup.ShowWindow(SW_HIDE);
		mByDayCaption.ShowWindow(SW_HIDE);
		break;
	}
}

void CRecurrenceDialog::DoOccursGroup(UInt32 value)
{
	mOccursCounter.EnableWindow(value == eOccurs_Count);
	mOccursCounterSpin.EnableWindow(value == eOccurs_Count);

	mOccursDateTimeZone->EnableWindow(value == eOccurs_Until);
}

void CRecurrenceDialog::SetRecurrence(const iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day)
{
	static const int cFreqValueToPopup[] =
	{
		CRecurrenceDialog::eOccurs_Secondly, CRecurrenceDialog::eOccurs_Minutely, CRecurrenceDialog::eOccurs_Hourly,
		CRecurrenceDialog::eOccurs_Daily, CRecurrenceDialog::eOccurs_Weekly, CRecurrenceDialog::eOccurs_Monthly, CRecurrenceDialog::eOccurs_Yearly
	};

	// Set frequency
	mOccursFreq.SetValue(cFreqValueToPopup[recur.GetFreq()] + IDM_CALENDAR_REPEAT_FREQ_YEARS);
	
	// Set interval
	mOccursInterval.SetValue(recur.GetInterval());
	
	// Set count
	if (recur.GetUseCount())
	{
		mOccursForEver.SetCheck(0);
		mOccursCount.SetCheck(1);
		mOccursUntil.SetCheck(0);
		DoOccursGroup(eOccurs_Count);
		mOccursCounter.SetValue(recur.GetCount());
	}
	else if (recur.GetUseUntil())
	{
		mOccursForEver.SetCheck(0);
		mOccursCount.SetCheck(0);
		mOccursUntil.SetCheck(1);
		DoOccursGroup(eOccurs_Until);
		
		// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time
		
		// Adjust UNTIL to new timezone
		iCal::CICalendarDateTime until(recur.GetUntil());
		until.AdjustTimezone(tzid);

		mOccursDateTimeZone->SetDateTimeZone(until, all_day);
	}
	else
	{
		mOccursForEver.SetCheck(1);
		mOccursCount.SetCheck(0);
		mOccursUntil.SetCheck(0);
		DoOccursGroup(eOccurs_ForEver);
	}
	
	// Set BYxxx items
	SetByMonth(recur);
	SetByMonthDay(recur);
	SetByDay(recur);
	SetBySetPos(recur);
}

void CRecurrenceDialog::SetByMonth(const iCal::CICalendarRecurrence& recur)
{
	// Unset all buttons first
	for(uint32_t i = 0; i < 12; i++)
		mByMonth[i].SetPushed(0);

	const vector<int32_t>& by_month = recur.GetByMonth();
	for(vector<int32_t>::const_iterator iter = by_month.begin(); iter != by_month.end(); iter++)
	{
		if ((*iter >= 1) && (*iter <= 12))
			mByMonth[*iter - 1].SetPushed(1);
	}
}

void CRecurrenceDialog::SetByMonthDay(const iCal::CICalendarRecurrence& recur)
{
	// Unset all buttons first
	for(uint32_t i = 0; i < 31; i++)
		mByMonthDay[i].SetPushed(0);
	for(uint32_t i = 0; i < 7; i++)
		mByMonthDayLast[i].SetPushed(0);

	const vector<int32_t>& by_month_day = recur.GetByMonthDay();
	for(vector<int32_t>::const_iterator iter = by_month_day.begin(); iter != by_month_day.end(); iter++)
	{
		// NB Cannot handle all negative values, or zero
		if ((*iter >= 1) && (*iter <= 31))
			mByMonthDay[*iter - 1].SetPushed(1);
		else if ((*iter >= -7) && (*iter <= -1))
			mByMonthDayLast[-(*iter) - 1].SetPushed(1);
	}
}

void CRecurrenceDialog::SetByDay(const iCal::CICalendarRecurrence& recur)
{
	static const int32_t cByDayNumToPopup[] =
	{
		eByDay_2ndLast, eByDay_1stLast, eByDay_Any, eByDay_1st, eByDay_2nd, eByDay_3rd, eByDay_4th
	};

	// Unset all buttons first
	for(uint32_t i = 0; i < 7; i++)
		mByDay[i].SetPushed(0);

	// Set the day buttons
	const vector<iCal::CICalendarRecurrence::CWeekDayNum>& by_day = recur.GetByDay();
	for(vector<iCal::CICalendarRecurrence::CWeekDayNum>::const_iterator iter = by_day.begin(); iter != by_day.end(); iter++)
	{
		if (((*iter).second >= 0) && ((*iter).second <= 6))
			mByDay[(*iter).second].SetPushed(1);
	}
	
	// Set the popup
	if (by_day.size())
	{
		// Validate range before setting
		int32_t number = by_day.front().first;
		if ((number >= -2) && (number <= 4))
		{
			mByDayPopup.SetValue(cByDayNumToPopup[number + 2] + IDM_BYDAY_ANYTIME);
		}
		else
			mByDayPopup.SetValue(IDM_BYDAY_ANYTIME);
	}
}

void CRecurrenceDialog::SetBySetPos(const iCal::CICalendarRecurrence& recur)
{
	// Can only have empty BYSETPOS or single +1 or -1
	mBySetPosPopup.SetValue(IDM_BYSETPOS_ALL);
	
	// Set the day buttons
	const vector<int32_t>& by_setpos = recur.GetBySetPos();
	if ((by_setpos.size() == 1) && (by_setpos[0] == 1))
		mBySetPosPopup.SetValue(IDM_BYSETPOS_FIRST);
	else if ((by_setpos.size() == 1) && (by_setpos[0] == -1))
		mBySetPosPopup.SetValue(IDM_BYSETPOS_LAST);
}

static const iCal::ERecurrence_FREQ cFreqPopupToValue[] = 
{
	iCal::eRecurrence_YEARLY, iCal::eRecurrence_MONTHLY, iCal::eRecurrence_WEEKLY, iCal::eRecurrence_DAILY,
	iCal::eRecurrence_HOURLY, iCal::eRecurrence_MINUTELY, iCal::eRecurrence_SECONDLY
};

void CRecurrenceDialog::GetRecurrence(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day)
{
	// Create a temp item first (this will have all fields cleared)
	iCal::CICalendarRecurrence temp;
	
	// Set frequency
	temp.SetFreq(cFreqPopupToValue[mOccursFreq.GetValue() - IDM_CALENDAR_REPEAT_FREQ_YEARS]);
	
	// Set interval
	temp.SetInterval(mOccursInterval.GetValue());
	
	// Determine end
	if (mOccursForEver.GetCheck() == 1)
	{
		// Nothing to add
	}
	else if (mOccursCount.GetCheck() == 1)
	{
		temp.SetUseCount(true);
		temp.SetCount(mOccursCounter.GetValue());
	}
	else if (mOccursUntil.GetCheck() == 1)
	{
		// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time,
		// so we must convert from dialog tzid to UTC
		
		// Get value from dialog
		iCal::CICalendarDateTime until;
		mOccursDateTimeZone->GetDateTimeZone(until, all_day);
		
		// Adjust to UTC
		until.AdjustToUTC();

		temp.SetUseUntil(true);
		temp.SetUntil(until);
	}
	
	// Get BYxxx items
	GetByMonth(temp);
	GetByMonthDay(temp);
	GetByDay(temp);
	GetBySetPos(temp);

	// Now set the actual recurrence
	recur = temp;
}

void CRecurrenceDialog::GetByMonth(iCal::CICalendarRecurrence& recur)
{
	// Get each button
	vector<int32_t> by_month;
	for(uint32_t i = 0; i < 12; i++)
	{
		if (mByMonth[i].IsPushed())
			by_month.push_back(i + 1);
	}
	recur.SetByMonth(by_month);
}

void CRecurrenceDialog::GetByMonthDay(iCal::CICalendarRecurrence& recur)
{
	// Get each button
	vector<int32_t> by_month_day;
	for(uint32_t i = 0; i < 31; i++)
	{
		if (mByMonthDay[i].IsPushed() == 1)
			by_month_day.push_back(i + 1);
	}
	for(uint32_t i = 0; i < 7; i++)
	{
		if (mByMonthDayLast[i].IsPushed() == 1)
			by_month_day.push_back(-1*(i + 1));
	}
	recur.SetByMonthDay(by_month_day);
}

static const int32_t cPopupToByDayNum[] =
{
	0, 0, 1, 2, 3, 4, -2, -1
};

void CRecurrenceDialog::GetByDay(iCal::CICalendarRecurrence& recur)
{
	// Get the popup number
	int32_t number = cPopupToByDayNum[mByDayPopup.GetValue() - IDM_BYDAY_ANYTIME];
	
	// Disable number for some frequencies
	switch(mOccursFreq.GetValue() - IDM_CALENDAR_REPEAT_FREQ_YEARS)
	{
	case eOccurs_Yearly:
	case eOccurs_Monthly:
		break;
	default:
		number = 0;
		break;
	}

	// Get each button
	vector<iCal::CICalendarRecurrence::CWeekDayNum> by_day;
	for(uint32_t i = 0; i < 7; i++)
	{
		if (mByDay[i].IsPushed() == 1)
			by_day.push_back(iCal::CICalendarRecurrence::CWeekDayNum(number, static_cast<iCal::CICalendarDateTime::EDayOfWeek>(i)));
	}
	recur.SetByDay(by_day);

}

void CRecurrenceDialog::GetBySetPos(iCal::CICalendarRecurrence& recur)
{
	// Can only have empty BYSETPOS or single +1 or -1
	vector<int32_t> by_setpos;
	switch(mBySetPosPopup.GetValue() - IDM_BYSETPOS_ALL)
	{
	case eBySetPos_All:
	default:
		// Leave it empty
		break;
	case eBySetPos_1st:
		by_setpos.push_back(1);
		break;
	case eBySetPos_Last:
		by_setpos.push_back(-1);
		break;
	}
	recur.SetBySetPos(by_setpos);
}

void CRecurrenceDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCreateCalendarDialog)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		GetRecurrence(*mRecur, *mTzid, mAllDay);
	}
	else
	{
		InitControls();
		SetRecurrence(*mRecur, *mTzid, mAllDay);
	}
}

bool CRecurrenceDialog::PoseDialog(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day)
{
	bool result = false;

	CRecurrenceDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mRecur = &recur;
	dlog.mTzid = &tzid;
	dlog.mAllDay = all_day;

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		result = true;
	}

	return result;
}
