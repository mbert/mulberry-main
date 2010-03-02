/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#ifndef H_CNewEventTiming
#define H_CNewEventTiming
#pragma once

#include "CNewTimingPanel.h"

#include "CSubPanelController.h"

class CDateTimeZoneSelect;
class CDurationSelect;

// ===========================================================================
//	CNewEventTiming

class CNewEventTiming : public CNewTimingPanel
{
public:
						CNewEventTiming();
	virtual				~CNewEventTiming();

	virtual bool	GetAllDay() const;
	virtual void	GetTimezone(iCal::CICalendarTimezone& tz) const;

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);
			void    GetPeriod(iCal::CICalendarPeriod& period);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eEnd_End = 1,
		eEnd_Duration
	};

	// UI Objects
	enum { IDD = IDD_CALENDAR_NEWEVENTTIMING };
	CButton					mAllDay;

	CSubPanelController		mStartDateTimeZoneItem;
	CDateTimeZoneSelect*	mStartDateTimeZone;

	CButton					mUseEnd;
	CButton					mUseDuration;

	CSubPanelController		mEndDateTimeZoneItem;
	CDateTimeZoneSelect*	mEndDateTimeZone;

	CSubPanelController		mDurationItem;
	CDurationSelect*		mDuration;

			void	DoAllDay(bool set);
			void	DoEnds(bool use_duration);
			
			void	SyncEnd();
			void	SyncDuration();

	// Generated message map functions
	//{{AFX_MSG(CNewEventTiming)
	virtual BOOL	OnInitDialog();
	afx_msg void	OnAllDay();
	afx_msg void	OnEnds();
	afx_msg void	OnDuration();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
