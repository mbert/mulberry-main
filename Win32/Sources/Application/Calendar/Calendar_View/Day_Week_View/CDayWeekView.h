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

#ifndef H_CDayWeekView
#define H_CDayWeekView
#pragma once

#include "CCalendarViewBase.h"

#include "CDayWeekViewTimeRange.h"
#include "CIconButton.h"
#include "CDayWeekTable.h"
#include "CDayWeekTitleTable.h"
#include "CPopupButton.h"
#include "CTimezonePopup.h"

class CCalendarEventBase;
class CTimezonePopup;

// ===========================================================================
//	CDayWeekView

class CDayWeekView : public CCalendarViewBase
{
public:
	enum EDayWeekType
	{
		eDay,
		eWorkWeek,
		eWeek
	};

						CDayWeekView(CCommander* cmdr);
	virtual				~CDayWeekView();

	virtual void ListenTo_Message(long msg, void* param);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

			void SetType(EDayWeekType type);

	virtual void ResetFont(CFont* font);			// Reset font

	virtual void DoPrint();

	CDayWeekViewTimeRange::ERanges GetRange() const
	{
		return mDayWeekRange;
	}
	void SetRange(CDayWeekViewTimeRange::ERanges range);

	uint32_t GetScale() const
	{
		return mDayWeekScale;
	}
	void SetScale(uint32_t scale);

	virtual CCalendarTableBase* GetTable() const;

protected:
	// UI Objects
	CIconButton		mPrevWeek;
	CIconButton		mNextWeek;
	CIconButton		mPrevDay;
	CIconButton		mNextDay;
	CTimezonePopup	mTimezonePopup;
	CPopupButton	mScalePopup;
	CPopupButton	mRangePopup;

	CDayWeekTitleTable	mTitles;
	CDayWeekTable		mTable;
	CCalendarEventBase*	mSelectedEvent;
	
	EDayWeekType		mType;

	CDayWeekViewTimeRange::ERanges	mDayWeekRange;
	uint32_t						mDayWeekScale;	

	virtual	void		ResetDate();

	afx_msg void		OnUpdateNewEvent(CCmdUI* pCmdUI);

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnPrevWeek();
	afx_msg void		OnNextWeek();
	afx_msg void		OnPrevDay();
	afx_msg void		OnNextDay();
	afx_msg void		OnToday();
	afx_msg void		OnNewEvent();
			void		OnNewEvent(const iCal::CICalendarDateTime& dt);
	afx_msg void		OnTimezone();
	afx_msg void		OnScale();
	afx_msg void		OnRange();

	DECLARE_MESSAGE_MAP()
};

#endif
