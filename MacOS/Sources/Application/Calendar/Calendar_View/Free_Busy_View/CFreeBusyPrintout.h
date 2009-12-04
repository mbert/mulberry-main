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

#ifndef H_CFreeBusyPrintout
#define H_CFreeBusyPrintout
#pragma once

#include "CCalendarViewPrintout.h"

#include "CICalendarDateTime.h"

#include "CFreeBusyView.h"
#include "CDayWeekViewTimeRange.h"

class CCalendarView;
class CFreeBusyTitleTable;
class CFreeBusyTable;
class CMonthIndicator;

class CPrintText;

// ===========================================================================
//	CFreeBusyPrintout

class	CFreeBusyPrintout : public CCalendarViewPrintout
{
public:
	enum { class_ID = 'CapF', pane_ID = 1825 };

						CFreeBusyPrintout(LStream *inStream);
	virtual				~CFreeBusyPrintout();

	virtual void SetDetails(const iCal::CICalendarDateTime& date, CDayWeekViewTimeRange::ERanges range, CCalendarView* view);

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
	CPrintText*				mTitle;
	CMonthIndicator*		mThisMonth;
	LView*					mView;
	CFreeBusyTitleTable*	mTitles;
	CFreeBusyTable*			mTable;
	
	virtual void		FinishCreateSelf();
};

#endif
