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
	CDayWeekViewTimeRange.cpp

	Author:			
	Description:	<describe the CDayWeekViewTimeRange class here>
*/

#include "CDayWeekViewTimeRange.h"

#include <strstream>

extern const char* cSpace;

CDayWeekViewTimeRange::CDayWeekViewTimeRange()
{
	// Set defaults
	SetRange(e24Hrs, iCal::CICalendarDateTime(2000, 1, 1, 0, 0, 0), iCal::CICalendarDateTime(2000, 1, 2, 0, 0, 0));
	SetRange(eMorning, iCal::CICalendarDateTime(2000, 1, 1, 6, 0, 0), iCal::CICalendarDateTime(2000, 1, 1, 12, 0, 0));
	SetRange(eAfternoon, iCal::CICalendarDateTime(2000, 1, 1, 12, 0, 0), iCal::CICalendarDateTime(2000, 1, 1, 18, 0, 0));
	SetRange(eEvening, iCal::CICalendarDateTime(2000, 1, 1, 18, 0, 0), iCal::CICalendarDateTime(2000, 1, 2, 0, 0, 0));
	SetRange(eAwake, iCal::CICalendarDateTime(2000, 1, 1, 6, 0, 0), iCal::CICalendarDateTime(2000, 1, 2, 0, 0, 0));
	SetRange(eWork, iCal::CICalendarDateTime(2000, 1, 1, 9, 0, 0), iCal::CICalendarDateTime(2000, 1, 1, 17, 0, 0));
}

// Read/write prefs

cdstring CDayWeekViewTimeRange::GetInfo(void) const
{
	cdstrpairvect details;
	details.push_back(GetInfoRange(eMorning));
	details.push_back(GetInfoRange(eAfternoon));
	details.push_back(GetInfoRange(eEvening));
	details.push_back(GetInfoRange(eAwake));
	details.push_back(GetInfoRange(eWork));
	
	cdstring result;
	result.CreateSExpression(details);
	
	return result;
}

cdstrpair CDayWeekViewTimeRange::GetInfoRange(ERanges range) const
{
	cdstrpair result;
	
	// Start date-time
	{
		std::ostrstream os;
		mRanges[range].GetStart().Generate(os);
		os << std::ends;
		cdstring temp;
		temp.steal(os.str());
		result.first = temp;
	}
	// End date-time
	{
		std::ostrstream os;
		mRanges[range].GetEnd().Generate(os);
		os << std::ends;
		cdstring temp;
		temp.steal(os.str());
		result.second = temp;
	}
	
	return result;
}

bool CDayWeekViewTimeRange::SetInfo(char_stream& info, NumVersion vers_prefs)
{
	cdstrpairvect details;
	cdstring::ParseSExpression(info, details, false);

	SetInfoRange(eMorning, details[0]);
	SetInfoRange(eAfternoon, details[1]);
	SetInfoRange(eEvening, details[2]);
	SetInfoRange(eAwake, details[3]);
	SetInfoRange(eWork, details[4]);
	
	return true;
}

void CDayWeekViewTimeRange::SetInfoRange(ERanges range, const cdstrpair& value)
{
	iCal::CICalendarDateTime dtstart;
	dtstart.Parse(value.first);

	iCal::CICalendarDateTime dtend;
	dtend.Parse(value.second);
	
	// Make sure end is after start
	if (dtend <= dtstart)
	{
		dtend = dtstart;
		dtend.SetHHMMSS(0, 0, 0);
		dtend.OffsetDay(1);
	}

	SetRange(range, dtstart, dtend);
}
