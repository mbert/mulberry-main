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


#ifndef _H_WIN_UTables
#define _H_WIN_UTables

typedef	UInt32	TableIndexT;			// Row/Column index


// ---------------------------------------------------------------------------
//	¥ STableCell												  STableCell ¥
// ---------------------------------------------------------------------------
//	A cell, specified by row and column index, in a two-dimensional table

class STableCell {
public:
	TableIndexT	row;
	TableIndexT	col;

					STableCell(
							TableIndexT			inRow = 0,
							TableIndexT			inCol = 0)
					{
						row = inRow;
						col = inCol;
					}

					STableCell(
							CPoint				inCellPoint)
					{
						row = inCellPoint.y;
						col = inCellPoint.x;
					}

	void			SetCell(
							TableIndexT			inRow,
							TableIndexT			inCol)
					{
						row = inRow;
						col = inCol;
					}

	bool			IsNullCell() const
					{
						return ((row == 0) && (col == 0));
					}

	void			ToPoint(
							CPoint				&outPoint) const
					{
						outPoint.x = (SInt16) col;
						outPoint.y = (SInt16) row;
					}

	bool			operator ==(
							const STableCell	&inCell) const
					{
						return ((row == inCell.row) && (col == inCell.col));
					}

	bool			operator !=(
							const STableCell	&inCell) const
					{
						return ((row != inCell.row) || (col != inCell.col));
					}

	bool			operator >(
							const STableCell	&inCell) const
					{
						return ( (row > inCell.row) ||
								((row == inCell.row) && (col > inCell.col)) );
					}

	bool			operator >=(
							const STableCell	&inCell) const
					{
						return ( (row > inCell.row) ||
								((row == inCell.row) && (col >= inCell.col)) );
					}

	bool			operator <(
							const STableCell	&inCell) const
					{
						return ( (row < inCell.row) ||
								((row == inCell.row) && (col < inCell.col)) );
					}

	bool			operator <=(
							const STableCell	&inCell) const
					{
						return ( (row < inCell.row) ||
								((row == inCell.row) && (col <= inCell.col)) );
					}
};

#endif
