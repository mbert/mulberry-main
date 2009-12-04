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


//	CQuotaTable.cp

#include "CQuotaTable.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSimpleTitleTable.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <UNX_LTableArrayStorage.h>

const short cHorizIndent = 18;

CQuotaTable::CQuotaTable(JXScrollbarSet* scrollbarSet, 
							JXContainer* enclosure,
							const HSizingOption hSizing, 
							const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(CQuotaRoot*));

	SetRowSelect(true);

	mQuotas = NULL;
	
	mHierarchyCol = 1;
}


CQuotaTable::~CQuotaTable()
{
	// Quota list is owned by table so delete it
	delete mQuotas;
}

// Get details of sub-panes
void CQuotaTable::OnCreate()
{
	CHierarchyTableDrag::OnCreate();

	// Create columns and adjust flag rect
	InsertCols(4, 1);

	// Name column has variable width
	JCoordinate cw = GetApertureWidth() - 240;
	if (cw < 32)
		cw = 32;
	SetColWidth(cw, 1, 1);

	// Remaining columns have fixed width
	SetColWidth(80, 2, 4);

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
}

void CQuotaTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	unsigned long woRow = GetWideOpenIndex(inCell.row);
	bool is_root = IsCollapsable(woRow);
	cdstring theTxt;

	STableCell	woCell(woRow, inCell.col);
	CQuotaRoot* root = NULL;
	CQuotaItem* item = NULL;
	UInt32 dataSize = (is_root ? sizeof(CQuotaRoot*) : sizeof(CQuotaItem*));
	GetCellData(woCell,  is_root ? (char*) &root : (char*) &item, dataSize);

	switch(inCell.col)
	{
	
	case 1:
		// Get name of item
		theTxt = (is_root ? root->GetName() : item->GetItem());
		DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, NULL);
		break;
	
	case 2:
		// Draw current
		if (!is_root)
		{
			theTxt = item->GetCurrent();
			::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	case 3:
		// Draw max
		if (!is_root)
		{
			theTxt = item->GetMax();
			::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	case 4:
		// Draw % used
		if (!is_root)
		{
			if (item->GetMax() > 0)
			{
				theTxt = (100 * item->GetCurrent()) / item->GetMax();
				theTxt += "%";
			}
			else
				theTxt = "-";
			::DrawClippedStringUTF8(pDC, theTxt, JPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	default:
		break;
	}
}

// Reset the table from the mbox
void CQuotaTable::ResetTable(void)
{
	// Delete all existing rows
	RemoveAllRows(false);

	TableIndexT next_row = 0;

	// Add each root
	for(CQuotaRootList::iterator iter1 = mQuotas->begin(); iter1 != mQuotas->end(); iter1++)
	{

		next_row = InsertSiblingRows(1, next_row, &iter1, sizeof(CQuotaRoot*), true, false);

		// Add items
		for(CQuotaItemList::const_iterator iter2 = (*iter1).GetItems().begin(); iter2 != (*iter1).GetItems().end(); iter2++)
			InsertChildRows(1, next_row, &iter2, sizeof(CQuotaItem*), false, false);

		// Always expand
		DeepExpandRow(next_row);
	}

	Refresh();
}

void CQuotaTable::SetList(CQuotaRootList* aList)
{
	mQuotas = aList;

	// Reset table based on new quotas
	if (mQuotas)
		ResetTable();
}

// Adjust column widths
void CQuotaTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Do inherited call
	CHierarchyTableDrag::ApertureResized(dw, dh);

	// NB Can be called before columns exist
	if (mCols > 0)
	{
		// Name column has variable width
		JCoordinate cw = GetApertureWidth() - 240;
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
	
		// Remaining columns have fixed width
		SetColWidth(80, 2, 4);
	}
}
