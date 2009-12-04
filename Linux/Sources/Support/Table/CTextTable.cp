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


// Source for CTextTable class

#include "CTextTable.h"

#include "CMulberryCommon.h"
#include "CMulSelectionData.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <UNX_LTableArrayStorage.h>

#include <JPainter.h>
#include <JXDisplay.h>
#include <JXSelectionManager.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

const JCharacter* CTextTable::kSelectionChanged  = "kSelectionChanged::CTextTable";
const JCharacter* CTextTable::kLDblClickCell     = "kLDblClickCell::CTextTable";

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextTable::CTextTable(JXScrollbarSet* scrollbarSet,
						  JXContainer* enclosure,
						  const HSizingOption hSizing, 
						  const VSizingOption vSizing,
						  const JCoordinate x, const JCoordinate y,
						  const JCoordinate w, const JCoordinate h)
	: CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)

{
	mTableGeometry = new CTableRowGeometry(this, 16, 16);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, (UInt32) 0);

	SetRowSelect(true);

	mDoubleClickMsg = false;
	mSelectionMsg = false;
	mStringColumn = 1;
	mDoubleClickAll = false;
}

// Default destructor
CTextTable::~CTextTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Resize columns
void CTextTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CTableDragAndDrop::ApertureResized(dw, dh);

	//Since only the name column has variable width, we just
	//adjust it.
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
	}
}

void CTextTable::EnableDragAndDrop()
{
	// Set Drag & Drop
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDropFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());
	AddDropFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());

	SetReadOnly(false);
	SetDropCell(false);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetExternalDrag(false);
}

// Broadcast message when selected cells change

void CTextTable::DoSelectionChanged()
{
	if (mSelectionMsg)
		Broadcast(TextTableMessage(kSelectionChanged));
	
	// Do inherited
	CTableDragAndDrop::DoSelectionChanged();
}

// Allow only single selection
void CTextTable::SetSingleSelection(void)
{
	delete mTableSelector;
	mTableSelector = new CTableSingleRowSelector(this);
}

// Allow no selection
void CTextTable::SetNoSelection(void)
{
	delete mTableSelector;
	mTableSelector = NULL;
}

void CTextTable::AddItem(const cdstring& item)
{
	// Prevent drawing to stop extra refresh
	StDeferTableAdjustment changing(this);

	// Make sure columns are present
	if (mCols != 1)
	{
		InsertCols(1, 0);
		SetColWidth(GetApertureWidth(), 1, 1);
	}

	InsertRows(1, mRows, NULL, 0, false);

	STableCell aCell(mRows, 1);
	SetCellData(aCell, item.c_str(), item.length() + 1);

	RefreshCell(aCell);
}

void CTextTable::SetContents(const cdstrvect& items)
{
	// Make sure columns are present
	if (mCols != 1)
	{
		InsertCols(1, 0);
		SetColWidth(GetApertureWidth(), 1, 1);
	}

	// Remove all rows first
	RemoveAllRows(false);

	InsertRows(items.size(), 0, NULL, 0, false);

	STableCell aCell(0, 1);
	TableIndexT row = 1;
	for(cdstrvect::const_iterator iter = items.begin(); iter != items.end(); iter++, row++)
	{
		aCell.row = row;
		SetCellData(aCell, (*iter).c_str(), (*iter).length() + 1);
	}
}

void CTextTable::GetSelection(ulvector& selection) const
{
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
	{
		if (aCell.col == mStringColumn)
			selection.push_back(aCell.row - 1);
	}
}

void CTextTable::GetSelection(cdstrvect& selection) const
{
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
	{
		if (aCell.col == mStringColumn)
		{
			cdstring temp;
			GetCellString(aCell, temp);

			selection.push_back(temp);
		}
	}
}

void CTextTable::SetItemText(const TableIndexT row, const cdstring& item)
{
	STableCell aCell(row, 1);
	SetCellData(aCell, item.c_str(), item.length() + 1);
	RefreshRow(row);
}

// Click in the cell
void CTextTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	if (((inCell.col == mStringColumn) || mDoubleClickAll) && mDoubleClickMsg)
		Broadcast(TextTableMessage(kLDblClickCell));
}

// Draw the entries
void CTextTable::DrawCell(JPainter* pDC, const STableCell& inCell,
							const JRect& inLocalRect)
{
	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Move to origin for text
	int x = inLocalRect.left + 4;
	int y = inLocalRect.top;

	// Draw the string
	cdstring name;
	GetCellString(inCell, name);
	::DrawClippedStringUTF8(pDC, name.c_str(), JPoint(x, y), inLocalRect, eDrawString_Left);
}

void CTextTable::GetCellString(const STableCell& inCell, cdstring& str) const
{
	// Get data length
	UInt32	len = 0;
	GetCellData(inCell, NULL, len);
	
	// Allocate data size and load in data
	str.reserve(len);
	GetCellData(inCell, str.c_str_mod(), len);
}

bool CTextTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	// gcc is stupid and does not know which ItemIsAcceptable to use
	// here even though the argument types are clearly different
	if (CTableDragAndDrop::ItemIsAcceptable(type))
	{
		int count = 0;

		// Allocate global memory for the count
		unsigned long dataLength = sizeof(int);
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			int* i = reinterpret_cast<int*>(data);
			*i = count;
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}

	return rendered;
}

// Drop data at cell
bool CTextTable::DropDataAtCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& before_cell)
{
	// Clear drag accumulation object
	mDragged.mTable = this;
	mDragged.mDragged.clear();
	mDragged.mDropped = 0;

	// Just add drag items to accumulator
	GetSelection(mDragged.mDragged);
	mDragged.mDropped = before_cell.row - 1;

	// Broadcast the entire drag
	Broadcast_Message(eBroadcast_Drag, &mDragged);
	
	return true;
}
