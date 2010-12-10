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

#include "CPreferences.h"
#include "CTextListChoice.h"

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

	// Always start with the current user default
	SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
}

void CTimezonePopup::Reset(const iCal::CICalendarTimezone& tz)
{
	// Get the set of favorite timezones and always include the default
	cdstrvect tzids;
	cdstrset menutzids(CPreferences::sPrefs->mFavouriteTimezones.GetValue());
	menutzids.insert(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezoneID());
	menutzids.insert(tz.GetTimezoneID());
	for(cdstrset::const_iterator iter = menutzids.begin(); iter != menutzids.end(); iter++)
		tzids.push_back(*iter);
	
	// Sort list of favorite timezones
	iCal::CICalendar::sICalendar.SortTimezones(tzids);
	
	// Clear existing menu
	SInt32 adjusted_separator = eSeparator - (mNoFloating ? 1 : 0);
	SInt32 adjusted_first = eFirstTimezone - (mNoFloating ? 1 : 0);
	MenuRef menu = GetMacMenuH();
	if (::CountMenuItems(menu) > adjusted_separator)
		::DeleteMenuItems(menu, adjusted_first, ::CountMenuItems(menu) - adjusted_separator);
	
	// Add to menu
	for(cdstrvect::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
	{
		// Make a CFString from UTF8 data
		MyCFString menu_title((*iter).c_str(), kCFStringEncodingUTF8);
		::AppendMenuItemTextWithCFString(menu, menu_title, 0, 0, NULL);
	}
	
	// Readjust max
	SetMaxValue(::CountMenuItems(menu));
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
	Reset(tz);

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
	{
		SetValue(new_value);
		mOldValue = new_value;
	}
}

void CTimezonePopup::GetTimezone(iCal::CICalendarTimezone& tz) const
{
	SInt32 value = GetValue();
	SInt32 adjusted_value = value + (mNoFloating ? 1 : 0);
	tz.SetUTC(adjusted_value == eUTC);
	if ((adjusted_value == eNoTimezone) || (adjusted_value == eUTC))
	{
		cdstring empty;
		tz.SetTimezoneID(empty);
	}
	else if (adjusted_value == eOther)
	{
		cdstrvect tzids;
		iCal::CICalendar::sICalendar.GetTimezones(tzids);
		std::sort(tzids.begin(), tzids.end());
		
		ulvector selected;
		if (CTextListChoice::PoseDialog(
			"Alerts::Calendar::TimezoneChoice::Title",
			"Alerts::Calendar::TimezoneChoice::Description",
			NULL, false, true, false, true, tzids, cdstring::null_str, selected,
			"Alerts::Calendar::TimezoneChoice::Button"))
		{
			// Get selection from list
			cdstring tzid = tzids.at(selected.front());
			tz.SetTimezoneID(tzid);
			CPreferences::sPrefs->mFavouriteTimezones.Value().insert(tzid);
			const_cast<CTimezonePopup*>(this)->Reset(iCal::CICalendar::sICalendar.GetTimezone(tzid));
			const_cast<CTimezonePopup*>(this)->SetTimezone(iCal::CICalendar::sICalendar.GetTimezone(tzid));
		}
		else
		{
			const_cast<CTimezonePopup*>(this)->SetValue(mOldValue);
		}
	}
	else
	{
		// Get name of menu title
		CFStringRef txt;
		::CopyMenuItemTextAsCFString(GetMacMenuH(), value, &txt);
		MyCFString temp(txt, false);

		cdstring tzid = temp.GetString();

		tz.SetTimezoneID(tzid);
		mOldValue = value;
	}
}
