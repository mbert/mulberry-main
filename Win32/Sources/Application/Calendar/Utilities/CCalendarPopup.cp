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
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

// ---------------------------------------------------------------------------
//	CCalendarPopup														  [public]
/**
	Default constructor */

CCalendarPopup::CCalendarPopup() :
	CPopupButton(true)
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

BEGIN_MESSAGE_MAP(CCalendarPopup, CPopupButton)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CCalendarPopup::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CPopupButton::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	// Set menu
	SetMenu(IDR_POPUP_CALENDARS);

	// Init the popup menu
	Reset();
	
	return 0;
}

BOOL CCalendarPopup::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	CPopupButton::SubclassDlgItem(nID, pParent, IDI_POPUPBTN, 0, 0, 0, true, false);

	// Set menu
	SetMenu(IDR_POPUP_CALENDARS);

	// Init the popup menu
	Reset();
	
	return true;
}

void CCalendarPopup::Reset()
{
	// Remove any existing items from main menu
	CMenu* pPopup = GetPopupMenu();
	UINT num_menu = pPopup->GetMenuItemCount();
	for(UINT i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);

	// Add to menu
	uint32_t cal_pos = 1;
	int menu_id = IDM_POPUP_CALENDARS_FIRST;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		for(iCal::CICalendarList::const_iterator iter = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().begin();
			iter != calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().end(); iter++, cal_pos++)
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

			// Convert from UTF8 data
			CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, menu_id++, name);
		}
	}
}

void CCalendarPopup::SetCalendar(const iCal::CICalendarRef& calref)
{
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
	if (cal)
	{
		int32_t index = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveIndex(cal);
		if (index >= 0)
			SetValue(IDM_POPUP_CALENDARS_FIRST + index);
	}
}

void CCalendarPopup::GetCalendar(iCal::CICalendarRef& calref) const
{
	UINT value = GetValue() - IDM_POPUP_CALENDARS_FIRST;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		if (value >= calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().size())
			value = 0;
		calref = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars().at(value)->GetRef();
	}
}