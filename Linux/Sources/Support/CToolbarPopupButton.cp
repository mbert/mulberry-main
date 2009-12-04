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


// Source for CToolbarPopupButton class


#include "CToolbarPopupButton.h"

#include "CMenu.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

const int cClickAndPopupWidth = 12;

const JCharacter* CToolbarPopupButton::kMenuSelect = "MenuSelect::CToolbarPopupButton";
const JCharacter* CToolbarPopupButton::kMenuUpdate = "MenuUpdate::CToolbarPopupButton";

// Default constructor
CToolbarPopupButton::CToolbarPopupButton(const JCharacter* label, JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h)
	: CToolbarButton(label, enclosure, hSizing, vSizing, x,y, w,h)
{
	mMenu = NULL;
	mMenu = new CMenu("", this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
	mMenu->Hide();
	mMenu->SetUpdateAction(JXMenu::kDisableNone);
	mMenu->SetToHiddenPopupMenu(kTrue);
	ListenTo(mMenu);
	mValue = 0;
}

// Default destructor
CToolbarPopupButton::~CToolbarPopupButton()
{
}

bool CToolbarPopupButton::HasPopup() const
{
	// This class always has a popup
	return true;
}

// Clicked somewhere
void CToolbarPopupButton::HandleMouseDown(const JPoint& pt, const JXMouseButton button,
									      const JSize clickCount, const JXButtonStates& buttonStates,
									      const JXKeyModifiers& modifiers)
{
	if (button == kJXLeftButton)
	{
		if (!GetClickAndPopup())
			return LButtonDownPopup(pt, button, clickCount, buttonStates, modifiers);
		else
		{
			// Determine which part it is in
			JRect rect = GetBounds();
			if (pt.x > rect.right - cClickAndPopupWidth)
				return LButtonDownPopup(pt, button, clickCount, buttonStates, modifiers);
			else	
				return LButtonDownBtn(pt, button, clickCount, buttonStates, modifiers);
		}
	}
	else if (button == kJXRightButton)
		{
		CToolbarButton::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
		}
}

// Clicked somewhere
void CToolbarPopupButton::LButtonDownBtn(const JPoint& pt, const JXMouseButton button,
									      const JSize clickCount, const JXButtonStates& buttonStates,
									      const JXKeyModifiers& modifiers)
{
	CToolbarButton::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

// Clicked somewhere
void CToolbarPopupButton::LButtonDownPopup(const JPoint& pt, const JXMouseButton button,
									      const JSize clickCount, const JXButtonStates& buttonStates,
									      const JXKeyModifiers& modifiers)
{
	GetPopupMenu()->PopUp(this, pt, buttonStates, modifiers);
}

// Check items before doing popup
void CToolbarPopupButton::UpdateMenu()
{
	// Make sure everything is enabled
	GetPopupMenu()->EnableAll();
	if (mValue)
		GetPopupMenu()->CheckItem(mValue);
	Broadcast(MenuUpdate(this));
}

// Respond to clicks in the icon buttons
void CToolbarPopupButton::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == GetPopupMenu())
	{
		if (message.Is(JXMenu::kNeedsUpdate))
			UpdateMenu();
		else if(message.Is(JXMenu::kItemSelected))
		{
			const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
			Broadcast(MenuSelect(GetPopupMenu(), is->GetIndex()));
		}
	}
	else
		CToolbarButton::Receive(sender, message);
}

