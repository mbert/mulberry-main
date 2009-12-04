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

#include "CCalendarPopup.h"

#include "CCalendarStoreManager.h"
#include "CXStringResources.h"

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CCalendarPopup														  [public]
/**
	Default constructor */

CCalendarPopup::CCalendarPopup(LStream* inStream) :
	LPopupButton(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CCalendarPopup														  [public]
/**
	Destructor */

CCalendarPopup::~CCalendarPopup()
{
}

#pragma mark -

void CCalendarPopup::FinishCreateSelf()
{
	LPopupButton::FinishCreateSelf();

	// Init the popup menu
	Reset();
}

void CCalendarPopup::Reset()
{
	// Clear existing menu
	MenuRef menu = GetMacMenuH();
	::DeleteMenuItems(menu, 1, ::CountMenuItems(menu));

	// Add to menu
	uint32_t cal_pos = 1;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		for(iCal::CICalendarList::const_iterator iter = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().begin(); iter != calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().end(); iter++,  cal_pos++)
		{
			// Make a CFString from UTF8 data
			cdstring name = (*iter)->GetName();
			if (name.empty())
			{
				name = rsrc::GetString("::NumberedCalendar");
				name.Substitute((unsigned long)cal_pos);
			}
			
			// Check read-only state
			if ((*iter)->IsReadOnly())
			{
				name.AppendResource("UI::Calendar::ReadOnly");
			}
			MyCFString menu_title(name.c_str(), kCFStringEncodingUTF8);
			::AppendMenuItemTextWithCFString(menu, menu_title, 0, 0, NULL);
		}
	}

	// Readjust max
	SetMaxValue(::CountMenuItems(menu));
}

void CCalendarPopup::SetCalendar(const iCal::CICalendarRef& calref)
{
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
	if (cal)
	{
		int32_t index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveIndex(cal);
		if (index >= 0)
			SetValue(index + 1);
	}
}

void CCalendarPopup::GetCalendar(iCal::CICalendarRef& calref) const
{
	SInt32 value = GetValue();
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		if (GetValue() > calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().size())
			value = 1;
		calref = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().at(value - 1)->GetRef();
	}
}