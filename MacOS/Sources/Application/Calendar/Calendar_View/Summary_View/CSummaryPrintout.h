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

#ifndef H_CSummaryPrintout
#define H_CSummaryPrintout
#pragma once

#include "CCalendarViewPrintout.h"

#include "CICalendarDateTime.h"

#include "CSummaryView.h"
#include "CDayWeekViewTimeRange.h"

class CCalendarView;
class CMonthIndicator;
class CSummaryTitleTable;
class CSummaryTable;

class CPrintText;

// ===========================================================================
//	CSummaryPrintout

class	CSummaryPrintout : public CCalendarViewPrintout
{
public:
	enum { class_ID = 'CapS', pane_ID = 1809 };

						CSummaryPrintout(LStream *inStream);
	virtual				~CSummaryPrintout();

	virtual void SetDetails(const iCal::CICalendarDateTime& date, NCalendarView::ESummaryType type, NCalendarView::ESummaryRanges range, CCalendarView* view);

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
	CSummaryTitleTable*	mTitles;
	CSummaryTable*		mTable;
	
	virtual void		FinishCreateSelf();
};

#endif
