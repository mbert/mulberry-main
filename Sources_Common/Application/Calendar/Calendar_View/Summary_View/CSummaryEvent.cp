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

#include "CSummaryEvent.h"

#include "CCalendarTableBase.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"

#include "CICalendar.h"
#include "CICalendarLocale.h"
#include "CICalendarVEvent.h"

#include <strstream>

// ---------------------------------------------------------------------------
//	CSummaryEvent														  [public]
/**
	Default constructor */

CSummaryEvent::CSummaryEvent(EType type, const iCal::CICalendarComponentExpandedShared& vevent, bool start, bool end)
{
	mVEvent = vevent;
	InitSummary(type, vevent, start, end);
}

CSummaryEvent::CSummaryEvent(EType type, uint32_t value)
{
	InitSummary(type, value);
}


// ---------------------------------------------------------------------------
//	~CSummaryEvent														  [public]
/**
	Destructor */

CSummaryEvent::~CSummaryEvent()
{
}

#pragma mark -

void CSummaryEvent::InitSummary(EType type, const iCal::CICalendarComponentExpandedShared& vevent, bool start, bool end)
{
	mType = type;
	mIsEvent = true;
	mSortDT = vevent->GetInstanceStart();
	mDayStart = start;
	mDayEnd = end;
	mStartDate = mSortDT.GetAdjustedTime().GetLocaleDate(iCal::CICalendarDateTime::eNumericDate);
	if (mSortDT.IsDateOnly())
		mStartTime = rsrc::GetString("CSummaryEvent::AllDay");
	else
		mStartTime = mSortDT.GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
	
	mSummary = vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
	
	const iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(vevent->GetMaster<iCal::CICalendarVEvent>()->GetCalendar());
	if (cal)
	{
		mCalendar = cal->GetName();
		mColour = calstore::CCalendarStoreManager::sCalendarStoreManager->GetCalendarColour(cal);
	}
	else
		mColour = 0xFFFFFFFF;
	
		// Setup a help tag;
	std::ostrstream ostr;
	ostr << rsrc::GetString("EventTip::Summary") << vevent->GetMaster<iCal::CICalendarVEvent>()->GetSummary() << std::endl;
	if (vevent->GetInstanceStart().IsDateOnly())
	{
		ostr << rsrc::GetString("EventTip::Starts on") << vevent->GetInstanceStart().GetAdjustedTime().GetLocaleDateTime(iCal::CICalendarDateTime::eNumericDate, false, !iCal::CICalendarLocale::Use24HourTime()) << std::endl;
		ostr << rsrc::GetString("EventTip::All Day Event") << std::endl;
	}
	else
	{
		ostr << rsrc::GetString("EventTip::Starts on") << vevent->GetInstanceStart().GetAdjustedTime().GetLocaleDateTime(iCal::CICalendarDateTime::eNumericDate, false, !iCal::CICalendarLocale::Use24HourTime()) << std::endl;
		ostr << rsrc::GetString("EventTip::Ends on") << vevent->GetInstanceEnd().GetAdjustedTime().GetLocaleDateTime(iCal::CICalendarDateTime::eNumericDate, false, !iCal::CICalendarLocale::Use24HourTime()) << std::endl;
	}
	if (!vevent->GetMaster<iCal::CICalendarVEvent>()->GetLocation().empty())
		ostr << rsrc::GetString("EventTip::Location") << vevent->GetMaster<iCal::CICalendarVEvent>()->GetLocation() << std::endl;
	if (!vevent->GetMaster<iCal::CICalendarVEvent>()->GetDescription().empty())
		ostr << rsrc::GetString("EventTip::Description") << vevent->GetMaster<iCal::CICalendarVEvent>()->GetDescription() << std::endl;
	
	// Add calendar name if more than one calendar in use
	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (cals.size() > 1)
	{
		iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(vevent->GetMaster<iCal::CICalendarVEvent>()->GetCalendar());
		if (cal != NULL)
			ostr << std::endl << rsrc::GetString("EventTip::Calendar") << cal->GetName() << std::endl;
	}
	
	ostr << std::ends;
	mTooltip.steal(ostr.str());
	
	mCancelled = (vevent->GetMaster<iCal::CICalendarVEvent>()->GetStatus() == iCal::eStatus_VEvent_Cancelled);
}

void CSummaryEvent::InitSummary(EType type, uint32_t value)
{
	mType = type;
	mIsEvent = false;

	mDayStart = true;
	mDayEnd = true;
	mSummary = rsrc::GetIndexedString("CSummaryEvent::When", type);
	mColour = 0xFFFFFFFF;
	mCancelled = false;
}
