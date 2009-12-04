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


// CTextButton.cpp : implementation file
//


#include "CTextButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CUnicodeUtils.h"

IMPLEMENT_DYNAMIC(CTextButton, CButton)

BEGIN_MESSAGE_MAP(CTextButton, CNoFocusButton)
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextButton

CTextButton::CTextButton()
{
	mPushed = false;
	mIconID = 0;
}

CTextButton::~CTextButton()
{
}

BOOL CTextButton::Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nIconID)
{
	mIconID = nIconID;
	return CNoFocusButton::Create(title, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, rect, pParentWnd, nID);
}

// Set text from number
void CTextButton::SetPushed(bool pushed)
{
	// Only if different
	if (pushed != mPushed)
	{
		// Reset state
		mPushed = pushed;

		// Force redraw
		RedrawWindow();
	}
}

// Draw the appropriate icon and button frame
void CTextButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw frame then content
	DrawFrame(lpDIS);
	DrawContent(lpDIS);
}

// Draw the appropriate button frame
void CTextButton::DrawFrame(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);
	bool pushed_frame = selected;
	if (mPushed) pushed_frame = !pushed_frame;

	CDrawUtils::DrawSimpleFrame(pDC, lpDIS->rcItem, pushed_frame, enabled);
}

// Draw the appropriate icon
void CTextButton::DrawContent(LPDRAWITEMSTRUCT lpDIS)
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

	// Check for icon
	int hoffset = 0;
	if (mIconID != 0)
	{
		int offset = (lpDIS->rcItem.right - lpDIS->rcItem.left >= 24) ? 4 : 0;
		int size = 16;

		pDC->SetMapMode(MM_TEXT);
		CIconLoader::DrawState(pDC, lpDIS->rcItem.left+ offset, lpDIS->rcItem.top + offset, mIconID, size, enabled ? DSS_NORMAL : DSS_DISABLED);
		
		hoffset += offset + size - 4;
	}
	
	// Reset rhe
	int voffset = ((lpDIS->rcItem.bottom - lpDIS->rcItem.top) - 16) / 2;

	// Set colors
	if (enabled)
	{
		if (pushed_frame)
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
	
	const int small_offset = CMulberryApp::sLargeFont ? -2 : 0;
	CRect clipRect = lpDIS->rcItem;
	clipRect.bottom += small_offset;
	::DrawClippedStringUTF8(pDC, theTxt, CPoint(clipRect.left + 8 + hoffset, clipRect.top + voffset + 2 + small_offset), clipRect, eDrawString_Left);
}

LRESULT CTextButton::OnSetCheck(WPARAM wParam, LPARAM lParam)
{
	SetPushed(wParam > 0);
	return 0;
}

void CTextButton::OnClick()
{
	if ((GetStyle() & BS_AUTOCHECKBOX) != 0)
		SetPushed(!IsPushed());
	GetParent()->SendMessage(WM_COMMAND, GetDlgCtrlID());
}
