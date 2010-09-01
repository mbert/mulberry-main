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

#include "CCalendarEventBase.h"

#include "CActionManager.h"
#include "CCalendarTableBase.h"
#include "CCalendarViewBase.h"
#include "CErrorDialog.h"
#include "CListener.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarLocale.h"
#include "CICalendarUtils.h"
#include "CICalendarVFreeBusy.h"
#include "CITIPProcessor.h"

#include <strstream>

void CCalendarEventBase::CheckNow(iCal::CICalendarDateTime& dt)
{
	if (mAllDay)
		return;

	bool is_now = IsNow();
	if (is_now ^ mIsNow)
	{
		mIsNow = is_now;
		FRAMEWORK_REFRESH_WINDOW(this)
	}
}

bool CCalendarEventBase::IsNow() const
{
	// Check instance start/end against current date-time (also do not highlight if cancelled)
	iCal::CICalendarDateTime now = iCal::CICalendarDateTime::GetNow(&mTable->GetTimezone());
	return !mIsCancelled && mPeriod.IsDateWithinPeriod(now);
}

void CCalendarEventBase::SetupTagText()
{
	if (IsEvent())
		SetupTagTextEvent();
	else if (IsFreeBusy())
		SetupTagTextFreeBusy();
}

void CCalendarEventBase::SetupTagTextEvent()
{
	// Setup a help tag;
	std::ostrstream ostr;
	ostr << rsrc::GetString("EventTip::Summary") << mVEvent->GetMaster<iCal::CICalendarVEvent>()->GetSummary();
	if (mVEvent->GetInstanceStart().IsDateOnly())
	{
		ostr << std::endl << rsrc::GetString("EventTip::All Day Event");
	}
	else
	{
		ostr << std::endl << rsrc::GetString("EventTip::Starts on") << mVEvent->GetInstanceStart().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
		ostr << std::endl << rsrc::GetString("EventTip::Ends on") << mVEvent->GetInstanceEnd().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
	}
	if (!mVEvent->GetMaster<iCal::CICalendarVEvent>()->GetLocation().empty())
		ostr << std::endl << rsrc::GetString("EventTip::Location") << mVEvent->GetMaster<iCal::CICalendarVEvent>()->GetLocation();
	if (!mVEvent->GetMaster<iCal::CICalendarVEvent>()->GetDescription().empty())
		ostr << std::endl << rsrc::GetString("EventTip::Description") << mVEvent->GetMaster<iCal::CICalendarVEvent>()->GetDescription();
	
	// Always add calendar name
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVEvent->GetMaster<iCal::CICalendarVEvent>()->GetCalendar());
	if (cal != NULL)
	{
		const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(cal);
		cdstring name = node->GetAccountDisplayName(calstore::CCalendarStoreManager::sCalendarStoreManager->HasMultipleProtocols());
		ostr << std::endl << std::endl << rsrc::GetString("EventTip::Calendar") << name;
	}

	ostr << std::ends;
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	SetTagText(ostr.str());

#elif __dest_os == __win32_os

	mTooltipText = ostr.str();

#elif __dest_os == __linux_os

	SetHint(ostr.str());

#endif
	ostr.freeze(false);
}

void CCalendarEventBase::SetupTagTextFreeBusy()
{
	// Setup a help tag;
	std::ostrstream ostr;
	ostr << rsrc::GetString("EventTip::Summary") << rsrc::GetString("CDayWeekTable::BlockedBusy");
	ostr << std::endl << rsrc::GetString("EventTip::Starts on") << mPeriod.GetStart().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
	ostr << std::endl << rsrc::GetString("EventTip::Ends on") <<  mPeriod.GetEnd().GetAdjustedTime().GetTime(false, !iCal::CICalendarLocale::Use24HourTime());
	
	// Always add calendar name
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVFreeBusy->GetCalendar());
	if (cal != NULL)
		ostr << std::endl << std::endl << rsrc::GetString("EventTip::Calendar") << cal->GetName();

	ostr << std::ends;
#if __dest_os == __mac_os || __dest_os == __mac_os_x

	SetTagText(ostr.str());

#elif __dest_os == __win32_os

	mTooltipText = ostr.str();

#elif __dest_os == __linux_os

	SetHint(ostr.str());

#endif
	ostr.freeze(false);
}
