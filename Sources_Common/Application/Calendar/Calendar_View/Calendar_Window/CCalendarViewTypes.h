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


// Forward Header for CCalendarViewTypes class

#ifndef __CCalendarViewTypes__MULBERRY__
#define __CCalendarViewTypes__MULBERRY__

namespace NCalendarView
{
	// Main view types
	enum EViewType
	{
		eViewDay,
		eViewWorkWeek,
		eViewWeek,
		eViewMonth,
		eViewYear,
		eViewSummary,
		eViewFreeBusy
	};

	// Year table layouts
	enum EYearLayout
	{
		e1x12,
		e2x6,
		e3x4,
		e4x3,
		e6x2,
		e12x1
	};

	// Summary view types
	enum ESummaryType
	{
		eList
	};

	// Summary view ranges
	enum ESummaryRanges
	{
		eToday,
		eDays,
		eThisWeek,
		eWeeks,
		eThisMonth,
		eMonths,
		eThisYear,
		eYears
	};


};

#endif
