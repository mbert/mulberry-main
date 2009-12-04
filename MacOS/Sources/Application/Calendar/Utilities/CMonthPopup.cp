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

#include "CMonthPopup.h"

#include "CICalendarDateTime.h"

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CMonthPopup														  [public]
/**
	Default constructor */

CMonthPopup::CMonthPopup(LStream* inStream) :
	LPopupButton(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CMonthPopup														  [public]
/**
	Destructor */

CMonthPopup::~CMonthPopup()
{
}

#pragma mark -

void CMonthPopup::FinishCreateSelf()
{
	LPopupButton::FinishCreateSelf();

	// Init the popup menu

	// Clear existing menu
	MenuRef menu = GetMacMenuH();
	::DeleteMenuItems(menu, 1, ::CountMenuItems(menu));

	// Add to menu
	iCal::CICalendarDateTime dt(1970, 1, 1);
	for(int32_t i = 1; i <= 12; i++)
	{
		// Set month in date
		dt.SetMonth(i);

		// Make a CFString from UTF8 data
		MyCFString menu_title(dt.GetMonthText(false), kCFStringEncodingUTF8);
		::AppendMenuItemTextWithCFString(menu, menu_title, 0, 0, NULL);
	}
	
	// Readjust max
	SetMaxValue(::CountMenuItems(menu));
}

void CMonthPopup::SetMonth(int32_t month)
{
	SetValue(month);
}

int32_t CMonthPopup::GetMonth() const
{
	return GetValue();
}