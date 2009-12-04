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

#ifndef H_CSummaryTable
#define H_CSummaryTable

#include "CCalendarTableBase.h"

#include "CSummaryEvent.h"

#include "CICalendarUtils.h"
#include "CICalendarVEvent.h"

#include <vector>

class CKeyModifiers;
class CSummaryTitleTable;

// ===========================================================================
//	CSummaryTable

class CSummaryTable : public CCalendarTableBase
{
public:
	enum ETitleTypes
	{
		eLink,
		eDate,
		eTime,
		eSummary,
		eStatus,
		eCalendar
	};
	typedef std::pair<ETitleTypes, uint32_t>	CColumnSpec;
	typedef std::vector<CColumnSpec> CColumnInfo;

			CSummaryTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
	virtual	~CSummaryTable();
	
	virtual void		OnCreate();

	void				SetTitles(CSummaryTitleTable* titles)
	{
		mTitles = titles;
	}
	const CColumnSpec&	GetColumnInfo(uint32_t index) const
	{
		return mColumnInfo[index];
	}

	void				ResetTable(iCal::CICalendarDateTime& date);
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);
	virtual void		ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									bool inRefresh);		// Keep titles in sync

	void				AddEvents(iCal::CICalendarExpandedComponents& vevents);

	const iCal::CICalendarDateTime&	GetCellStartDate(const STableCell& cell) const;

	virtual void		GetTooltipText(cdstring& txt, const STableCell& cell);

protected:
	CSummaryTitleTable*				mTitles;
	CColumnInfo						mColumnInfo;
	iCal::CICalendarDateTime		mDate;
	CSummaryEventList				mEvents;

			void		ResetColumns();

	virtual void		LClickCell(const STableCell& inCell, const JXKeyModifiers& mods);
	virtual void		LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods);

	virtual void		DoSelectionChanged();
	virtual void		DoSingleClick(const STableCell& cell, const CKeyModifiers& mods);					// Single click on a message
	virtual void		DoDoubleClick(const STableCell& cell, const CKeyModifiers& mods);					// Double click on a message

	virtual void		DrawCellRange(JPainter* pDC,
								const STableCell&	inTopLeftCell,
								const STableCell&	inBottomRightCell);
	
	virtual void		DrawRow(JPainter* pDC, TableIndexT row, const JRect& inLocalRect);
	virtual void		DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect);

	virtual void		HiliteCellActively(
								const STableCell&		inCell,
								bool					inHilite);
								
	virtual void		HiliteCellInactively(
								const STableCell&		inCell,
								bool					inHilite);

	virtual void		SelectEvent(bool next);
};

#endif
