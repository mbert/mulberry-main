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


//	Manages the selection for a TableView that can have multiple
//	selected cells
//
//	Strategy: Uses a QuickDraw region to store the selected cells. Points
//	within the region represent selected cells, with the cell row and
//	column being interpreted as the "v" and "h" coords of a Point.
//
//	Since QuickDraw Points use 16-bit numbers, this selector class can
//	only be used for Tables with less than 32K rows and columns.

#include <WIN_LTableMultiSelector.h>

// ---------------------------------------------------------------------------
//	¥ LTableMultiSelector					Constructor				  [public]
// ---------------------------------------------------------------------------
//	Associates Selector with a particular TableView

LTableMultiSelector::LTableMultiSelector(
	LTableView*	inTableView)
	
	: LTableSelector(inTableView)
{
	//SignalIf_(mTableView == nil);

	mFirstSelection.SetCell(0, 0);
	mLastSelection.SetCell(0, 0);
	mAnchorCell.SetCell(0, 0);
	mBoatCell.SetCell(0, 0);
}


// ---------------------------------------------------------------------------
//	¥ ~LTableMultiSelector					Destructor				  [public]
// ---------------------------------------------------------------------------

LTableMultiSelector::~LTableMultiSelector()
{
}

#pragma mark --- Inquiries ---

std::vector<bool>::size_type LTableMultiSelector::GetSelectionIndex(const STableCell& cell) const
{
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);
	return (cell.row - 1) * cols + cell.col - 1;
}

// ---------------------------------------------------------------------------
//	¥ CellIsSelected
// ---------------------------------------------------------------------------
//	Return whether the specified cell is selected

bool
LTableMultiSelector::CellIsSelected(
	const STableCell&	inCell) const
{
	std::vector<bool>::size_type index = GetSelectionIndex(inCell);
	return (index < mSelection.size()) ? mSelection.at(index) : false;
}


// ---------------------------------------------------------------------------
//	¥ GetFirstSelectedCell
// ---------------------------------------------------------------------------
//	Return the first selected cell, defined as the min row & col (closest to
//  top-left corner)
//
//	Returns (0,0) if there is no selection

STableCell
LTableMultiSelector::GetFirstSelectedCell() const
{
	return mFirstSelection;
}


// ---------------------------------------------------------------------------
//	¥ GetFirstSelectedRow
// ---------------------------------------------------------------------------
//	Return the first selected cell's row, defined as the min row & col (closest
//  to top-left corner)
//
//	Returns 0 if there is no selection

TableIndexT
LTableMultiSelector::GetFirstSelectedRow() const
{
	return mFirstSelection.row;
}

// ---------------------------------------------------------------------------
//	¥ GetLastSelectedCell
// ---------------------------------------------------------------------------
//	Return the last selected cell, defined as the min row & col (closest to
//  top-left corner)
//
//	Returns (0,0) if there is no selection

STableCell
LTableMultiSelector::GetLastSelectedCell() const
{
	return mLastSelection;
}


// ---------------------------------------------------------------------------
//	¥ GetLastSelectedRow
// ---------------------------------------------------------------------------
//	Return the last selected cell's row, defined as the min row & col (closest
//  to top-left corner)
//
//	Returns 0 if there is no selection

TableIndexT
LTableMultiSelector::GetLastSelectedRow() const
{
	return mLastSelection.row;
}

STableCell
LTableMultiSelector::GetSelectionAnchor() const
{
	return mAnchorCell;
}

STableCell
LTableMultiSelector::GetSelectionBoat() const
{
	return mBoatCell;
}

#pragma mark --- Selecting ---

// ---------------------------------------------------------------------------
//	¥ SelectCell
// ---------------------------------------------------------------------------
//	Select the specified Cell
//
//	Does nothing if the specified Cell is invalid

void
LTableMultiSelector::SelectCell(
	const STableCell&	inCell)
{
	if ( mTableView->IsValidCell(inCell) &&
		 !CellIsSelected(inCell) ) {

		// Select the specified row
		mSelection[GetSelectionIndex(inCell)] = true;
		
		if (mFirstSelection.IsNullCell())
			mBoatCell = mAnchorCell = mFirstSelection = mLastSelection = inCell;
		else
		{
			if (inCell < mFirstSelection)
				mBoatCell = mFirstSelection = inCell;
			if (inCell > mLastSelection)
				mBoatCell = mLastSelection = inCell;
		}

		mTableView->HiliteCell(inCell, true);
		mTableView->SelectionChanged();
	}
}


// ---------------------------------------------------------------------------
//	¥ SelectOneCell
// ---------------------------------------------------------------------------
//	Select only the specified Cell
//
//	Since only one cell can be selected, this deselects the currently
//	selected cells (except for the one we want)
//
//	Does nothing if the specified Cell is invalid

void
LTableMultiSelector::SelectOneCell(
	const STableCell&	inCell)
{
	if ( mTableView->IsValidCell(inCell))
	{
		bool was_selected = CellIsSelected(inCell);
		
		// Remove from selection list prior to unhiliting current selection
		if (was_selected)
		{
			mSelection[GetSelectionIndex(inCell)] = false;
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
		
		// Add in the cell we want
		mSelection[GetSelectionIndex(inCell)] = true;
		
		if (mFirstSelection.IsNullCell())
			mBoatCell = mAnchorCell = mFirstSelection = mLastSelection = inCell;
		else
		{
			if (inCell < mFirstSelection)
				mBoatCell = mFirstSelection = inCell;
			if (inCell > mLastSelection)
				mBoatCell = mLastSelection = inCell;
		}
		
		// Hilite it if it was not previously selected
		if (!was_selected)
		{
			mTableView->HiliteCell(inCell, true);
		}

		if (sel_changed || !was_selected)
			mTableView->SelectionChanged();
	}
}

// ---------------------------------------------------------------------------
//	¥ SelectAllCells
// ---------------------------------------------------------------------------
//	Select all the cells in a Table

void
LTableMultiSelector::SelectAllCells()
{
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	if ((rows > 0) && (cols > 0)) {			// Table has some cells

		// Flip all bits in current selection so that unselected cells now look selected
		mSelection.flip();

		// Do hilite of new selected cells
		mAnchorCell.SetCell(1, 1);
		mBoatCell.SetCell(rows, cols);
		mTableView->HiliteSelection(true, true);

		// Set all items to true
		std::fill(mSelection.begin(), mSelection.end(), true);
		mFirstSelection = STableCell(1, 1);
		mLastSelection = STableCell(rows, cols);

		mTableView->SelectionChanged();
	}
}


// ---------------------------------------------------------------------------
//	¥ SelectCellBlock
// ---------------------------------------------------------------------------
//	Select cells within the specified range, where CellA and CellB are
//	the corners of a rectangular block of cells.

void
LTableMultiSelector::SelectCellBlock(
	const STableCell&	inCellA,
	const STableCell&	inCellB)
{
	TableIndexT	left, top, right, bottom;

	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	if (inCellA.col < inCellB.col) {
		left = inCellA.col;
		right = inCellB.col;
	} else {
		left = inCellB.col;
		right = inCellA.col;
	}

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
		// Loop over each matching row setting each matching column to true
		for(TableIndexT row = top; row <= bottom; row++)
		{
			STableCell first(row, left);
			STableCell second(row, right);
			
			// Set selection to range
			std::fill(mSelection.begin() + GetSelectionIndex(first), mSelection.begin() + GetSelectionIndex(second) + 1, true);
		}

		mTableView->HiliteSelection(true, true);

		mFirstSelection = STableCell(top, left);
		mLastSelection = STableCell(bottom, right);
	}
	else
	{
		// Get old selection
		std::vector<bool> old_selection = mSelection;
		unsigned long old_first_index = GetSelectionIndex(mFirstSelection);
		unsigned long old_last_index = GetSelectionIndex(mLastSelection);
		
		// Create new selection
		std::vector<bool> new_selection;
		new_selection.insert(new_selection.begin(), old_selection.size(), false);
		unsigned long new_first_index = GetSelectionIndex(STableCell(top, left));
		unsigned long new_last_index = GetSelectionIndex(STableCell(bottom, right));
		
		// Fill new selection
		for(TableIndexT row = top; row <= bottom; row++)
		{
			STableCell first(row, left);
			STableCell second(row, right);
			
			// Set selection to range
			std::fill(new_selection.begin() + GetSelectionIndex(first), new_selection.begin() + GetSelectionIndex(second) + 1, true);
		}

		// Determine difference for unhilite
		bool do_difference = false;
		vector<bool> difference;
		difference.insert(difference.begin(), old_selection.size(), false);
		mFirstSelection.SetCell(0, 0);
		mLastSelection.SetCell(0, 0);
		for(unsigned long i = old_first_index; i <= old_last_index; i++)
		{
			if (old_selection[i] && !new_selection[i])
			{
				do_difference = true;
				difference[i] = true;
				if (mFirstSelection.IsNullCell())
					mFirstSelection.SetCell(i/cols + 1, (i % cols) + 1);
				mLastSelection.SetCell(i/cols + 1, (i % cols) + 1);
			}
		}

		// Unhilite the difference
		if (do_difference)
		{
			mSelection = difference;
			mTableView->HiliteSelection(true, false);
		}
		
		// Determine difference for hilite
		do_difference = false;
		difference.clear();
		difference.insert(difference.begin(), old_selection.size(), false);
		mFirstSelection.SetCell(0, 0);
		mLastSelection.SetCell(0, 0);
		for(unsigned long i = new_first_index; i <= new_last_index; i++)
		{
			if (new_selection[i] && !old_selection[i])
			{
				do_difference = true;
				difference[i] = true;
				if (mFirstSelection.IsNullCell())
					mFirstSelection.SetCell(i/cols + 1, (i % cols) + 1);
				mLastSelection.SetCell(i/cols + 1, (i % cols) + 1);
			}
		}
		
		// Hilite the difference
		if (do_difference)
		{
			mSelection = difference;
			mTableView->HiliteSelection(true, true);
		}

		// Restore region and dump unwanted
		mSelection = new_selection;

		mFirstSelection = STableCell(top, left);
		mLastSelection = STableCell(bottom, right);
	}

	mTableView->SelectionChanged();
}

#pragma mark --- Unselecting ---

// ---------------------------------------------------------------------------
//	¥ UnselectCell
// ---------------------------------------------------------------------------
//	Unselected the specified cell

void
LTableMultiSelector::UnselectCell(
	const STableCell&	inCell)
{
	if (CellIsSelected(inCell)) {

		TableIndexT	rows, cols;
		mTableView->GetTableSize(rows, cols);

		// Turn off selection bit
		std::vector<bool>::size_type sel_index = GetSelectionIndex(inCell);
		mSelection[sel_index] = false;
		
		// Adjust selection rfange markers
		if (mFirstSelection == inCell)
		{
			if (mFirstSelection == mLastSelection)
				mAnchorCell = mBoatCell = mFirstSelection = mLastSelection = STableCell(0, 0);
			else
			{
				// Find next true item
				while((sel_index < mSelection.size()) && !mSelection.at(++sel_index)) {}
				mFirstSelection = STableCell(sel_index/cols + 1, (sel_index % cols) + 1);
			}
		}
		else if (mLastSelection == inCell)
		{
			// Find next true item
			while(!mSelection.at(--sel_index) && (sel_index != 0)) {}
			mLastSelection = STableCell(sel_index/cols + 1, (sel_index % cols) + 1);
		}

		mTableView->HiliteCell(inCell, false);
		mTableView->SelectionChanged();
		
		if (inCell == mBoatCell)
		{
			if (mAnchorCell < mBoatCell)
				mBoatCell = mLastSelection;
			else if (mAnchorCell > mBoatCell)
				mBoatCell = mFirstSelection;
			else
				mBoatCell = mAnchorCell;
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ UnselectAllCells
// ---------------------------------------------------------------------------
//	Unselect all the cells in a Table

void
LTableMultiSelector::UnselectAllCells()
{
	UnselectAllCells(true);
}


// ---------------------------------------------------------------------------
//	¥ UnselectAllCells
// ---------------------------------------------------------------------------
//	Unselect all the cells in a Table
//
//	Parameter specifies whether or not to tell the TableView that the
//	selection changed

void
LTableMultiSelector::UnselectAllCells(
	bool	inReportChange)
{
	if (!mFirstSelection.IsNullCell()) {
		mTableView->HiliteSelection(true, false);
		TableIndexT rows, columns;
		mTableView->GetTableSize(rows, columns);

		std::fill(mSelection.begin(), mSelection.end(), false);
		mFirstSelection = mLastSelection = STableCell(0, 0);

		mAnchorCell.SetCell(0, 0);
		mBoatCell.SetCell(0, 0);
		if (inReportChange) {
			mTableView->SelectionChanged();
		}
	}
}

#pragma mark --- Mouse Tracking ---

// ---------------------------------------------------------------------------
//	¥ ClickSelect
// ---------------------------------------------------------------------------
//	Adjust selection when clicking on a specified cell

void
LTableMultiSelector::ClickSelect(
	const STableCell&		inCell,
	UINT nFlags,
	bool one_only)
{
	if (nFlags & MK_CONTROL) {
									// Control Key is down
									//   Allow discontiguous selection
		if (CellIsSelected(inCell)) {
			UnselectCell(inCell);	//   Remove Cell from selection
			mAnchorCell.SetCell(0, 0);
			mBoatCell.SetCell(0, 0);
		} else {
			SelectCell(inCell);		//   Add Cell to selection
		}

	} else if (nFlags & MK_SHIFT) {
									// Shift Key is down
		if (mAnchorCell.IsNullCell()) {
			if (!CellIsSelected(inCell)) {
				UnselectAllCells(false);
				SelectCell(inCell);
			}
			mAnchorCell = inCell;
			mBoatCell = inCell;

		} else {					//   Extend selection from anchor cell
			STableCell	saveAnchor = mAnchorCell;
			//UnselectAllCells(false);
			SelectCellBlock(saveAnchor, inCell);
			mAnchorCell = saveAnchor;
			mBoatCell = inCell;
		}

	} else {						// No signficant modifier keys
		if (!CellIsSelected(inCell)) {
			UnselectAllCells(false);
			SelectCell(inCell);
		}
		else if (one_only)
		{
			SelectOneCell(inCell);
		}
		mBoatCell = mAnchorCell = inCell;
	}
}


// ---------------------------------------------------------------------------
//	¥ DragSelect
// ---------------------------------------------------------------------------
//	Adjust selection while the user is pressing the mouse in a Table
//
//	Return true if the mouse never leaves inCell

bool
LTableMultiSelector::DragSelect(
	const STableCell&		inCell,
	UINT nFlags)
{
	bool		inSameCell = true;

	ClickSelect(inCell, nFlags);	// First handle as a normal click

	if (nFlags & MK_CONTROL) {

			// I really have no good idea what to do when the
			// control key is down. Should it toggle the selection
			// of the cells dragged over? Should it set a mode
			// based on the state of the cell originally clicked,
			// and then continue selecting or deselecting the cells
			// dragged over?

			// For the moment, do the simple thing and not support
			// drag select when the command key is down.

		return true;
	}

	STableCell	currCell = inCell;

#ifdef _TODO
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

				STableCell	saveAnchor = mAnchorCell;
				//UnselectAllCells(false);
				SelectCellBlock(saveAnchor, hitCell);

				mAnchorCell = saveAnchor;
				currCell = hitCell;
			}
		}
	}
#endif

	return inSameCell;
}

#pragma mark --- Adjusting Selection ---

// ---------------------------------------------------------------------------
//	¥ InsertRows
// ---------------------------------------------------------------------------
//	Adjust selection when inserting rows into a Table

void
LTableMultiSelector::InsertRows(
	UInt32			inHowMany,
	TableIndexT		inAfterRow)
{
	TableIndexT rows, cols;					// Get dimensions of Table (these are the dimensions AFTER it has been resized)
	mTableView->GetTableSize(rows, cols);

	// Create new selection array with inserted row
	STableCell inAtRow(inAfterRow + 1, 1);
	mSelection.insert(mSelection.begin() + GetSelectionIndex(inAtRow), inHowMany * cols, false);

	if (mFirstSelection.row > inAfterRow)
		mFirstSelection.row += inHowMany;
	if (mLastSelection.row > inAfterRow)
		mLastSelection.row += inHowMany;
	
	if (mAnchorCell.row > inAfterRow) {		// Adjust anchor cell
		mAnchorCell.row += inHowMany;
	}
	
	if (mBoatCell.row > inAfterRow) {		// Adjust boat cell
		mBoatCell.row += inHowMany;
	}
}


// ---------------------------------------------------------------------------
//	¥ InsertCols
// ---------------------------------------------------------------------------
//	Adjust selection when inserting columns into a Table

void
LTableMultiSelector::InsertCols(
	UInt32			inHowMany,
	TableIndexT		inAfterCol)
{
	TableIndexT rows, cols;					// Get dimensions of Table (these are the dimensions AFTER it has been resized)
	mTableView->GetTableSize(rows, cols);
	size_t new_size = rows * cols;

	// Optimise for case of zero selection
	if (mFirstSelection.IsNullCell())
	{
		// Just resize the entire selection state vector to the size we want
		size_t current_size = mSelection.size();
		mSelection.insert(mSelection.end(), new_size - current_size, false);
	}
	else
	{
		// Create new selection array with inserted column
		TableIndexT col_offset = inAfterCol;
		mSelection.reserve(new_size);
		for(TableIndexT row = 0; row < rows; row++, col_offset += cols)
		{
			mSelection.insert(mSelection.begin() + col_offset, inHowMany, false);
		}

		if (mFirstSelection.col > inAfterCol)
			mFirstSelection.col += inHowMany;
		if (mLastSelection.col > inAfterCol)
			mLastSelection.col += inHowMany;
		
		if (mAnchorCell.col > inAfterCol) {		// Adjust anchor cell
			mAnchorCell.col += inHowMany;
		}
		
		if (mBoatCell.col > inAfterCol) {		// Adjust anchor cell
			mBoatCell.col += inHowMany;
		}
	}
}


// ---------------------------------------------------------------------------
//	¥ RemoveRows
// ---------------------------------------------------------------------------
//	Adjust selection when removing rows from a Table

void
LTableMultiSelector::RemoveRows(
	UInt32			inHowMany,
	TableIndexT		inFromRow)
{
	TableIndexT rows, cols;					// Get dimensions of Table (these are the dimensions AFTER it has been resized)
	mTableView->GetTableSize(rows, cols);

	// Added by CD - must alert table if removed row was selected
	STableCell start(inFromRow, 1);
	STableCell stop(inFromRow + inHowMany - 1, cols);
	bool val = true;
	size_t sel_count = std::count(mSelection.begin() + GetSelectionIndex(start), mSelection.begin() + GetSelectionIndex(stop) + 1, val);
	bool sel_changed = (sel_count > 0);

	mSelection.erase(mSelection.begin() + GetSelectionIndex(start), mSelection.begin() + GetSelectionIndex(stop) + 1);

	if (!mFirstSelection.IsNullCell())
	{
		if ((mFirstSelection.row >= inFromRow) && (mLastSelection.row < inFromRow + inHowMany))
		{
			mFirstSelection = mLastSelection = STableCell(0, 0);
		}
		else
		{
			if (mFirstSelection.row >= inFromRow + inHowMany)
				mFirstSelection.row -= inHowMany;
			else if (mFirstSelection.row >= inFromRow)
			{
				// Find next true item
				bool val = true;
				std::vector<bool>::const_iterator found = std::find(mSelection.begin() + GetSelectionIndex(STableCell(inFromRow, 1)), mSelection.end(), val);
				if (found != mSelection.end())
				{
					std::vector<bool>::size_type index = found - mSelection.begin();
					mFirstSelection = STableCell(index/cols + 1, (index % cols) + 1);
				}
			}
			if (mLastSelection.row >= inFromRow + inHowMany)
				mLastSelection.row -= inHowMany;
			else if (mLastSelection >= inFromRow)
			{
				// Find next true item
				bool val = true;
				std::vector<bool>::reverse_iterator found = std::find(mSelection.rbegin(), mSelection.rend(), val);
				if (found != mSelection.rend())
				{
					std::vector<bool>::size_type index = found.base() - mSelection.begin();
					mLastSelection = STableCell(index/cols + 1, (index % cols) + 1);
				}
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
	
	if (inFromRow <= mBoatCell.row) {		// Adjust boat cell

		if ((inFromRow + inHowMany - 1) >= mBoatCell.row) {

				// Row containing boat cell is being removed,
				// so make anchor cell null.

			mBoatCell.SetCell(0, 0);

		} else {

				// Rows before the boat cell are being removed.
				// Adjust row number of boat cell so that the same
				// logical cell remains the boat.

			mBoatCell.row -= inHowMany;
		}
	}
	
	// Added by CD - must alert table if removed row was selected
	if (sel_changed)
		mTableView->SelectionChanged();
}


// ---------------------------------------------------------------------------
//	¥ RemoveCols
// ---------------------------------------------------------------------------
//	Adjust selection when removing columns from a Table

void
LTableMultiSelector::RemoveCols(
	UInt32			inHowMany,
	TableIndexT		inFromCol)
{
	TableIndexT rows, cols;					// Get dimensions of Table (these are the dimensions AFTER it has been resized)
	mTableView->GetTableSize(rows, cols);

	bool sel_changed = false;
	for(TableIndexT row = 0; row < rows; row++)
	{
		// Added by CD - must alert table if removed row was selected
		STableCell start(row, inFromCol);
		STableCell stop(row, inFromCol + inHowMany - 1);
		bool val = true;
		size_t sel_count = std::count(mSelection.begin() + GetSelectionIndex(start), mSelection.begin() + GetSelectionIndex(stop) + 1, val);
		sel_changed = sel_changed || (sel_count > 0);

		mSelection.erase(mSelection.begin() + GetSelectionIndex(start), mSelection.begin() + GetSelectionIndex(stop) + 1);
	}

	if (!mFirstSelection.IsNullCell())
	{
		if ((mFirstSelection.col >= inFromCol) && (mLastSelection.col < inFromCol + inHowMany))
		{
			mFirstSelection = mLastSelection = STableCell(0, 0);
		}
		else
		{
			if (mFirstSelection.col >= inFromCol + inHowMany)
				mFirstSelection.col -= inHowMany;
			else if (mFirstSelection.col >= inFromCol)
			{
				// Find next true item
				bool val = true;
				std::vector<bool>::const_iterator found = std::find(mSelection.begin(), mSelection.end(), val);
				if (found != mSelection.end())
				{
					std::vector<bool>::size_type index = found - mSelection.begin();
					mFirstSelection = STableCell(index/cols + 1, (index % cols) + 1);
				}
			}
			if (mLastSelection.col >= inFromCol + inHowMany)
				mLastSelection.col -= inHowMany;
			else if (mLastSelection.col >= inFromCol)
			{
				// Find next true item
				bool val = true;
				std::vector<bool>::reverse_iterator found = std::find(mSelection.rbegin(), mSelection.rend(), val);
				if (found != mSelection.rend())
				{
					std::vector<bool>::size_type index = found.base() - mSelection.begin();
					mLastSelection = STableCell(index/cols, (index % cols) + 1);
				}
			}
		}
	}
 
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
	
	if (inFromCol <= mBoatCell.col) {		// Adjust boat cell

		if ((inFromCol + inHowMany - 1) >= mBoatCell.col) {

				// Column containing boat cell is being removed,
				// so make the boat cell null

			mBoatCell.SetCell(0, 0);

		} else {

				// Columns before the boat cell are being removed.
				// Adjust column number of the boat cell so that the
				// same logical cell remains the boat.

			mBoatCell.col -= inHowMany;
		}
	}
	
	// Added by CD - must alert table if removed row was selected
	if (sel_changed)
		mTableView->SelectionChanged();
}
