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

#include "CGUtils.h"
#include "CPreferences.h"
#include "CXStringResources.h"

#include "CICalendarLocale.h"

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CMonthTitleTable														  [public]
/**
	Default constructor */

CMonthTitleTable::CMonthTitleTable(LStream *inStream)
	: CSimpleTitleTable(inStream)
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

void CMonthTitleTable::FinishCreateSelf()
{
	CSimpleTitleTable::FinishCreateSelf();

	InsertCols(6, 0, NULL);
	
	SDimension16 frame;
	GetFrameSize(frame);
	SetRowHeight(frame.height, 1, 1);

	AdaptToNewSurroundings();
}

void CMonthTitleTable::AdaptToNewSurroundings()
{
	// Allow frame adapter to adjust size
	CSimpleTitleTable::AdaptToNewSurroundings();

	SDimension16 my_frame;
	GetFrameSize(my_frame);

	// Determine new row and column sizes
	SInt32 col_size = (mCols == 0) ? 64L : std::max((SInt32) (my_frame.width / mCols), 64L);
	
	SetColWidth(col_size, 1, mCols);
}

void CMonthTitleTable::DrawCell(const STableCell& inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;
	adjustedRect.size.width += 1.0;
	adjustedRect.size.height -= 2.0;
	
	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	// Draw header as themed button
	{
		Rect r;
		CGUtils::HIToQDRect(adjustedRect, r);
		ThemeButtonDrawInfo theme;
		theme.state = (IsActive() && IsEnabled()) ? kThemeStateActive : kThemeStateInactive;
		theme.value = kThemeButtonOff;
		theme.adornment = kThemeAdornmentNone;
		::DrawThemeButton(&r, kThemeSmallBevelButton, &theme, NULL, NULL, NULL, NULL);
	}

	adjustedRect = ::CGRectInset(adjustedRect, 2.0, 2.0);
	
	uint32_t indexit = (inCell.col - 1 + CPreferences::sPrefs->mWeekStartDay.GetValue()) % 7;
	MyCFString temp(iCal::CICalendarLocale::GetDay(indexit, (GetColWidth(1) < 96) ? iCal::CICalendarLocale::eShort : iCal::CICalendarLocale::eLong), kCFStringEncodingUTF8);

	Rect box;
	CGUtils::HIToQDRect(adjustedRect, box);
	::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
	::DrawThemeTextBox(temp, kThemeSystemFont, kThemeStateActive, false, &box, teJustCenter, inContext);
}
