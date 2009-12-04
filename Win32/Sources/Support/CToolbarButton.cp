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


// CToolbarButton.cpp : implementation file
//


#include "CToolbarButton.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#ifdef __MULBERRY
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#endif
#include "CUnicodeUtils.h"

HICON CToolbarButton::sPopupIcon = NULL;

IMPLEMENT_DYNAMIC(CToolbarButton, CIconButton)

BEGIN_MESSAGE_MAP(CToolbarButton, CIconButton)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(BM_SETCHECK, OnSetCheck)
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CToolbarButton

CToolbarButton::CToolbarButton()
{
	nIconIDBase = 0;
	mCapture = false;
	mTracking = false;
	mSmallIcon = false;
	mShowIcon = true;
	mShowCaption = true;
	mClickAndPopup = false;
	mDragMode = false;
}

CToolbarButton::~CToolbarButton()
{
}

const int cTitleHeight = 12;
const int cTitleOffset = 2;
const int cClickAndPopupWidth = 12;

BOOL CToolbarButton::Create(LPCTSTR title, const RECT& rect, CWnd* pParentWnd, UINT nID, UINT nTitle,
							UINT nIDIcon, UINT nIDIconSel, UINT nIDIconPushed, UINT nIDIconPushedSel, bool frame)
{
	if (CButton::Create(_T(""), WS_CHILD | WS_VISIBLE | BS_OWNERDRAW, rect, pParentWnd, nID))
	{
		// Create title if required
		if (nTitle && *title)
		{
			SetWindowText(title);
			SetFont(CMulberryApp::sAppSmallFont);
			mUseTitle = true;
		}

		mFrame = frame;

		nIconIDBase = nIDIcon;
		SetIcons(nIDIcon, nIDIconSel, nIDIconPushed, nIDIconPushedSel);

		return true;
	}
	else
		return false;
}

BOOL CToolbarButton::SubclassDlgItem(UINT nID, CWnd* pParent, UINT nIDIcon, UINT nIDIconSel,
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
	pParent->ScreenToClient(&r);
	MoveWindow(&r);

	// Get existing title and remove
	CString s;
	GetWindowText(s);

	// Create new title
	if (s.GetLength())
	{
		SetFont(CMulberryApp::sAppSmallFont);
		mUseTitle = true;
	}

	mFrame = frame;

	// Set icons
	nIconIDBase = nIDIcon;
	SetIcons(nIDIcon, nIDIconSel, nIDIconPushed, nIDIconPushedSel);
	return true;
}

bool CToolbarButton::HasPopup() const
{
	// Not this class - derived class will have popup
	return false;
}

void CToolbarButton::SetSmallIcon(bool small_icon)
{
	if (mSmallIcon ^ small_icon)
	{
		mSmallIcon = small_icon;

		// Load in new icon sizes
		mSize = (small_icon ? 16 : 32);
		SetIcons(nIconIDBase, 0, 0, 0);
		RedrawWindow();
	}
}

void CToolbarButton::SetShowIcon(bool icon)
{
	if (mShowIcon ^ icon)
	{
		mShowIcon = icon;
		RedrawWindow();
	}
}

void CToolbarButton::SetShowCaption(bool caption)
{
	if (mShowCaption ^ caption)
	{
		mShowCaption = caption;
		RedrawWindow();
		EnableToolTips(!mShowCaption);
	}
}

// Draw the appropriate button frame
void CToolbarButton::DrawFrame(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);
	bool pushed_frame = selected;
	if (mPushed) pushed_frame = !pushed_frame;

	if (mPushed || mCapture || mTracking)
	{
		// Standard frame
		CDrawUtils::DrawSimpleFrame(pDC, lpDIS->rcItem, pushed_frame, enabled, mFrame);
		
		// Special line for click-and-popup
		if (GetClickAndPopup() && mFrame)
		{
			CRect rect = lpDIS->rcItem;
			rect.left = rect.right - cClickAndPopupWidth;
			rect.top += 2;
			rect.bottom -= 2;
			CDrawUtils::DrawSimpleLine(pDC, rect, true, pushed_frame, enabled);
		}
	}	
	else
		pDC->FillRect(&lpDIS->rcItem, &CDrawUtils::sGrayBrush);
}

// Draw the appropriate icon
void CToolbarButton::DrawContent(LPDRAWITEMSTRUCT lpDIS)
{
	if (mShowIcon)
		DrawIcon(lpDIS);

	if (mShowCaption)
		DrawTitle(lpDIS);

	if (HasPopup())
		DrawPopupGlyph(lpDIS);
}

// Draw the appropriate icon
void CToolbarButton::DrawIcon(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	// Adjust rect for click-and -popup
	CRect rect = lpDIS->rcItem;
	if (GetClickAndPopup())
		rect.right -= cClickAndPopupWidth;
	
	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);

	UINT draw_icon;
	if (!selected && !mPushed)
		draw_icon = nIcon;
	else if (!selected && mPushed)
		draw_icon = (nIconPushed != 0) ? nIconPushed : nIcon;
	else if (selected && !mPushed)
		draw_icon = (nIconSel != 0) ? nIconSel : nIcon;
	else
		draw_icon = (nIconPushedSel != 0) ? nIconPushedSel : ((nIconSel != 0) ? nIconSel : nIcon);

	// Always center horizontally
	int hpos = (rect.right - rect.left - mSize) / 2;
	if (HasPopup() && !mShowCaption)
		hpos -= 2;
	
	// Center vertically if no caption
	int vpos = rect.top + (mSmallIcon ? 2 : 0);

	pDC->SetMapMode(MM_TEXT);
	CIconLoader::DrawState(pDC, hpos, vpos, draw_icon, mSize, enabled ? DSS_NORMAL : DSS_DISABLED);
}

// Draw the appropriate text
void CToolbarButton::DrawTitle(LPDRAWITEMSTRUCT lpDIS)
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
		pDC->SetTextColor(CDrawUtils::sGrayTextColor);
	}

	cdstring theTxt = CUnicodeUtils::GetWindowTextUTF8(this);

	// Need to remove single & characters from string
	cdstring temp(theTxt);
	char* p = temp.c_str_mod();
	char* q = p;
	while(*p)
	{
		if (*p == '&')
		{
			p++;
			if (*p != '&')
				continue;
		}
		*q++ = *p++;
	}
	*q = 0;
	theTxt = temp.c_str();

	int small_offset = CMulberryApp::sLargeFont ? (mShowIcon ? 4 : 2) : 0;
	CRect clipRect = lpDIS->rcItem;
	if (GetClickAndPopup())
		clipRect.right -= cClickAndPopupWidth;
	clipRect.top = clipRect.bottom - small_offset - cTitleHeight - cTitleOffset;
	clipRect.bottom -= cTitleOffset;
	::DrawClippedStringUTF8(pDC, theTxt, CPoint(clipRect.left, clipRect.top), clipRect, eDrawString_Center);
}

// Draw the appropriate icon
void CToolbarButton::DrawPopupGlyph(LPDRAWITEMSTRUCT lpDIS)
{
	// Make sure popup indicator is loaded
	if (!sPopupIcon)
		sPopupIcon = (HICON) ::LoadImage(AfxFindResourceHandle(MAKEINTRESOURCE(IDI_POPUPBTN), RT_GROUP_ICON), MAKEINTRESOURCE(IDI_POPUPBTN), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	// Save drawing state
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);

	// Always on right-hand side horizontally
	int hpos = lpDIS->rcItem.right - 18;
	
	// Center vertically if no caption
	int vpos = lpDIS->rcItem.top;
	if (mShowIcon)
		vpos += mSmallIcon ? 2 : 15;
	else
		vpos = lpDIS->rcItem.bottom - 18;

	pDC->SetMapMode(MM_TEXT);
	pDC->DrawState(CPoint(hpos, vpos), CSize(16, 16),
							sPopupIcon, DST_ICON | (enabled ? DSS_NORMAL : DSS_DISABLED), (CBrush*) nil);

}

void CToolbarButton::OnLButtonDown(UINT nFlags, CPoint pt)
{
	// Always change tracking state
	mTracking = true;

	CIconButton::OnLButtonDown(nFlags, pt);
}

VOID CALLBACK TrackMouseTimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime);

void CToolbarButton::OnLButtonUp(UINT nFlags, CPoint pt)
{
	mTracking = false;

	CRect rect;
	GetClientRect(rect);

	// Check whether its still inside and make sure it is captured
	if (rect.PtInRect(pt))
	{
		// Change capture state if not already
		if (!mCapture)
		{
			mCapture = true;
			RedrawWindow();
			
			// Set up track notifier for WM_MOUSELEAVE
			SetTimer(0x10001234, 100, (TIMERPROC)TrackMouseTimerProc);
		}
	}
	//else
	//	RedrawWindow();

	CIconButton::OnLButtonUp(nFlags, pt);
}

VOID CALLBACK TrackMouseTimerProc(HWND hWnd, UINT uMsg, UINT idEvent, DWORD dwTime)
{
	RECT rect;
	POINT pt;

	::GetClientRect(hWnd, &rect);
	::MapWindowPoints(hWnd, NULL, (LPPOINT)&rect,2);
	::GetCursorPos(&pt);
	if (!::PtInRect(&rect, pt) || (::WindowFromPoint(pt) != hWnd))
	{
		::KillTimer(hWnd, idEvent);

		::PostMessage(hWnd, WM_MOUSELEAVE, 0, 0);
	}
}

// Capture mouse for button fly-over hiliting
void CToolbarButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	if (rect.PtInRect(point) && !mDragMode)
	{
		// Change capture state if not already
		if (!mCapture)
		{
			mCapture = true;
			RedrawWindow();
			
			// Set up track notifier for WM_MOUSELEAVE
			SetTimer(0x10001234, 100, (TIMERPROC)TrackMouseTimerProc);
		}
	}

	CIconButton::OnMouseMove(nFlags, point);
}

LRESULT CToolbarButton::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	// Change capture state if not already
	if (mCapture)
	{
		mCapture = false;
		RedrawWindow();
	}

	return 0;
}

// Button moved
void CToolbarButton::OnMove(int x, int y)
{
	// By-pass immediate base class
	CNoFocusBitmapButton::OnMove(x, y);
}

LRESULT CToolbarButton::OnSetCheck(WPARAM wParam, LPARAM lParam)
{
	SetPushed(wParam > 0);
	return 0;
}

// Control show/hide
void CToolbarButton::OnShowWindow(BOOL bShow, UINT nStatus)
{
	// By-pass immediate base class
	CNoFocusBitmapButton::OnShowWindow(bShow, nStatus);
}

#pragma mark ____________________________________Tooltips

int CToolbarButton::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	CRect rect;
	GetClientRect(rect);
	if (!mShowCaption && rect.PtInRect(point))
	{
		// Hits against child windows always center the tip
		if (pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
		{
			CString theTxt;
			GetWindowText(theTxt);
			TCHAR* txt = (TCHAR*)::malloc((theTxt.GetLength() + 1) * sizeof(TCHAR));
			::memcpy(txt, (const TCHAR*)theTxt, (theTxt.GetLength() + 1) * sizeof(TCHAR));

			// Setup the TOOLINFO structure
			pTI->hwnd = m_hWnd;
			pTI->uId = 1;
			pTI->lpszText = txt;
			pTI->rect = rect;
			return pTI->uId;
		}
	}
	
	// Not found
	return -1;
}
