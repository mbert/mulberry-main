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

#include "CDayWeekPrintout.h"

#include "CCalendarView.h"
#include "CDayWeekTitleTable.h"
#include "CDayWeekTable.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include <PPxPane.h>
#include <PPxStaticText.h>
#include <PPxViewUtils.h>

// ---------------------------------------------------------------------------
//	CDayWeekPrintout														  [public]
/**
	Default constructor */

CDayWeekPrintout::CDayWeekPrintout()
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

// ---------------------------------------------------------------------------
//	ClassName														 [private]
/**
	Returns the name of the class
	
	@return	Name of the class												*/

CFStringRef
CDayWeekPrintout::ClassName() const
{
	return CFSTR("CDayWeekPrintout");
}

void CDayWeekPrintout::FinishInit()
{
	// Get the UI objects
	mMonth = dynamic_cast<PPx::StaticText*>(FindViewByID(eMonth_ID));
	mYear = dynamic_cast<PPx::StaticText*>(FindViewByID(eYear_ID));
	mTzid = dynamic_cast<PPx::StaticText*>(FindViewByID(eTimezone_ID));
	
	mView = dynamic_cast<PPx::PPxScrollable*>(FindViewByID(eView_ID));
	
	mTitles = dynamic_cast<CDayWeekTitleTable*>(FindViewByID(eTitleTable_ID));
	mTable = dynamic_cast<CDayWeekTable*>(FindViewByID(eTable_ID));
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
}

void CDayWeekPrintout::SetDetails(const iCal::CICalendarDateTime& date, CDayWeekView::EDayWeekType type, CDayWeekViewTimeRange::ERanges range, CCalendarView* view)
{
	// Set static text
	PPx::CFString month(date.GetMonthText(false));
	mMonth->SetText(month);
	
	char buf[256];
	std::snprintf(buf, 256, "%ld", date.GetYear());
	PPx::CFString year(buf);
	mYear->SetText(year);

	PPx::CFString zone(date.GetTimezone().GetTimezoneID(), kCFStringEncodingUTF8);
	mTzid->SetText(zone);

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
	iCal::CICalendarVEventList vevents;
	iCal::CICalendarPeriod period(dtstart, dtend);
	
	// Only do visible calendars
	const calstore::CCalendarStoreNodeList& nodes = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedNodes();
	for(calstore::CCalendarStoreNodeList::const_iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		if ((*iter)->IsActive())
		{
			const iCal::CICalendar* cal = (*iter)->GetCalendar();
			if (cal != NULL)
				cal->GetVEvents(period, vevents);
		}
	}
	
	// Add events to table
	mTable->AddEvents(vevents);
}
