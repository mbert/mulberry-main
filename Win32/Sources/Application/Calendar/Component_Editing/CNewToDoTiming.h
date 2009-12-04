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

#ifndef H_CNewToDoTiming
#define H_CNewToDoTiming
#pragma once

#include "CNewTimingPanel.h"

#include "CSubPanelController.h"

class CDateTimeZoneSelect;
class CDurationSelect;

// ===========================================================================
//	CNewToDoTiming

class CNewToDoTiming : public CNewTimingPanel
{
public:
						CNewToDoTiming();
	virtual				~CNewToDoTiming();

	virtual bool	GetAllDay() const;
	virtual void	GetTimezone(iCal::CICalendarTimezone& tz) const;

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eDue_NoDue = 0,
		eDue_DueBy,
		eDue_Start
	};

	// UI Objects
	CButton					mAllDay;

	CButton					mNoDue;
	CButton					mDueBy;
	CButton					mStarts;

	CSubPanelController		mDueDateTimeZoneItem;
	CDateTimeZoneSelect*	mDueDateTimeZone;
	CSubPanelController		mStartDateTimeZoneItem;
	CDateTimeZoneSelect*	mStartDateTimeZone;
	CSubPanelController		mDurationItem;
	CDurationSelect*		mDuration;
	
			void	DoAllDay(bool set);
			void	DoDueGroup(uint32_t group);

	// Generated message map functions
	//{{AFX_MSG(CNewComponentDescription)
	virtual BOOL	OnInitDialog();
	afx_msg void	OnAllDay();
	afx_msg void	OnNoDue();
	afx_msg void	OnDueBy();
	afx_msg void	OnStarts();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
