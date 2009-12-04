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

#include "CSummaryPrintout.h"

#include "CCalendarView.h"
#include "CMonthIndicator.h"
#include "CPreferences.h"
#include "CPrintText.h"
#include "CSummaryTitleTable.h"
#include "CSummaryTable.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

// ---------------------------------------------------------------------------
//	CSummaryPrintout														  [public]
/**
	Default constructor */

CSummaryPrintout::CSummaryPrintout(LStream *inStream) :
	CCalendarViewPrintout(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CSummaryPrintout														  [public]
/**
	Destructor */

CSummaryPrintout::~CSummaryPrintout()
{
}

#pragma mark -

void CSummaryPrintout::FinishCreateSelf()
{
	// Do inherited
	CCalendarViewPrintout::FinishCreateSelf();

	// Get the UI objects
	mTitle = dynamic_cast<CPrintText*>(FindPaneByID(eTitle_ID));
	
	mThisMonth = dynamic_cast<CMonthIndicator*>(FindPaneByID(eThisMonth_ID));
	
	mView = dynamic_cast<LView*>(FindPaneByID(eView_ID));
	
	mTitles = dynamic_cast<CSummaryTitleTable*>(FindPaneByID(eTitleTable_ID));
	mTable = dynamic_cast<CSummaryTable*>(FindPaneByID(eTable_ID));
	mTable->SetPrinting();
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
	mTitles->SetPrinting();

	mTable->SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);
	
	short old_height = mTitles->GetRowHeight(1);

	// Change fonts
	mTitles->SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Get new row height
	short diff_height = mTitles->GetRowHeight(1) - old_height;

	// Change panes if row height changed
	if (diff_height)
	{
		// Offset and resize table
		mTable->MoveBy(0, diff_height, false);
		mTable->ResizeFrameBy(0, -diff_height, true);

		// Do header and button
		mTitles->ResizeFrameBy(0, diff_height, true);
	}
	
	// Install table as occupant of place holder so it drives the panel printing
	static_cast<LPlaceHolder*>(mPage)->InstallOccupant(mTable);
}

void CSummaryPrintout::SetDetails(const iCal::CICalendarDateTime& date, NCalendarView::ESummaryType type, NCalendarView::ESummaryRanges range, CCalendarView* view)
{
	// Set static text
	cdstring title;
	title += date.GetMonthText(false);
	
	char buf[256];
	std::snprintf(buf, 256, "%ld", date.GetYear());
	title += " ";
	title += buf;
	title += " (";
	title += date.GetTimezone().GetTimezoneID();
	title += ")";
	mTitle->SetText(title);

	// Set the indicator
	iCal::CICalendarDateTime temp(date);
	mThisMonth->ResetTable(temp, 75, true);

	// Date must be at start of day
	iCal::CICalendarDateTime dt(date);
	dt.SetHHMMSS(0, 0, 0);

	// Adjust date based on type
	mTable->ResetTable(dt);

	// Get date range for event lookup
	iCal::CICalendarDateTime dtstart(dt);
	dtstart.SetDateOnly(false);
	iCal::CICalendarDateTime dtend(dt);
	dtend.SetDateOnly(false);
	
	// Offset start/stop based on range
	switch(range)
	{
	case NCalendarView::eToday:
		// Just the current day
		dtend.OffsetDay(1);
		break;
	case NCalendarView::eDays:
		// Seven days either side of the current day
		dtstart.OffsetDay(-7);
		dtend.OffsetDay(8);
		break;
	case NCalendarView::eThisWeek:
		// This week - step back to beginning
		if (dtstart.GetDayOfWeek() != CPreferences::sPrefs->mWeekStartDay.GetValue())
			dtstart.OffsetDay((CPreferences::sPrefs->mWeekStartDay.GetValue() - dtstart.GetDayOfWeek() - 7) % 7);
		dtend = dtstart;
		dtend.OffsetDay(7);
		break;
	case NCalendarView::eWeeks:
		// This week - step back to beginning
		if (dtstart.GetDayOfWeek() != CPreferences::sPrefs->mWeekStartDay.GetValue())
			dtstart.OffsetDay((CPreferences::sPrefs->mWeekStartDay.GetValue() - dtstart.GetDayOfWeek() - 7) % 7);
		dtend = dtstart;
		dtend.OffsetDay(7);
		
		// Two weeks either side
		dtstart.OffsetDay(-2 * 7);
		dtend.OffsetDay(2 * 7);
		break;
	case NCalendarView::eThisMonth:
		// This month - step back to beginning
		dtstart.SetDay(1);
		dtend = dtstart;
		dtend.OffsetMonth(1);
		break;
	case NCalendarView::eMonths:
		// This month - step back to beginning
		dtstart.SetDay(1);
		dtend = dtstart;
		dtend.OffsetMonth(1);
		
		// Two months either side
		dtstart.OffsetMonth(-2);
		dtend.OffsetMonth(2);
		break;
	case NCalendarView::eThisYear:
		// This year - step back to beginning
		dtstart.SetDay(1);
		dtstart.SetMonth(1);
		dtend = dtstart;
		dtend.OffsetYear(1);
		break;
	case NCalendarView::eYears:
		// This year - step back to beginning
		dtstart.SetDay(1);
		dtstart.SetMonth(1);
		dtend = dtstart;
		dtend.OffsetYear(1);
		
		// Two years either side
		dtstart.OffsetYear(-2);
		dtend.OffsetYear(2);
		break;
	}
	
	// Get events in the range
	iCal::CICalendarExpandedComponents vevents;
	iCal::CICalendarPeriod period(dtstart, dtend);
	
	// Look for single calendar
	if (view->IsSingleCalendar())
	{
		// Get events
		if (view->GetCalendar() != NULL)
			view->GetCalendar()->GetVEvents(period, vevents, false);
	}

	// Only do visible calendars
	else if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			// Get events
			(*iter)->GetVEvents(period, vevents, false);
		}
	}
	
	// Add events to table
	mTable->AddEvents(vevents);
}
