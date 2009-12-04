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

#ifndef H_CMonthPrintout
#define H_CMonthPrintout
#pragma once

#include "CCalendarViewPrintout.h"

#include "CICalendarDateTime.h"

class CCalendarView;
class CMonthIndicator;
class CMonthTable;

class CPrintText;

// ===========================================================================
//	CMonthPrintout

class	CMonthPrintout : public CCalendarViewPrintout
{
public:
	enum { class_ID = 'CapM', pane_ID = 1807 };

						CMonthPrintout(LStream *inStream);
	virtual				~CMonthPrintout();

	virtual void SetDetails(const iCal::CICalendarDateTime& date, CCalendarView* view);

protected:
	enum
	{
		eTitle_ID = 'TITL',
		ePrevTitle_ID = 'PTIT',
		ePreviousMonth_ID = 'PREV',
		eNextTitle_ID = 'NTIT',
		eNextMonth_ID = 'NEXT',
		eView_ID = 'VIEW',
		eTable_ID = 'TABL'
	};

	// UI Objects
	CPrintText*			mTitle;
	CPrintText*			mPrevTitle;
	CMonthIndicator*	mPrev;
	CPrintText*			mNextTitle;
	CMonthIndicator*	mNext;
	LView*				mView;
	CMonthTable*		mTable;
	
	virtual void		FinishCreateSelf();
};

#endif
