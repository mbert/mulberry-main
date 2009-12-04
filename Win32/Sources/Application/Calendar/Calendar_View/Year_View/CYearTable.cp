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
#include "CDrawUtils.h"
#include "CMulberryCommon.h"

#include <WIN_LTableMultiGeometry.h>

const uint32_t cMonthTitleHeight = 16;

BEGIN_MESSAGE_MAP(CYearTable, CCalendarTableBase)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CYearTable														  [public]
/**
	Default constructor */

CYearTable::CYearTable()
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
	for(uint32_t i = 0; i < 12; i++)
	{
		delete mMonths[i];
		mMonths[i] = NULL;
	}
}

#pragma mark -
int CYearTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarTableBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create each indicator before doing anything else
	for(uint32_t i = 0; i < 12; i++)
	{
		CRect frame(0, 0, 140, 140);

		mMonths[i] = CMonthIndicator::Create(this, frame);
		mMonths[i]->Add_Listener(this);
	}

	InsertCols(3, 0, NULL);
	InsertRows(4, 0, NULL);
	
	ResetFrame();
	
	return 0;
}

// Resize columns
void CYearTable::OnSize(UINT nType, int cx, int cy)
{
	CCalendarTableBase::OnSize(nType, cx, cy);

	if (!mChanging)
		ResetFrame();
}

void CYearTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	// Broadcast double-click if in month area
	iCal::CICalendarDateTime dt(mYear, (inCell.row - 1) * mCols + inCell.col, 1);
	Broadcast_Message(eBroadcast_DblClkToMonth, &dt);
}

void CYearTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	CRect adjustedRect = inLocalRect;
	adjustedRect.left += mSubCellSize / 2;
	adjustedRect.right -= mSubCellSize / 2;
	adjustedRect.bottom = adjustedRect.top + cMonthTitleHeight;

	StDCState		save(pDC);

	// Do background
	pDC->SetBkColor(CCalendarUtils::GetWinColor(0.0, 0.25, 0.5));
	pDC->ExtTextOut(adjustedRect.left, adjustedRect.top, ETO_OPAQUE, adjustedRect, _T(""), 0, NULL);

	// Get month name as text
	iCal::CICalendarDateTime dt(mYear, (inCell.row - 1) * mCols + inCell.col, 1);
	cdstring month = dt.GetMonthText(false);

	pDC->SetTextColor(CDrawUtils::sWhiteColor);
	::DrawClippedStringUTF8(pDC, month, CPoint(adjustedRect.left, adjustedRect.top), adjustedRect, eDrawString_Center);
}

void CYearTable::ResetFrame()
{
	// Get super frame's width - scroll bar
	CRect my_frame;
	GetClientRect(my_frame);

	if ((mRows == 0) || (mCols == 0))
		return;

	StDeferTableAdjustment _defer(this);

	// Determine the indicator column size
	int32_t ind_row_count = 0;
	STableCell cell;
	for(cell.row = 1; cell.row <= mRows; cell.row++)
	{
		// Get maximum rows from indicators in the row
		int32_t max_row_count = 0;
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			TableIndexT rows, cols;
			GetIndicator(cell)->GetTableSize(rows, cols);
			max_row_count = max(max_row_count, (int32_t)rows);
		}
		
		// Set row height
		ind_row_count += max_row_count + 1;
	}
	int32_t ind_col_count = 8 * mCols;

	// Determine the smallest size
	int32_t ind_row_size = my_frame.Height() / ind_row_count;
	int32_t ind_col_size = my_frame.Width() / ind_col_count;
	mSubCellSize = min(ind_row_size, ind_col_size);
	
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
			
			::ResizeWindowTo(GetIndicator(cell), cols * mSubCellSize, rows * mSubCellSize, false);
		}
	}
	
	// Set new row and column sizes for this table
	int32_t total_row_height = 0;
	for(cell.row = 1; cell.row <= mRows; cell.row++)
	{
		// Get maximum rows from indicators in the row
		int32_t max_row_count = 0;
		for(cell.col = 1; cell.col <= mCols; cell.col++)
		{
			TableIndexT rows, cols;
			GetIndicator(cell)->GetTableSize(rows, cols);
			max_row_count = max(max_row_count, (int32_t)rows);
		}
		
		// Set row height
		SetRowHeight((max_row_count + 1) * mSubCellSize, cell.row, cell.row);
		total_row_height += (max_row_count + 1) * mSubCellSize;
	}

	SetColWidth(mSubCellSize * 8, 1, mCols);

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

			CRect r;
			GetLocalCellRectAlways(cell, r);
			
			::MoveWindowTo(ind, r.left + mSubCellSize / 2, r.top + cMonthTitleHeight);
		}
}

