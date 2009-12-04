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


// Header for CTableUtils class

#ifndef __CTABLEUTILS__MULBERRY__
#define __CTABLEUTILS__MULBERRY__

// ---------------------------------------------------------------------------
//	¥ STableCell												  STableCell ¥
// ---------------------------------------------------------------------------
//	A cell, specified by row and column index, in a two-dimensional table

class STableCell
{
public:
	TableIndexT	row;
	TableIndexT	col;

					STableCell(
							TableIndexT			inRow = -1,
							TableIndexT			inCol = -1)
					{
						row = inRow;
						col = inCol;
					}

	void			SetCell(
							TableIndexT			inRow,
							TableIndexT			inCol)
					{
						row = inRow;
						col = inCol;
					}

	Boolean			IsNullCell() const
					{
						return ((row == -1) && (col == -1));
					}

	Boolean			operator ==(
							const STableCell	&inCell) const
					{
						return ((row == inCell.row) && (col == inCell.col));
					}

	Boolean			operator !=(
							const STableCell	&inCell) const
					{
						return ((row != inCell.row) || (col != inCell.col));
					}

	Boolean			operator >(
							const STableCell	&inCell) const
					{
						return ( (row > inCell.row) ||
								((row == inCell.row) && (col > inCell.col)) );
					}

	Boolean			operator >=(
							const STableCell	&inCell) const
					{
						return ( (row > inCell.row) ||
								((row == inCell.row) && (col >= inCell.col)) );
					}

	Boolean			operator <(
							const STableCell	&inCell) const
					{
						return ( (row < inCell.row) ||
								((row == inCell.row) && (col < inCell.col)) );
					}

	Boolean			operator <=(
							const STableCell	&inCell) const
					{
						return ( (row < inCell.row) ||
								((row == inCell.row) && (col <= inCell.col)) );
					}
};

#endif
