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

#include "CMonthTitleTable.h"

#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

BEGIN_MESSAGE_MAP(CMonthTitleTable, CSimpleTitleTable)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CMonthTitleTable														  [public]
/**
	Default constructor */

CMonthTitleTable::CMonthTitleTable()
{
}

// ---------------------------------------------------------------------------
//	~CMonthTitleTable														  [public]
/**
	Destructor */

CMonthTitleTable::~CMonthTitleTable()
{
}

#pragma mark -

int CMonthTitleTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CSimpleTitleTable::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	InsertCols(6, 0, NULL);
	
	return 0;
}

// Resize columns
void CMonthTitleTable::OnSize(UINT nType, int cx, int cy)
{
	CSimpleTitleTable::OnSize(nType, cx, cy);

	// Determine new row and column sizes
	SInt32 col_size = max((SInt32) (cx / mCols), 64L);
	
	SetColWidth(col_size, 1, mCols);
}

// Draw a row
void CMonthTitleTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	// Draw frame
	pDC->FillRect(&inLocalRect, &CDrawUtils::sGrayBrush);
	pDC->SelectObject(CDrawUtils::sWhitePen);
	pDC->MoveTo(inLocalRect.left, inLocalRect.bottom - 1);
	pDC->LineTo(inLocalRect.left, inLocalRect.top);
	pDC->LineTo(inLocalRect.right, inLocalRect.top);
	pDC->SelectObject(CDrawUtils::sDkGrayPen);
	pDC->MoveTo(inLocalRect.right - 2, inLocalRect.top + 1);
	pDC->LineTo(inLocalRect.right - 2, inLocalRect.bottom - 2);
	pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 2);
	pDC->SelectObject(CDrawUtils::sBlackPen);
	pDC->MoveTo(inLocalRect.right - 1, inLocalRect.top);
	pDC->LineTo(inLocalRect.right - 1, inLocalRect.bottom - 1);
	pDC->LineTo(inLocalRect.left, inLocalRect.bottom - 1);
	pDC->SetTextColor(CDrawUtils::sBtnTextColor);
	pDC->SetBkColor(CDrawUtils::sGrayColor);

	uint32_t index = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
	cdstring txt(iCal::CICalendarLocale::GetDay(index, (inLocalRect.Width() < 96) ? iCal::CICalendarLocale::eShort : iCal::CICalendarLocale::eLong));
	pDC->SelectObject(CFontCache::GetListFont());
	::DrawClippedStringUTF8(pDC, txt, CPoint(inLocalRect.left + 6, inLocalRect.top + 1), inLocalRect, eDrawString_Center);
}
