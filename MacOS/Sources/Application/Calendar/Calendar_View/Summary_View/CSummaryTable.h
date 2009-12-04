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
#pragma once

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
	enum { class_ID = 'CtbS' };

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
	typedef std::vector<CColumnSpec>			CColumnInfo;

						CSummaryTable(LStream *inStream);
	virtual				~CSummaryTable();
	
	void				SetTitles(CSummaryTitleTable* titles)
	{
		mTitles = titles;
	}
	const CColumnSpec&	GetColumnInfo(uint32_t index) const
	{
		return mColumnInfo[index];
	}
	void	SetPrinting()
	{
		mPrinting = true;
	}

	void				ResetTable(iCal::CICalendarDateTime& date);
	virtual void		AdaptToNewSurroundings();					// Adjust column widths
	virtual void		ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);		// Keep titles in sync

	void				AddEvents(iCal::CICalendarExpandedComponents& vevents);

	const iCal::CICalendarDateTime&	GetCellStartDate(const STableCell& cell) const;

	virtual void		GetTooltipText(cdstring& txt, const STableCell& cell);

protected:
	CSummaryTitleTable*				mTitles;
	CColumnInfo						mColumnInfo;
	iCal::CICalendarDateTime		mDate;
	CSummaryEventList				mEvents;
	bool							mPrinting;

	virtual void		FinishCreateSelf();
			void		ResetColumns();

	virtual void		ClickCell(const STableCell& inCell,
									const SMouseDownEvent& inMouseDown);

	virtual void		DoSelectionChanged();
	virtual void		DoSingleClick(const STableCell& cell, const CKeyModifiers& mods);					// Single click on a message
	virtual void		DoDoubleClick(const STableCell& cell, const CKeyModifiers& mods);					// Double click on a message

	virtual void		DrawCellRange(
								const STableCell&	inTopLeftCell,
								const STableCell&	inBottomRightCell);
	
	virtual void		DrawRow(TableIndexT row,
									const Rect &inLocalRect);
	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual void		HiliteCellActively(const STableCell &inCell,
											Boolean inHilite);
	virtual void		HiliteCellInactively(const STableCell &inCell,
											Boolean inHilite);

	virtual void		SelectEvent(bool next);
};

#endif
