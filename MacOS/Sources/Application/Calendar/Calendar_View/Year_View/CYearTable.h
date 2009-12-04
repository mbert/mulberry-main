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

#ifndef H_CYEARTABLE
#define H_CYEARTABLE
#pragma once

#include "CCalendarTableBase.h"

#include "CCalendarViewTypes.h"

#include "CICalendarDateTime.h"

class CMonthIndicator;

// ===========================================================================
//	CYearTable

class CYearTable : public CCalendarTableBase
{
public:
	enum { class_ID = 'CtbY' };

						CYearTable(LStream *inStream);
	virtual				~CYearTable();
	
	virtual void		ListenTo_Message(long msg, void* param);

	void				ResetTable(int32_t year, NCalendarView::EYearLayout layout);
	virtual void		AdaptToNewSurroundings();					// Adjust column widths

	bool GetSelectedDate(iCal::CICalendarDateTime& date) const;

protected:
	int32_t				mYear;
	NCalendarView::EYearLayout	mLayout;
	CMonthIndicator*	mMonths[12];
	CMonthIndicator*	mLastSelected;
	int32_t				mSubCellSize;

	virtual void		FinishCreateSelf();

	virtual void 		Idle();

	virtual void		ClickCell(const STableCell& inCell,
									const SMouseDownEvent& inMouseDown);

	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

private:
			void		ResetFrame();
			
	CMonthIndicator*	GetIndicator(const STableCell& inCell);
	
			void		DoIndicatorSelectionChanged(CMonthIndicator* indicator);
			void		RespositionIndicators();
};

#endif
