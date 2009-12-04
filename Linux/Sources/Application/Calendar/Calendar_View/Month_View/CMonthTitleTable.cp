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

#include "CCalendarUtils.h"
#include "CDrawUtils.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

// ---------------------------------------------------------------------------
//	CMonthTitleTable														  [public]
/**
	Default constructor */

CMonthTitleTable::CMonthTitleTable(JXScrollbarSet* scrollbarSet,
				     JXContainer* enclosure,
				     const HSizingOption hSizing, 
				     const VSizingOption vSizing,
				     const JCoordinate x, const JCoordinate y,
				     const JCoordinate w, const JCoordinate h)
	: CSimpleTitleTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
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

void CMonthTitleTable::OnCreate()
{
	CSimpleTitleTable::OnCreate();

	InsertCols(7, 0, NULL);

	ApertureResized(0, 0);
}

void CMonthTitleTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Allow frame adapter to adjust size
	CSimpleTitleTable::ApertureResized(dw, dh);

	if ((mRows != 0) && (mCols != 0))
	{
		// Determine new row and column sizes
		SInt32 col_size = std::max((SInt32) (GetApertureWidth() / mCols), 64L);
		
		SetColWidth(col_size, 1, mCols);
	}
}

void CMonthTitleTable::DrawCell(JPainter* p, const STableCell& inCell, const JRect &cellRect)
{
	CDrawUtils::DrawSimpleBorder(*p, cellRect, false, true);

	JRect adjustedRect(cellRect);
	adjustedRect.bottom--;
	adjustedRect.Shrink(2, 0);
	
	uint32_t index = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
	cdstring txt(iCal::CICalendarLocale::GetDay(index, (adjustedRect.width() < 96) ? iCal::CICalendarLocale::eShort : iCal::CICalendarLocale::eLong));
	::DrawClippedStringUTF8(p, txt, JPoint(adjustedRect.left + 6, adjustedRect.top + 1), adjustedRect, eDrawString_Center);
}
