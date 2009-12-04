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

#include "CSummaryTitleTable.h"

#include "CGUtils.h"
#include "CMulberryCommon.h"
#include "CSummaryTable.h"
#include "CXStringResources.h"

// ---------------------------------------------------------------------------
//	CSummaryTitleTable														  [public]
/**
	Default constructor */

CSummaryTitleTable::CSummaryTitleTable(LStream *inStream)
	: CSimpleTitleTable(inStream)
{
	mTable = NULL;
	mPrinting = false;
}


// ---------------------------------------------------------------------------
//	~CSummaryTitleTable														  [public]
/**
	Destructor */

CSummaryTitleTable::~CSummaryTitleTable()
{
}

#pragma mark -

void CSummaryTitleTable::FinishCreateSelf()
{
	CSimpleTitleTable::FinishCreateSelf();

	LoadTitles("UI::Titles::CalendarSummary", 6);

	SDimension16 frame;
	GetFrameSize(frame);
	SetRowHeight(frame.height, 1, 1);
}

void CSummaryTitleTable::TableChanged()
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
	for(TableIndexT col = 1; col <= mCols; col++)
		SetColWidth(mTable->GetColWidth(col), col, col);
}

void CSummaryTitleTable::DrawCell(const STableCell& inCell, const Rect &inLocalQDRect)
{
	if (!mPrinting)
		CTitleTable::DrawCell(inCell, inLocalQDRect);
	else
	{
		HIRect inLocalRect;
		CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
		HIRect adjustedRect = inLocalRect;
		
		CGUtils::CGContextFromQD inContext(GetMacPort());
		CGUtils::CGContextSaver _cg(inContext);

		// Draw cell frame
		{
			// Draw frame components by hand (no aliasing for lines)
			::CGContextSetShouldAntialias(inContext, false);
			
			::CGContextBeginPath(inContext);
			::CGContextAddRect(inContext, adjustedRect);
			::CGContextSetGrayStrokeColor(inContext, 0.5, 1.0);
			::CGContextStrokePath(inContext);
			//::CGContextClosePath(inContext);
			
			// Turn on aliasing for text
			::CGContextSetShouldAntialias(inContext, true);
		}

		// First column is the year
		MyCFString txt(mTitles[inCell.col - 1], kCFStringEncodingUTF8);
		adjustedRect = ::CGRectInset(adjustedRect, 2.0, 0.0);
		Rect box;
		CGUtils::HIToQDRect(adjustedRect, box);
		::CGContextSetGrayFillColor(inContext, 0.0, 1.0);
		::DrawThemeTextBox(txt, kThemeSmallSystemFont, (IsActive() && IsEnabled()) ? kThemeStateActive : kThemeStateInactive, false, &box, teJustCenter, inContext);
	}
}

