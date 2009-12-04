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

//	Adjust selection when clicking on a specified cell

void CTableSingleRowSelector::ClickSelect(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	SMouseDownEvent modified = inMouseDown;
	modified.macEvent.modifiers &= ~(cmdKey | shiftKey);
	LTableMultiSelector::ClickSelect(inCell, modified);
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

