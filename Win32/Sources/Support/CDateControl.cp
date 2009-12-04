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


// CDateControl.cpp : implementation file
//


#include "CDateControl.h"

#include "CRFC822.h"

IMPLEMENT_DYNAMIC(CDateControl, CWnd)

/////////////////////////////////////////////////////////////////////////////
// CDateControl

CDateControl::CDateControl()
{
	mCommonCtrlEx = false;
}

CDateControl::~CDateControl()
{
}


BEGIN_MESSAGE_MAP(CDateControl, CWnd)
	//{{AFX_MSG_MAP(CDateControl)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CDateControl::SetDateEx(time_t date)
{
	CTime win_time(date);

	SYSTEMTIME sys_time;
	sys_time.wYear = win_time.GetYear();
	sys_time.wMonth = win_time.GetMonth();
	sys_time.wDayOfWeek = win_time.GetDayOfWeek();
	sys_time.wDay = win_time.GetDay();
	sys_time.wHour = 0;
	sys_time.wMinute = 0;
	sys_time.wSecond = 0;
	sys_time.wMilliseconds = 0;

	BOOL temp = DateTime_SetSystemtime(GetSafeHwnd(), GDT_VALID, &sys_time);
	
}

time_t CDateControl::GetDateEx() const
{
	SYSTEMTIME sys_time;
	
	BOOL temp = DateTime_GetSystemtime(GetSafeHwnd(), &sys_time);

	CTime win_time(sys_time);

	return win_time.GetTime();
}

void CDateControl::SetDateOld(time_t date)
{
	struct tm* now_tm = ::localtime(&date);
	SYSTEMTIME sys;
	sys.wYear = now_tm->tm_year + 1900;
	sys.wMonth = now_tm->tm_mon + 1;		// Adjust for month offset
	sys.wDayOfWeek = now_tm->tm_wday;
	sys.wDay = now_tm->tm_mday;
	sys.wHour = now_tm->tm_hour;
	sys.wMinute = now_tm->tm_min;
	sys.wSecond = now_tm->tm_sec;
	sys.wMilliseconds = 0;

	TCHAR date_str[256];
	::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sys, NULL, date_str, 256);
	
	SetWindowText(date_str);
}

time_t CDateControl::GetDateOld() const
{
	CString text;
	GetWindowText(text);
	long day;
	long month;
	long year;
	bool ddmmyyyy = true;
	
	// Determine whether dd/mm/yyyy or mm/dd/yyyy format
	{
		SYSTEMTIME sys;
		sys.wYear = 1998;
		sys.wMonth = 2;
		sys.wDayOfWeek = 0;
		sys.wDay = 1;
		sys.wHour = 0;
		sys.wMinute = 0;
		sys.wSecond = 0;
		sys.wMilliseconds = 0;

		TCHAR date_str[256];
		::GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &sys, NULL, date_str, 256);
		cdstring temp(date_str);
		
		ddmmyyyy = (::atol(temp) == 1);
	}

	cdstring temp(text);
	char* str = temp.c_str_mod();
	if (ddmmyyyy)
	{
		day = ::strtol(str, &str, 10);
		while(str && !::isdigit(*str)) str++;
		month = ::strtol(str, &str, 10);
	}
	else
	{
		month = ::strtol(str, &str, 10);
		while(str && !::isdigit(*str)) str++;
		day = ::strtol(str, &str, 10);
	}
	while(str && !::isdigit(*str)) str++;
	year = ::strtol(str, &str, 10);
	if (year < 70)
		year += 2000;
	else if (year < 100)
		year += 1900;

	cdstring date(day);
	date += "-";
	
	switch(month)
	{
	case 1:
		date += "Jan";
		break;
	case 2:
		date += "Feb";
		break;
	case 3:
		date += "Mar";
		break;
	case 4:
		date += "Apr";
		break;
	case 5:
		date += "May";
		break;
	case 6:
		date += "Jun";
		break;
	case 7:
		date += "Jul";
		break;
	case 8:
		date += "Aug";
		break;
	case 9:
		date += "Sep";
		break;
	case 10:
		date += "Oct";
		break;
	case 11:
		date += "Nov";
		break;
	case 12:
		date += "Dec";
		break;
	}
	date += "-";

	date += cdstring(year);

	return CRFC822::ParseDate(date);
}