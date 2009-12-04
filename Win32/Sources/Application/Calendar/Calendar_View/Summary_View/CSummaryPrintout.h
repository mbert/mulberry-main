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

#ifndef H_CDayWeekPrintout
#define H_CDayWeekPrintout
#pragma once

#include <PPxPrintout.h>

#include "CICalendarDateTime.h"

#include "CDayWeekView.h"
#include "CDayWeekViewTimeRange.h"

class CCalendarView;
class CDayWeekTitleTable;
class CDayWeekTable;

namespace PPx {

class PPxScrollable;
class StaticText;

};

// ===========================================================================
//	CDayWeekPrintout

class	CDayWeekPrintout : public PPx::PPxPrintout
{
public:
						CDayWeekPrintout();
	virtual				~CDayWeekPrintout();

	virtual void SetDetails(const iCal::CICalendarDateTime& date, CDayWeekView::EDayWeekType type, CDayWeekViewTimeRange::ERanges range, CCalendarView* view);

protected:
	enum
	{
		eMonth_ID = 'MNTH',
		eYear_ID = 'YEAR',
		eTimezone_ID = 'TZID',
		eView_ID = 'VIEW',
		eTitleTable_ID = 'TITL',
		eTable_ID = 'TABL'
	};

	// UI Objects
	PPx::StaticText*	mMonth;
	PPx::StaticText*	mYear;
	PPx::StaticText*	mTzid;
	
	PPx::PPxScrollable*	mView;
	CDayWeekTitleTable*	mTitles;
	CDayWeekTable*		mTable;
	

	virtual CFStringRef	ClassName() const;
	virtual void		FinishInit();
};

#endif
