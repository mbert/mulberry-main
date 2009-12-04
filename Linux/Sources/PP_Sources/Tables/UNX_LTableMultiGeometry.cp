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


//	Manages geometry of a TableView where rows and columns may have
//	different sizes

#include <UNX_LTableMultiGeometry.h>

// ---------------------------------------------------------------------------
//	 LTableMultiGeometry					Parameterized Constructor [public]
// ---------------------------------------------------------------------------

LTableMultiGeometry::LTableMultiGeometry(
	LTableView*		inTableView,
	UInt16			inColWidth,
	UInt16			inRowHeight)

	: LTableGeometry(inTableView)
{
	//SignalIf_(mTableView == nil);

	mDefaultRowHeight = inRowHeight;
	mDefaultColWidth = inColWidth;

	TableIndexT		numRows, numCols;
	inTableView->GetTableSize(numRows, numCols);

	if (numRows > 0) {
		mRowHeights.insert(mRowHeights.begin(), numRows, inRowHeight);
	}

	if (numCols > 0) {
		mColWidths.insert(mColWidths.begin(), numCols, inColWidth);
	}

	JPoint	theScrollUnit;
	theScrollUnit.x = inColWidth;
	theScrollUnit.y = inRowHeight;
	inTableView->SetScrollUnit(theScrollUnit);
}


// ---------------------------------------------------------------------------
//	 ~LTableMultiGeometry					Destructor				  [public]
// ---------------------------------------------------------------------------

LTableMultiGeometry::~LTableMultiGeometry()
{
}


// ---------------------------------------------------------------------------
//	 GetImageCellBounds
// ---------------------------------------------------------------------------
//	Pass back the location in Image coords of the specified Cell

void
LTableMultiGeometry::GetImageCellBounds(
	const STableCell&	inCell,
	SInt32&				outLeft,
	SInt32&				outTop,
	SInt32&				outRight,
	SInt32&				outBottom) const
{
	outLeft		= 0;
	outRight	= 0;
	outTop		= 0;
	outBottom	= 0;

	if (mTableView->IsValidCell(inCell)) {

		for (TableIndexT row = 1; row < inCell.row; row++) {
			outTop += mRowHeights.at(row - 1);
		}
		outBottom = outTop + mRowHeights.at(inCell.row - 1);

		for (TableIndexT col = 1; col < inCell.col; col++) {
			outLeft += mColWidths.at(col - 1);
		}
		outRight = outLeft + mColWidths.at(inCell.col - 1);
	}
}


// ---------------------------------------------------------------------------
//	 GetRowHitBy
// ---------------------------------------------------------------------------
//	Return the index number of the Row containing the specified point

TableIndexT
LTableMultiGeometry::GetRowHitBy(
	const JPoint&		inImagePt) const
{
	TableIndexT		hitRow;
	TableIndexT		numRows = mRowHeights.size();
	SInt32			vert = 0;

	for (hitRow = 1; hitRow <= numRows; hitRow++) {
		vert += mRowHeights.at(hitRow - 1);
		if (vert > inImagePt.y) break;
	}

	return hitRow;
}


// ---------------------------------------------------------------------------
//	 GetColHitBy
// ---------------------------------------------------------------------------
//	Return the index number of the Column containing the specified point

TableIndexT
LTableMultiGeometry::GetColHitBy(
	const JPoint&		inImagePt) const
{
	TableIndexT		hitCol;
	TableIndexT		numCols = mColWidths.size();
	SInt32			horiz = 0;

	for (hitCol = 1; hitCol <= numCols; hitCol++) {
		horiz += mColWidths.at(hitCol - 1);
		if (horiz > inImagePt.x) break;
	}

	return hitCol;
}


// ---------------------------------------------------------------------------
//	 GetTableDimensions
// ---------------------------------------------------------------------------
//	Pass back size of the Table based on the number and size of the rows
//	and columns

void
LTableMultiGeometry::GetTableDimensions(
	UInt32&		outWidth,
	UInt32&		outHeight) const
{
	UInt32	numRows, numCols;
	mTableView->GetTableSize(numRows, numCols);

	outHeight = 0;
	for (TableIndexT row = 1; row <= numRows; row++) {
		outHeight += mRowHeights.at(row - 1);
	}

	outWidth = 0;
	for (TableIndexT col = 1; col <= numCols; col++) {
		outWidth += mColWidths.at(col - 1);
	}
}


// ---------------------------------------------------------------------------
//	 GetRowHeight
// ---------------------------------------------------------------------------
//	Return the height of the specified row

UInt16
LTableMultiGeometry::GetRowHeight(
	TableIndexT		inRow) const
{
	UInt16	height = 0;

	if (mTableView->IsValidRow(inRow)) {
		height = mRowHeights.at(inRow - 1);
	}

	return height;
}


// ---------------------------------------------------------------------------
//	 SetRowHeight
// ---------------------------------------------------------------------------
//	Set the height of the specified rows

void
LTableMultiGeometry::SetRowHeight(
	UInt16			inHeight,
	TableIndexT		inFromRow,
	TableIndexT		inToRow)
{
	if ( (inToRow >= inFromRow) &&				// Check parameters
		 mTableView->IsValidRow(inFromRow) &&
		 mTableView->IsValidRow(inToRow) ) {

		 	// Cells starting at inFromRow and below must be refreshed.
		 	// We refresh both before and after changing the heights
		 	// to be sure that the proper area is refreshed.

		STableCell	fromCell(inFromRow, 1);
		STableCell	lastCell;
		mTableView->GetTableSize(lastCell.row, lastCell.col);

		mTableView->RefreshCellRange(fromCell, lastCell);

		for(TableIndexT row = inFromRow; row <= inToRow; row++)
			mRowHeights.at(row - 1) = inHeight;

		mTableView->RefreshCellRange(fromCell, lastCell);
	}
}


// ---------------------------------------------------------------------------
//	 GetColWidth
// ---------------------------------------------------------------------------
//	Return the width of the specified column

UInt16
LTableMultiGeometry::GetColWidth(
	TableIndexT		inCol) const
{
	UInt16	width = 0;

	if (mTableView->IsValidCol(inCol)) {
		width = mColWidths.at(inCol - 1);
	}

	return width;
}


// ---------------------------------------------------------------------------
//	 SetColWidth
// ---------------------------------------------------------------------------
//	Set the width of the specified columns

void
LTableMultiGeometry::SetColWidth(
	UInt16			inWidth,
	TableIndexT		inFromCol,
	TableIndexT		inToCol)
{
	if ( (inToCol >= inFromCol) &&				// Check parameters
		 mTableView->IsValidCol(inFromCol) &&
		 mTableView->IsValidCol(inToCol) ) {

		 	// Cells starting at inFromCol and to the right must
		 	// be refreshed. We refresh both before and after changing
		 	// the widths to be sure that the proper area is refreshed.

		STableCell	fromCell(1, inFromCol);
		STableCell	lastCell;
		mTableView->GetTableSize(lastCell.row, lastCell.col);

		mTableView->RefreshCellRange(fromCell, lastCell);

		for(TableIndexT col = inFromCol; col <= inToCol; col++)
			mColWidths.at(col - 1) = inWidth;

		mTableView->RefreshCellRange(fromCell, lastCell);
	}
}


// ---------------------------------------------------------------------------
//	 InsertRows
// ---------------------------------------------------------------------------
//	Insert specified number of rows into Geometry after a certain row

void
LTableMultiGeometry::InsertRows(
	UInt32			inHowMany,
	TableIndexT		inAfterRow)
{
	mRowHeights.insert(mRowHeights.begin() + inAfterRow, inHowMany, mDefaultRowHeight);
}


// ---------------------------------------------------------------------------
//	 InsertCols
// ---------------------------------------------------------------------------
//	Insert specified number of columns into Geometry after a certain column

void
LTableMultiGeometry::InsertCols(
	UInt32			inHowMany,
	TableIndexT		inAfterCol)
{
	mColWidths.insert(mColWidths.begin() + inAfterCol, inHowMany, mDefaultColWidth);
}


// ---------------------------------------------------------------------------
//	 RemoveRows
// ---------------------------------------------------------------------------
//	Remove specified number of rows from Geometry starting from a certain row

void
LTableMultiGeometry::RemoveRows(
	UInt32			inHowMany,
	TableIndexT		inFromRow)
{
	mRowHeights.erase(mRowHeights.begin() + (inFromRow - 1), mRowHeights.begin() + (inFromRow - 1 + inHowMany));
}


// ---------------------------------------------------------------------------
//	 RemoveCols
// ---------------------------------------------------------------------------
//	Remove specified number of cooumns from Geometry starting from a
//	certain column

void
LTableMultiGeometry::RemoveCols(
	UInt32			inHowMany,
	TableIndexT		inFromCol)
{
	mColWidths.erase(mColWidths.begin() + (inFromCol - 1), mColWidths.begin() + (inFromCol - 1 + inHowMany));
}
