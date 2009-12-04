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

#include "CDrawUtils.h"
#include "CMulberryCommon.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <WIN_LTableArrayStorage.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

BEGIN_MESSAGE_MAP(CTextTable, CTableDragAndDrop)
	ON_WM_SIZE()
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextTable::CTextTable()
{
	mTableGeometry = new CTableRowGeometry(this, 16, 16);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, (UInt32) 0);

	SetRowSelect(true);

	mMsgTarget = NULL;
	mDoubleClickMsg = 0;
	mSelectionMsg = 0;
	mStringColumn = 1;
	mDoubleClickAll = false;
}

// Default destructor
CTextTable::~CTextTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Resize columns
void CTextTable::OnSize(UINT nType, int cx, int cy)
{
	CTableDragAndDrop::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Name column: variable
	SetColWidth(cx, 1, 1);
}

void CTextTable::EnableDragAndDrop()
{
	// Set Drag & Drop
	AddDropFlavor(CF_UNICODETEXT);
	AddDragFlavor(CF_UNICODETEXT);

	SetReadOnly(false);
	SetDropCell(false);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetExternalDrag(false);
}

// Broadcast message when selected cells change

void CTextTable::SelectionChanged()
{
	if (mSelectionMsg)
	{
		if (mMsgTarget)
			mMsgTarget->SendMessage(WM_COMMAND, mSelectionMsg);
		else
			GetParent()->SendMessage(WM_COMMAND, mSelectionMsg);
	}
	
	// Do inherited
	CTableDragAndDrop::SelectionChanged();
}

// Allow only single selection
void CTextTable::SetSingleSelection(void)
{
	delete mTableSelector;
	mTableSelector = new CTableSingleRowSelector(this);
}

void CTextTable::AddItem(const cdstring& item)
{
	// Prevent drawing to stop extra refresh
	StDeferTableAdjustment changing(this);

	// Make sure columns are present
	if (mCols != 1)
	{
		InsertCols(1, 0);

		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		SetColWidth(cx, 1, 1);
	}

	InsertRows(1, mRows, NULL, 0, false);

	STableCell aCell(mRows, 1);
	SetCellData(aCell, item.c_str(), item.length() + 1);

	RefreshCell(aCell);
}

void CTextTable::SetContents(const cdstrvect& items)
{
	// Prevent drawing to stop extra refresh
	StDeferTableAdjustment changing(this);

	// Make sure columns are present
	if (mCols != 1)
	{
		InsertCols(1, 0);

		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		SetColWidth(cx, 1, 1);
	}

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

// Handle key down
bool CTextTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_RETURN:
		// Fake double-click
		LDblClickCell(STableCell(1, mStringColumn), nFlags);
		break;

	default:;
	}

	// Did not handle key
	return CTableDragAndDrop::HandleKeyDown(nChar, nRepCnt, nFlags);
}

// Click in the cell
void CTextTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	if (((inCell.col == mStringColumn) || mDoubleClickAll) && mDoubleClickMsg)
	{
		if (mMsgTarget)
			mMsgTarget->SendMessage(WM_COMMAND, mDoubleClickMsg);
		else
			GetParent()->SendMessage(WM_COMMAND, mDoubleClickMsg);
	}
}

// Draw the entries
void CTextTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	// Save text state in stack object
	cdstring			theTxt;
	GetCellString(inCell, theTxt);
	StDCState		save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	// Move to origin for text
	int x = inLocalRect.left + 4;
	int y = inLocalRect.top + mTextOrigin;

	// Draw the string
	::DrawClippedStringUTF8(pDC, theTxt, CPoint(x, y), inLocalRect, eDrawString_Left);

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

BOOL CTextTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  (lpFormatEtc->cfFormat == CF_UNICODETEXT)
	{
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, sizeof(unsigned long));
		if (*phGlobal)
		{
			// Copy to global after lock
			unsigned long* pAtch = (unsigned long*) ::GlobalLock(*phGlobal);
			*pAtch = 0;
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
	}
	
	return rendered;
}

// Drop data at cell
bool CTextTable::DropDataAtCell(unsigned int theFlavor, char* drag_data,
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
