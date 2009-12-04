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


//	CTableRowGeometry.cp

// Optimised version of LTableMultiGeometry for constant row height

#include "CTableRowGeometry.h"

// Constructor
CTableRowGeometry::CTableRowGeometry(LTableView *inTableView, UInt16 inColWidth, UInt16 inRowHeight)
	: LTableMultiGeometry(inTableView, inColWidth, inRowHeight)

{
}

// Destructor
CTableRowGeometry::~CTableRowGeometry()
{
}

// ---------------------------------------------------------------------------
//		¥ GetImageCellBounds
// ---------------------------------------------------------------------------
//	Pass back the location in Image coords of the specified Cell

void
CTableRowGeometry::GetImageCellBounds(
	const STableCell	&inCell,
	SInt32				&outLeft,
	SInt32				&outTop,
	SInt32				&outRight,
	SInt32				&outBottom) const
{
	outLeft = 0;
	outRight = 0;
	outTop = 0;
	outBottom = 0;

	if (mTableView->IsValidCell(inCell)) {

		outTop = (inCell.row - 1) * mDefaultRowHeight;
		outBottom = outTop + mDefaultRowHeight;

		for (TableIndexT col = 1; col < inCell.col; col++) {
			outLeft += mColWidths.at(col - 1);
		}
		outRight = outLeft + mColWidths.at(inCell.col - 1);
	}
}

// ---------------------------------------------------------------------------
//		¥ GetRowHitBy
// ---------------------------------------------------------------------------
//	Return the index number of the Row containing the specified point

TableIndexT
CTableRowGeometry::GetRowHitBy(
	const CPoint	&inImagePt) const
{
	return inImagePt.y / mDefaultRowHeight + 1;
}

// ---------------------------------------------------------------------------
//		¥ GetTableDimensions
// ---------------------------------------------------------------------------
//	Pass back size of the Table based on the number and size of the rows
//	and columns

void
CTableRowGeometry::GetTableDimensions(
	UInt32	&outWidth,
	UInt32	&outHeight) const
{
	UInt32	numRows, numCols;
	mTableView->GetTableSize(numRows, numCols);

	outHeight = mDefaultRowHeight * numRows;

	outWidth = 0;
	for (TableIndexT col = 1; col <= numCols; col++) {
		outWidth += mColWidths.at(col - 1);
	}
}

// ---------------------------------------------------------------------------
//		¥ GetRowHeight
// ---------------------------------------------------------------------------
//	Return the height of the specified row

UInt16
CTableRowGeometry::GetRowHeight(
	TableIndexT		inRow) const
{
	return mDefaultRowHeight;
}

void CTableRowGeometry::SetRowHeight(
								UInt16				inHeight,
								TableIndexT			inFromRow,
								TableIndexT			inToRow)
{
	// Change all rows together
	UInt32	numRows, numCols;
	mTableView->GetTableSize(numRows, numCols);
	
	LTableMultiGeometry::SetRowHeight(inHeight, 1, numRows);
	mDefaultRowHeight = inHeight;
}
