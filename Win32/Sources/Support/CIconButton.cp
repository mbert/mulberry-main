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


// CIconButton.cpp : implementation file
//


#include "CIconButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#ifdef __MULBERRY
#include "CMulberryApp.h"
#endif
#include "CUnicodeUtils.h"

IMPLEMENT_DYNAMIC(CIconButton, CBitmapButton)

BEGIN_MESSAGE_MAP(CIconButton, CNoFocusBitmapButton)
	ON_WM_MOVE()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIconButton

CIconButton::CIconButton()
{
	mUseTitle = false;
	nIcon = 0;
	nIconSel = 0;
	nIconPushed = 0;
	nIconPushedSel = 0;
	mDisposeSel = true;
	mDisposePushed = true;
	mDisposePushedSel = true;
	mPushed = false;
	mSize = 32;
	mFrame = true;
	mSimpleFrame = false;
	mLargeTitle = false;
}

CIconButton::~CIconButton()
{
}

BOOL CIconButton::Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nTitle,
							UINT nIDIcon, UINT nIDIconSel, UINT nIDIconPushed, UINT nIDIconPushedSel, bool frame)
{
	if (CButton::Create(_T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rect, pParentWnd, nID))
	{
		// Create title if required
		if (nTitle && *title)
		{
#ifdef __MULBERRY
			int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
#else
			int small_offset = 0;
#endif
			mLargeTitle = (::lstrlen(title) > 6);
			if (mLargeTitle)
				small_offset += 2;
			mTitle.Create(title, WS_CHILD | WS_VISIBLE | SS_CENTER,
							CRect(rect.left - 4 - small_offset, rect.bottom, rect.left + 36 + small_offset, rect.bottom + small_offset + 12), pParentWnd, nTitle);
#ifdef __MULBERRY
			mTitle.SetFont(CMulberryApp::sAppSmallFont);
#endif
			mUseTitle = true;
		}

		mSize = ((rect.bottom - rect.top >= 32) || (rect.right - rect.left >= 32)) ? 32 : 16;

		mFrame = frame;

		SetIcons(nIDIcon, nIDIconSel, nIDIconPushed, nIDIconPushedSel);
		return true;
	}
	else
		return false;
}

BOOL CIconButton::SubclassDlgItem(UINT nID, CWnd* pParent, UINT nIDIcon, UINT nIDIconSel,
									UINT nIDIconPushed, UINT nIDIconPushedSel, bool frame, bool clip)
{
	// Do inherited
	CButton::SubclassDlgItem(nID, pParent);

	// Get current rect
	CRect r;
	GetWindowRect(&r);
	if (clip)
	{

		if (r.Width() >= 32)
			r.right = r.left + 32;
		else if (r.Width() >= 24)
			r.right = r.left + 24;
		else if (r.Width() >= 16)
			r.right = r.left + 16;
		if (r.Height() >= 32)
			r.bottom = r.top + 32;
		else if (r.Height() >= 24)
			r.bottom = r.top + 24;
		else if (r.Height() >= 16)
			r.bottom = r.top + 16;
	}
			
	mSize = ((r.Height() >= 32) && (r.Width() >= 32)) ? 32 : 16;

	pParent->ScreenToClient(&r);
	MoveWindow(&r);

	// Get existing title and remove
	CString s;
	GetWindowText(s);
	SetWindowText(_T(""));

	// Create new title
	if (s.GetLength())
	{
#ifdef __MULBERRY
		int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
#else
		int small_offset = 0;
#endif
		mLargeTitle = (s.GetLength() > 6);
		if (mLargeTitle)
			small_offset += 2;

		mTitle.Create(s, WS_CHILD | WS_VISIBLE | SS_CENTER,
						CRect(r.left - 4 - small_offset, r.bottom, r.left + 36 + small_offset, r.bottom + small_offset + 12), pParent, IDC_STATIC);
#ifdef __MULBERRY
		mTitle.SetFont(CMulberryApp::sAppSmallFont);
#endif
	}

	mFrame = frame;

	// Set icons
	SetIcons(nIDIcon, nIDIconSel, nIDIconPushed, nIDIconPushedSel);
	return true;
}

	// Change to new title
void CIconButton::SetTitle(const cdstring& s)
{
	CUnicodeUtils::SetWindowTextUTF8(&mTitle, s);
}

// Get title
void CIconButton::GetTitle(cdstring& s) const
{
	s = CUnicodeUtils::GetWindowTextUTF8(&mTitle);
}

// Set text from number
void CIconButton::SetPushed(bool pushed)
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

void CIconButton::SetIcons(UINT nIDIcon, UINT nIDIconSel, UINT nIDIconPushed, UINT nIDIconPushedSel)
{
	// Only if different
	if ((nIcon != nIDIcon ) ||
		(nIconSel != nIDIconSel) ||
		(nIconPushed != nIDIconPushed) ||
		(nIconPushedSel != nIDIconPushedSel))
	{
		// cache icon ids
		nIcon = nIDIcon;
		nIconSel = nIDIconSel;
		nIconPushed = nIDIconPushed;
		nIconPushedSel = nIDIconPushedSel;
		
		// Force redraw
		RedrawWindow();
	}
}

// Draw the appropriate icon and button frame
void CIconButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw frame then content
	DrawFrame(lpDIS);
	DrawContent(lpDIS);
}

// Draw the appropriate button frame
void CIconButton::DrawFrame(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);
	bool pushed_frame = selected;
	if (mPushed) pushed_frame = !pushed_frame;

	if (mSimpleFrame)
		CDrawUtils::DrawSimpleFrame(pDC, lpDIS->rcItem, pushed_frame, enabled, mFrame);
	else
		CDrawUtils::Draw3DFrame(pDC, lpDIS->rcItem, pushed_frame, enabled, mFrame);
}

// Draw the appropriate icon
void CIconButton::DrawContent(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);

	UINT draw_icon;
	if (!selected && !mPushed)
		draw_icon = nIcon;
	else if (!selected && mPushed)
		draw_icon = (nIconPushed != 0) ? nIconPushed : ((nIconPushed != 0) ? nIconPushed : nIcon);
	else if (selected && !mPushed)
		draw_icon = (nIconSel != 0) ? nIconSel : nIcon;
	else
		draw_icon = (nIconPushedSel != 0) ? nIconPushedSel : nIcon;

	int size = (mSize >= 32) ? 32 : 16;
	int hoffset = (lpDIS->rcItem.left + lpDIS->rcItem.right - mSize) / 2;
	int voffset = (lpDIS->rcItem.top + lpDIS->rcItem.bottom - mSize) / 2;

	pDC->SetMapMode(MM_TEXT);
	if (enabled)
		CIconLoader::DrawIcon(pDC, lpDIS->rcItem.left + hoffset, lpDIS->rcItem.top + voffset, draw_icon, size);
	else
		CIconLoader::DrawState(pDC, lpDIS->rcItem.left+ hoffset, lpDIS->rcItem.top + voffset, draw_icon, size, DSS_DISABLED);

}

// Button moved
void CIconButton::OnMove(int x, int y)
{
	// Do default
	CNoFocusBitmapButton::OnMove(x, y);

	// Only move title if created
	if (mUseTitle)
	{
		// Move caption
#ifdef __MULBERRY
		int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
#else
		int small_offset = 0;
#endif
		if (mLargeTitle)
			small_offset += 2;
		mTitle.MoveWindow(x - 4 - small_offset, y + 32, 40 + small_offset, 12 + small_offset);
	}
}

// Control show/hide
void CIconButton::OnShowWindow(BOOL bShow, UINT nStatus)
{
	// Do default
	CNoFocusBitmapButton::OnShowWindow(bShow, nStatus);

	// Show/hide text if created
	if (mUseTitle)
		mTitle.ShowWindow(bShow);
}
