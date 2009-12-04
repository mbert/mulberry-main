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

#include "CCalendarUtils.h"
#include "CMonthIndicator.h"
#include "CMulberryCommon.h"

#include "StPenState.h"

#include <UNX_LTableMultiGeometry.h>

#include <JPainter.h>
#include <JXColormap.h>

const uint32_t cMonthTitleHeight = 16;

// ---------------------------------------------------------------------------
//	CYearTable														  [public]
/**
	Default constructor */

CYearTable::CYearTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CCalendarTableBase(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
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
CYearTable::OnCreate()
{
	// Call inherited
	CCalendarTableBase::OnCreate();

	InsertCols(3, 0, NULL);
	InsertRows(4, 0, NULL);
	
	// Create each indicator
	for(uint32_t i = 0; i < 12; i++)
	{
		JRect frame(0, 0, 140, 140);

		mMonths[i] = CMonthIndicator::Create(this, frame);
		mMonths[i]->Add_Listener(this);
	}

	ApertureResized(0, 0);
}

void CYearTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods)
{
	// Broadcast double-click if in month area
	iCal::CICalendarDateTime dt(mYear, (inCell.row - 1) * mCols + inCell.col, 1);
	Broadcast_Message(eBroadcast_DblClkToMonth, &dt);
}

void CYearTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	JRect adjustedRect = inLocalRect;
	adjustedRect.left += mSubCellSize / 2;
	adjustedRect.right -= mSubCellSize / 2;
	adjustedRect.bottom = adjustedRect.top + cMonthTitleHeight;

	StPenState		save(pDC);

	// Do background
	JColorIndex cindex;
	GetColormap()->JColormap::AllocateStaticColor(CCalendarUtils::GetRGBColor(0.0, 0.25, 0.5), &cindex);
	pDC->SetPenColor(cindex);
	pDC->SetFilling(kTrue);
	pDC->Rect(adjustedRect);
	pDC->SetFilling(kFalse);

	// Get month name as text
	iCal::CICalendarDateTime dt(mYear, (inCell.row - 1) * mCols + inCell.col, 1);
	cdstring month = dt.GetMonthText(false);

	JFontStyle text_style = pDC->GetFontStyle();
	text_style.color = pDC->GetColormap()->GetWhiteColor();
	pDC->SetFontStyle(text_style);
	::DrawClippedStringUTF8(pDC, month, JPoint(adjustedRect.left, adjustedRect.top), adjustedRect, eDrawString_Center);
}

void CYearTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Allow frame adapter to adjust size
	CCalendarTableBase::ApertureResized(dw, dh);

	// Ignore if no columns/rows setup yet
	if ((mRows == 0) || (mCols == 0))
		return;

	// Get super frame's width - scroll bar
	JRect my_frame = GetApertureGlobal();

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
	SInt32 ind_row_size = my_frame.height() / ind_row_count;
	SInt32 ind_col_size = my_frame.width() / ind_col_count;
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
			
			GetIndicator(cell)->SetSize(cols * mSubCellSize, rows * mSubCellSize);
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
	ApertureResized(0, 0);
	
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
			
			JPoint ind_frame;
			ind_frame.x = cellLeft + mSubCellSize / 2;
			ind_frame.y = cellTop + cMonthTitleHeight;
			ind->Place(ind_frame.x, ind_frame.y);
		}
}
