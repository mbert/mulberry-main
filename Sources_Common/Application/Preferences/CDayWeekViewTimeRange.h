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

/* 
	CDayWeekViewTimeRange.h

	Author:			
	Description:	<describe the CDayWeekViewTimeRange class here>
*/

#ifndef CDayWeekViewTimeRange_H
#define CDayWeekViewTimeRange_H

#include "CPreferenceItem.h"

#include "CICalendarPeriod.h"

class CDayWeekViewTimeRange : public CPreferenceItem
{
public:
	enum ERanges
	{
		e24Hrs = 0,
		eMorning,
		eAfternoon,
		eEvening,
		eAwake,
		eWork,
		eRangesLast
	};

	CDayWeekViewTimeRange();
	virtual ~CDayWeekViewTimeRange() {}
	
	int operator==(const CDayWeekViewTimeRange& comp) const				// compare with same
		{ return 0; }

	// Read/write prefs
	virtual cdstring GetInfo(void) const;
	virtual bool SetInfo(char_stream& info, NumVersion vers_prefs);

	const iCal::CICalendarDateTime& GetStart(ERanges range) const
	{
		return mRanges[range].GetStart();
	}
	const iCal::CICalendarDateTime& GetEnd(ERanges range) const
	{
		return mRanges[range].GetEnd();
	}
	void SetRange(ERanges range, const iCal::CICalendarDateTime& dtstart, const iCal::CICalendarDateTime& dtend)
	{
		mRanges[range] = iCal::CICalendarPeriod(dtstart, dtend);
	}

protected:
	iCal::CICalendarPeriod	mRanges[eRangesLast];

	cdstrpair GetInfoRange(ERanges range) const;
	void SetInfoRange(ERanges range, const cdstrpair& value);
};

#endif	// CDayWeekViewTimeRange_H
