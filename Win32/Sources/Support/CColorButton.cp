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


// CColorPickerButton.cpp : implementation file
//


#include "CColorButton.h"

#include "CDrawUtils.h"
#include "CSDIFrame.h"

IMPLEMENT_DYNAMIC(CColorPickerButton, CButton)

BEGIN_MESSAGE_MAP(CColorPickerButton, CButton)
	ON_CONTROL_REFLECT(BN_CLICKED, OnClicked)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPickerButton

CColorPickerButton::CColorPickerButton()
{
	mColor = RGB(0,0,0);
}

CColorPickerButton::~CColorPickerButton()
{
}

BOOL CColorPickerButton::Create(const RECT& rect, CWnd* pParentWnd, COLORREF color, UINT nID)
{
	mColor = color;
	return CButton::Create(_T(""), WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW, rect, pParentWnd, nID);
}

// Set color
void CColorPickerButton::SetColor(COLORREF color)
{
	// Only if different
	if (color != mColor)
	{
		// Reset color
		mColor = color;

		// Force redraw
		RedrawWindow();
	}
}

// Clicked item
void CColorPickerButton::OnClicked()
{
	CColorDialog dlg(0, 0, CSDIFrame::GetAppTopWindow());
	dlg.m_cc.rgbResult = mColor;
	dlg.m_cc.Flags |= CC_FULLOPEN | CC_RGBINIT;

	if (dlg.DoModal() == IDOK)
	{
		mColor = dlg.GetColor();
		RedrawWindow();
	}
}

// Draw the appropriate icon and button frame
void CColorPickerButton::DrawItem(LPDRAWITEMSTRUCT lpDIS)
{
	// Draw 3D frame
	CDC* pDC = CDC::FromHandle(lpDIS->hDC);
	StDCState save(pDC);

	// Check selected state/enabled state (adjust for toggle operation)
	UINT state = lpDIS->itemState;
	bool enabled = !(state & ODS_DISABLED);
	bool selected = (state & ODS_SELECTED);

	// Black outline
	CRect rect = lpDIS->rcItem;
	rect.right -= 2;
	rect.bottom -= 2;
	pDC->FrameRect(&rect, enabled ? &CDrawUtils::sBlackBrush : &CDrawUtils::sDkGrayBrush);
	if (selected)
	{
		rect.DeflateRect(1, 1);
		pDC->FrameRect(&rect, &CDrawUtils::sBlackBrush);
	}

	// Shadow outline
	pDC->SelectObject(enabled ? &CDrawUtils::sDkGrayPen : &CDrawUtils::sGrayPen);
	pDC->MoveTo(lpDIS->rcItem.right - 2, lpDIS->rcItem.top + 1);
	pDC->LineTo(lpDIS->rcItem.right - 2, lpDIS->rcItem.bottom - 2);
	pDC->LineTo(lpDIS->rcItem.left, lpDIS->rcItem.bottom - 2);
	pDC->MoveTo(lpDIS->rcItem.right - 1, lpDIS->rcItem.top + 1);
	pDC->LineTo(lpDIS->rcItem.right - 1, lpDIS->rcItem.bottom - 1);
	pDC->LineTo(lpDIS->rcItem.left, lpDIS->rcItem.bottom - 1);

	// Fill color
	CBrush fill;
	if (enabled)
		fill.CreateSolidBrush(mColor);
	else
		fill.CreateHatchBrush(HS_DIAGCROSS, mColor);
	rect.DeflateRect(1, 1);
	pDC->FillRect(&rect, &fill);
}
