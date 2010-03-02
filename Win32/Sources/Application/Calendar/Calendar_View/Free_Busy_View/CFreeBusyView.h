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

#ifndef H_CFreeBusyView
#define H_CFreeBusyView
#pragma once

#include "CCalendarViewBase.h"

#include "CDayWeekViewTimeRange.h"

#include "CCalendarStoreFreeBusy.h"

#include "CIconButton.h"
#include "CFreeBusyTable.h"
#include "CFreeBusyTitleTable.h"
#include "CPopupButton.h"
#include "CTimezonePopup.h"

class CCalendarEventBase;
class CTimezonePopup;

// ===========================================================================
//	CFreeBusyView

class CFreeBusyView : public CCalendarViewBase
{
public:
	enum EDayWeekType
	{
		eDay,
		eWorkWeek,
		eWeek
	};

						CFreeBusyView(CCommander* cmdr);
	virtual				~CFreeBusyView();

	virtual void ListenTo_Message(long msg, void* param);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void ResetFont(CFont* font);			// Reset font

	virtual void DoPrint();

	CDayWeekViewTimeRange::ERanges GetRange() const
	{
		return mFreeBusyRange;
	}
	void SetRange(CDayWeekViewTimeRange::ERanges range);

	uint32_t GetScale() const
	{
		return mFreeBusyScale;
	}
	void SetScale(uint32_t scale);

	virtual CCalendarTableBase* GetTable() const;

	void SetFreeBusy(iCal::CICalendarRef calref, const cdstring& id, const iCal::CICalendarProperty& organizer, const iCal::CICalendarPropertyList& attendees, const iCal::CICalendarDateTime& date);

protected:
	// UI Objects
	CIconButton		mPrevDay;
	CIconButton		mNextDay;
	CTimezonePopup	mTimezonePopup;
	CPopupButton	mScalePopup;
	CPopupButton	mRangePopup;

	CFreeBusyTitleTable	mTitles;
	CFreeBusyTable		mTable;
	CCalendarEventBase*	mSelectedEvent;
	
	CDayWeekViewTimeRange::ERanges	mFreeBusyRange;
	uint32_t						mFreeBusyScale;

	iCal::CICalendarRef						mCalendarRef;
	cdstring								mIdentityRef;
	iCal::CICalendarProperty				mOrganizer;
	iCal::CICalendarPropertyList			mAttendees;
	calstore::CCalendarStoreFreeBusyList	mDetails;

	virtual	void		ResetDate();

	afx_msg void		OnUpdateNewEvent(CCmdUI* pCmdUI);

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnPrevDay();
	afx_msg void		OnNextDay();
	afx_msg void		OnToday();
	afx_msg void		OnNewEvent();
	afx_msg void		OnNewEvent(const iCal::CICalendarDateTime& dt);
	afx_msg void		OnTimezone();
	afx_msg void		OnScale();
	afx_msg void		OnRange();

	DECLARE_MESSAGE_MAP()
};

#endif
