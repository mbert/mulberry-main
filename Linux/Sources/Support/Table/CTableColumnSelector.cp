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


//	CTableColumnSelector.cp

#include "CTableColumnSelector.h"

// Constructor
CTableColumnSelector::CTableColumnSelector(LTableView *inTableView)
	: LTableMultiSelector(inTableView)

{
}

// Destructor
CTableColumnSelector::~CTableColumnSelector()
{
}

// Select column
void CTableColumnSelector::SelectCell(const STableCell &inCell)
{
	// Select all columns in this row
	STableCell select(0, inCell.col);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	for(TableIndexT row = 1; row <= rows; row++)
	{
		select.row = row;
		LTableMultiSelector::SelectCell(select);
	}
}

// Unselect column
void CTableColumnSelector::UnselectCell(const STableCell &inCell)
{
	// Unselect all columns in this row
	STableCell select(0, inCell.col);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	for(TableIndexT row = 1; row <= rows; row++)
	{
		select.row = row;
		LTableMultiSelector::UnselectCell(select);
	}
}

// Select column block
void CTableColumnSelector::SelectCellBlock(const STableCell &inCellA, const STableCell &inCellB)
{
	STableCell cellA(inCellA);
	STableCell cellB(inCellB);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);
	
	if (inCellA.col <= inCellB.col)
	{
		cellA.row = 1;
		cellB.row = rows;
	}
	else if (inCellA.col > inCellB.col)
	{
		cellA.row = rows;
		cellB.row = 1;
	}
	
	LTableMultiSelector::SelectCellBlock(cellA, cellB);
}

#pragma mark -

// Constructor
CTableSingleColumnSelector::CTableSingleColumnSelector(LTableView *inTableView)
	: LTableMultiSelector(inTableView)

{
}

// Destructor
CTableSingleColumnSelector::~CTableSingleColumnSelector()
{
}

// Select column
void CTableSingleColumnSelector::SelectCell(const STableCell &inCell)
{
	// Select all columns in this row
	STableCell select(0, inCell.col);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	UnselectAllCells();

	for(TableIndexT row = 1; row <= rows; row++)
	{
		select.row = row;
		LTableMultiSelector::SelectCell(select);
	}
}

//	Adjust selection when clicking on a specified cell

void CTableSingleColumnSelector::ClickSelect(const STableCell &inCell, const JXKeyModifiers& modifiers)
{
	// Make sure shoft/control is off
	JXKeyModifiers mods(modifiers);
	mods.SetState(kJXShiftKeyIndex, kFalse);
	mods.SetState(kJXControlKeyIndex, kFalse);
	LTableMultiSelector::ClickSelect(inCell, mods);
}

// Unselect column
void CTableSingleColumnSelector::UnselectCell(const STableCell &inCell)
{
	// Unselect all columns in this row
	STableCell select(0, inCell.col);
	TableIndexT	rows, cols;
	mTableView->GetTableSize(rows, cols);

	for(TableIndexT row = 1; row <= rows; row++)
	{
		select.row = row;
		LTableMultiSelector::UnselectCell(select);
	}
}

