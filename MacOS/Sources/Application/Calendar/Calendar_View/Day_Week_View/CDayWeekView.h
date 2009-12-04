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

#ifndef H_CDayWeekView
#define H_CDayWeekView
#pragma once

#include "CCalendarViewBase.h"
#include <LListener.h>

#include "CDayWeekViewTimeRange.h"

class CCalendarEventBase;
class CDayWeekTable;
class CDayWeekTitleTable;
class CTimezonePopup;

class LBevelButton;
class LPopupButton;
class LScrollerView;

// ===========================================================================
//	CDayWeekView

class CDayWeekView : public CCalendarViewBase,
					public LListener
{
public:
	enum { class_ID = 'CalD', pane_ID = 1803 };

	enum EDayWeekType
	{
		eDay,
		eWorkWeek,
		eWeek
	};

						CDayWeekView(LStream *inStream);
	virtual				~CDayWeekView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

			void SetType(EDayWeekType type);

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
	enum
	{
		ePrevWeek_ID = 'PWEE',
		eNextWeek_ID = 'NWEE',
		ePrevDay_ID = 'PDAY',
		eNextDay_ID = 'NDAY',
		eTimezone_ID = 'TZID',
		eScale_ID = 'SCAL',
		eRange_ID = 'HOUR',
		eTitleTable_ID = 'TITL',
		eScroller_ID = 'SCRL',
		eTable_ID = 'TABL'
	};

	// UI Objects
	LBevelButton*	mPrevWeek;
	LBevelButton*	mNextWeek;
	LBevelButton*	mPrevDay;
	LBevelButton*	mNextDay;
	CTimezonePopup*	mTimezonePopup;
	LPopupButton*	mScalePopup;
	LPopupButton*	mRangePopup;

	CDayWeekTitleTable*	mTitles;
	LScrollerView*		mScroller;
	CDayWeekTable*		mTable;
	CCalendarEventBase*	mSelectedEvent;
	
	EDayWeekType		mType;

	CDayWeekViewTimeRange::ERanges	mDayWeekRange;
	uint32_t						mDayWeekScale;	

	virtual void		FinishCreateSelf();
	
	virtual	void		ResetDate();

			void		OnPrevWeek();
			void		OnNextWeek();
			void		OnPrevDay();
			void		OnNextDay();
			void		OnToday();
			void		OnNewEvent();
			void		OnNewEvent(const iCal::CICalendarDateTime& dt);
			void		OnTimezone();
			void		OnScale();
			void		OnRange();
};

#endif
