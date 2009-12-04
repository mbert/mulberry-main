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
#include "CMonthIndicator.h"

#include "CICalendarDateTime.h"

// ===========================================================================
//	CYearTable

class CYearTable : public CCalendarTableBase
{
public:
						CYearTable();
	virtual				~CYearTable();
	
	virtual void		ListenTo_Message(long msg, void* param);

	void				ResetTable(int32_t year, NCalendarView::EYearLayout layout);

	bool GetSelectedDate(iCal::CICalendarDateTime& date) const;

protected:
	int32_t						mYear;
	NCalendarView::EYearLayout	mLayout;
	CMonthIndicator*			mMonths[12];
	CMonthIndicator*			mLastSelected;
	int32_t						mSubCellSize;

	virtual void 		Idle();

	virtual void		LDblClickCell(const STableCell& inCell, UINT nFlags);

	virtual void		DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);							// Draw the items

private:
			void		ResetFrame();
			
	CMonthIndicator*	GetIndicator(const STableCell& inCell);
	
			void		DoIndicatorSelectionChanged(CMonthIndicator* indicator);
			void		RespositionIndicators();

protected:

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);

	DECLARE_MESSAGE_MAP()
};

#endif
