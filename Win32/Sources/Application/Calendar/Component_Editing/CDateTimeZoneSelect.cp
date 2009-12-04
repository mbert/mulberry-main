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
#include "CMulberryCommon.h"
#include "CTimezonePopup.h"

#include "CICalendarManager.h"

// ---------------------------------------------------------------------------
//	CDateTimeZoneSelect														  [public]
/**
	Default constructor */

CDateTimeZoneSelect::CDateTimeZoneSelect() :
	CTabPanel(IDD_CALENDAR_DATETZSELECT)
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

BEGIN_MESSAGE_MAP(CDateTimeZoneSelect, CTabPanel)
	//{{AFX_MSG_MAP(CDateTimeZoneSelect)
	ON_WM_ENABLE()

	ON_COMMAND(IDC_CALENDAR_DATETZSELECT_DATECHOOSE, OnDateBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CDateTimeZoneSelect::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Get UI items
	mDate.SubclassDlgItem(IDC_CALENDAR_DATETZSELECT_DATE, this);
	mDateBtn.SubclassDlgItem(IDC_CALENDAR_DATETZSELECT_DATECHOOSE, this, IDI_CALENDAR);
	mTime.SubclassDlgItem(IDC_CALENDAR_DATETZSELECT_TIME, this);
	mZone.SubclassDlgItem(IDC_CALENDAR_DATETZSELECT_TZPOPUP, this);

	return true;
}

void CDateTimeZoneSelect::OnEnable(BOOL bEnable)
{
	mDate.EnableWindow(bEnable);
	mDateBtn.EnableWindow(bEnable);
	mTime.EnableWindow(bEnable);
	mZone.EnableWindow(bEnable);
}

void CDateTimeZoneSelect::SetAllDay(bool all_day)
{
	mAllDay = all_day;
	mTime.ShowWindow(all_day ? SW_HIDE : SW_SHOW);
	mZone.ShowWindow(all_day ? SW_HIDE : SW_SHOW);
}

void CDateTimeZoneSelect::SetDateTimeZone(const iCal::CICalendarDateTime& dt, bool all_day)
{
	SetAllDay(all_day);
	
	SYSTEMTIME sys_time;
	sys_time.wYear = dt.GetYear();
	sys_time.wMonth = dt.GetMonth();
	sys_time.wDayOfWeek = 0;
	sys_time.wDay = dt.GetDay();
	sys_time.wHour = dt.GetHours();
	sys_time.wMinute = dt.GetMinutes();
	sys_time.wSecond = dt.GetSeconds();
	sys_time.wMilliseconds = 0;

	BOOL temp = DateTime_SetSystemtime(mDate, GDT_VALID, &sys_time);
	temp = DateTime_SetSystemtime(mTime, GDT_VALID, &sys_time);
	
	if (all_day)
		mZone.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	else
		mZone.SetTimezone(dt.GetTimezone());
}

void CDateTimeZoneSelect::GetDateTimeZone(iCal::CICalendarDateTime& dt, bool all_day)
{
	dt.SetDateOnly(all_day);

	// Get date
	{
		SYSTEMTIME sys_time;
		BOOL temp = DateTime_GetSystemtime(mDate, &sys_time);
		
		dt.SetYear(sys_time.wYear);
		dt.SetMonth(sys_time.wMonth);
		dt.SetDay(sys_time.wDay );
	}
	
	// Get time & zone if required
	if (!all_day)
	{
		SYSTEMTIME sys_time;
		BOOL temp = DateTime_GetSystemtime(mTime, &sys_time);
				
		dt.SetHours(sys_time.wHour);
		dt.SetMinutes(sys_time.wMinute);
		dt.SetSeconds(sys_time.wSecond);
		
		// Get timezone
		mZone.GetTimezone(dt.GetTimezone());
	}
}

void CDateTimeZoneSelect::SetReadOnly(bool read_only)
{
	mDate.EnableWindow(!read_only);
	mDateBtn.EnableWindow(!read_only);
	mTime.EnableWindow(!read_only);
	mZone.EnableWindow(!read_only);
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