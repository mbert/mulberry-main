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

#include "CIconLoader.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

#include <JPainter.h>
#include <JXImage.h>
#include <JXWindow.h>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

const int cIconColumnWidth = 15;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIconTextTable::CIconTextTable(JXScrollbarSet* scrollbarSet,
								  JXContainer* enclosure,
								  const HSizingOption hSizing, 
								  const VSizingOption vSizing,
								  const JCoordinate x, const JCoordinate y,
								  const JCoordinate w, const JCoordinate h)
	: CTextTable(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
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
void CIconTextTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CTableDragAndDrop::ApertureResized(dw, dh);

	// Icon column: fixed; Name column: variable
	if (mCols)
	{
		SetColWidth(cIconColumnWidth, 1, 1);
		JCoordinate cw = GetApertureWidth() - cIconColumnWidth;
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 2, 2);
	}
}

void CIconTextTable::SetIcon(TableIndexT row, unsigned long set)
{
	STableCell aCell(row, 1);
	unsigned long old_set;
	UInt32 res_size = sizeof(unsigned long);
	GetCellData(aCell, &old_set, res_size);
	
	if (old_set != set)
	{
		SetCellData(aCell, &set, sizeof(unsigned long));
		
		// Do immediate redraw
		RefreshRow(row);
		Redraw();
	}
}

void CIconTextTable::SetAllIcons(unsigned long set)
{
	for(TableIndexT row = 1; row <= mRows; row++)
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

		SetColWidth(cIconColumnWidth, 1, 1);
		SetColWidth(GetApertureWidth() - cIconColumnWidth, 2, 2);	
		
		mStringColumn = 2;
	}

	STableCell aCell1(mRows, 1);
	STableCell aCell2(mRows, 2);
	unsigned long set = 0;
	SetCellData(aCell1, &set, sizeof(unsigned long));
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

		SetColWidth(cIconColumnWidth, 1, 1);
		SetColWidth(GetApertureWidth() - cIconColumnWidth, 2, 2);	
		
		mStringColumn = 2;
	}

	STableCell aCell1(0, 1);
	STableCell aCell2(0, 2);
	TableIndexT row = 1;
	cdstrvect::const_iterator iter1 = items.begin();
	ulvector::const_iterator iter2 = states.begin();
	for(; iter1 != items.end(); iter1++, iter2++, row++)
	{
		unsigned long set = *iter2;
		aCell1.row = row;
		aCell2.row = row;
		SetCellData(aCell1, &set, sizeof(unsigned long));
		SetCellData(aCell2, *iter1, (*iter1).length() + 1);
	}
}

// Handle key presses
bool CIconTextTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch (key)
	{
	case '\t':
		// Do tab selection of next hit item
		if (mSelectWithTab && mSelectItem)
		{
			// Scan down from last row
			TableIndexT row = GetLastSelectedRow();
			for(row++; row <= GetItemCount(); row++)
			{
				unsigned long set;
				UInt32 len = sizeof(unsigned long);
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

		// Need to force focus to next widget			
		GetWindow()->UnregisterFocusWidget(this);
		GetWindow()->RegisterFocusWidget(this);
		break;

	default:;
	}

	return CTextTable::HandleChar(key, modifiers);
}

// Draw the entries
void CIconTextTable::DrawCell(JPainter* pDC, const STableCell&	inCell,
							const JRect& inLocalRect)
{
	// Draw selection
	DrawCellSelection(pDC, inCell);

	if (inCell.col == 1)
	{
		unsigned long icon;
		UInt32 len = sizeof(unsigned long);
		GetCellData(inCell, &icon, len);
		if (icon)
		{
			JXImage* icon_image = CIconLoader::GetIcon(icon, this, 16, 0x00FFFFFF);
			pDC->Image(*icon_image,
				   icon_image->GetBounds(),
				   inLocalRect.left,
				   inLocalRect.top + mIconOrigin);
		}
	}
	else
	{
		cdstring			theTxt;
		GetCellString(inCell, theTxt);

		// Move to origin for text
		int x = inLocalRect.left + 4;
		int y = inLocalRect.top + mTextOrigin;

		// Draw the string
		::DrawClippedStringUTF8(pDC, theTxt, JPoint(x, y), inLocalRect, eDrawString_Left);
	}
}
