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

#ifndef H_CMonthIndicator
#define H_CMonthIndicator
#pragma once

#include "CTable.h"

#include "CICalendarUtils.h"

// ===========================================================================
//	CMonthIndicator

class CMonthIndicator : public CTable
{
public:
	enum
	{
		eBroadcast_DblClkMonth = 'DblM',		// param = iCal::CICalendarDateTime*
		eBroadcast_DblClkDay = 'DblD',			// param = iCal::CICalendarDateTime*
		eBroadcast_SelectionChanged = 'SelC'	// param = CMonthIndicator*
	};

	static CMonthIndicator* Create(CWnd* parent, const CRect& frame);

						CMonthIndicator();
	virtual				~CMonthIndicator();
	
	void				ResetTable(const iCal::CICalendarDateTime date, uint32_t width = 0, bool printing = false);

	iCal::CICalendarDateTime	GetCellDate(const STableCell& cell) const;
	bool				IsTodayCell(const STableCell& cell) const
		{ return cell == mTodayCell; }

			void		SelectDate(const iCal::CICalendarDateTime date);
	virtual void		SelectionChanged();

protected:
	iCal::CICalendarDateTime				mDate;
	iCal::CICalendarUtils::CICalendarTable	mData;
	STableCell	 							mTodayCell;
	bool									mPrinting;


	virtual	bool		HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down

	virtual void		LDblClickCell(const STableCell& inCell, UINT nFlags);

	virtual void		DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect);							// Draw the items
			void		DrawCellScreen(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect);
			void		DrawCellPrint(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect);
	virtual bool		DrawCellSelection(CDC* pDC, const STableCell& inCell);
};

#endif
