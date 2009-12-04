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


// Source for CPopupButton class


#include "CPopupButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#ifdef __MULBERRY
#include "CMulberryApp.h"
#endif
#include "CMulberryCommon.h"
#include "CUnicodeUtils.h"

IMPLEMENT_DYNCREATE(CPopupButton, CIconButton)

BEGIN_MESSAGE_MAP(CPopupButton, CIconButton)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// __________________________________________________________________________________________________
// C L A S S __ C P O P U P B U T T O N
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CPopupButton::CPopupButton(bool handle, bool handle_dlgid)
{
	mMenu = NULL;
	mMenuID = 0;
	mValue = -1;
	mText = NULL;
	mButtonText = true;
	mMainMenu = false;
	mAlwaysEnable = false;
	mSize = 16;
	mHandleCommand = handle;
	mHandleSendDlgID = handle_dlgid;
}

// Default destructor
CPopupButton::~CPopupButton()
{
	delete mMenu;
}

void CPopupButton::SetMenu(UINT menu_id)
{
	// Create menu if not already
	delete mMenu;
	mMenu = new CMenu;
	mMenu->LoadMenu(menu_id);
	mMenuID = menu_id;
}

const CMenu* CPopupButton::GetPopupMenu(void) const
{
	return mMenu->GetSubMenu(0);
}

CMenu* CPopupButton::GetPopupMenu(void)
{
	return mMenu->GetSubMenu(0);
}

void CPopupButton::SetValue(UINT value)
{
	if (value != mValue)
	{
		mValue = value;
		cdstring item;
		CMenu* pPopup = GetPopupMenu();
		if (pPopup)
			item = CUnicodeUtils::GetMenuStringUTF8(pPopup, mValue, MF_BYCOMMAND);
		
		// Update text
		if (mText)
			CUnicodeUtils::SetWindowTextUTF8(mText, item);
		if (mButtonText)
			CUnicodeUtils::SetWindowTextUTF8(this, item);
	}
}

void CPopupButton::RefreshValue()
{
	UINT current = mValue;
	mValue = 0;
	SetValue(current);
	RedrawWindow();
}

cdstring CPopupButton::GetValueText() const
{
	return CUnicodeUtils::GetMenuStringUTF8(mMenu, mValue, MF_BYCOMMAND);
}

// Clicked somewhere
void CPopupButton::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Get buttons top-right corner as start pt
	CRect screen;
	GetWindowRect(screen);
	CPoint global(screen.right, screen.top);

	// Push button
	SetPushed(true);

	CMenu* pPopup = GetPopupMenu();
	if (pPopup)
	{
		// May handle the command ourselves
		if (mHandleCommand)
		{
			// Check current value
			SetupCurrentMenuItem(true);

			UINT popup_result = pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_RETURNCMD, global.x, global.y, this);

			// Uncheck current value
			if(!mMainMenu)
				SetupCurrentMenuItem(false);

			// Unpush button
			SetPushed(false);

			if (popup_result)
			{
				SetValue(popup_result);
				GetParent()->SendMessage(WM_COMMAND, mHandleSendDlgID ? GetDlgCtrlID() : popup_result);
			}
		}
		else
		{
			// Check current value
			SetupCurrentMenuItem(true);

			CWnd* pWndPopupOwner = GetParent();
			while (pWndPopupOwner->GetStyle() & WS_CHILD)
				pWndPopupOwner = pWndPopupOwner->GetParent();
			pPopup->TrackPopupMenu(TPM_LEFTBUTTON | TPM_LEFTALIGN, global.x, global.y, GetParent());

			// Uncheck current value
			if(!mMainMenu)
				SetupCurrentMenuItem(false);

			// Unpush button
			SetPushed(false);
		}
	}
}

// Check items before doing popup
void CPopupButton::SetupCurrentMenuItem(bool check)
{
	CMenu* pPopup = GetPopupMenu();
	if (pPopup)
	{
		if (check && mAlwaysEnable)
		{
			for(int i = 0; i < pPopup->GetMenuItemCount(); i++)
				pPopup->EnableMenuItem(i, MF_BYPOSITION | MF_ENABLED);
		}

		// Check current value
		pPopup->CheckMenuItem(mValue, (check ? MF_CHECKED : MF_UNCHECKED) | MF_BYCOMMAND);
	}
}

// Draw the appropriate icon & text
void CPopupButton::DrawContent(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);
	bool pushed_frame = selected;
	if (mPushed) pushed_frame = !pushed_frame;

	UINT draw_icon;
	if (!selected && !mPushed)
		draw_icon = nIcon;
	else if (!selected && mPushed)
		draw_icon = (nIconPushed != 0) ? nIconPushed : nIcon;
	else if (selected && !mPushed)
		draw_icon = (nIconSel != 0) ? nIconSel : nIcon;
	else
		draw_icon = (nIconPushedSel != 0) ? nIconPushedSel : ((nIconSel != 0) ? nIconSel : nIcon);

	// Icon always at rhs
	int hoffset = 20;
	int voffset = ((lpDIS->rcItem.bottom - lpDIS->rcItem.top) - 16) / 2;

	if (enabled)
		CIconLoader::DrawIcon(pDC, lpDIS->rcItem.right - hoffset, lpDIS->rcItem.top + voffset, draw_icon, 16);
	else
		CIconLoader::DrawState(pDC, lpDIS->rcItem.right - hoffset, lpDIS->rcItem.top + voffset, draw_icon, 16, DSS_DISABLED);

	// Draw text if required
	if (mButtonText)
	{
		// Reset rhe
		hoffset = 8;

		// Set colors
		if (enabled)
		{
			if (pushed_frame && mFrame)
			{
				pDC->SetBkColor(CDrawUtils::sDkGrayColor);
				pDC->SetTextColor(CDrawUtils::sWhiteColor);
			}
			else
			{
				pDC->SetBkColor(CDrawUtils::sGrayColor);
				pDC->SetTextColor(CDrawUtils::sBtnTextColor);
			}
		}
		else
		{
			pDC->SetBkColor(CDrawUtils::sGrayColor);
			pDC->SetTextColor(CDrawUtils::sDkGrayColor);
		}

		cdstring theTxt = CUnicodeUtils::GetWindowTextUTF8(this);
		
#ifdef __MULBERRY
		const int small_offset = CMulberryApp::sLargeFont ? -2 : 0;
#else
		const int small_offset = 0;
#endif
		CRect clipRect = lpDIS->rcItem;
		clipRect.right -= hoffset;
		clipRect.bottom += small_offset;
		::DrawClippedStringUTF8(pDC, theTxt, CPoint(clipRect.left + 8, clipRect.top + voffset + 2 + small_offset), clipRect, eDrawString_Left);
	}
}
