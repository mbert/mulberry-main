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


// Source for CIconTextTable class

#include "CIconTextTable.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

const int cIconColumnWidth = 15;

BEGIN_MESSAGE_MAP(CIconTextTable, CTextTable)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIconTextTable::CIconTextTable()
{
	mSelectWithTab = false;
	mSelectItem = 0;
	SetOneColumnSelect(2);
}

// Default destructor
CIconTextTable::~CIconTextTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Resize columns
void CIconTextTable::OnSize(UINT nType, int cx, int cy)
{
	// Skip CTextTable implementation as we don't want to resize columns twice!
	CTableDragAndDrop::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Icon column: fixed; Name column: variable
	SetColWidth(cIconColumnWidth, 1, 1);
	SetColWidth(cx - cIconColumnWidth, 2, 2);
}

void CIconTextTable::SetIcon(TableIndexT row, UINT set)
{
	STableCell aCell(row, 1);
	ResIDT old_set;
	UInt32 res_size = sizeof(ResIDT);
	GetCellData(aCell, &old_set, res_size);
	
	if (old_set != set)
	{
		SetCellData(aCell, &set, sizeof(ResIDT));
		
		// Do immediate redraw
		RefreshRow(row);
		UpdateWindow();
	}
}

void CIconTextTable::SetAllIcons(UINT set)
{
	for(int row = 1; row <= mRows; row++)
		SetIcon(row, set);
}

void CIconTextTable::AddItem(const cdstring& item)
{
	// Prevent drawing to stop extra refresh
	StDeferTableAdjustment changing(this);

	InsertRows(1, mRows, NULL, 0, false);

	// Make sure columns are present
	if (mCols != 2)
	{
		InsertCols(2, 1);

		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		SetColWidth(cIconColumnWidth, 1, 1);
		SetColWidth(cx - cIconColumnWidth, 2, 2);
		
		mStringColumn = 2;
	}

	STableCell aCell1(mRows, 1);
	STableCell aCell2(mRows, 2);
	ResIDT set = 0;
	SetCellData(aCell1, &set, sizeof(ResIDT));
	SetCellData(aCell2, item.c_str(), item.length() + 1);

	RefreshRow(mRows);
}

void CIconTextTable::SetContents(const cdstrvect& items, const ulvector& states)
{
	// Prevent drawing to stop extra refresh
	StDeferTableAdjustment changing(this);

	RemoveAllRows(false);
	InsertRows(items.size(), 0, NULL, 0, false);

	// Make sure columns are present
	if (mCols != 2)
	{
		InsertCols(2, 1);

		CRect client;
		GetClientRect(client);
		int cx = client.Width() - 16;

		SetColWidth(cIconColumnWidth, 1, 1);
		SetColWidth(cx - cIconColumnWidth, 2, 2);
		
		mStringColumn = 2;
	}

	STableCell aCell1(0, 1);
	STableCell aCell2(0, 2);
	TableIndexT row = 1;
	cdstrvect::const_iterator iter1 = items.begin();
	ulvector::const_iterator iter2 = states.begin();
	for(; iter1 != items.end(); iter1++, iter2++, row++)
	{
		ResIDT set = *iter2;
		aCell1.row = row;
		aCell2.row = row;
		SetCellData(aCell1, &set, sizeof(ResIDT));
		SetCellData(aCell2, *iter1, (*iter1).length() + 1);
	}
}

// Handle key down
bool CIconTextTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_TAB:
		// Do tab selection of next hit item
		if (mSelectWithTab && mSelectItem)
		{
			// Scan down from last row
			TableIndexT row = GetLastSelectedRow();
			for(row++; row <= GetItemCount(); row++)
			{
				ResIDT set;
				UInt32 len = sizeof(ResIDT);
				STableCell cell(row, 1);
				GetCellData(cell, &set, len);

				// Look for icon data that matches selectable type
				if (set == mSelectItem)
				{
					// Select/show the row and return
					ScrollToRow(row, true, true, eScroll_Center);
					return true;
				}
			}
			
			// Remove selection and fall through for default behaviour
			UnselectAllCells();
		}
		break;

	default:;
	}

	// Did not handle key
	return CTextTable::HandleKeyDown(nChar, nRepCnt, nFlags);
}

// Draw the entries
void CIconTextTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	// Save text state in stack object
	StDCState		save(pDC);

	// Draw selection
	DrawCellSelection(pDC, inCell);

	if (inCell.col == 1)
	{
		ResIDT icon;
		UInt32 len = sizeof(ResIDT);
		GetCellData(inCell, &icon, len);
		if (icon)
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, icon, 16);
	}
	else
	{
		cdstring			theTxt;
		GetCellString(inCell, theTxt);

		// Move to origin for text
		int x = inLocalRect.left + 4;
		int y = inLocalRect.top + mTextOrigin;

		// Draw the string
		::DrawClippedStringUTF8(pDC, theTxt, CPoint(x, y), inLocalRect, eDrawString_Left);
	}
}
