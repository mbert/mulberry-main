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

#include "CYearTable.h"

#include "CGUtils.h"
#include "CMonthIndicator.h"

#include <LTableMultiGeometry.h>
#include "MyCFString.h"

const uint32_t cMonthTitleHeight = 16;

// ---------------------------------------------------------------------------
//	CYearTable														  [public]
/**
	Default constructor */

CYearTable::CYearTable(LStream *inStream) :
	CCalendarTableBase(inStream)
{
	// Multi geometry (some rows will be bigger than others due to 4/5 week months)
	mTableGeometry = new LTableMultiGeometry(this, 140, 140);
	
	mYear = 0;
	mLayout = NCalendarView::e4x3;
	mLastSelected = NULL;
	mSubCellSize = 16;
}


// ---------------------------------------------------------------------------
//	~CYearTable														  [public]
/**
	Destructor */

CYearTable::~CYearTable()
{
}

#pragma mark -
void
CYearTable::FinishCreateSelf()
{
	// Call inherited
	CCalendarTableBase::FinishCreateSelf();

	InsertCols(3, 0, NULL);
	InsertRows(4, 0, NULL);
	
	// Create each indicator
	for(uint32_t i = 0; i < 12; i++)
	{
		HIRect frame;
		frame.origin.x = frame.origin.y = 0;
		frame.size.width = frame.size.height = 140;

		mMonths[i] = CMonthIndicator::Create(this, frame);
		mMonths[i]->Add_Listener(this);
	}

	AdaptToNewSurroundings();
}

void CYearTable::ClickCell(const STableCell& inCell, const SMouseDownEvent& inMouseDown)
{
	// Broadcast double-click if in month area
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
	{
		iCal::CICalendarDateTime dt(mYear, (inCell.row - 1) * mCols + inCell.col, 1);
		Broadcast_Message(eBroadcast_DblClkToMonth, &dt);
	}
	else
		CCalendarTableBase::ClickCell(inCell, inMouseDown);
}

void CYearTable::DrawCell(const STableCell &inCell, const Rect &inLocalQDRect)
{
	HIRect inLocalRect;
	CGUtils::QDToHIRect(inLocalQDRect, inLocalRect);
	HIRect adjustedRect = inLocalRect;
	adjustedRect.origin.x += mSubCellSize / 2;
	adjustedRect.size.width -= mSubCellSize;
	adjustedRect.size.height = cMonthTitleHeight;

	CGUtils::CGContextFromQD inContext(GetMacPort());
	CGUtils::CGContextSaver _cg(inContext);

	// Do background
	::CGContextSetRGBFillColor(inContext, 0.0, 0.25, 0.5, 1.0);
	::CGContextFillRect(inContext, adjustedRect);

	// Get month name as text
	iCal::CICalendarDateTime dt(mYear, (inCell.row - 1) * mCols + inCell.col, 1);
	MyCFString month(dt.GetMonthText(false), kCFStringEncodingUTF8);

	Rect box;
	CGUtils::HIToQDRect(adjustedRect, box);
	::CGContextSetGrayFillColor(inContext, 1.0, 1.0);
	::DrawThemeTextBox(month, kThemeSystemFont, kThemeStateActive, false, &box, teJustCenter, inContext);
}

void CYearTable::AdaptToNewSurroundings()
{
	// Allow frame adapter to adjust size
	CCalendarTableBase::AdaptToNewSurroundings();

	// Get super frame's width - scroll bar
	SDimension16 my_frame;
	GetSuperView()->GetFrameSize(my_frame);
	SPoint32 pos;
	GetFrameLocation(pos);
	my_frame.height -= pos.v;

	// Determine the indicator column size
	SInt32 ind_row_count = 0;
	STableCell cell;
	for(cell.row = 1; cell.row <= mRows; cell.row++)
	{
		// Get maximum rows from indicators in the row
		SInt32 max_row_count = 0;
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			TableIndexT rows, cols;
			GetIndicator(cell)->GetTableSize(rows, cols);
			max_row_count = std::max(max_row_count, (SInt32)rows);
		}
		
		// Set row height
		ind_row_count += max_row_count + 1;
	}
	SInt32 ind_col_count = 8 * mCols;

	// Determine the smallest size
	SInt32 ind_row_size = my_frame.height / ind_row_count;
	SInt32 ind_col_size = my_frame.width / ind_col_count;
	mSubCellSize = std::min(ind_row_size, ind_col_size);
	
	// Never go below 16 pixels square
	if (mSubCellSize < 16)
		mSubCellSize = 16;
	
	// Now set each indicator to new size
	for(cell.row = 1; cell.row <= mRows; cell.row++)
	{
		// Get maximum rows from indicators in the row
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			TableIndexT rows, cols;
			GetIndicator(cell)->GetTableSize(rows, cols);
			GetIndicator(cell)->SetRowHeight(mSubCellSize, 1, rows);
			GetIndicator(cell)->SetColWidth(mSubCellSize, 1, cols);
			
			SDimension16 iframe;
			GetIndicator(cell)->GetFrameSize(iframe);
			iframe.width = cols * mSubCellSize;
			iframe.height = rows * mSubCellSize;
			GetIndicator(cell)->ResizeFrameTo(iframe.width, iframe.height, false);
		}
	}
	
	// Set new row and column sizes for this table
	SInt32 total_row_height = 0;
	for(cell.row = 1; cell.row <= mRows; cell.row++)
	{
		// Get maximum rows from indicators in the row
		SInt32 max_row_count = 0;
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			TableIndexT rows, cols;
			GetIndicator(cell)->GetTableSize(rows, cols);
			max_row_count = std::max(max_row_count, (SInt32)rows);
		}
		
		// Set row height
		SetRowHeight((max_row_count + 1) * mSubCellSize, cell.row, cell.row);
		total_row_height += (max_row_count + 1) * mSubCellSize;
	}

	SetColWidth(mSubCellSize * 8, 1, mCols);

	RespositionIndicators();
}

void CYearTable::ResetFrame()
{
	AdaptToNewSurroundings();
	
	// Now put each indicator in the appropriate cell
	RespositionIndicators();
}

void CYearTable::DoIndicatorSelectionChanged(CMonthIndicator* indicator)
{
	// NULL implies remove all selection
	if (indicator == NULL)
	{
		if (mLastSelected != NULL)
		{
			Stop_Listening();
			mLastSelected->UnselectAllCells();
			Start_Listening();
		}
		mLastSelected = NULL;
	}
	else
	{
		if ((mLastSelected != indicator) && (mLastSelected != NULL))
		{
			Stop_Listening();
			mLastSelected->UnselectAllCells();
			Start_Listening();
		}
		mLastSelected = indicator;
	}
}

void CYearTable::RespositionIndicators()
{
	// Clear all cell data
	STableCell cell(1, 1);
	for(cell.row = 1; cell.row <= mRows; cell.row++)
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			CMonthIndicator* ind = GetIndicator(cell);

			SInt32	cellLeft, cellTop, cellRight, cellBottom;
			GetImageCellBounds(cell, cellLeft, cellTop, cellRight, cellBottom);
			
			SPoint32 ind_frame;
			ind->GetFrameLocation(ind_frame);
			ind_frame.h = cellLeft + mSubCellSize / 2;
			ind_frame.v = cellTop + cMonthTitleHeight;
			ind->PlaceInSuperFrameAt(ind_frame.h, ind_frame.v, true);
		}
}
