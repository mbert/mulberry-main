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


//	CTableRowSelector.cp

#include "CTableRowSelector.h"

// Constructor
CTableRowSelector::CTableRowSelector(LTableView *inTableView)
	: LTableMultiSelector(inTableView)

{
}

// Destructor
CTableRowSelector::~CTableRowSelector()
{
}

// Select row
void CTableRowSelector::SelectCell(const STableCell &inCell)
{
	// Select all columns in this row
	STableCell select(inCell.row, 0);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	for(TableIndexT col = 1; col <= cols; col++)
	{
		select.col = col;
		LTableMultiSelector::SelectCell(select);
	}
}

// Select one row
void CTableRowSelector::SelectOneCell(const STableCell &inCell)
{
	STableCell select(inCell.row, 0);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	if ( mTableView->IsValidCell(inCell))
	{
		bool was_selected = CellIsSelected(inCell);
		
		// Remove from selection list prior to unhiliting current selection
		if (was_selected)
		{
			for(TableIndexT col = 1; col <= cols; col++)
			{
				select.col = col;
				mSelection[GetSelectionIndex(select)] = false;
			}
		}

		bool val = true;
		size_t sel_count = std::count(mSelection.begin(), mSelection.end(), val);
		bool sel_changed = (sel_count > 0);
		
		// Now unhilite current selection and empty it
		mTableView->HiliteSelection(true, false);

		std::fill(mSelection.begin(), mSelection.end(), false);
		mFirstSelection = mLastSelection = STableCell(0, 0);

		mAnchorCell.SetCell(0, 0);
		mBoatCell.SetCell(0, 0);
		
		// Add in the cells we want
		for(TableIndexT col = 1; col <= cols; col++)
		{
			select.col = col;
			mSelection[GetSelectionIndex(select)] = true;
			
			if (mFirstSelection.IsNullCell())
				mBoatCell = mAnchorCell = mFirstSelection = mLastSelection = select;
			else
			{
				if (select < mFirstSelection)
					mBoatCell = mFirstSelection = select;
				if (select > mLastSelection)
					mBoatCell = mLastSelection = select;
			}
			
			// Hilite it if it was not previously selected
			if (!was_selected)
			{
				mTableView->HiliteCell(select, true);
			}
		}

		if (sel_changed || !was_selected)
			mTableView->SelectionChanged();
	}
}

// Unselect row
void CTableRowSelector::UnselectCell(const STableCell &inCell)
{
	// Unselect all columns in this row
	STableCell select(inCell.row, 0);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	for(TableIndexT col = 1; col <= cols; col++)
	{
		select.col = col;
		LTableMultiSelector::UnselectCell(select);
	}
}

// Select row block
void CTableRowSelector::SelectCellBlock(const STableCell &inCellA, const STableCell &inCellB)
{
	STableCell cellA(inCellA);
	STableCell cellB(inCellB);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);
	
	if (inCellA.row <= inCellB.row)
	{
		cellA.col = 1;
		cellB.col = cols;
	}
	else if (inCellA.row > inCellB.row)
	{
		cellA.col = cols;
		cellB.col = 1;
	}
	
	LTableMultiSelector::SelectCellBlock(cellA, cellB);
}

#pragma mark -

// Constructor
CTableSingleRowSelector::CTableSingleRowSelector(LTableView *inTableView)
	: LTableMultiSelector(inTableView)

{
}

// Destructor
CTableSingleRowSelector::~CTableSingleRowSelector()
{
}

// Select row
void CTableSingleRowSelector::SelectCell(const STableCell &inCell)
{
	// Select all columns in this row
	STableCell select(inCell.row, 0);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	UnselectAllCells();

	for(TableIndexT col = 1; col <= cols; col++)
	{
		select.col = col;
		LTableMultiSelector::SelectCell(select);
	}
}

// Select row
void CTableSingleRowSelector::SelectOneCell(const STableCell &inCell)
{
	// Just select if not already
	if (!CellIsSelected(inCell))
		SelectCell(inCell);
}

//	Adjust selection when clicking on a specified cell

void CTableSingleRowSelector::ClickSelect(const STableCell &inCell, UINT nFlags, bool one_only)
{
	UINT modified = nFlags;
	modified &= ~(MK_CONTROL | MK_SHIFT);
	LTableMultiSelector::ClickSelect(inCell, modified, one_only);
}

// Unselect row
void CTableSingleRowSelector::UnselectCell(const STableCell &inCell)
{
	// Unselect all columns in this row
	STableCell select(inCell.row, 0);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	for(TableIndexT col = 1; col <= cols; col++)
	{
		select.col = col;
		LTableMultiSelector::UnselectCell(select);
	}
}

