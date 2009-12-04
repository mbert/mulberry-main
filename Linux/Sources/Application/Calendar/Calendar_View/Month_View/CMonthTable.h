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

#ifndef H_CMONTHTABLE
#define H_CMONTHTABLE

#include "CCalendarEventTableBase.h"

#include "CICalendarUtils.h"
#include "CICalendarVEvent.h"
#include "CICalendarComponentExpanded.h"

class CMonthEvent;
typedef std::vector<std::pair<CMonthEvent*, bool> > CMonthEventSlotList;
typedef std::vector<CMonthEventSlotList> CMonthEventMatrixRow;
typedef std::vector<CMonthEventMatrixRow> CMonthEventMatrix;

// ===========================================================================
//	CMonthTable

class	CMonthTable : public CCalendarEventTableBase
{
public:
			CMonthTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
						
	virtual	~CMonthTable();

	virtual void		OnCreate();

	void				ResetTable(const iCal::CICalendarUtils::CICalendarTable& table, const std::pair<int32_t, int32_t>& today_index, const iCal::CICalendarTimezone& timezone);
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

	void				AddEvents(iCal::CICalendarExpandedComponents& vevents);
	void				AddEvent(iCal::CICalendarComponentExpandedShared& vevent);

	const iCal::CICalendarDateTime&	GetCellStartDate(const STableCell& cell) const;
	const iCal::CICalendarDateTime&	GetCellEndDate(const STableCell& cell) const;
	bool				IsTodayCell(const STableCell& cell) const
		{ return cell == mTodayCell; }

protected:
	iCal::CICalendarUtils::CICalendarTable	mData;
	STableCell		 						mTodayCell;
	CMonthEventMatrix						mEvents;
	iCal::CICalendarDateTimeList			mCellDates;

	virtual void 		Idle();
			void		ResetToday();

	virtual void		LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods);

	virtual void		DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect);
	virtual bool		DrawCellSelection(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect);

	virtual void		SelectEvent(bool next);
	virtual void		SelectDay();

private:
			void	CacheCellDates();
			void	CheckEventsNow();

			void	InitEvents(TableIndexT rows, TableIndexT cols);
			void	RepositionEvents();
			void	PositionEvent(CMonthEvent* event, const STableCell& cell, size_t offset);
			void	ClearEvents();
};

#endif
