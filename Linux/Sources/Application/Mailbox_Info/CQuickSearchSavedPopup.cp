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


// CQuickSearchSavedPopup.cpp : implementation file
//


#include "CQuickSearchSavedPopup.h"

#include "CPreferences.h"
#include "CSearchStyle.h"

/////////////////////////////////////////////////////////////////////////////
// CQuickSearchSavedPopup

CQuickSearchSavedPopup::CQuickSearchSavedPopup(
	 const JCharacter*	title,
	 JXContainer*		enclosure,
	 const HSizingOption	hSizing,
	 const VSizingOption	vSizing,
	 const JCoordinate	x,
	 const JCoordinate	y,
	 const JCoordinate	w,
	 const JCoordinate	h)
	: HPopupMenu(title, enclosure, hSizing, vSizing, x, y, w, h)
{
}

CQuickSearchSavedPopup::~CQuickSearchSavedPopup()
{
}


void CQuickSearchSavedPopup::OnCreate()
{
	// Init the popup menu
	SyncMenu();
}

void CQuickSearchSavedPopup::HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									const JSize clickCount,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers)
{
	SyncMenu();
	HPopupMenu::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

void CQuickSearchSavedPopup::SyncMenu()
{
	// Delete previous items
	RemoveAllItems();
	SetMenuItems("Choose %l"); 

	// Remove any existing items from main menu
	for(CSearchStyleList::const_iterator iter = CPreferences::sPrefs->mSearchStyles.GetValue().begin();
			iter != CPreferences::sPrefs->mSearchStyles.GetValue().end(); iter++)
		AppendItem((*iter)->GetName(), kFalse, kFalse);
}
