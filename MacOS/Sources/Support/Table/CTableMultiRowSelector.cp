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


//	CTableMultiRowSelector.cp

#include "CTableMultiRowSelector.h"

#include <LTableSelState.h>
#include <UTBAccessors.h>

#include <algorithm>

//#define DEBUGIT

// ---------------------------------------------------------------------------
//	¥ CTableMultiRowSelector
// ---------------------------------------------------------------------------
//	Constructor, associates Selector with a particular TableView

CTableMultiRowSelector::CTableMultiRowSelector(
	LTableView*		inTableView)

	: LTableSelector(inTableView)
{
	mFirstSelection = 0;
	mLastSelection = 0;
}


// ---------------------------------------------------------------------------
//	¥ ~CTableMultiRowSelector
// ---------------------------------------------------------------------------
//	Destructor

CTableMultiRowSelector::~CTableMultiRowSelector()
{
}


// ---------------------------------------------------------------------------
//	¥ CellIsSelected
// ---------------------------------------------------------------------------
//	Return whether the specified cell is selected

Boolean
CTableMultiRowSelector::CellIsSelected(
	const STableCell	&inCell) const
{
	return mSelection.at(inCell.row - 1);
}

#pragma mark --- Selecting ---

// ---------------------------------------------------------------------------
//	¥ GetFirstSelectedCell
// ---------------------------------------------------------------------------
//	Return the first selected cell, defined as the min row & col (closest to
//  top-left corner)

STableCell
CTableMultiRowSelector::GetFirstSelectedCell() const
{
	return STableCell(mFirstSelection, (mFirstSelection != 0) ? 1 : 0);
}



// ---------------------------------------------------------------------------
//	¥ GetFirstSelectedRow
// ---------------------------------------------------------------------------
//	Return the first selected cell's row, defined as the min row & col (closest to
//  top-left corner)

TableIndexT
CTableMultiRowSelector::GetFirstSelectedRow() const
{
	return mFirstSelection;
}

// ---------------------------------------------------------------------------
//	¥ GetLastSelectedCell
// ---------------------------------------------------------------------------
//	Return the last selected cell, defined as the min row & col (closest to
//  top-left corner)

STableCell
CTableMultiRowSelector::GetLastSelectedCell() const
{
	return STableCell(mLastSelection, (mLastSelection != 0) ? 1 : 0);
}



// ---------------------------------------------------------------------------
//	¥ GetLastSelectedRow
// ---------------------------------------------------------------------------
//	Return the last selected cell's row, defined as the min row & col (closest to
//  top-left corner)

TableIndexT
CTableMultiRowSelector::GetLastSelectedRow() const
{
	return mLastSelection;
}

unsigned long CTableMultiRowSelector::CountSelection() const
{
	unsigned long result = std::count(mSelection.begin(), mSelection.end(), true);
	return result;
}

// ---------------------------------------------------------------------------
//	¥ SelectCell
// ---------------------------------------------------------------------------
//	Select the specified Cell

void
CTableMultiRowSelector::SelectCell(
	const STableCell	&inCell)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	if (!CellIsSelected(inCell)) {

		// Select the specified row
		mSelection[inCell.row - 1] = true;
		
		if (mFirstSelection == 0)
			mFirstSelection = mLastSelection = inCell.row;
		else
		{
			if (inCell.row < mFirstSelection)
				mFirstSelection = inCell.row;
			if (inCell.row > mLastSelection)
				mLastSelection = inCell.row;
		}

		HiliteRow(inCell.row, true);

		mTableView->SelectionChanged();
		mAnchorCell = inCell;
	}

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}


// ---------------------------------------------------------------------------
//	¥ SelectAllCells
// ---------------------------------------------------------------------------
//	Select all the cells in a Table

void
CTableMultiRowSelector::SelectAllCells()
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);
	
	if ((rows > 0) && (cols > 0)) {			// Table has some cells
	
		// Flip all bits in current selection so that unselected cells now look selected
		mSelection.flip();

		// Do hilite of new selected cells
		mAnchorCell.SetCell(1, 1);
		mTableView->HiliteSelection(mTableView->IsActive(), true);

		// Set all items to true
		std::fill(mSelection.begin(), mSelection.end(), true);
		mFirstSelection = 1;
		mLastSelection = rows;

		mTableView->SelectionChanged();
	}

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}


// ---------------------------------------------------------------------------
//	¥ SelectCellBlock
// ---------------------------------------------------------------------------
//	Select cells within the specified range, where CellA and CellB are
//	the corners of a rectangular block of cells.

void
CTableMultiRowSelector::SelectCellBlock(
	const STableCell	&inCellA,
	const STableCell	&inCellB)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	TableIndexT	left, top, right, bottom;
	left = 1;
	right = 2;	
	if (inCellA.row < inCellB.row) {
		top = inCellA.row;
		bottom = inCellB.row;
	} else {
		top = inCellB.row;
		bottom = inCellA.row;
	}
	
	// Quick way if selectin is null
	if (mFirstSelection == 0)
	{
		// Set selection to range
		std::fill(mSelection.begin() + (top - 1), mSelection.end() + bottom, true);
	}
	else
	{
		// Create new selection
		std::vector<bool> old_selection = mSelection;
		unsigned long old_first_index = mFirstSelection - 1;
		unsigned long old_last_index = mLastSelection - 1;
		std::vector<bool> new_selection;
		new_selection.insert(new_selection.begin(), rows, false);
		std::fill(new_selection.begin() + (top - 1), new_selection.begin() + bottom, true);
		unsigned long new_first_index = top - 1;
		unsigned long new_last_index = bottom - 1;

		// Determine difference for unhilite
		std::vector<bool> difference;
		difference.insert(difference.begin(), rows, false);
		mFirstSelection = 0;
		mLastSelection = 0;
		for(unsigned long i = old_first_index; i <= old_last_index; i++)
		{
			if (old_selection[i] && !new_selection[i])
			{
				difference[i] = true;
				if (mFirstSelection == 0)
					mFirstSelection = i + 1;
				mLastSelection = i + 1;
			}
		}

		// Unhilite the difference
		mSelection = difference;
		mTableView->HiliteSelection(mTableView->IsActive(), false);
		
		// Determine difference for hilite
		difference.clear();
		difference.insert(difference.begin(), rows, false);
		mFirstSelection = 0;
		mLastSelection = 0;
		for(unsigned long i = new_first_index; i <= new_last_index; i++)
		{
			if (new_selection[i] && !old_selection[i])
			{
				difference[i] = true;
				if (mFirstSelection == 0)
					mFirstSelection = i + 1;
				mLastSelection = i + 1;
			}
		}
		
		// Hilite the difference
		mSelection = difference;
		mTableView->HiliteSelection(mTableView->IsActive(), true);
		
		// Restore region and dump unwanted
		mSelection = new_selection;
	}

	mFirstSelection = top;
	mLastSelection = bottom;

	mTableView->SelectionChanged();

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}

#pragma mark --- Unselecting ---

// ---------------------------------------------------------------------------
//	¥ UnselectCell
// ---------------------------------------------------------------------------
//	Unselected the specified cell

void
CTableMultiRowSelector::UnselectCell(
	const STableCell	&inCell)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	if (CellIsSelected(inCell)) {
		mSelection[inCell.row - 1] = false;
		if (mFirstSelection == inCell.row)
		{
			if (mFirstSelection == mLastSelection)
				mFirstSelection = mLastSelection = 0;
			else
			{
				TableIndexT rows, columns;
				mTableView->GetTableSize(rows, columns);
				while(!mSelection[++mFirstSelection - 1] && (mFirstSelection <= rows)) {}
			}
		}
		else if (mLastSelection == inCell.row)
			while(!mSelection.at(--mLastSelection - 1) && (mLastSelection > 0)) {}
			
		HiliteRow(inCell.row, false);
		mTableView->SelectionChanged();
	}

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}


// ---------------------------------------------------------------------------
//	¥ UnselectAllCells
// ---------------------------------------------------------------------------
//	Unselect all the cells in a Table

void
CTableMultiRowSelector::UnselectAllCells()
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	if (mFirstSelection != 0) {

		mTableView->HiliteSelection(mTableView->IsActive(), false);

		TableIndexT rows, columns;
		mTableView->GetTableSize(rows, columns);

		std::fill(mSelection.begin(), mSelection.end(), false);
		mFirstSelection = mLastSelection = 0;
		mTableView->SelectionChanged();
	}

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}

#pragma mark --- Mouse Tracking ---

// ---------------------------------------------------------------------------
//	¥ ClickSelect
// ---------------------------------------------------------------------------
//	Adjust selection when clicking on a specified cell

void
CTableMultiRowSelector::ClickSelect(
	const STableCell		&inCell,
	const SMouseDownEvent	&inMouseDown)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	if (inMouseDown.macEvent.modifiers & cmdKey) {
									// Command Key is down
									//   Allow discontinguous selection
		if (CellIsSelected(inCell)) {
			UnselectCell(inCell);	//   Remove Cell from selection
			mAnchorCell.SetCell(0, 0);
		} else {
			SelectCell(inCell);		//   Add Cell to selection
		}

	} else if (inMouseDown.macEvent.modifiers & shiftKey) {
									// Shift Key is down
		if (mAnchorCell.IsNullCell()) {
			if (!CellIsSelected(inCell)) {
				UnselectAllCells();
				SelectCell(inCell);
			}

		} else {					//   Extend selection from anchor cell
			//UnselectAllCells();
			SelectCellBlock(mAnchorCell, inCell);
		}

	} else {
		mAnchorCell = inCell;
		if (!CellIsSelected(inCell)) {
			UnselectAllCells();
			SelectCell(inCell);
		}
	}

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}


// ---------------------------------------------------------------------------
//	¥ DragSelect
// ---------------------------------------------------------------------------
//	Adjust selection while the user is pressing the mouse in a Table
//
//	Return true if the mouse never leaves inCell

Boolean
CTableMultiRowSelector::DragSelect(
	const STableCell		&inCell,
	const SMouseDownEvent	&inMouseDown)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	Boolean		inSameCell = true;

	ClickSelect(inCell, inMouseDown);	// First handle as a normal click

	if (inMouseDown.macEvent.modifiers & cmdKey) {

			// I really have no good idea what to do when the
			// command key is down. Should it toggle the selection
			// of the cells dragged over? Should it set a mode
			// based on the state of the cell originally clicked,
			// and then continue selecting or deselecting the cells
			// dragged over?

			// For the moment, do the simple thing and not support
			// drag select when the command key is down.

		return true;
	}

	STableCell	currCell = inCell;

	while (::StillDown()) {
		STableCell	hitCell;
		SPoint32	imageLoc;
		Point		mouseLoc;
		mTableView->FocusDraw();
		::GetMouse(&mouseLoc);
		if (mTableView->AutoScrollImage(mouseLoc)) {
			mTableView->FocusDraw();
			Rect	frame;
			mTableView->CalcLocalFrameRect(frame);
			SInt32 pt = ::PinRect(&frame, mouseLoc);
			mouseLoc = *(Point*)&pt;
		}
		mTableView->LocalToImagePoint(mouseLoc, imageLoc);
		mTableView->GetCellHitBy(imageLoc, hitCell);

		if (currCell != hitCell) {
			inSameCell = false;

			if (mTableView->IsValidCell(hitCell)) {

				// Right now, all cells between the Anchor cell and
				// Curr cell are selected. New selection will be
				// all cells between the Anchor and the Hit cell.

				// ### This method of unhiliting everything and
				// then hiliting the proper selection will cause
				// a lot of flashing. But it works, and we'll
				// implement a better algorithm in the future.

				//UnselectAllCells();
				SelectCellBlock(mAnchorCell, hitCell);

				currCell = hitCell;
			}
		}
	}


#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
	return inSameCell;
}

#pragma mark --- Adjusting Selection ---

// ---------------------------------------------------------------------------
//	¥ InsertRows
// ---------------------------------------------------------------------------
//	Adjust selection when inserting rows into a Table

void
CTableMultiRowSelector::InsertRows(
	UInt32			inHowMany,
	TableIndexT		inAfterRow)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	mSelection.insert(mSelection.begin() + inAfterRow, inHowMany, false);
	if (mFirstSelection > inAfterRow)
		mFirstSelection += inHowMany;
	if (mLastSelection > inAfterRow)
		mLastSelection += inHowMany;

	if (mAnchorCell.row > inAfterRow) {		// Adjust anchor cell
		mAnchorCell.row += inHowMany;
	}

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}


// ---------------------------------------------------------------------------
//	¥ InsertCols
// ---------------------------------------------------------------------------
//	Adjust selection when inserting columns into a Table

void
CTableMultiRowSelector::InsertCols(
	UInt32			inHowMany,
	TableIndexT		inAfterCol)
{
	// No change to row based selection

	if (mAnchorCell.col > inAfterCol) {		// Adjust anchor cell
		mAnchorCell.col += inHowMany;
	}
}


// ---------------------------------------------------------------------------
//	¥ RemoveRows
// ---------------------------------------------------------------------------
//	Adjust selection when removing rows from a Table

void
CTableMultiRowSelector::RemoveRows(
	UInt32			inHowMany,
	TableIndexT		inFromRow)
{
#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif

	TableIndexT rows, cols;					// Get dimensions of Table
	mTableView->GetTableSize(rows, cols);

	// Added by CD - must alert table if removed row was selected
	size_t sel_count = std::count(mSelection.begin() + (inFromRow - 1), mSelection.begin() + (inFromRow + inHowMany - 1), true	);
	bool sel_changed = (sel_count > 0);

	mSelection.erase(mSelection.begin() + (inFromRow - 1), mSelection.begin() + (inFromRow + inHowMany - 1));

	if (mFirstSelection != 0)
	{
		if ((mFirstSelection >= inFromRow) && (mLastSelection < inFromRow + inHowMany))
		{
			mFirstSelection = mLastSelection = 0;
		}
		else
		{
			if (mFirstSelection >= inFromRow + inHowMany)
				mFirstSelection -= inHowMany;
			else if (mFirstSelection >= inFromRow)
			{
				mFirstSelection = inFromRow - 1;
				while(!mSelection[++mFirstSelection - 1] && (mFirstSelection <= rows)) {}
			}
			if (mLastSelection >= inFromRow + inHowMany)
				mLastSelection -= inHowMany;
			else if (mLastSelection >= inFromRow)
			{
				mLastSelection = inFromRow;
				while(!mSelection[--mLastSelection - 1] && (mLastSelection > 0)) {}
			}
		}
	}
 
	if (inFromRow <= mAnchorCell.row) {		// Adjust anchor cell

		if ((inFromRow + inHowMany - 1) >= mAnchorCell.row) {

				// Row containing anchor cell is being removed,
				// so make anchor cell null.

			mAnchorCell.SetCell(0, 0);

		} else {

				// Rows before the anchor cell are being removed.
				// Adjust row number of anchor cell so that the same
				// logical cell remains the anchor.

			mAnchorCell.row -= inHowMany;
		}
	}
	
	// Added by CD - must alert table if removed row was selected
	if (sel_changed)
		mTableView->SelectionChanged();

#ifdef DEBUGIT
	if ((mFirstSelection != 0) && !mSelection[mFirstSelection - 1])
		DebugStr("\pError");
#endif
}


// ---------------------------------------------------------------------------
//	¥ RemoveCols
// ---------------------------------------------------------------------------
//	Adjust selection when removing columns from a Table

void
CTableMultiRowSelector::RemoveCols(
	UInt32			inHowMany,
	TableIndexT		inFromCol)
{
	if (mFirstSelection == 0) return;	// Nothing is selected

	if (inFromCol <= mAnchorCell.col) {		// Adjust anchor cell

		if ((inFromCol + inHowMany - 1) >= mAnchorCell.col) {

				// Column containing anchor cell is being removed,
				// so make the anchor cell null

			mAnchorCell.SetCell(0, 0);

		} else {

				// Columns before the anchor cell are being removed.
				// Adjust column number of the anchor cell so that the
				// same logical cell remains the anchor.

			mAnchorCell.col -= inHowMany;
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ HiliteRow												[protected]
// ---------------------------------------------------------------------------
//	Called by SelectCell and UnselectCell to cause the selection to be
//	redrawn for an entire row.

void
CTableMultiRowSelector::HiliteRow(
	TableIndexT	inRow, Boolean inSelected)
{
	TableIndexT rows, columns;
	mTableView->GetTableSize(rows, columns);

	STableCell cell(inRow, 1);
	while (cell.col <= columns) {
		mTableView->HiliteCell(cell, inSelected);
		cell.col++;
	}
}


#pragma mark --- Saved Selections ---

// ---------------------------------------------------------------------------
//	¥ Set & GetSavedSelection
// ---------------------------------------------------------------------------
LTableSelState*
CTableMultiRowSelector::GetSavedSelection() const
{
	return NULL;
}


void
CTableMultiRowSelector::SetSavedSelection(const LTableSelState* inSS)
{
}

PP_End_Namespace_PowerPlant
