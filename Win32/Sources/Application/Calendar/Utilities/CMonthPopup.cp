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

#include "CUnicodeUtils.h"

#include "CICalendarDateTime.h"

// ---------------------------------------------------------------------------
//	CMonthPopup														  [public]
/**
	Default constructor */

CMonthPopup::CMonthPopup() :
	CPopupButton(true)
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

BEGIN_MESSAGE_MAP(CMonthPopup, CPopupButton)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CMonthPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPopupButton::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Set menu
	SetMenu(IDR_POPUP_MONTHS);

	// Init the popup menu
	Reset();
	
	return 0;
}

BOOL CMonthPopup::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	CPopupButton::SubclassDlgItem(nID, pParent, IDI_POPUPBTN, 0, 0, 0, true, false);

	// Set menu
	SetMenu(IDR_POPUP_MONTHS);

	// Init the popup menu
	Reset();
	
	return true;
}

void CMonthPopup::Reset()
{
	// Remove any existing items from main menu
	CMenu* pPopup = GetPopupMenu();
	UINT num_menu = pPopup->GetMenuItemCount();
	for(UINT i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add to menu
	UINT menu_id = IDM_MONTH1;
	iCal::CICalendarDateTime dt(1970, 1, 1);
	for(int32_t i = 1; i <= 12; i++, menu_id++)
	{
		// Set month in date
		dt.SetMonth(i);

		// Convert from UTF8 data
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id, dt.GetMonthText(false));
	}
}

void CMonthPopup::SetMonth(int32_t month)
{
	SetValue(month - 1 + IDM_MONTH1);
}

int32_t CMonthPopup::GetMonth() const
{
	return GetValue() - IDM_MONTH1 + 1;
}