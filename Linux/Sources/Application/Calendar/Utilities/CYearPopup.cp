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

#include "CYearPopup.h"

#include "CMulberryCommon.h"

#include <stdio.h>

#pragma mark -

void CYearPopup::OnCreate()
{
	// Add to menu
	for(int32_t i = 0; i < 7; i++)
	{
		AppendItem("xxxx", kFalse, kTrue);
	}
	
	// Init the popup menu
	Reset();

	SetUpdateAction(JXMenu::kDisableNone);
	SetToPopupChoice(kTrue, 4);
}

void CYearPopup::SetValue(JIndex inValue)
{
	// Do inherited first
	HPopupMenu::SetValue(inValue);

	// Ignore if setting back to center item
	if (inValue != 4)
	{
		SetYear(GetYear());
	}
}

void CYearPopup::SetYear(int32_t year)
{
	mYearStart = year - 3;
	Reset();
	
	// The value set is always the middle one
	mValue = 0;
	SetValue(4);
}

int32_t CYearPopup::GetYear() const
{
	return mYearStart + GetValue() - 1;
}

void CYearPopup::Reset()
{
	// Change menu titles
	for(JIndex i = 0; i < 7; i++)
	{
		char buf[32];
		::snprintf(buf, 32, "%ld", mYearStart + i);

		SetItemText(i + 1, buf);
	}
}
