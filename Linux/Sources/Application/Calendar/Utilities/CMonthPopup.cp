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

#pragma mark -

void CMonthPopup::OnCreate()
{
	// Init the popup menu

	// Add to menu
	iCal::CICalendarDateTime dt(1970, 1, 1);
	for(int32_t i = 1; i <= 12; i++)
	{
		// Set month in date
		dt.SetMonth(i);
		AppendItem(dt.GetMonthText(false), kFalse, kTrue);
	}

	SetUpdateAction(JXMenu::kDisableNone);
	SetToPopupChoice(kTrue, 1);
}

void CMonthPopup::SetMonth(int32_t month)
{
	SetValue(month);
}

int32_t CMonthPopup::GetMonth() const
{
	return GetValue();
}
