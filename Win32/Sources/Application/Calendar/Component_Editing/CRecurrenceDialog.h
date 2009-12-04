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

#ifndef H_CRecurrenceDialog
#define H_CRecurrenceDialog
#pragma once

#include "CHelpDialog.h"

#include "CICalendarRecurrence.h"

#include "CNumberEdit.h"
#include "CPopupButton.h"
#include "CSubPanelController.h"
#include "CTextButton.h"

class CDateTimeZoneSelect;

// ===========================================================================
//	CRecurrenceDialog

class CRecurrenceDialog : public CHelpDialog
{
public:
	static bool PoseDialog(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);

						CRecurrenceDialog(CWnd* pParent = NULL);
	virtual				~CRecurrenceDialog();

protected:

	enum
	{
		eOccurs_Yearly = 0,
		eOccurs_Monthly,
		eOccurs_Weekly,
		eOccurs_Daily,
		eOccurs_Hourly,
		eOccurs_Minutely,
		eOccurs_Secondly
	};

	enum
	{
		eOccurs_ForEver = 0,
		eOccurs_Count,
		eOccurs_Until
	};

	enum
	{
		eByDay_Any = 0,
		eByDay_1st,
		eByDay_2nd,
		eByDay_3rd,
		eByDay_4th,
		eByDay_2ndLast,
		eByDay_1stLast
	};

	enum
	{
		eBySetPos_All = 0,
		eBySetPos_1st,
		eBySetPos_Last
	};

// Dialog Data
	//{{AFX_DATA(CRecurrenceDialog)
	enum { IDD = IDD_RECURRENCE };
	CNumberEdit				mOccursInterval;
	CSpinButtonCtrl			mOccursIntervalSpin;
	CPopupButton			mOccursFreq;

	CButton					mOccursForEver;
	CButton					mOccursCount;
	CButton					mOccursUntil;

	CNumberEdit				mOccursCounter;
	CSpinButtonCtrl			mOccursCounterSpin;

	CSubPanelController		mOccursDateTimeZoneItem;
	CDateTimeZoneSelect*	mOccursDateTimeZone;

	CTextButton				mByMonth[12];
	CTextButton				mByMonthDay[31];
	CTextButton				mByMonthDayLast[7];
	CTextButton				mByDay[7];
	CPopupButton			mByDayPopup;
	CStatic					mByDayCaption;

	CPopupButton			mBySetPosPopup;
	//}}AFX_DATA
	iCal::CICalendarRecurrence* mRecur;
	const iCal::CICalendarTimezone* mTzid;
	bool mAllDay;

			void	InitControls();
			void	DoOccursFreq(UInt32 value);
			void	DoOccursGroup(UInt32 value);
			
			void	SetRecurrence(const iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);
			void	SetByMonth(const iCal::CICalendarRecurrence& recur);
			void	SetByMonthDay(const iCal::CICalendarRecurrence& recur);
			void	SetByDay(const iCal::CICalendarRecurrence& recur);
			void	SetBySetPos(const iCal::CICalendarRecurrence& recur);

			void	GetRecurrence(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day);
			void	GetByMonth(iCal::CICalendarRecurrence& recur);
			void	GetByMonthDay(iCal::CICalendarRecurrence& recur);
			void	GetByDay(iCal::CICalendarRecurrence& recur);
			void	GetBySetPos(iCal::CICalendarRecurrence& recur);

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRecurrenceDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	// Generated message map functions
	//{{AFX_MSG(CRecurrenceDialog)
	afx_msg	void	OnOccursFreq();
	afx_msg	void	OnOccursForEver();
	afx_msg	void	OnOccursCount();
	afx_msg	void	OnOccursUntil();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
