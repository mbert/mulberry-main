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

#include "CICalendar.h"
#include "CICalendarManager.h"

// ---------------------------------------------------------------------------
//	CTimezonePopup														  [public]
/**
	Default constructor */

CTimezonePopup::CTimezonePopup(
	 const JCharacter*	title,
	 JXContainer*		enclosure,
	 const HSizingOption	hSizing,
	 const VSizingOption	vSizing,
	 const JCoordinate	x,
	 const JCoordinate	y,
	 const JCoordinate	w,
	 const JCoordinate	h) :
	HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h)
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

void CTimezonePopup::OnCreate()
{
	// Init the popup menu

	// Get list of timezones from built-in calendar
	cdstrvect tzids;
	iCal::CICalendar::sICalendar.GetTimezones(tzids);
	
	// Clear existing menu
	RemoveAllItems();
	SetMenuItems("Any Timezone %r | UTC %r %l"); 

	// Add to menu
	for(cdstrvect::const_iterator iter = tzids.begin(); iter != tzids.end(); iter++)
	{
		AppendItem((*iter).c_str(), kFalse, kTrue);
	}
	
	SetUpdateAction(JXMenu::kDisableNone);
	SetToPopupChoice(kTrue, 1);

	// Always start with the current user default
	SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
}

void CTimezonePopup::NoFloating()
{
	// Remove first menu item
	RemoveItem(eNoTimezone);
	mNoFloating = true;

	// Always start with the current user default
	SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
}

void CTimezonePopup::SetTimezone(const iCal::CICalendarTimezone& tz)
{
	// Extract zone from dt and apply to menu
	JIndex new_value;
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
			for(JIndex i = eFirstTimezone - (mNoFloating ? 1 : 0); i <= GetItemCount(); i++)
			{
				// Get name of menu title
				cdstring txt = GetItemText(i).GetCString();
				
				// Compare with tzid
				if (txt.compare(tzid, true) == 0)
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
	JIndex value = GetValue();
	JIndex adjusted_value = value + (mNoFloating ? 1 : 0);
	tz.SetUTC(adjusted_value == eUTC);
	if ((adjusted_value == eNoTimezone) || (adjusted_value == eUTC))
	{
		cdstring empty;
		tz.SetTimezoneID(empty);
	}
	else
	{
		// Get name of menu title
		cdstring tzid = GetItemText(value).GetCString();

		tz.SetTimezoneID(tzid);
	}
}
