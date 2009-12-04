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

#ifndef H_CCalendarTableBase
#define H_CCalendarTableBase
#pragma once

#include "CTable.h"
#include "CListener.h"

#include "CICalendarDateTime.h"
#include "CICalendarTimezone.h"

class CCalendarViewBase;

// ===========================================================================
//	CCalendarTableBase

class CCalendarTableBase : public CTable,
							public CListener
{
public:
	enum
	{
		eBroadcast_DblClkToMonth = 'DblM',	// param = iCal::CICalendarDateTime*
		eBroadcast_DblClkToDay = 'DblD',	// param = iCal::CICalendarDateTime*
		eBroadcast_CreateNew = 'NewI'		// param = NULL
	};

						CCalendarTableBase();
	virtual				~CCalendarTableBase();
	
	CCalendarViewBase*	GetCalendarView() const
		{ return mCalendarView; }
	void SetCalendarView(CCalendarViewBase* view)
		{ mCalendarView = view; }

	const iCal::CICalendarTimezone& GetTimezone() const
		{ return mTimezone; }

protected:
	CCalendarViewBase*			mCalendarView;
	iCal::CICalendarTimezone	mTimezone;
	iCal::CICalendarDateTime	mLastIdleTime;
	UINT						mTimerID;

	virtual void 	Idle();

	void	OnCreateNew();

	// Generated message map functions
	//{{AFX_MSG(CChangePswdDialog)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
