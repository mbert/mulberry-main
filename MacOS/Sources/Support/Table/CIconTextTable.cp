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

#include "CMulberryCommon.h"
#include "CTableRowGeometry.h"

// __________________________________________________________________________________________________
// C L A S S __ C T E X T T A B L E
// __________________________________________________________________________________________________

const int cIconColumnWidth = 15;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CIconTextTable::CIconTextTable()
{
	InitIconTextTable();
}

// Default constructor - just do parents' call
CIconTextTable::CIconTextTable(LStream *inStream)
		: CTextTable(inStream)

{
	InitIconTextTable();
}

// Default destructor
CIconTextTable::~CIconTextTable()
{
}

// Common init
void CIconTextTable::InitIconTextTable(void)
{
	mSelectWithTab = false;
	mSelectItem = 0;
	SetOneColumnSelect(2);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CIconTextTable::SetIcon(TableIndexT row, ResIDT set)
{
	STableCell aCell(row, 1);
	ResIDT old_set;
	size_t res_size = sizeof(ResIDT);
	GetCellData(aCell, &old_set, res_size);

	if (old_set != set)
	{
		SetCellData(aCell, &set, sizeof(ResIDT));

		// Do immediate redraw
		RefreshCell(aCell);
		UpdatePort();
	}
}

void CIconTextTable::SetAllIcons(ResIDT set)
{
	for(TableIndexT row = 1; row <= mRows; row++)
		SetIcon(row, set);
}

void CIconTextTable::AddItem(const cdstring& item)
{
	InsertRows(1, mRows, NULL, 0, false);

	STableCell aCell1(mRows, 1);
	STableCell aCell2(mRows, 2);
	ResIDT set = 0;
	SetCellData(aCell1, &set, sizeof(ResIDT));
	SetCellData(aCell2, item.c_str(), item.length() + 1);

	RefreshRow(mRows);
}

void CIconTextTable::SetContents(const cdstrvect& items, const ulvector& states)
{
	RemoveAllRows(false);
	InsertRows(items.size(), 0, NULL, 0, false);

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

	Refresh();
}

// Get details of sub-panes
void CIconTextTable::FinishCreateSelf(void)
{
	// Do inherited
	CTextTable::FinishCreateSelf();

	// Row height is smaller
	static_cast<CTableRowGeometry*>(mTableGeometry)->SetDefaultRowHeight(12);

	// Make it fit to the superview
	// NB One column has already been inserted by base class
	InsertCols(1, 1, NULL, 0, false);
	mStringColumn = 2;

	// Get super frame's width - scroll bar
	SDimension16 super_frame;
	mSuperView->GetFrameSize(super_frame);
	super_frame.width -= 16;

	// Set image to super frame size
	ResizeImageTo(super_frame.width, mImageSize.height, true);

	// Set column width
	SetColWidth(cIconColumnWidth, 1, 1);
	SetColWidth(super_frame.width - cIconColumnWidth, 2, 2);
}

// Handle key presses
Boolean CIconTextTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
	// Delete item
	case char_Tab:
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

	return CTextTable::HandleKeyPress(inKeyEvent);
}

// Draw line between columns
void CIconTextTable::DrawSelf()
{
	// Do inherited
	CTextTable::DrawSelf();

	// Draw line
	Rect localRect;
	CalcLocalFrameRect(localRect);
	::MoveTo(localRect.left + cIconColumnWidth, localRect.top);
	::LineTo(localRect.left + cIconColumnWidth, localRect.bottom);
}

// Draw the entries
void CIconTextTable::DrawCell(const STableCell& inCell, const Rect& inLocalRect)
{
	// Save text & color state in stack objects
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Make sure there's a white background for the selection InvertRect
	{
		Rect tempRect = inLocalRect;
		if (inCell.col == 1)
			tempRect.right--;
		::EraseRect(&tempRect);
	}

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper,&inLocalRect,&clipper);
	StClipRgnState	clip(clipper);

	if (inCell.col == 1)
	{
		ResIDT set;
		UInt32 len = sizeof(ResIDT);
		GetCellData(inCell, &set, len);

		if (set)
		{
			Rect iconRect = inLocalRect;
			iconRect.right = iconRect.left + 16;
			iconRect.bottom = inLocalRect.bottom - mIconDescent;
			iconRect.top = iconRect.bottom - 16;

			::Ploticns(&iconRect, atNone, ttNone, set);
		}
	}
	else
	{
		// Redraw column boundary i case selection wipes it
		::MoveTo(inLocalRect.left, inLocalRect.top);
		::LineTo(inLocalRect.left, inLocalRect.bottom - 1);

		// Move to origin for text
		::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);

		// Get data
		cdstring str;
		GetCellString(inCell, str);

		// Draw the string
		::DrawClippedStringUTF8(str, inLocalRect.right - inLocalRect.left, eDrawString_Left);
	}
}

void CIconTextTable::HiliteCellActively(const STableCell &inCell, Boolean inHilite)
{
	// Ignore icon column
	if (inCell.col != 1)
		CTextTable::HiliteCellActively(inCell, inHilite);
}

void CIconTextTable::HiliteCellInactively(const STableCell &inCell, Boolean inHilite)
{
	// Ignore icon column
	if (inCell.col != 1)
		CTextTable::HiliteCellInactively(inCell, inHilite);
}