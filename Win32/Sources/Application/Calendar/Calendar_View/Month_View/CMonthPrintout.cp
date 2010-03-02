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

#include "CMonthPrintout.h"

#include "CCalendarView.h"
#include "CMonthIndicator.h"
#include "CMonthTable.h"
#include "CPreferences.h"
#include "CStaticText.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

#include <SysCFString.h>

// ---------------------------------------------------------------------------
//	CMonthPrintout														  [public]
/**
	Default constructor */

CMonthPrintout::CMonthPrintout(LStream *inStream) :
	CCalendarViewPrintout(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CMonthPrintout														  [public]
/**
	Destructor */

CMonthPrintout::~CMonthPrintout()
{
}

#pragma mark -

void CMonthPrintout::FinishCreateSelf()
{
	// Do inherited
	CCalendarViewPrintout::FinishCreateSelf();

	// Get the UI objects
	mMonth = dynamic_cast<CStaticText*>(FindPaneByID(eMonth_ID));
	mYear = dynamic_cast<CStaticText*>(FindPaneByID(eYear_ID));
	mTzid = dynamic_cast<CStaticText*>(FindPaneByID(eTimezone_ID));
	
	mPrev = dynamic_cast<CMonthIndicator*>(FindPaneByID(ePreviousMonth_ID));
	mNext = dynamic_cast<CMonthIndicator*>(FindPaneByID(eNextMonth_ID));
	
	mView = dynamic_cast<LView*>(FindPaneByID(eView_ID));
	mTable = dynamic_cast<CMonthTable*>(FindPaneByID(eTable_ID));
}

void CMonthPrintout::SetDetails(const iCal::CICalendarDateTime& date, CCalendarView* view)
{
	// Resize based on orientation
	SDimension16 frame;
	GetFrameSize(frame);
	SDimension16 vframe;
	mView->GetFrameSize(vframe);
	SInt32 x_offset = 0;
	if (frame.height > frame.width)
	{
		// Full width, height is 2/3 width
		vframe.height = vframe.width * 2.0 / 3.0;
	}
	else if (vframe.width >= vframe.height * 3.0 / 2.0)
	{
		// Full height, width is 3/2 height plus left offset to center it
		vframe.width = vframe.height * 3.0 / 2.0;
		x_offset = (frame.width - vframe.width) / 2.0;
	}
	mView->ResizeFrameTo(vframe.width, vframe.height, false);
	if (x_offset != 0)
		mView->MoveBy(x_offset, 0, false);

	// Set static text
	PPx::CFString month(date.GetMonthText(false), kCFStringEncodingUTF8);
	mMonth->SetCFDescriptor(month);
	
	char buf[256];
	std::snprintf(buf, 256, "%ld", date.GetYear());
	PPx::CFString year(buf, kCFStringEncodingUTF8);
	mYear->SetCFDescriptor(year);

	PPx::CFString zone(date.GetTimezone().GetTimezoneID(), kCFStringEncodingUTF8);
	mTzid->SetCFDescriptor(zone);

	// Set the two indicators
	iCal::CICalendarDateTime temp(date);
	temp.OffsetMonth(-1);
	mPrev->ResetTable(temp, 75, true);
	temp.OffsetMonth(2);
	mNext->ResetTable(temp, 75, true);

	// Now generate new month date table
	iCal::CICalendarUtils::CICalendarTable monthDays;
	std::pair<int32_t, int32_t> today_index;
	iCal::CICalendarUtils::GetMonthTable(date.GetMonth(), date.GetYear(), CPreferences::sPrefs->mWeekStartDay.GetValue(), monthDays, today_index);
	
	// Reset table
	mTable->ResetTable(monthDays, today_index, date.GetTimezone());

	// Get date range for event lookup
	int32_t packed_start = monthDays[0][0];
	int32_t packed_end = monthDays[monthDays.size() - 1][6];

	iCal::CICalendarDateTime dtstart(packed_start);
	dtstart.SetDateOnly(false);
	dtstart.SetTimezone(date.GetTimezone());
	iCal::CICalendarDateTime dtend(packed_end);
	dtend.SetDateOnly(false);
	dtend.SetTimezone(date.GetTimezone());
	dtend.OffsetDay(1);
	
	// Get events in the range
	iCal::CICalendarExpandedComponents vevents;
	iCal::CICalendarPeriod period(dtstart, dtend);
	
	// Only do visible calendars
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			(*iter)->GetVEvents(period, vevents);
		}
	}
	
	// Add events to table
	mTable->AddEvents(vevents);
}
