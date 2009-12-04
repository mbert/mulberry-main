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

#ifndef H_CDurationSelect
#define H_CDurationSelect
#pragma once

#include "CTabPanel.h"

#include "CICalendarDuration.h"

#include "CNumberEdit.h"
#include "CPopupButton.h"

// ===========================================================================
//	CDurationSelect

class CDurationSelect : public CTabPanel
{
public:
						CDurationSelect();
	virtual				~CDurationSelect();

			void	SetAllDay(bool all_day);
			
			void	SetDuration(const iCal::CICalendarDuration& du, bool all_day);
			void	GetDuration(iCal::CICalendarDuration& du, bool all_day);

			void	SetReadOnly(bool read_only);

	virtual void	SetContent(void* data) {}						// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

protected:
	enum
	{
		eDurationPeriod_Days = 0,
		eDurationPeriod_Weeks
	};

// Dialog Data
	//{{AFX_DATA(CDurationSelect)
	enum { IDD = IDD_CALENDAR_DURATIONSELECT };
	CNumberEdit				mNumber;
	CSpinButtonCtrl			mNumberSpin;
	CPopupButton			mPeriod;
	CStatic					mCaption1;
	CNumberEdit				mHours;
	CSpinButtonCtrl			mHoursSpin;
	CStatic					mCaption2;
	CNumberEdit				mMinutes;
	CSpinButtonCtrl			mMinutesSpin;
	CStatic					mCaption3;
	CNumberEdit				mSeconds;
	CSpinButtonCtrl			mSecondsSpin;
	CStatic					mCaption4;
	//}}AFX_DATA

	bool	mAllDay;

	void ShowTime(bool show);

	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnEnable(BOOL bEnable);
	afx_msg	void	OnDurationPopup();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
};

#endif
