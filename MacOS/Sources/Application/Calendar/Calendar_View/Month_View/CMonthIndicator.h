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

#include "CTableDrag.h"

#include "CICalendarUtils.h"

// ===========================================================================
//	CMonthIndicator

class	CMonthIndicator : public CTableDrag
{
public:
	enum { class_ID = 'Mind' };

	enum
	{
		eBroadcast_DblClkMonth = 'DblM',		// param = iCal::CICalendarDateTime*
		eBroadcast_DblClkDay = 'DblD',			// param = iCal::CICalendarDateTime*
		eBroadcast_SelectionChanged = 'SelC'	// param = CMonthIndicator*
	};

	static CMonthIndicator* Create(LView* parent, const HIRect& frame);

						CMonthIndicator(const SPaneInfo &inPaneInfo,
										const SViewInfo &inViewInfo);
						CMonthIndicator(LStream* inStream);
						
	virtual				~CMonthIndicator();
	
	void				ResetTable(const iCal::CICalendarDateTime date, uint32_t width = 0, bool printing = false);

	iCal::CICalendarDateTime	GetCellDate(const STableCell& cell) const;
	bool				IsTodayCell(const STableCell& cell) const
		{ return cell == mTodayCell; }

			void		SelectDate(const iCal::CICalendarDateTime date);
	virtual void		SelectionChanged();

	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);

protected:
	iCal::CICalendarDateTime				mDate;
	iCal::CICalendarUtils::CICalendarTable	mData;
	STableCell	 							mTodayCell;
	bool									mPrinting;


	virtual void		ClickSelf(const SMouseDownEvent &inMouseDown);	// Click
	virtual void		ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown);

	virtual void		DrawCell(const STableCell &inCell, const Rect &inLocalRect);
			void		DrawCellScreen(const STableCell &inCell, const Rect &inLocalRect);
			void		DrawCellPrint(const STableCell &inCell, const Rect &inLocalRect);
	virtual bool		DrawCellSelection(const STableCell& inCell, CGContextRef inContext);
};

#endif
