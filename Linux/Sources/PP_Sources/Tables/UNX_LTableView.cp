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


//	A two-dimensional Table of rectangular cells.
//
//	Rows and Columns use one-based indexes.

#include <UNX_LTableView.h>
#include <UNX_UTableHelpers.h>

#include <JXColormap.h>
#include <JXWindowPainter.h>

#include <jXKeysym.h>

#pragma mark --- Construction/Destruction ---

// ---------------------------------------------------------------------------
//	 LTableView							Default Constructor		  [public]
// ---------------------------------------------------------------------------

LTableView::LTableView(JXScrollbarSet* scrollbarSet, 
				 JXContainer* enclosure,
				 const HSizingOption hSizing, 
				 const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h):
	LScrollable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	SetBackColor((GetColormap())->GetWhiteColor());

	InitTable();
}


// ---------------------------------------------------------------------------
//	 InitTable								Initializer				 [private]
// ---------------------------------------------------------------------------

void
LTableView::InitTable()
{
	mRows					= 0;
	mCols					= 0;
	mTableGeometry			= NULL;
	mTableSelector			= NULL;
	mTableStorage			= NULL;
	mUseDragSelect			= false;
	mDeferAdjustment		= false;
}


// ---------------------------------------------------------------------------
//	 ~LTableView							Destructor				  [public]
// ---------------------------------------------------------------------------

LTableView::~LTableView()
{
	delete mTableGeometry;
	delete mTableSelector;
	delete mTableStorage;
}

#pragma mark --- Setters/Getters ---

// ---------------------------------------------------------------------------
//	 GetTableSize
// ---------------------------------------------------------------------------
//	Pass back the number of rows and columns in a TableView

void
LTableView::GetTableSize(
	TableIndexT&	outRows,
	TableIndexT&	outCols) const
{
	outRows = mRows;
	outCols = mCols;
}


// ---------------------------------------------------------------------------
//	 SetDeferAdjustment
// ---------------------------------------------------------------------------
//	Specify whether to defer adjustment of Table to account for a change
//	in the size of the Table (number or size of rows or columns).
//
//	Calling AdjustImageSize() can take a lot of time, especially for
//	large Tables and those with variable row or column sizes. If you are
//	going to make a series of changes to a Table, there's no need to adjust
//	the Image size until after all those changes are made.
//
//	Therefore, call SetDeferAdjustment(true) before making your changes,
//	then call SetDeferAdjustment(false) afterwards. Or use the
//	StDeferTableAdjustment stack-based class.

void
LTableView::SetDeferAdjustment(
	bool		inDefer)
{
	if (inDefer != mDeferAdjustment) {
		mDeferAdjustment = inDefer;
		if (!inDefer) {
			AdjustImageSize(false);
		}
	}
}


#pragma mark --- Cell Verification ---

// ---------------------------------------------------------------------------
//	 IsValidRow
// ---------------------------------------------------------------------------
//	Return whether a TableView includes a specified row

bool
LTableView::IsValidRow(
	TableIndexT	inRow) const
{
	return ( (inRow > 0) && (inRow <= mRows) );
}


// ---------------------------------------------------------------------------
//	 IsValidCol
// ---------------------------------------------------------------------------
//	Return whether a TableView includes a specified column

bool
LTableView::IsValidCol(
	TableIndexT	inCol) const
{
	return ( (inCol > 0) && (inCol <= mCols) );
}


// ---------------------------------------------------------------------------
//	 IsValidCell
// ---------------------------------------------------------------------------
//	Return whether a TableView includes a specified Cell

bool
LTableView::IsValidCell(
	const STableCell&	inCell) const
{
	return ( IsValidRow(inCell.row) && IsValidCol(inCell.col) );
}

#pragma mark --- Row & Col Management ---

// ---------------------------------------------------------------------------
//	 InsertRows
// ---------------------------------------------------------------------------
//	Add rows to a TableView
//
//	Use inAfterRow of 0 to insert rows at the beginning
//	All cells in the newly inserted rows have the same data

void
LTableView::InsertRows(
	UInt32		inHowMany,
	TableIndexT	inAfterRow,
	const void*	inDataPtr,
	UInt32		inDataSize,
	bool		inRefresh)
{
	//SignalIf_(mTableGeometry == NULL);

	if (inAfterRow > mRows) {				// Enforce upper bound
		inAfterRow = mRows;
	}

	mRows += inHowMany;

	if (mTableStorage != NULL) {
		mTableStorage->InsertRows(inHowMany, inAfterRow,
								inDataPtr, inDataSize);
	}

	mTableGeometry->InsertRows(inHowMany, inAfterRow);

	if (mTableSelector != NULL) {
		mTableSelector->InsertRows(inHowMany, inAfterRow);
	}

	AdjustImageSize(false);

	if (inRefresh) {						// Refresh inserted rows and
											//   all rows below
		STableCell	topLeftCell(inAfterRow + 1, 1);
		STableCell	botRightCell(mRows, mCols);

		RefreshCellRange(topLeftCell, botRightCell);
	}
}


// ---------------------------------------------------------------------------
//	 InsertCols
// ---------------------------------------------------------------------------
//	Add columns to a TableView
//
//	Use inAfterCol of 0 to insert columns at the beginning
//	All cells in the newly inserted rows have the same data

void
LTableView::InsertCols(
	UInt32		inHowMany,
	TableIndexT	inAfterCol,
	const void*	inDataPtr,
	UInt32		inDataSize,
	bool		inRefresh)
{
	//SignalIf_(mTableGeometry == NULL);

	if (inAfterCol > mCols) {				// Enforce upper bound
		inAfterCol= mCols;
	}

	mCols += inHowMany;

	if (mTableStorage != NULL) {
		mTableStorage->InsertCols(inHowMany, inAfterCol,
								inDataPtr, inDataSize);
	}

	mTableGeometry->InsertCols(inHowMany, inAfterCol);

	if (mTableSelector != NULL) {
		mTableSelector->InsertCols(inHowMany, inAfterCol);
	}

	AdjustImageSize(false);

	if (inRefresh) {						// Refresh inserted columns
											//   and all cols to the right
		STableCell	topLeftCell(1, inAfterCol + 1);
		STableCell	botRightCell(mRows, mCols);

		RefreshCellRange(topLeftCell, botRightCell);
	}
}


// ---------------------------------------------------------------------------
//	 RemoveRows
// ---------------------------------------------------------------------------
//	Delete rows from a TableView

void
LTableView::RemoveRows(
	UInt32		inHowMany,
	TableIndexT	inFromRow,
	bool		inRefresh)
{
										// Check input parameters
	if (inFromRow > mRows) {
		//SignalStringLiteral_("inFromRow > number of rows");
		return;
	}

	if (inFromRow < 1) {				// Assume user means Row 1 if
		inFromRow = 1;					//   inFromRow is 0
	}

	if ((inFromRow + inHowMany - 1) > mRows) {
										// inHowMany is too big. Remove
										//   from inFromRow to last row
		inHowMany = mRows - inFromRow + 1;
	}

	if (inRefresh) {					// Refresh deleted rows and all
										//   rows below
		STableCell	topLeftCell(inFromRow, 1);
		STableCell	botRightCell(mRows, mCols);

		RefreshCellRange(topLeftCell, botRightCell);
	}

	mRows -= inHowMany;

	mTableGeometry->RemoveRows(inHowMany, inFromRow);

	if (mTableStorage != NULL) {
		mTableStorage->RemoveRows(inHowMany, inFromRow);
	}

	if (mTableSelector != NULL) {
		mTableSelector->RemoveRows(inHowMany, inFromRow);
	}

	AdjustImageSize(inRefresh);
}


// ---------------------------------------------------------------------------
//	 RemoveAllRows
// ---------------------------------------------------------------------------

void
LTableView::RemoveAllRows(
	bool		inRefresh)
{
	if (mRows > 0) {
		RemoveRows(mRows, 1, inRefresh);
	}
}


// ---------------------------------------------------------------------------
//	 RemoveCols
// ---------------------------------------------------------------------------
//	Delete columns from a TableView

void
LTableView::RemoveCols(
	UInt32		inHowMany,
	TableIndexT	inFromCol,
	bool		inRefresh)
{
										// Check input parameters
	if (inFromCol > mCols) {
		//SignalStringLiteral_("inFromCol > number of columns");
		return;
	}

	if (inFromCol < 1) {				// Assume user means Column 1 if
		inFromCol = 1;					//   inFromCol is 0
	}

	if ((inFromCol + inHowMany - 1) > mCols) {
										// inHowMany is too big. Remove
										//   from inFromRow to last row
		inHowMany = mCols - inFromCol + 1;
	}

	if (inRefresh) {					// Refresh deleted columns and all
										//   columns to the right
		STableCell	topLeftCell(1, inFromCol);
		STableCell	botRightCell(mRows, mCols);

		RefreshCellRange(topLeftCell, botRightCell);
	}

	mCols -= inHowMany;

	mTableGeometry->RemoveCols(inHowMany, inFromCol);

	if (mTableStorage != NULL) {
		mTableStorage->RemoveCols(inHowMany, inFromCol);
	}

	if (mTableSelector != NULL) {
		mTableSelector->RemoveCols(inHowMany, inFromCol);
	}

	AdjustImageSize(inRefresh);
}


// ---------------------------------------------------------------------------
//	 RemoveAllCols
// ---------------------------------------------------------------------------

void
LTableView::RemoveAllCols(
	bool		inRefresh)
{
	if (mCols > 0) {
		RemoveCols(mCols, 1, inRefresh);
	}
}


// ---------------------------------------------------------------------------
//	 AdjustImageSize
// ---------------------------------------------------------------------------
//	Adjust the Image size of the Table to reflect the number and
//	size of the rows and columns

void
LTableView::AdjustImageSize(
	bool	inRefresh)
{
	if (!mDeferAdjustment) {
		UInt32	width, height;
		mTableGeometry->GetTableDimensions(width, height);
		ResizeImageTo((SInt32) width, (SInt32) height, inRefresh);
	}
}

#pragma mark --- Accessing Cells ---

// ---------------------------------------------------------------------------
//	 CellToIndex
// ---------------------------------------------------------------------------
//	Pass back the index number for a specified Cell
//
//	Cell does not have to be in Table, but Index is zero if Table
//	has no columns, or if inCell.row or inCell.col is zero.
//
//	Cells are ordered by column (across), and then by row (down)

void
LTableView::CellToIndex(
	const STableCell&	inCell,
	TableIndexT&		outIndex) const
{
	outIndex = 0;
	if ( (mCols > 0) && (inCell.row > 0) && (inCell.col > 0) ) {
		outIndex = (inCell.row - 1) * mCols + inCell.col;
	}
}


// ---------------------------------------------------------------------------
//	 IndexToCell
// ---------------------------------------------------------------------------
//	Pass back the cell for a specified index number
//
//	Index number does not have to refer to an actual Cell, but Cell is
//	(0,0) if Table has no columns or inIndex is zero

void
LTableView::IndexToCell(
	TableIndexT		inIndex,
	STableCell&		outCell) const
{
	outCell.SetCell(0, 0);

	if ((mCols > 0) && (inIndex > 0)) {
		outCell.row = (inIndex - 1) / mCols + 1;
		outCell.col = (inIndex - 1) % mCols + 1;
	}
}


// ---------------------------------------------------------------------------
//	 GetNextCell
// ---------------------------------------------------------------------------
//	Pass back the Cell after the specified Cell
//
//	Cells are ordered by column (across), and then by row (down)
//
//	Return false if there is no cell after the specified one, and
//		pass back Cell (0,0)
//	Otherwise, return true and pass back the next Cell's indexes
//
//	Row zero is before the first row. The next cell after row zero
//		and any column is Cell (1,1)
//
//	Column zero is before column one. The next cell after row "r" and
//		column zero is Cell (r,1).

bool
LTableView::GetNextCell(
	STableCell&	ioCell) const
{
	bool	nextCellExists = true;

	TableIndexT	row = ioCell.row;		// Next Cell is in same row and
	TableIndexT col = ioCell.col + 1;	//   the next column

	if (col > mCols) {					// Wrap around to first column
		row += 1;						//   of the next row
		col = 1;
	}

	if (row == 0) {						// Cell (1,1) is after all cells
		row = 1;						//   in row zero
		col = 1;
	}

	if (row > mRows) {					// Test if beyond last row
		nextCellExists = false;
		row = 0;
		col = 0;
	}

	ioCell.SetCell(row, col);
	return nextCellExists;
}


// ---------------------------------------------------------------------------
//	 GetFirstSelectedCell
// ---------------------------------------------------------------------------
//	Return the first selected cell, using the LTableSelector helper object

STableCell
LTableView::GetFirstSelectedCell() const
{
	STableCell	selectedCell(0, 0);

	if (mTableSelector != NULL) {
		selectedCell = mTableSelector->GetFirstSelectedCell();
	}

	return selectedCell;
}


// ---------------------------------------------------------------------------
//	 GetNextSelectedCell
// ---------------------------------------------------------------------------
//	Pass back the selected Cell after the specified Cell
//
//	Return false if there is no selected Cell after the specified one
//
//	This function uses the same ordering rules as GetNextCell().
//	Pass in Cell (0,0) to find the first selected Cell.

bool
LTableView::GetNextSelectedCell(
	STableCell&	ioCell) const
{
	bool	nextSelectionExists = false;

	while (GetNextCell(ioCell)) {
		if (CellIsSelected(ioCell)) {
			nextSelectionExists = true;
			break;
		}
	}

	return nextSelectionExists;
}


// ---------------------------------------------------------------------------
//	 GetPreviousCell
// ---------------------------------------------------------------------------
//	Pass back the Cell before the specified Cell
//
//	Cells are ordered by column (across), and then by row (down)
//
//	Return false if there is no cell before the specified one, and
//		pass back Cell (0,0)
//	Otherwise, return true and pass back the previous Cell's indexes
//
//	Cell (mRows,mCols) is the one before Cell (0,c), where c is any column.
//	Thus, you can pass in Cell (0,0) to get the last cell.
//
//	Cell (mRows,mCols) is also the cell before any Cell that is beyond
//	the limits of the table.
//
//	Column zero is before column one. The cell before row "r" and
//		column zero is Cell (r-1,mCols).

bool
LTableView::GetPreviousCell(
	STableCell&	ioCell) const
{
	bool	prevCellExists = true;

	TableIndexT	row = ioCell.row;
	TableIndexT col = ioCell.col;

	if ((row == 0) || (row > mRows)) {	// Last Cell is before all cells
		row = mRows;					//   in row zero and rows beyond
		col = mCols;					//   the last row

	} else if (col <= 1) {				// Wrap around to last column in
		row -= 1;						//   previous row
		col = mCols;

	} else {							// Previous Cell is on the same
		col -= 1;						//   row. Usually one column back,
		if (col > mCols) {				//   but the last column if input
			col = mCols;				//   column is beyond the last
		}								//   column
	}

	if (row == 0) {						// There is no previous Cell.
		prevCellExists = false;			//   This happens only if the input
		row = 0;						//   Cell is (1,1), (1,0), or the
		col = 0;						//   Table has no rows.
	}

	ioCell.SetCell(row, col);
	return prevCellExists;
}


// ---------------------------------------------------------------------------
//		# GetPreviousSelectedCell
// ---------------------------------------------------------------------------
//	Pass back the selected Cell before the specified Cell
//
//	Return false if there is no selected Cell before the specified one
//
//	This function uses the same ordering rules as GetPreviosCell().
//	Pass in Cell (0,0) to find the last selected Cell.

bool
LTableView::GetPreviousSelectedCell(
	STableCell&	ioCell) const
{
	bool	prevSelectionExists = false;

	while (GetPreviousCell(ioCell)) {
		if (CellIsSelected(ioCell)) {
			prevSelectionExists = true;
			break;
		}
	}

	return prevSelectionExists;
}


// ---------------------------------------------------------------------------
//	 ScrollCellIntoFrame
// ---------------------------------------------------------------------------
//	Scroll the TableView as little as possible to move the specified Cell
//	so that it's entirely within the Frame of the TableView
//
//	If Cell is wider and/or taller than Frame, align Cell to left/top
//	of Frame.

void
LTableView::ScrollCellIntoFrame(
	const STableCell&	inCell)
{
	SInt32	cellLeft, cellTop, cellRight, cellBottom;
	mTableGeometry->GetImageCellBounds(inCell, cellLeft, cellTop,
							cellRight, cellBottom);

	if (ImagePointIsInFrame(cellLeft, cellTop) &&
		ImagePointIsInFrame(cellRight, cellBottom)) {

		return;						// Entire Cell is already within Frame
	}

	JRect	image_frame = GetAperture();					// Get Frame in Image coords
	JPoint	frameTopLeft = image_frame.topLeft();
	JPoint	frameBotRight = image_frame.bottomRight();

	SInt32	horizScroll = 0;		// Find horizontal scroll amount
	if (cellRight > frameBotRight.x) {
										// Cell is to the right of frame
		horizScroll = cellRight - frameBotRight.x;
	}

	if ((cellLeft - horizScroll) < frameTopLeft.x) {
										// Cell is to the left of frame
										//   or Cell is wider than frame
		horizScroll = cellLeft - frameTopLeft.x;
	}

	SInt32	vertScroll = 0;			// Find vertical scroll amount
	if (cellBottom > frameBotRight.y) {
										// Cell is below bottom of frame
		vertScroll = cellBottom - frameBotRight.y;
	}

	if ((cellTop - vertScroll) < frameTopLeft.y) {
										// Cell is above top of frame
										//   or Cell is taller than frame
		vertScroll = cellTop - frameTopLeft.y;
	}

	ScrollPinnedImageBy(horizScroll, vertScroll, true);
}

STableCell LTableView::GetFirstFullyVisibleCell() const
{
	JRect	image_frame = GetAperture();					// Get image frame of aperture
	JRect   local_frame;
	ImageToLocalRect(image_frame, local_frame);
	STableCell	cell, topLeftCell, botRightCell;
	FetchIntersectingCells(local_frame, topLeftCell, botRightCell);

	// Adjust top one if partially visible
	JRect cellRect;
	GetLocalCellRect(topLeftCell, cellRect);
	if ((cellRect.top < local_frame.top) && (topLeftCell.row < mRows))
		topLeftCell.row++;
	return topLeftCell;
}

STableCell LTableView::GetLastFullyVisibleCell() const
{
	JRect	image_frame = GetAperture();					// Get image frame of aperture
	JRect   local_frame;
	ImageToLocalRect(image_frame, local_frame);
	STableCell	cell, topLeftCell, botRightCell;
	FetchIntersectingCells(local_frame, topLeftCell, botRightCell);

	// Adjust bottom one if partially visible
	JRect cellRect;
	GetLocalCellRect(botRightCell, cellRect);
	if ((cellRect.bottom > local_frame.bottom) && (botRightCell.row > 1))
		botRightCell.row--;
	return botRightCell;
}

STableCell LTableView::GetSelectionAnchor() const
{
	if (mTableSelector != NULL)
		return mTableSelector->GetSelectionAnchor();
	else
		return STableCell(0, 0);
}

STableCell LTableView::GetSelectionBoat() const
{
	if (mTableSelector != NULL)
		return mTableSelector->GetSelectionBoat();
	else
		return STableCell(0, 0);
}

#pragma mark --- Geometry ---

// ---------------------------------------------------------------------------
//	 SetTableGeometry
// ---------------------------------------------------------------------------
//	Specify the Geometry for a TableView.
//
//	The Geometry determines the dimensions of the Cells in the Table

void
LTableView::SetTableGeometry(
	LTableGeometry*	inTableGeometry)
{
	mTableGeometry = inTableGeometry;
}


// ---------------------------------------------------------------------------
//	 GetImageCellBounds
// ---------------------------------------------------------------------------
//	Pass back the location in Image coords of the specified Cell

void
LTableView::GetImageCellBounds(
	const STableCell&	inCell,
	JRect&				outRect) const
{
	mTableGeometry->GetImageCellBounds(inCell, outRect);
}


// ---------------------------------------------------------------------------
//	 GetImageCellBounds
// ---------------------------------------------------------------------------
//	Pass back the location in Image coords of the specified Cell

void
LTableView::GetImageCellBounds(
	const STableCell&	inCell,
	SInt32&				outLeft,
	SInt32&				outTop,
	SInt32&				outRight,
	SInt32&				outBottom) const
{
	mTableGeometry->GetImageCellBounds(inCell, outLeft, outTop,
							outRight, outBottom);
}


// ---------------------------------------------------------------------------
//	 GetLocalCellRect
// ---------------------------------------------------------------------------
//	Pass back the bounding rectangle of the specified Cell and return
//	whether it intersects the Frame of the TableView
//
//	The bounding rectangle is in Local coordinates so it will always be
//	within QuickDraw space when its within the Frame. If the bounding
//	rectangle is outside the Frame, return false and set the rectangle
//	to (0,0,0,0)

bool
LTableView::GetLocalCellRect(
	const STableCell&	inCell,
	JRect&				outCellRect) const
{
	SInt32	cellLeft, cellTop, cellRight, cellBottom;
	mTableGeometry->GetImageCellBounds(inCell, cellLeft, cellTop,
										cellRight, cellBottom);

	bool	insideFrame =
		ImageRectIntersectsFrame(cellLeft, cellTop, cellRight, cellBottom);

	if (insideFrame) {
		JRect localRect(cellTop, cellLeft, cellBottom, cellRight);
		ImageToLocalRect(localRect, outCellRect);

	} else {
		outCellRect.Set(0, 0, 0, 0);
	}

	return insideFrame;
}


// ---------------------------------------------------------------------------
//	 GetCellHitBy
// ---------------------------------------------------------------------------
//	Pass back the cell which contains the specified point.
//
//	If no cell contains the point, return false and
//
//		outCell.row = 0 if point is above the Table
//		outCell.row = mRows + 1 if point is below the Table
//
//		outCell.col = 0 if point is to the left of the Table
//		outCell.col = mCols + 1 if point is to the right of the Table
//
//	For example, if the horizontal coord. is within Column 2,
//	but the vertical coord. is above the Table,
//		outCell.row = 0
//		outCell.col = 2

bool
LTableView::GetCellHitBy(
	const JPoint&	inImagePt,
	STableCell&		outCell) const
{
	bool	containsPoint = true;

	if (inImagePt.y < 0) {
		outCell.row = 0;
		containsPoint = false;

	} else if (inImagePt.y >= GetBounds().height()) {
		outCell.row = mRows + 1;
		containsPoint = false;

	} else {
		outCell.row = mTableGeometry->GetRowHitBy(inImagePt);
	}

	if (inImagePt.x < 0) {
		outCell.col = 0;
		containsPoint = false;

	} else if (inImagePt.x >= GetBounds().width()) {
		outCell.col = mCols + 1;
		containsPoint = false;

	} else {
		outCell.col = mTableGeometry->GetColHitBy(inImagePt);
	}

	return containsPoint;
}


// ---------------------------------------------------------------------------
//	 GetRowHeight
// ---------------------------------------------------------------------------
//	Return the height of the specified row

UInt16
LTableView::GetRowHeight(
	TableIndexT	inRow) const
{
	return mTableGeometry->GetRowHeight(inRow);
}


// ---------------------------------------------------------------------------
//	 SetRowHeight
// ---------------------------------------------------------------------------
//	Set the height of the specified rows

void
LTableView::SetRowHeight(
	UInt16		inHeight,
	TableIndexT	inFromRow,
	TableIndexT	inToRow)
{
	mTableGeometry->SetRowHeight(inHeight, inFromRow, inToRow);
	AdjustImageSize(true);
}


// ---------------------------------------------------------------------------
//	 GetColWidth
// ---------------------------------------------------------------------------
//	Return the width of the specified column

UInt16
LTableView::GetColWidth(
	TableIndexT	inCol) const
{
	return mTableGeometry->GetColWidth(inCol);
}


// ---------------------------------------------------------------------------
//	 SetColWidth
// ---------------------------------------------------------------------------
//	Set the width of the specified columns

void
LTableView::SetColWidth(
	UInt16		inWidth,
	TableIndexT	inFromCol,
	TableIndexT	inToCol)
{
	mTableGeometry->SetColWidth(inWidth, inFromCol, inToCol);
	AdjustImageSize(true);
}


// ---------------------------------------------------------------------------
//	 FetchIntersectingCells
// ---------------------------------------------------------------------------
//	Pass back the rectangular range of cells, specified by the top left
//	and bottom right cells, that intersects a given rectangle.

void
LTableView::FetchIntersectingCells(
	const JRect&	inLocalRect,			// In Local Coordinates
	STableCell&		outTopLeft,
	STableCell&		outBotRight) const
{
								//  Top Left Cell

	JPoint	topLeftPt;			// Convert top left corner to Image coords
								//   and find cell that contains it
	LocalToImagePoint(inLocalRect.topLeft(), topLeftPt);
	GetCellHitBy(topLeftPt, outTopLeft);

	if (outTopLeft.row < 1) {	// Lower bound is cell (1,1)
		outTopLeft.row = 1;
	}
	if (outTopLeft.col < 1) {
		outTopLeft.col = 1;
	}

								//  Bottom Right Cell

	JPoint	botRightPt;			// Convert bottom right corner to Image coords
	LocalToImagePoint(inLocalRect.bottomRight(), botRightPt);

	botRightPt.x -= 1;			// Pixel hangs below and to the right of
	botRightPt.y -= 1;			//   coordinate

	GetCellHitBy(botRightPt, outBotRight);

								// Upper bound is cell (mRows,mCols)
	if (outBotRight.row > mRows) {
		outBotRight.row = mRows;
	}
	if (outBotRight.col > mCols) {
		outBotRight.col = mCols;
	}
}

UInt16
LTableView::GetVisibleRows() const
{
	return (mRows > 0) ? (GetApertureHeight() / GetRowHeight(1)) : 1;
}

#pragma mark --- Selection ---

// ---------------------------------------------------------------------------
//	 SetTableSelector
// ---------------------------------------------------------------------------
//	Specify the Selector for a TableView.
//
//	The Selector stores and controls which Cells in a Table are selected

void
LTableView::SetTableSelector(
	LTableSelector*	inTableSelector)
{
	mTableSelector = inTableSelector;
}


// ---------------------------------------------------------------------------
//	 CellIsSelected
// ---------------------------------------------------------------------------
//	Return whether the specified Cell is part of the current selection

bool
LTableView::CellIsSelected(
	const STableCell&	inCell) const
{
	bool	selected = false;

	if (mTableSelector != NULL) {
		selected = mTableSelector->CellIsSelected(inCell);
	}

	return selected;
}


// ---------------------------------------------------------------------------
//	 SelectCell
// ---------------------------------------------------------------------------
//	Add the specified cell to the current selection

void
LTableView::SelectCell(
	const STableCell&	inCell)
{
	if (mTableSelector != NULL) {
		mTableSelector->SelectCell(inCell);
	}
}


// ---------------------------------------------------------------------------
//	 SelectAllCells
// ---------------------------------------------------------------------------
//	Select all Cells in a Table

void
LTableView::SelectAllCells()
{
	if (mTableSelector != NULL) {
		mTableSelector->SelectAllCells();
	}
}


// ---------------------------------------------------------------------------
//	 UnselectCell
// ---------------------------------------------------------------------------
//	Remove the specified cell from the current selection

void
LTableView::UnselectCell(
	const STableCell&	inCell)
{
	if (mTableSelector != NULL) {
		mTableSelector->UnselectCell(inCell);
	}
}


// ---------------------------------------------------------------------------
//	 UnselectAllCells
// ---------------------------------------------------------------------------
//	Unselect all currently selected cells so there is no selection

void
LTableView::UnselectAllCells()
{
	if (mTableSelector != NULL) {
		mTableSelector->UnselectAllCells();
	}
}


// ---------------------------------------------------------------------------
//	 ClickSelect
// ---------------------------------------------------------------------------
//	Adjust selection in response to a click in the specified cell
//
//	Return whether or not to process the click as a normal click

bool
LTableView::ClickSelect(
	const STableCell&		inCell,
	const JXKeyModifiers& modifiers)
{
	if (mTableSelector != NULL) {
		if (mUseDragSelect) {
			return mTableSelector->DragSelect(inCell, modifiers);

		} else {
			mTableSelector->ClickSelect(inCell, modifiers);
		}
	}

	return true;
}


// ---------------------------------------------------------------------------
//	 SelectionChanged
// ---------------------------------------------------------------------------
//	Notification that the cells which are selected has changed

void
LTableView::SelectionChanged()
{
}

#pragma mark --- Storage ---

// ---------------------------------------------------------------------------
//	 SetTableStorage
// ---------------------------------------------------------------------------
//	Specify the Storage for a TableView
//
//	The Storage holds the data for each Cell in a Table

void
LTableView::SetTableStorage(
	LTableStorage*	inTableStorage)
{
	mTableStorage = inTableStorage;

	TableIndexT	storageRows, storageCols;
	inTableStorage->GetStorageSize(storageRows, storageCols);

	mRows = storageRows;
	if (storageRows > 0) {
		mTableGeometry->InsertRows(storageRows, 0);
	}

	mCols = storageCols;
	if (storageCols > 0) {
		mTableGeometry->InsertCols(storageCols, 0);
	}

	if ((storageRows > 0) || (storageCols > 0)) {
		AdjustImageSize(false);
	}
}


// ---------------------------------------------------------------------------
//	 SetCellData
// ---------------------------------------------------------------------------
//	Specify the data associated with a particular Cell

void
LTableView::SetCellData(
	const STableCell&	inCell,
	const void*			inDataPtr,
	UInt32				inDataSize)
{
	if (mTableStorage != NULL) {
		mTableStorage->SetCellData(inCell, inDataPtr, inDataSize);

	} else {
		//SignalStringLiteral_("No TableStorage for this Table");
	}
}


// ---------------------------------------------------------------------------
//	 GetCellData
// ---------------------------------------------------------------------------
//	Pass back the data for a particular Cell
//
//	outDataPtr points to storage which must be allocated by the caller.
//	It may be NULL, in which case only the size of the data is passed back.
//
//	ioDataSize (if outDataPtr is NULL)
//		input:	<ignored>
//		output:	size in bytes of cell's data
//
//	ioDataSize (if outDataPtr is not NULL)
//		input:	maximum bytes of data to retrieve
//		output:	actual bytes of data passed back

void
LTableView::GetCellData(
	const STableCell&	inCell,
	void*				outDataPtr,
	UInt32&				ioDataSize) const
{
	if (mTableStorage != NULL) {
		mTableStorage->GetCellData(inCell, outDataPtr, ioDataSize);
	} else {
		ioDataSize = 0;
	}
}


// ---------------------------------------------------------------------------
//	 FindCellData
// ---------------------------------------------------------------------------
//	Pass back the cell that contains the specified data
//
//	Return false if no match is found

bool
LTableView::FindCellData(
	STableCell&			outCell,
	const void*			inDataPtr,
	UInt32				inDataSize) const
{
	bool	found = false;

	if (mTableStorage != NULL) {
		found = mTableStorage->FindCellData(outCell, inDataPtr, inDataSize);
	}

	return found;
}

#pragma mark --- Hiliting ---

// ---------------------------------------------------------------------------
//	 HiliteSelection
// ---------------------------------------------------------------------------
//	Draw or undraw hiliting for the current selection in either the
//	active or inactive state

void
LTableView::HiliteSelection(
	bool	inActively,
	bool	inHilite)
{
	JRect	image_frame = GetAperture();					// Get image frame of aperture
	JRect   local_frame;
	ImageToLocalRect(image_frame, local_frame);
	STableCell	cell, topLeftCell, botRightCell;
	FetchIntersectingCells(local_frame, topLeftCell, botRightCell);

	for (cell.row = topLeftCell.row; cell.row <= botRightCell.row; cell.row++) {
		for (cell.col = topLeftCell.col; cell.col <= botRightCell.col; cell.col++) {
			if (CellIsSelected(cell)) {
				if (inActively) {
					HiliteCellActively(cell, inHilite);
				} else {
					HiliteCellInactively(cell, inHilite);
				}
			}
		}
	}

}


// ---------------------------------------------------------------------------
//	 HiliteCell
// ---------------------------------------------------------------------------
//	Draw or undraw hiliting for the specified Cell

void
LTableView::HiliteCell(
	const STableCell&	inCell,
	bool				inHilite)
{
	//if (IsActive()) {
		HiliteCellActively(inCell, inHilite);

	//} else {
	//	HiliteCellInactively(inCell, inHilite);
	//}
}


// ---------------------------------------------------------------------------
//	 HiliteCellActively
// ---------------------------------------------------------------------------
//	Draw or undraw active hiliting for a Cell

void
LTableView::HiliteCellActively(
	const STableCell&	inCell,
	bool				/* inHilite */)
{
	JRect	cellFrame;
    if (GetLocalCellRect(inCell, cellFrame)) {
    	RefreshCell(inCell);
	}
}


// ---------------------------------------------------------------------------
//	 HiliteCellInactively
// ---------------------------------------------------------------------------
//	Draw or undraw inactive hiliting for a Cell

void
LTableView::HiliteCellInactively(
	const STableCell&	inCell,
	bool				/* inHilite */)
{
	JRect	cellFrame;
	if (GetLocalCellRect(inCell, cellFrame)) {
    	RefreshCell(inCell);
	}
}


#pragma mark --- Clicking ---

// ---------------------------------------------------------------------------
//	 OnLButtonDown
// ---------------------------------------------------------------------------
//	Handle a mouse click within a TableView

void
LTableView::HandleMouseDown(const JPoint& pt,
							const JXMouseButton button,
							const JSize clickCount, 
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers)
{
	// Handle scroll wheel
	if (ScrollForWheel(button, modifiers))
		return;

	STableCell	hitCell;

	mDragStartPoint = pt;

	if (GetCellHitBy(pt, hitCell)) {
		if (clickCount == 1)
		{
			if (button == kJXButton1)
			{
				if (ClickSelect(hitCell, modifiers)) {
					LClickCell(hitCell, modifiers);
				}
			}
			else if (button == kJXRightButton)
			{
				if (ClickSelect(hitCell, modifiers)) {
					RClickCell(hitCell, modifiers);
				}
			}
		}
		else
		{
			if (button == kJXButton1)
				LDblClickCell(hitCell, modifiers);
		}

	} else {							// Click is outside of any Cell
		UnselectAllCells();
	}
}

void LTableView::HandleMouseDrag(const JPoint& pt, const JXButtonStates& buttonStates,
								const JXKeyModifiers& modifiers)
{
#ifdef _TODO
	if (IsDraggingSelection())
	{
		ScrollForDrag(pt);
		STableCell	hitCell;
		if (GetCellHitBy(pt, hitCell))
		{
			SelectCell(hitCell);
		}
		mDragType = kInvalidDrag;
	}
#endif
}

void LTableView::HandleMouseUp(const JPoint& pt, const JXMouseButton button,
							 const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// if we get with kWaitForDNDDrag, then we thought there might be be a drag
	// but there wasn't.
	if (mDragType == kWaitForDNDDrag)
	{
		// Already done in HandleMouseDown, but if it wasn't a drag we need to
		// clear the rest of the selection and just select what we clicked on.
		// We know that no modifier keys were pressed because you can't start
		// a drag with modifier keys pressed.
		STableCell	hitCell;
		if (GetCellHitBy(pt, hitCell))
		{
			SelectCell(hitCell);
		}
	}
	mDragType = kInvalidDrag;
}

// ---------------------------------------------------------------------------
//	 LClickCell
// ---------------------------------------------------------------------------
//	Handle a mouse click within the specified Cell

void
LTableView::LClickCell(
	const STableCell&		/* inCell */,
	const JXKeyModifiers& 	/* modifiers */)
{
}

// ---------------------------------------------------------------------------
//	 LDblClickCell
// ---------------------------------------------------------------------------
//	Handle a mouse double-click within the specified Cell

void
LTableView::LDblClickCell(
	const STableCell&		/* inCell */,
	const JXKeyModifiers& 	/* modifiers */)
{
}

// ---------------------------------------------------------------------------
//	 RClickCell
// ---------------------------------------------------------------------------
//	Handle a mouse click within the specified Cell

void
LTableView::RClickCell(
	const STableCell&		/* inCell */,
	const JXKeyModifiers& 	/* modifiers */)
{
}

#pragma mark --- Key board ---

// Handle key down
void LTableView::HandleKeyPress(const int key, const JXKeyModifiers& modifiers)
{
	if (!HandleChar(key, modifiers))
		LScrollable::HandleKeyPress(key, modifiers);
}

// Handle character
bool LTableView::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch(key)
	{
	case XK_Page_Up:
	case XK_KP_Page_Up:
		if ((mRows != 0) && (mCols != 0))
		{
			// Get top visible cell
			STableCell top = GetFirstFullyVisibleCell();
			if (CellIsSelected(top))
			{
				// Select cell one page up
				UInt16 count = GetVisibleRows();
				if (top.row > count)
					top.row -= count;
				else
					top.row = 1;
			}
			
			// Do scroll when current top is selected
			ClickSelect(top, modifiers);
			ScrollCellIntoFrame(top);
			Refresh();
		}
		break;
	case XK_Page_Down:
	case XK_KP_Page_Down:
		if ((mRows != 0) && (mCols != 0))
		{
			// Get bottom visible cell
			STableCell bottom = GetLastFullyVisibleCell();
			if (CellIsSelected(bottom))
			{
				// Select cell one page down
				UInt16 count = GetVisibleRows();
				bottom.row += count;
				if (bottom.row > mRows)
					bottom.row = mRows;
			}

			// Do scroll when current bottom is selected
			ClickSelect(bottom, modifiers);
			ScrollCellIntoFrame(bottom);
			Refresh();
		}
		break;
	case XK_Home:
	case XK_KP_Home:
		ScrollPinnedImageTo(0, 0, true);
		if ((mRows != 0) && (mCols != 0))
		{
			ClickSelect(STableCell(1, 1), modifiers);
		}
		break;
	case XK_End:
	case XK_KP_End:
		ScrollPinnedImageTo(INT_MAX/2, INT_MAX/2, true);
		if ((mRows != 0) && (mCols != 0))
		{
			ClickSelect(STableCell(mRows, mCols), modifiers);
		}
		break;
	default:
		// Do inherited action
		return false;
	}
	
	return true;
}

#pragma mark --- Drawing ---

// ---------------------------------------------------------------------------
//	 DrawSelf
// ---------------------------------------------------------------------------
//	Draw a TableView

void
LTableView::Draw(JXWindowPainter& pDC, const JRect& rect)
{
	DrawBackground(pDC, rect);

	// Undo the origin offset as we want to draw in local co-ords,
	// not image co-ords

	const JRect boundsG = GetBoundsGlobal();
	JRect localboundsG;
	LocalToImageRect(boundsG, localboundsG);
	pDC.SetOrigin(localboundsG.left, localboundsG.top);

		// Determine cells that need updating. Rather than checking
		// on a cell by cell basis, we just see which cells intersect
		// the bounding box of the update region. This is relatively
		// fast, but may result in unnecessary cell updates for
		// non-rectangular update regions.

	// Input rect is in image (bounds) coordinates
	JRect localRect;
	ImageToLocalRect(rect, localRect);

	STableCell	topLeftCell, botRightCell;
	FetchIntersectingCells(localRect, topLeftCell, botRightCell);
	
	DrawCellRange(&pDC, topLeftCell, botRightCell);
}


// ---------------------------------------------------------------------------
//	 DrawCellRange
// ---------------------------------------------------------------------------

void
LTableView::DrawCellRange(JPainter* pDC,
	const STableCell&	inTopLeftCell,
	const STableCell&	inBottomRightCell)
{
	STableCell	cell;
	
	for ( cell.row = inTopLeftCell.row;
		  cell.row <= inBottomRightCell.row;
		  cell.row++ ) {
		  
		for ( cell.col = inTopLeftCell.col;
			  cell.col <= inBottomRightCell.col;
			  cell.col++ ) {
			  
			JRect	cellRect;
			GetLocalCellRect(cell, cellRect);
			DrawCell(pDC, cell, cellRect);
		}
	}
}


// ---------------------------------------------------------------------------
//	 DrawCell
// ---------------------------------------------------------------------------
//	Draw the contents of the specified Cell

void
LTableView::DrawCell(JPainter* pDC,
	const STableCell&	/* inCell */,
	const JRect&			/* inLocalRect */)
{
}


// ---------------------------------------------------------------------------
//	 DrawCellSelection
// ---------------------------------------------------------------------------
//	Draw the selection of a Table cell

bool
LTableView::DrawCellSelection(JPainter* pDC, const STableCell& inCell)
{
	return false;
}


// ---------------------------------------------------------------------------
//	 DrawBackground
// ---------------------------------------------------------------------------
//	Draw the background of the Table

void
LTableView::DrawBackground(JXWindowPainter& pDC, const JRect& inLocalRect)
{
	LScrollable::DrawBackground(pDC, inLocalRect);
}


// ---------------------------------------------------------------------------
//	 RefreshCell
// ---------------------------------------------------------------------------
//	Invalidate the area occupied by the specified cell so that its
//	contents will be redrawn during the next update event

void
LTableView::RefreshCell(
	const STableCell&	inCell)
{
	JRect	cellRect;
	if (GetLocalCellRect(inCell, cellRect)) {
		JRect imageRect;
		LocalToImageRect(cellRect, imageRect);
		RefreshRect(imageRect);
	}
}


// ---------------------------------------------------------------------------
//	 RefreshCellRange
// ---------------------------------------------------------------------------
//	Invalidate a rectangular block of cells so that their contents will
//	be redrawn during the next update event

void
LTableView::RefreshCellRange(
	const STableCell&	inTopLeft,
	const STableCell&	inBotRight)
{
		// We only need to refresh the portion of the cell range
		// that is revealed within the Frame of the Table

	if (!IsVisible()) return;			// Nothing is visible

		// Find the range of cells that are revealed

	JRect	image_frame = GetAperture();					// Get image frame of aperture
	JRect   local_frame;
	ImageToLocalRect(image_frame, local_frame);

	STableCell	refreshTopLeft, refreshBotRight;
	FetchIntersectingCells(local_frame, refreshTopLeft, refreshBotRight);

		// Find the intersection of the input cell range
		// with the revealed cell range

	if (refreshTopLeft.row < inTopLeft.row) {	// At top left, intersection
		refreshTopLeft.row = inTopLeft.row;		//   is the greater of the
	}											//   two row and col values

	if (refreshTopLeft.col < inTopLeft.col) {
		refreshTopLeft.col = inTopLeft.col;
	}

	if (refreshBotRight.row > inBotRight.row) {	// At bot right, intersection
		refreshBotRight.row = inBotRight.row;	//   is the lesser of the
	}											//   two row and col values

	if (refreshBotRight.col > inBotRight.col) {
		refreshBotRight.col = inBotRight.col;
	}

		// There's nothing to refresh if the intersection is empty

	if ( (refreshTopLeft.row > refreshBotRight.row) ||
		 (refreshTopLeft.col > refreshBotRight.col) ) {
		return;
	}

		// Get rectangle in Local coords that bounds the
		// refresh cell range

	JRect	refreshRect;
	JRect	cellRect;

	GetLocalCellRect(refreshTopLeft, refreshRect);
	GetLocalCellRect(refreshBotRight, cellRect);
	refreshRect.right = cellRect.right;
	refreshRect.bottom = cellRect.bottom;

	JRect imageRect;
	LocalToImageRect(refreshRect, imageRect);
	RefreshRect(imageRect);
}

#pragma mark --- StDeferTableAdjustment ---

// ===========================================================================
//  StDeferTableAdjustment
// ===========================================================================
//	Stack-based class for setting and restoring the "defer adjustment"
//	state of a TableView. Use for efficiency when making a series of
//	changes to a Table. For example,
//
//		{
//			StDeferTableAdjustment  defer(myTable);
//
//				// Make calls to Insert or Remove cells from myTable
//		}

StDeferTableAdjustment::StDeferTableAdjustment(
	LTableView*		inTable)
{
	mTable = inTable;
	mSaveDefer = inTable->GetDeferAdjustment();
	inTable->SetDeferAdjustment(true);
}


StDeferTableAdjustment::~StDeferTableAdjustment()
{
	mTable->SetDeferAdjustment(mSaveDefer);
}
