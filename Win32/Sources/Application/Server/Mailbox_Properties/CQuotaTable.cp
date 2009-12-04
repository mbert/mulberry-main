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

#include "CDrawUtils.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

#include <WIN_LTableArrayStorage.h>

IMPLEMENT_DYNCREATE(CQuotaTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CQuotaTable, CHierarchyTableDrag)
END_MESSAGE_MAP()

CQuotaTable::CQuotaTable()
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

BOOL CQuotaTable::SubclassDlgItem(UINT nID, CWnd* pParent)
{
	// Do inherited
	BOOL result = CHierarchyTableDrag::SubclassDlgItem(nID, pParent);

	// Drag and drop
	SetAllowDrag(false);

	// Create columns and adjust flag rect
	InsertCols(4, 1);

	CRect client;
	GetClientRect(client);
	int cx = client.Width() - 16;

	// Name column has variable width
	SetColWidth(cx - 180, 1, 1);
	
	// Remaining columns have fixed width
	SetColWidth(60, 2, 4);

	return result;
}

void CQuotaTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	TableIndexT	woRow = GetWideOpenIndex(inCell.row);
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
		theTxt = is_root ? root->GetName() : item->GetItem();
		DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, 0U);
		break;
	
	case 2:
		// Draw current
		if (!is_root)
		{
			theTxt = item->GetCurrent();
			::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	case 3:
		// Draw max
		if (!is_root)
		{
			theTxt = item->GetMax();
			::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
		}
		break;
	
	case 4:
		// Draw % used
		if (!is_root)
		{
			if (item->GetMax() > 0)
			{
				theTxt = (100* item->GetCurrent()) / item->GetMax();
				theTxt += '%';
			}
			else
				theTxt = "-";
			::DrawClippedStringUTF8(pDC, theTxt, CPoint(inLocalRect.left, inLocalRect.top + mTextOrigin), inLocalRect, eDrawString_Right);
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
	while (mRows)
		RemoveRows(1, mRows, false);

	TableIndexT next_row = 0;

	// Add each root
	for(CQuotaRootList::iterator iter1 = mQuotas->begin(); iter1 != mQuotas->end(); iter1++)
	{
		
		next_row = InsertSiblingRows(1, next_row, &iter1, sizeof(CQuotaRoot*), true, false);
		
		// Add items
		for(CQuotaItemList::const_iterator iter2 = (*iter1).GetItems().begin(); iter2 != (*iter1).GetItems().end(); iter2++)
			InsertChildRows(1, next_row, &iter2, sizeof(CQuotaItem*), false, false);
	}

	// Expand all rows
	TableIndexT expand_row = 1;
	while(expand_row < mRows)
	{
		TableIndexT	woRow;
		woRow = mCollapsableTree->GetWideOpenIndex(expand_row);
		if (mCollapsableTree->IsCollapsable(woRow))
			DeepExpandRow(woRow);
		expand_row++;
	}
}

void CQuotaTable::SetList(CQuotaRootList* aList)
{
	mQuotas = aList;

	// Reset table based on new quotas
	if (mQuotas)
		ResetTable();
}
