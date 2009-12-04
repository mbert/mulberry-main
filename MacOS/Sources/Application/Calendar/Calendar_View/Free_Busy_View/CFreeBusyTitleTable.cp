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

// ===========================================================================
//	PPxStaticText.cp			PowerPlantX 1.0			©2003 Metrowerks Corp.
// ===========================================================================

#include "CFreeBusyTitleTable.h"

#include "CFreeBusyTable.h"
#include "CGUtils.h"

#include "CICalendarLocale.h"

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CFreeBusyTitleTable														  [public]
/**
	Default constructor */

CFreeBusyTitleTable::CFreeBusyTitleTable(LStream *inStream)
	: CSimpleTitleTable(inStream)
{
	mTable = NULL;
}


// ---------------------------------------------------------------------------
//	~CFreeBusyTitleTable														  [public]
/**
	Destructor */

CFreeBusyTitleTable::~CFreeBusyTitleTable()
{
}

#pragma mark -

void CFreeBusyTitleTable::FinishCreateSelf()
{
	CSimpleTitleTable::FinishCreateSelf();

	SDimension16 frame;
	GetFrameSize(frame);
	SetRowHeight(frame.height, 1, 1);

	AdaptToNewSurroundings();
}

void CFreeBusyTitleTable::TableChanged()
{
	// Only if we have a table
	if (mTable == NULL)
		return;

	// Sync column count
	TableIndexT rows;
	TableIndexT cols;
	mTable->GetTableSize(rows, cols);
	if (mCols != cols)
	{
		// Add/remove required amount
		if (mCols > cols)
			RemoveCols(mCols - cols, 2, false);
		else
			InsertCols(cols - mCols, mCols, NULL);
	}
	
	// Sync column widths
	SetColWidth(mTable->GetColWidth(1), 1, 1);
	SetColWidth(mTable->GetColWidth(2), 2, mCols);

	// Reset titles
	mTitles.clear();
}

void CFreeBusyTitleTable::DrawCell(const STableCell& inCell, const Rect &inLocalQDRect)
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

	// cache suitable titles
	if (mTitles.empty())
		GenerateTitles(inContext);

	// First column is the year
	MyCFString txt(mTitles[inCell.col - 1], kCFStringEncodingUTF8);
	adjustedRect = ::CGRectInset(adjustedRect, 2.0, 0.0);
	Rect box;
	CGUtils::HIToQDRect(adjustedRect, box);
	::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
	::DrawThemeTextBox(txt, kThemeSmallSystemFont, (IsActive() && IsEnabled()) ? kThemeStateActive : kThemeStateInactive, false, &box, teJustLeft, inContext);
}

void CFreeBusyTitleTable::GenerateTitles(CGContextRef inContext)
{
	// Fill each string
	mTitles.clear();
	STableCell cell(2, 2);
	for(cell.col = 2; cell.col <= mCols; cell.col++)
	{
		iCal::CICalendarDateTime dt = mTable->GetTimedStartDate(cell.col).GetAdjustedTime();
		
		mTitles.push_back(dt.GetTime(false, !iCal::CICalendarLocale::Use24HourTime()));
	}
	
	// Always add empty first item
	cdstring temp;
	mTitles.insert(mTitles.begin(), temp);
}
