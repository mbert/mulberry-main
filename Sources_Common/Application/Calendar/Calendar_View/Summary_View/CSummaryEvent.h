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

#ifndef H_CSummaryEvent
#define H_CSummaryEvent

#include "cdstring.h"
#include "ptrvector.h"

#include "CICalendarComponentExpanded.h"

// ===========================================================================
//	CSummaryEvent

class CSummaryEvent
{
	friend class CSummaryTable;

public:
	enum EType
	{
		ePast = 0,
		eEarlierThisWeek,
		eYesterday,
		eToday,
		eTomorrow,
		eRemainderOfWeek,
		eNextWeek,
		eFuture,
		eWeek,
		eMonth,
		eYear
	};

						CSummaryEvent(EType type, const iCal::CICalendarComponentExpandedShared& vevent, bool start, bool end);
						CSummaryEvent(EType type, uint32_t value);
						
	virtual				~CSummaryEvent();

	const iCal::CICalendarComponentExpandedShared& GetVEvent() const
		{ return mVEvent; }

	EType GetType() const
	{
		return mType;
	}
	bool IsEvent() const
	{
		return mIsEvent;
	}

protected:
	iCal::CICalendarComponentExpandedShared	mVEvent;

	EType						mType;
	bool						mIsEvent;
	iCal::CICalendarDateTime	mSortDT;
	bool						mDayStart;
	bool						mDayEnd;
	cdstring					mStartDate;
	cdstring					mStartTime;
	cdstring					mSummary;
	cdstring					mStatus;
	cdstring					mCalendar;
	uint32_t					mColour;
	cdstring					mTooltip;
	bool						mCancelled;
	
	void InitSummary(EType type, const iCal::CICalendarComponentExpandedShared& vevent, bool start, bool end);
	void InitSummary(EType type, uint32_t value);
};

typedef ptrvector<CSummaryEvent> CSummaryEventList;

#endif
