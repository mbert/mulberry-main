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

#include "CDayWeekTitleTable.h"

#include "CDayWeekTable.h"
#include "CGUtils.h"

#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CDayWeekTitleTable														  [public]
/**
	Default constructor */

CDayWeekTitleTable::CDayWeekTitleTable(LStream *inStream)
	: CSimpleTitleTable(inStream)
{
	mTable = NULL;
}


// ---------------------------------------------------------------------------
//	~CDayWeekTitleTable														  [public]
/**
	Destructor */

CDayWeekTitleTable::~CDayWeekTitleTable()
{
}

#pragma mark -

void CDayWeekTitleTable::FinishCreateSelf()
{
	CSimpleTitleTable::FinishCreateSelf();

	SDimension16 frame;
	GetFrameSize(frame);
	SetRowHeight(frame.height, 1, 1);

	AdaptToNewSurroundings();
}

void CDayWeekTitleTable::TableChanged()
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

void CDayWeekTitleTable::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
{
	// Broadcast double-click
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
	{
		STableCell cell(2, inCell.col);
		iCal::CICalendarDateTime dt( mTable->GetCellStartDate(cell));
		Broadcast_Message(eBroadcast_DblClick, &dt);
	}
	else
		CSimpleTitleTable::ClickCell(inCell, inMouseDown);
}

void CDayWeekTitleTable::DrawCell(const STableCell& inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;
	adjustedRect.size.width += 1.0;
	adjustedRect.size.height -= 2.0;
	
	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	// Look for selected column
	bool selected = false;
	if (mTable != NULL)
	{
		STableCell selCell = mTable->GetFirstSelectedCell();
		selected = !selCell.IsNullCell() && (selCell.col > 1) && (inCell.col == selCell.col);
	}

	// Draw header as themed button
	{
		Rect r;
		CGUtils::HIToQDRect(adjustedRect, r);
		ThemeButtonDrawInfo theme;
		theme.state = (IsActive() && IsEnabled()) ? (selected ? kThemeStatePressed : kThemeStateActive) : kThemeStateInactive;
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
	if ((mTable != NULL) && mTable->IsTodayColumn(inCell))
		::CGContextSetRGBFillColor(inContext, 0.0, 0.0, 1.0, 1.0);
	else
		::CGContextSetGrayFillColor(inContext, selected ? 1.0 : 0.0, 1.0);
	::DrawThemeTextBox(txt, kThemeSmallSystemFont, (IsActive() && IsEnabled()) ? kThemeStateActive : kThemeStateInactive, false, &box, teJustCenter, inContext);
}

void CDayWeekTitleTable::GenerateTitles(CGContextRef inContext)
{
	// Clear existing titles
	mTitles.clear();
	
	// Determine format for title
	
	// Try the following formats (but localised) to see what fits the column width:
	// <<full day name>>, <<full month>> <<day number>>
	// <<short day name>>, <<short month>> <<day number>>
	// <<month number>>/<<day number>>
	// 
	
	// Fill each string
	mTitles.clear();
	STableCell cell(2, 2);
	for(cell.col = 2; cell.col <= mCols; cell.col++)
	{
		const iCal::CICalendarDateTime& dt = mTable->GetCellStartDate(cell);
		
		mTitles.push_back(dt.GetLocaleDate(iCal::CICalendarDateTime::eFullDateNoYear));
		
		// See if too large for column width
		UInt16 width = GetColWidth(cell.col);
		width -= 4;
		
		// Attempt to truncate text 
		MyCFString trunc(mTitles.back(), kCFStringEncodingUTF8);
		Boolean truncd = false;
		::TruncateThemeText(trunc, kThemeSmallSystemFont, kThemeStateActive, width, truncEnd, &truncd);
		if (truncd)
		{
			mTitles.clear();
			break;
		}
	}
	
	// Try again with next shortest type if none were added
	if (mTitles.empty())
	{
		STableCell cell(2, 2);
		for(cell.col = 2; cell.col <= mCols; cell.col++)
		{
			const iCal::CICalendarDateTime& dt = mTable->GetCellStartDate(cell);
			
			mTitles.push_back(dt.GetLocaleDate(iCal::CICalendarDateTime::eAbbrevDateNoYear));
			
			// See if too large for column width
			UInt16 width = GetColWidth(cell.col);
			width -= 4;
			
			// Attempt to truncate text 
			MyCFString trunc(mTitles.back(), kCFStringEncodingUTF8);
			Boolean truncd = false;
			::TruncateThemeText(trunc, kThemeSmallSystemFont, kThemeStateActive, width, truncEnd, &truncd);
			if (truncd)
			{
				mTitles.clear();
				break;
			}
		}
	}
	
	// Try again with next shortest type if none were added - these we must always use as they are the shortest
	if (mTitles.empty())
	{
		STableCell cell(2, 2);
		for(cell.col = 2; cell.col <= mCols; cell.col++)
		{
			const iCal::CICalendarDateTime& dt = mTable->GetCellStartDate(cell);
			
			mTitles.push_back(dt.GetLocaleDate(iCal::CICalendarDateTime::eNumericDateNoYear));
		}
	}
	
	// Always add year as the first item
	cdstring temp((long)mTable->GetCellStartDate(STableCell(2, 2)).GetYear());
	mTitles.insert(mTitles.begin(), temp);
}
