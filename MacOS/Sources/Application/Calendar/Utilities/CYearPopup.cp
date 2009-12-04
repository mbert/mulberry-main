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

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CYearPopup														  [public]
/**
	Default constructor */

CYearPopup::CYearPopup(LStream* inStream) :
	LPopupButton(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CYearPopup														  [public]
/**
	Destructor */

CYearPopup::~CYearPopup()
{
}

#pragma mark -

void CYearPopup::FinishCreateSelf()
{
	LPopupButton::FinishCreateSelf();

	// Init the popup menu

	// Clear existing menu
	MenuRef menu = GetMacMenuH();
	::DeleteMenuItems(menu, 1, ::CountMenuItems(menu));

	// Add to menu
	for(int32_t i = 0; i < 7; i++)
	{
		// Make a CFString from UTF8 data
		MyCFString menu_title("xxxx", kCFStringEncodingUTF8);
		::AppendMenuItemTextWithCFString(menu, menu_title, 0, 0, NULL);
	}
	
	// Readjust max
	SetMaxValue(::CountMenuItems(menu));
}

void CYearPopup::SetValue(SInt32 inValue)
{
	// Do inherited first
	LPopupButton::SetValue(inValue);

	// Ignore if setting back to center item
	if (inValue != 4)
	{
		SetYear(GetYear());
	}
}

void CYearPopup::SetYear(int32_t year)
{
	mYearStart = year - 3;
	SetupMenuItems();
	
	// The value set is always the middle one
	SetValue(4);
}

int32_t CYearPopup::GetYear() const
{
	return mYearStart + GetValue() - 1;
}

void CYearPopup::SetupMenuItems()
{
	// Change menu titles
	MenuRef menu = GetMacMenuH();
	for(int32_t i = 0; i < 7; i++)
	{
		char buf[32];
		std::snprintf(buf, 32, "%ld", mYearStart + i);

		::SetMenuItemTextUTF8(menu, i + 1, buf);
	}

	if (IsVisible())
		Refresh();
}
