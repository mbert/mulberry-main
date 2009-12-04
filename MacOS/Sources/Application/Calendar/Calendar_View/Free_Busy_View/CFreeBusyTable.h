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

#ifndef H_CFreeBusyTable
#define H_CFreeBusyTable
#pragma once

#include "CCalendarEventTableBase.h"

#include "CFreeBusyView.h"
#include "CDayWeekViewTimeRange.h"

#include "CCalendarStoreFreeBusy.h"

#include <vector>

class CFreeBusyTitleTable;

// ===========================================================================
//	CFreeBusyTable

class CFreeBusyTable : public CCalendarEventTableBase
{
public:
	enum { class_ID = 'CtbF' };

						CFreeBusyTable(LStream *inStream);
	virtual				~CFreeBusyTable();
	
	void				SetTitles(CFreeBusyTitleTable* titles)
	{
		mTitles = titles;
	}

	void				ResetTable(iCal::CICalendarDateTime& date, CDayWeekViewTimeRange::ERanges range);
	virtual void		AdaptToNewSurroundings();					// Adjust column widths
	virtual void		ScrollImageBy(SInt32 inLeftDelta,
									SInt32 inTopDelta,
									Boolean inRefresh);				// Keep titles in sync

	void				ScaleColumns(uint32_t scale);

	void				AddItems(const calstore::CCalendarStoreFreeBusyList& list);

	const iCal::CICalendarDateTime&	GetTimedStartDate(const TableIndexT& col) const;
	const iCal::CICalendarDateTime&	GetTimedEndDate(const TableIndexT& col) const;

protected:
	struct SFreeBusyInfo
	{
		typedef std::pair<iCal::CICalendarFreeBusy::EBusyType, uint32_t>	SFreeBusyPeriod;

		cdstring 						mName;
		std::vector<SFreeBusyPeriod>			mPeriods;
		
	};
	std::vector<SFreeBusyInfo> mItems;

	CFreeBusyTitleTable*			mTitles;
	iCal::CICalendarDateTime		mDate;
	uint32_t						mColumnSeconds;
	uint32_t						mScaleColumns;
	uint32_t						mStartHour;
	uint32_t						mEndHour;
	iCal::CICalendarDateTimeList	mColumnDates;

	virtual void		FinishCreateSelf();

	virtual void		DrawCellRange(
								const STableCell&	inTopLeftCell,
								const STableCell&	inBottomRightCell);
	virtual void		DrawRow(TableIndexT row,
									const Rect &inLocalRect);
	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

private:


			void	AddItem(const calstore::CCalendarStoreFreeBusy& item);
			void	CacheCellDates();
			void	RefreshNow();

			void	RescaleWidth();
};

#endif
