/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CDayWeekPrintout.h"

#include "CCalendarView.h"
#include "CDayWeekTitleTable.h"
#include "CDayWeekTable.h"
#include "CMonthIndicator.h"
#include "CStaticText.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include <SysCFString.h>

// ---------------------------------------------------------------------------
//	CDayWeekPrintout														  [public]
/**
	Default constructor */

CDayWeekPrintout::CDayWeekPrintout(LStream *inStream) :
	CCalendarViewPrintout(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CDayWeekPrintout														  [public]
/**
	Destructor */

CDayWeekPrintout::~CDayWeekPrintout()
{
}

#pragma mark -

void CDayWeekPrintout::FinishCreateSelf()
{
	// Do inherited
	CCalendarViewPrintout::FinishCreateSelf();

	// Get the UI objects
	mMonth = dynamic_cast<CStaticText*>(FindPaneByID(eMonth_ID));
	mYear = dynamic_cast<CStaticText*>(FindPaneByID(eYear_ID));
	mTzid = dynamic_cast<CStaticText*>(FindPaneByID(eTimezone_ID));
	
	mThisMonth = dynamic_cast<CMonthIndicator*>(FindPaneByID(eThisMonth_ID));
	
	mView = dynamic_cast<LView*>(FindPaneByID(eView_ID));
	
	mTitles = dynamic_cast<CDayWeekTitleTable*>(FindPaneByID(eTitleTable_ID));
	mTable = dynamic_cast<CDayWeekTable*>(FindPaneByID(eTable_ID));
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
}

void CDayWeekPrintout::SetDetails(const iCal::CICalendarDateTime& date, CDayWeekView::EDayWeekType type, CDayWeekViewTimeRange::ERanges range, CCalendarView* view)
{
	// Set static text
	PPx::CFString month(date.GetMonthText(false), kCFStringEncodingUTF8);
	mMonth->SetCFDescriptor(month);
	
	char buf[256];
	std::snprintf(buf, 256, "%ld", date.GetYear());
	PPx::CFString year(buf, kCFStringEncodingUTF8);
	mYear->SetCFDescriptor(year);

	PPx::CFString zone(date.GetTimezone().GetTimezoneID(), kCFStringEncodingUTF8);
	mTzid->SetCFDescriptor(zone);

	// Set the indicator
	iCal::CICalendarDateTime temp(date);
	mThisMonth->ResetTable(temp, 75, true);

	// Date must be at start of day
	iCal::CICalendarDateTime dt(date);
	dt.SetHHMMSS(0, 0, 0);

	// Adjust date based on type
	int32_t num_days = 1;
	switch(type)
	{
	case CDayWeekView::eDay:
	default:
		// Nothing to do;
		break;
	case CDayWeekView::eWorkWeek:
		// Adjust date to the Monday at or before mDate
		if (dt.GetDayOfWeek() != iCal::CICalendarDateTime::eMonday)
			dt.OffsetDay(iCal::CICalendarDateTime::eMonday - dt.GetDayOfWeek());
		num_days = 5;
		break;
	case CDayWeekView::eWeek:
		// Adjust date to the Sunday at or before mDate
		if (dt.GetDayOfWeek() != iCal::CICalendarDateTime::eSunday)
			dt.OffsetDay(-dt.GetDayOfWeek());
		num_days = 7;
		break;
	}
	mTable->ResetTable(dt, type, range);

	// Get date range for event lookup
	iCal::CICalendarDateTime dtstart(dt);
	dtstart.SetDateOnly(false);
	iCal::CICalendarDateTime dtend(dt);
	dtend.SetDateOnly(false);
	dtend.OffsetDay(num_days);
	
	// Get events in the range
	iCal::CICalendarExpandedComponents vevents;
	iCal::CICalendarComponentList vfreebusy;
	iCal::CICalendarPeriod period(dtstart, dtend);
	
	// Only do visible calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			(*iter)->GetVEvents(period, vevents);
			(*iter)->GetVFreeBusy(period, vfreebusy);
		}
	}
	
	// Add events to table
	mTable->AddItems(vevents, vfreebusy);
}
