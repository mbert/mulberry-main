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

#include "CTimezonePopup.h"

#include "MyCFString.h"

#include "CICalendar.h"
#include "CICalendarManager.h"

// ---------------------------------------------------------------------------
//	CTimezonePopup														  [public]
/**
	Default constructor */

CTimezonePopup::CTimezonePopup(LStream* inStream) :
	LPopupButton(inStream)
{
	mNoFloating = false;
}


// ---------------------------------------------------------------------------
//	~CTimezonePopup														  [public]
/**
	Destructor */

CTimezonePopup::~CTimezonePopup()
{
}

#pragma mark -

void CTimezonePopup::FinishCreateSelf()
{
	// Init the popup menu

	// Get list of timezones from built-in calendar
	cdstrvect tzids;
	iCal::CICalendar::sICalendar.GetTimezones(tzids);
	
	// Clear existing menu
	MenuRef menu = GetMacMenuH();
	if (::CountMenuItems(menu) > eSeparator)
		::DeleteMenuItems(menu, eFirstTimezone, ::CountMenuItems(menu) - eSeparator);

	// Add to menu
	for(cdstrvect::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
	{
		// Make a CFString from UTF8 data
		MyCFString menu_title((*iter).c_str(), kCFStringEncodingUTF8);
		::AppendMenuItemTextWithCFString(menu, menu_title, 0, 0, NULL);
	}
	
	// Readjust max
	SetMaxValue(::CountMenuItems(menu));

	// Always start with the current user default
	SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
}

void CTimezonePopup::NoFloating()
{
	// Remove first menu item
	MenuRef menu = GetMacMenuH();
	::DeleteMenuItems(menu, eNoTimezone, 1);
	mNoFloating = true;
	
	// Always start with the current user default
	SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
}

void CTimezonePopup::SetTimezone(const iCal::CICalendarTimezone& tz)
{
	// Extract zone from dt and apply to menu
	SInt32 new_value;
	if (tz.GetUTC())
	{
		new_value = eUTC - (mNoFloating ? 1 : 0);
	}
	else
	{
		// Always set no timezone (or UTC when floating) so that if named timezone is not found we have a default
		new_value = eNoTimezone;

		cdstring tzid(tz.GetTimezoneID());
		if (!tzid.empty())
		{
			// Start with no timezone in 
			// Scan menu looking for match
			MenuRef menu = GetMacMenuH();
			MyCFString comp(tzid.c_str(), kCFStringEncodingUTF8);
			for(UInt32 i = eFirstTimezone - (mNoFloating ? 1 : 0); i <= ::CountMenuItems(menu); i++)
			{
				// Get name of menu title
				CFStringRef txt;
				::CopyMenuItemTextAsCFString(menu, i, &txt);
				MyCFString temp(txt, false);
				
				// Compare with tzid
				if (comp.CompareTo(temp, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
				{
					new_value = i;
					break;
				}
			}
		}
	}
	
	// Set value only if different
	if (new_value != GetValue())
		SetValue(new_value);
}

void CTimezonePopup::GetTimezone(iCal::CICalendarTimezone& tz) const
{
	SInt32 value = GetValue();
	tz.SetUTC(value + (mNoFloating ? 1 : 0) == eUTC);
	if ((value == eNoTimezone) || (value == eUTC))
	{
		cdstring empty;
		tz.SetTimezoneID(empty);
	}
	else
	{
		// Get name of menu title
		CFStringRef txt;
		::CopyMenuItemTextAsCFString(GetMacMenuH(), value, &txt);
		MyCFString temp(txt, false);

		cdstring tzid = temp.GetString();

		tz.SetTimezoneID(tzid);
	}
}
