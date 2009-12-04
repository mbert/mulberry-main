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

// ===========================================================================
//	PPxStaticText.cp			PowerPlantX 1.0			©2003 Metrowerks Corp.
// ===========================================================================

#include "CFreeBusyPrintout.h"

#include "CCalendarView.h"
#include "CFreeBusyTitleTable.h"
#include "CFreeBusyTable.h"
#include "CMonthIndicator.h"
#include "CPrintText.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarPeriod.h"
#include "CICalendarUtils.h"

// ---------------------------------------------------------------------------
//	CFreeBusyPrintout														  [public]
/**
	Default constructor */

CFreeBusyPrintout::CFreeBusyPrintout(LStream *inStream) :
	CCalendarViewPrintout(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CFreeBusyPrintout														  [public]
/**
	Destructor */

CFreeBusyPrintout::~CFreeBusyPrintout()
{
}

#pragma mark -

void CFreeBusyPrintout::FinishCreateSelf()
{
	// Do inherited
	CCalendarViewPrintout::FinishCreateSelf();

	// Get the UI objects
	mTitle = dynamic_cast<CPrintText*>(FindPaneByID(eTitle_ID));
	
	mThisMonth = dynamic_cast<CMonthIndicator*>(FindPaneByID(eThisMonth_ID));
	
	mView = dynamic_cast<LView*>(FindPaneByID(eView_ID));
	
	mTitles = dynamic_cast<CFreeBusyTitleTable*>(FindPaneByID(eTitleTable_ID));
	mTable = dynamic_cast<CFreeBusyTable*>(FindPaneByID(eTable_ID));
	mTable->SetTitles(mTitles);
	mTitles->SetTable(mTable);
}

void CFreeBusyPrintout::SetDetails(const iCal::CICalendarDateTime& date, CDayWeekViewTimeRange::ERanges range, CCalendarView* view)
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
	int32_t num_days = 1;
	mTable->ResetTable(dt, range);

	// Get date range for event lookup
	iCal::CICalendarDateTime dtstart(dt);
	dtstart.SetDateOnly(false);
	iCal::CICalendarDateTime dtend(dt);
	dtend.SetDateOnly(false);
	dtend.OffsetDay(num_days);
}
