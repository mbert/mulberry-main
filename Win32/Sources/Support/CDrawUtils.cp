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


// CDrawUtils.cpp : implementation file
//

#include "CDrawUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CDrawUtils

COLORREF CDrawUtils::sWhiteColor = RGB(255,255,255);
COLORREF CDrawUtils::sBlackColor = RGB(0,0,0);
COLORREF CDrawUtils::sGrayColor = ::GetSysColor(COLOR_BTNFACE);
COLORREF CDrawUtils::sDkGrayColor = ::GetSysColor(COLOR_BTNSHADOW);
COLORREF CDrawUtils::sBtnTextColor = ::GetSysColor(COLOR_BTNTEXT);
COLORREF CDrawUtils::sGrayTextColor = ::GetSysColor(COLOR_GRAYTEXT);
COLORREF CDrawUtils::sHiliteColor = ::GetSysColor(COLOR_HIGHLIGHT);
COLORREF CDrawUtils::sHiliteTextColor = ::GetSysColor(COLOR_HIGHLIGHTTEXT);
COLORREF CDrawUtils::sWindowTextColor = ::GetSysColor(COLOR_WINDOWTEXT);

CPen CDrawUtils::sBlackPen(PS_SOLID, 0, CDrawUtils::sBlackColor);
CPen CDrawUtils::sWhitePen(PS_SOLID, 0, CDrawUtils::sWhiteColor);
CPen CDrawUtils::sGrayPen(PS_SOLID, 0, CDrawUtils::sGrayColor);
CPen CDrawUtils::sDkGrayPen(PS_SOLID, 0, CDrawUtils::sDkGrayColor);
CPen CDrawUtils::sSmallHilitePen(PS_SOLID | PS_INSIDEFRAME, 2, CDrawUtils::sHiliteColor);
CPen CDrawUtils::sHilitePen(PS_SOLID | PS_INSIDEFRAME, 3, CDrawUtils::sHiliteColor);
CBrush CDrawUtils::sBlackBrush(CDrawUtils::sBlackColor);
CBrush CDrawUtils::sWhiteBrush(CDrawUtils::sWhiteColor);
CBrush CDrawUtils::sGrayBrush(CDrawUtils::sGrayColor);
CBrush CDrawUtils::sDkGrayBrush(CDrawUtils::sDkGrayColor);

// Draw the appropriate icon and button frame
void CDrawUtils::Draw3DFrame(CDC* pDC, const CRect& rect, bool selected, bool enabled, bool frame, bool filled)
{
	StDCState save(pDC);

	// Draw 3D frame
	if (enabled)
	{
		if (filled)
			pDC->FillRect(&rect, selected && frame ? &sDkGrayBrush : &sGrayBrush);

		if (frame)
		{
			// Black outline
			pDC->SelectObject(&CDrawUtils::sBlackPen);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right - 1, rect.top);
			pDC->MoveTo(rect.right - 1, rect.top);
			pDC->LineTo(rect.right - 1, rect.bottom - 1);
			pDC->MoveTo(rect.right - 1, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.bottom - 1);
			pDC->MoveTo(rect.left, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.top);

			// Outer line #1
			pDC->SelectObject(selected ? &CDrawUtils::sBlackPen : &CDrawUtils::sGrayPen);
			pDC->MoveTo(rect.left + 1, rect.bottom - 3);
			pDC->LineTo(rect.left + 1, rect.top + 1);
			pDC->LineTo(rect.right - 2, rect.top + 1);

			// Outer line #2
			pDC->SelectObject(selected ? &CDrawUtils::sGrayPen : &CDrawUtils::sDkGrayPen);
			pDC->MoveTo(rect.right - 2, rect.top + 2);
			pDC->LineTo(rect.right - 2, rect.bottom - 2);
			pDC->LineTo(rect.left + 1, rect.bottom - 2);

			// Inner line #1
			pDC->SelectObject(selected ? &CDrawUtils::sDkGrayPen : &CDrawUtils::sWhitePen);
			pDC->MoveTo(rect.left + 2, rect.bottom - 4);
			pDC->LineTo(rect.left + 2, rect.top + 2);
			pDC->LineTo(rect.right - 3, rect.top + 2);

			// Inner line #2
			pDC->SelectObject(selected ? &CDrawUtils::sDkGrayPen : &CDrawUtils::sGrayPen);
			pDC->MoveTo(rect.right - 3, rect.top + 3);
			pDC->LineTo(rect.right - 3, rect.bottom - 3);
			pDC->LineTo(rect.left + 2, rect.bottom - 3);
		}
	}
	else
	{
		if (filled)
			pDC->FillRect(&rect, &sGrayBrush);

		if (frame)
		{
			// Dark gray outline
			pDC->SelectObject(&CDrawUtils::sDkGrayPen);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right - 1, rect.top);
			pDC->MoveTo(rect.right - 1, rect.top);
			pDC->LineTo(rect.right - 1, rect.bottom - 1);
			pDC->MoveTo(rect.right - 1, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.bottom - 1);
			pDC->MoveTo(rect.left, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.top);
		}
	}

}

// Draw the appropriate button frame
void CDrawUtils::DrawSimpleFrame(CDC* pDC, const CRect& rect, bool selected,
										bool enabled, bool frame, bool filled)
{
	// Draw simple 3D frame
	StDCState save(pDC);

	if (enabled)
	{
		if (filled)
			pDC->FillRect(&rect, selected && frame ?  &CDrawUtils::sDkGrayBrush : &CDrawUtils::sGrayBrush);

		if (frame)
		{
			// Black outline
			pDC->SelectObject(&CDrawUtils::sBlackPen);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
			pDC->MoveTo(rect.right - 1, rect.top);
			pDC->LineTo(rect.right - 1, rect.bottom);
			pDC->MoveTo(rect.right - 1, rect.bottom - 1);
			pDC->LineTo(rect.left - 1, rect.bottom - 1);
			pDC->MoveTo(rect.left, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.top - 1);

			// Outer line #1
			pDC->SelectObject(selected ? &CDrawUtils::sBlackPen : &CDrawUtils::sWhitePen);
			pDC->MoveTo(rect.left + 1, rect.bottom - 2);
			pDC->LineTo(rect.left + 1, rect.top + 1);
			pDC->LineTo(rect.right - 1, rect.top + 1);

			// Outer line #2
			pDC->SelectObject(selected ? &CDrawUtils::sGrayPen : &CDrawUtils::sDkGrayPen);
			pDC->MoveTo(rect.right - 2, rect.top + 2);
			pDC->LineTo(rect.right - 2, rect.bottom - 2);
			pDC->LineTo(rect.left, rect.bottom - 2);
		}
	}
	else
	{
		if (filled)
			pDC->FillRect(&rect, &CDrawUtils::sGrayBrush);

		if (frame)
		{
			// Dark gray outline
			pDC->SelectObject(&CDrawUtils::sDkGrayPen);
			pDC->MoveTo(rect.left, rect.top);
			pDC->LineTo(rect.right, rect.top);
			pDC->MoveTo(rect.right - 1, rect.top);
			pDC->LineTo(rect.right - 1, rect.bottom);
			pDC->MoveTo(rect.right - 1, rect.bottom - 1);
			pDC->LineTo(rect.left - 1, rect.bottom - 1);
			pDC->MoveTo(rect.left, rect.bottom - 1);
			pDC->LineTo(rect.left, rect.top - 1);
		}
	}
}

// Draw the appropriate button frame
void CDrawUtils::DrawSimpleLine(CDC* pDC, const CRect& rect, bool vertical, bool selected, bool enabled)
{
	// Draw simple 3D frame
	StDCState save(pDC);

	if (enabled)
	{
		// Line #1
		pDC->SelectObject(selected ? &CDrawUtils::sWhitePen : &CDrawUtils::sDkGrayPen);
		pDC->MoveTo(rect.left, rect.top);
		pDC->LineTo(vertical ? rect.left : rect.right - 1, vertical ? rect.bottom - 1 : rect.top);

		// Line #2
		pDC->SelectObject(selected ? &CDrawUtils::sDkGrayPen : &CDrawUtils::sWhitePen);
		pDC->MoveTo(rect.left + (vertical ? 1 : 0), rect.top + (vertical ? 0 : 1));
		pDC->LineTo(vertical ? rect.left + 1 : rect.right - 1, vertical ? rect.bottom - 1 : rect.top + 1);
	}
}

// Draw the appropriate button frame
void CDrawUtils::DrawFocusFrame(CDC* pDC, const CRect& rect, bool small_frame)
{
	// Draw focus frame
	StDCState save(pDC);

	// Hilite outline
	pDC->SelectObject(small_frame ? &CDrawUtils::sSmallHilitePen : &CDrawUtils::sHilitePen);
	pDC->Rectangle(rect);
}
