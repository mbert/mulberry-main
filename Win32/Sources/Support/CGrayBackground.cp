/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CGrayBackground.cpp : implementation file
//


#include "CGrayBackground.h"

#include "CDrawUtils.h"
#include "StValueChanger.h"

IMPLEMENT_DYNAMIC(CGrayBackground, CStatic)

/////////////////////////////////////////////////////////////////////////////
// CGrayBackground

CBrush CGrayBackground::sBkBrush(::GetSysColor(COLOR_BTNFACE));

CGrayBackground::CGrayBackground()
{
	mImageHeight = 0;
	mImageWidth = 0;
	mUseFocus = false;
	mHasFocus = false;
}

CGrayBackground::~CGrayBackground()
{
}


BEGIN_MESSAGE_MAP(CGrayBackground, CStatic)
	//{{AFX_MSG_MAP(CGrayBackground)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_CTLCOLOR()
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_VSCROLL()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Gray background support

int CGrayBackground::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CStatic::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	return 0;
}

// CContainerWnd columns
void CGrayBackground::OnSize(UINT nType, int cx, int cy)
{
	CStatic::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);
}

HBRUSH CGrayBackground::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Change background for CStatics only
	if ((nCtlColor == CTLCOLOR_STATIC) ||
		(nCtlColor == CTLCOLOR_BTN))
	{
		pDC->SetBkColor(::GetSysColor(COLOR_BTNFACE));
		pDC->SetTextColor(::GetSysColor(nCtlColor == CTLCOLOR_STATIC) ? CDrawUtils::sWindowTextColor : CDrawUtils::sBtnTextColor);
		return sBkBrush;
	}
	else
		return CWnd::OnCtlColor(pDC, pWnd, nCtlColor);
}

HBRUSH CGrayBackground::CtlColor(CDC* pDC, UINT nCtlColor)
{
	// use helper in CWnd
	//return GrayCtlColor(pDC, this, nCtlColor);
	return sBkBrush;
}

BOOL CGrayBackground::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
   // Extend the framework's command route from the view to
   // the application-specific CMyShape that is currently selected
   // in the view. m_pActiveShape is NULL if no shape object
   // is currently selected in the view.
   if (GetParent() && GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
      return true;

   // If the object(s) in the extended command route don't handle
   // the command, then let the base class OnCmdMsg handle it.
   return CStatic::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CGrayBackground::CreateDialogItems(UINT nID, CWnd* pParent)
{
	CreateDlg(MAKEINTRESOURCE(nID), pParent);

	// Set initial size for control repositioner
	CRect rect;
	GetWindowRect(rect);
	InitResize(rect.Width(), rect.Height());

	// Initialise extra items
	InitDialog();
}

void CGrayBackground::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CStatic::OnVScroll(nSBCode, nPos, pScrollBar);

	// calc new y position
	int y = GetScrollPos(SB_VERT);
	int yOrig = y;
	int yMax = GetScrollLimit(SB_VERT);
	if (yMax == 1)
		yMax = 0;

	switch (nSBCode)
	{
	case SB_TOP:
		y = 0;
		break;
	case SB_BOTTOM:
		y = INT_MAX;
		break;
	case SB_LINEUP:
		y -= 16;
		break;
	case SB_LINEDOWN:
		y += 16;
		break;
	case SB_PAGEUP:
		y -= 160;
		break;
	case SB_PAGEDOWN:
		y += 160;
		break;
	case SB_THUMBTRACK:
		y = nPos;
		break;
	}

	if (y < 0)
		y = 0;
	else if (y > yMax)
		y = yMax;

	// do scroll and update scroll positions
	ScrollImage(0, y - yOrig);
	if (y != yOrig)
		SetScrollPos(SB_VERT, y);
}

void CGrayBackground::ScrollImage(int dx, int dy)
{
	// Scroll screen bits
	ScrollWindowEx(-dx, -dy, NULL, NULL, NULL, NULL, SW_ERASE);
	
	// Move each control
	HWND hWndChild = ::GetWindow(GetSafeHwnd(), GW_CHILD);
	if (hWndChild != NULL)
	{
		for (; hWndChild != NULL;
			hWndChild = ::GetNextWindow(hWndChild, GW_HWNDNEXT))
		{
			CRect rect;
			::GetWindowRect(hWndChild, &rect);
			ScreenToClient(&rect);
			::SetWindowPos(hWndChild, NULL,
				rect.left-dx, rect.top-dy, 0, 0,
				SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOZORDER|SWP_NOREDRAW);
		}
	}
	
	if (dx || dy)
		RedrawWindow();
}

void CGrayBackground::ResizeImage(int imageWidth, int imageHeight, bool client_resize)
{
	// Only worry about height for now
	CRect client;
	GetClientRect(client);

	if (client_resize || (imageHeight != mImageHeight))
	{
		SCROLLINFO scinfo;
		scinfo.fMask = SIF_ALL | SIF_DISABLENOSCROLL;
		scinfo.nMin = 0;
		scinfo.nMax = 0;
		scinfo.nPage = 0;
		scinfo.nPos = 0;

		// Increase
		if (imageHeight > mImageHeight)
		{
			// Look for resize
			if (imageHeight > client.Height())
			{
				// Show scroll bar at new settings
				scinfo.nMax = imageHeight;
				scinfo.nPage = client.Height();
				scinfo.nPos = GetScrollPos(SB_VERT);
			}
		}
		
		// Decrease
		else
		{
			// Scroll bar still active
			if ((imageHeight > client.Height()) && (mImageHeight > client.Height()))
			{
				// Reset scroll if shrunk
				if (GetScrollPos(SB_VERT) + client.Height() > imageHeight)
					ScrollImage(0, imageHeight - GetScrollPos(SB_VERT) - client.Height());
					
				
				// Show scroll bar at new settings
				scinfo.nMax = imageHeight;
				scinfo.nPage = client.Height();
				scinfo.nPos = std::min(imageHeight - client.Height(), GetScrollPos(SB_VERT));
			}
			
			// Scroll bar was active, but must be deactiviated
			else if ((imageHeight <= client.Height()) && (mImageHeight > client.Height()))
			{
				// Scroll all the way to the top
				if (GetScrollPos(SB_VERT))
					ScrollImage(0, -GetScrollPos(SB_VERT));
			}
		}
		
		SetScrollInfo(SB_VERT, &scinfo);
		mImageHeight = imageHeight;
	}
}

void CGrayBackground::SetBorderFocus(bool focus)
{
	if (mUseFocus && (mHasFocus ^ focus))
	{
		// Set flag
		mHasFocus = focus;
		
		// Determine 
		
		// Get entire client region
		CRgn focus_rgn;
		CRect client;
		GetClientRect(client);
		focus_rgn.CreateRectRgnIndirect(client);

		// Get region inside focus border
		CRgn inside_rgn;
		client.DeflateRect(4, 4);
		inside_rgn.CreateRectRgnIndirect(client);
		
		// Diff them
		focus_rgn.CombineRgn(&focus_rgn, &inside_rgn, RGN_DIFF);

		// Now redraw only the focus region
		RedrawWindow(NULL, &focus_rgn);
	}
}

void CGrayBackground::SetFocusBorder()
{
	mUseFocus = true;
	mHasFocus = false;
}

void CGrayBackground::OnPaint()
{
	// Fill the middle
	CPaintDC dc(this);
	CRect client;
	GetClientRect(client);
	dc.FillSolidRect(client, afxData.clrBtnFace);

	// If using focus, dra a focus border if focussed
	if (mUseFocus)
	{
		// Focus is 3 pixels wide, sunken frame is 2
		if (mHasFocus)
			CDrawUtils::DrawFocusFrame(&dc, client);
		else
		{
			client.DeflateRect(1, 1);
			_AfxDrawBorders(&dc, client, false, true);
		}
	}
}
