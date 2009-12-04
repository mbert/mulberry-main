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

#ifndef H_CDateTimeZoneSelect
#define H_CDateTimeZoneSelect
#pragma once

#include "CTabPanel.h"

#include "CICalendarDateTime.h"

#include "CDateControl.h"
#include "CIconButton.h"
#include "CTimezonePopup.h"

// ===========================================================================
//	CDateTimeZoneSelect

class CDateTimeZoneSelect : public CTabPanel
{
public:
						CDateTimeZoneSelect();
	virtual				~CDateTimeZoneSelect();

			void	SetAllDay(bool all_day);
			
			void	SetDateTimeZone(const iCal::CICalendarDateTime& dt, bool all_day);
			void	GetDateTimeZone(iCal::CICalendarDateTime& dt, bool all_day);

			void	SetReadOnly(bool read_only);

	virtual void	SetContent(void* data) {}						// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

protected:

// Dialog Data
	//{{AFX_DATA(CDateTimeZoneSelect)
	enum { IDD = IDD_CALENDAR_DATETZSELECT };
	CDateControl	mDate;
	CIconButton		mDateBtn;
	CDateControl	mTime;
	CTimezonePopup	mZone;
	//}}AFX_DATA

	bool	mAllDay;

	// Generated message map functions
	//{{AFX_MSG(CDateTimeZoneSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnEnable(BOOL bEnable);
	afx_msg	void	OnDateBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
};

#endif
