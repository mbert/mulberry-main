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

#include "CDrawUtils.h"
#ifdef __MULBERRY
#include "CMulberryApp.h"
#endif
#include "CMulberryCommon.h"
#include "CToolbar.h"

IMPLEMENT_DYNCREATE(CToolbarPopupButton, CToolbarButton)

BEGIN_MESSAGE_MAP(CToolbarPopupButton, CToolbarButton)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C P O P U P B U T T O N
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

const int cClickAndPopupWidth = 12;

// Default constructor
CToolbarPopupButton::CToolbarPopupButton()
{
	mMenu = nil;
	mMenuID = 0;
	mValue = -1;
	mText = nil;
	mMainMenu = false;
	mAlwaysEnable = false;
	mPopupSetValue = false;
}

// Default destructor
CToolbarPopupButton::~CToolbarPopupButton()
{
	delete mMenu;
}

bool CToolbarPopupButton::HasPopup() const
{
	// This class always has a popup
	return true;
}

void CToolbarPopupButton::SetMenu(UINT menu_id)
{
	// Create menu if not already
	delete mMenu;
	mMenu = new CMenu;
	mMenu->LoadMenu(menu_id);
	mMenuID = menu_id;
}

const CMenu* CToolbarPopupButton::GetPopupMenu(void) const
{
	return mMenu->GetSubMenu(0);
}

CMenu* CToolbarPopupButton::GetPopupMenu(void)
{
	return mMenu->GetSubMenu(0);
}

void CToolbarPopupButton::SetValue(UINT value)
{
	if (value != mValue)
	{
		mValue = value;
		CString item;
		CMenu* pPopup = GetPopupMenu();
		pPopup->GetMenuString(mValue, item, MF_BYCOMMAND);
		
		// Update text
		if (mText)
			mText->SetWindowText(item);
	}
}

void CToolbarPopupButton::RefreshValue()
{
	UINT current = mValue;
	mValue = 0;
	SetValue(current);
	RedrawWindow();
}

// Clicked somewhere
void CToolbarPopupButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	if (!GetClickAndPopup())
		return LButtonDownPopup(nFlags, point);
	else
	{
		// Determine which part it is in
		CRect rect;
		GetClientRect(rect);
		if (point.x > rect.right - cClickAndPopupWidth)
			return LButtonDownPopup(nFlags, point);
		else	
			return LButtonDownBtn(nFlags, point);
	}
}

// Clicked somewhere
void CToolbarPopupButton::LButtonDownBtn(UINT nFlags, CPoint point)
{
	CToolbarButton::OnLButtonDown(nFlags, point);
}

// Clicked somewhere
void CToolbarPopupButton::LButtonDownPopup(UINT nFlags, CPoint point)
{
	// Get buttons top-right corner as start pt
	CRect screen;
	GetWindowRect(screen);
	CPoint global(screen.right, screen.top);

	// Push button
	SetPushed(true);

	// Need to update the menu here right before it is shown
	CToolbar* tb = dynamic_cast<CToolbar*>(GetParent());
	if (tb)
		tb->UpdatePopupState(this);
	
	CMenu* pPopup = GetPopupMenu();

	// Check current value
	SetupCurrentMenuItem(true);

	CWnd* pWndPopupOwner = GetParent();
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, global.x, global.y, GetParent());

	// Uncheck current value
	if(!mMainMenu)
		SetupCurrentMenuItem(false);

	// Unpush button
	SetPushed(false);

	// Now handle popup result
	if (popup_result)
	{
		// Set the value
		if (mPopupSetValue)
			SetValue(popup_result);
		
		// Must send message to parent as TPM_RETURNCMD does not
		GetParent()->SendMessage(WM_COMMAND, popup_result);
	}
}

// Check items before doing popup
void CToolbarPopupButton::SetupCurrentMenuItem(bool check)
{
	if (check && mAlwaysEnable)
	{
		CMenu* pPopup = GetPopupMenu();
		for(int i = 0; i < pPopup->GetMenuItemCount(); i++)
			pPopup->EnableMenuItem(i, MF_BYPOSITION | MF_ENABLED);
	}

	// Check current value
	GetPopupMenu()->CheckMenuItem(mValue, (check ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
}
