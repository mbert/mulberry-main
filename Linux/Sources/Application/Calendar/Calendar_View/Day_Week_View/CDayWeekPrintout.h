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

#include "CCalendarViewPrintout.h"

#include "CICalendarDateTime.h"

#include "CDayWeekView.h"
#include "CDayWeekViewTimeRange.h"

class CCalendarView;
class CDayWeekTitleTable;
class CDayWeekTable;
class CMonthIndicator;

class CPrintText;

// ===========================================================================
//	CDayWeekPrintout

class	CDayWeekPrintout : public CCalendarViewPrintout
{
public:
	enum { class_ID = 'CapD', pane_ID = 1808 };

						CDayWeekPrintout(LStream *inStream);
	virtual				~CDayWeekPrintout();

	virtual void SetDetails(const iCal::CICalendarDateTime& date, CDayWeekView::EDayWeekType type, CDayWeekViewTimeRange::ERanges range, CCalendarView* view);

protected:
	enum
	{
		eTitle_ID = 'CAPT',
		eThisMonth_ID = 'THIS',
		eView_ID = 'VIEW',
		eTitleTable_ID = 'TITL',
		eTable_ID = 'TABL'
	};

	// UI Objects
	CPrintText*			mTitle;
	CMonthIndicator*	mThisMonth;
	LView*				mView;
	CDayWeekTitleTable*	mTitles;
	CDayWeekTable*		mTable;
	
	virtual void		FinishCreateSelf();
};

#endif
