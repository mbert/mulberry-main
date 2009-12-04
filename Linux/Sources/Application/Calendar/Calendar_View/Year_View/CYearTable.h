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

#include "CCalendarTableBase.h"

#include "CCalendarViewTypes.h"

#include "CICalendarDateTime.h"

class CMonthIndicator;

// ===========================================================================
//	CYearTable

class CYearTable : public CCalendarTableBase
{
public:
			CYearTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
	virtual	~CYearTable();
	
	virtual void		OnCreate();

	virtual void		ListenTo_Message(long msg, void* param);

	void				ResetTable(int32_t year, NCalendarView::EYearLayout layout);
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

	bool GetSelectedDate(iCal::CICalendarDateTime& date) const;

protected:
	int32_t				mYear;
	NCalendarView::EYearLayout	mLayout;
	CMonthIndicator*	mMonths[12];
	CMonthIndicator*	mLastSelected;
	int32_t				mSubCellSize;

	virtual void 		Idle();

	virtual void		LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods);

	virtual void		DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect);

private:
			void		ResetFrame();
			
	CMonthIndicator*	GetIndicator(const STableCell& inCell);
	
			void		DoIndicatorSelectionChanged(CMonthIndicator* indicator);
			void		RespositionIndicators();
};

#endif
