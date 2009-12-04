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
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

#include <LDropFlag.h>
#include <LNodeArrayTree.h>
#include <LTableArrayStorage.h>

#include <stdio.h>

const short cHorizIndent = 18;

CQuotaTable::CQuotaTable(LStream *inStream)
	: CHierarchyTableDrag(inStream)

{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(CQuotaRoot*));
	mSelectionIconIndent = false;
	mQuotas = NULL;
}


CQuotaTable::~CQuotaTable()
{
	// Quota list is owned by table so delete it
	delete mQuotas;
}

// Get details of sub-panes
void CQuotaTable::FinishCreateSelf(void)
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Create columns and adjust flag rect
	InsertCols(4, 1, NULL, 0, false);
	AdaptToNewSurroundings();

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set read only status of Drag and Drop
	SetAllowDrag(false);
}

void CQuotaTable::DrawCell(const STableCell &inCell, const Rect &inLocalRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	bool is_root = IsCollapsable(woRow);

	STableCell	woCell(woRow, inCell.col);
	CQuotaRoot* root = NULL;
	CQuotaItem* item = NULL;
	UInt32 dataSize = (is_root ? sizeof(CQuotaRoot*) : sizeof(CQuotaItem*));
	GetCellData(woCell,  is_root ? (char*) &root : (char*) &item, dataSize);

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	switch(inCell.col)
	{

	case 1:
		DrawDropFlag(inCell, woRow);

#if PP_Target_Carbon
		// Draw selection
		DrawCellSelection(inCell);
#endif

		UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

		// Get name of item
		theTxt = is_root ? root->GetName() : item->GetItem();
		::MoveTo(inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent, inLocalRect.bottom - mTextDescent);
		short width = inLocalRect.right - (inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent);
		::DrawClippedStringUTF8(theTxt, width, eDrawString_Left);
		break;

	case 2:
		// Draw current
		if (!is_root)
		{
			theTxt = item->GetCurrent();
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Right);
		}
		break;

	case 3:
		// Draw max
		if (!is_root)
		{
			theTxt = item->GetMax();
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Right);
		}
		break;

	case 4:
		// Draw percent
		if (!is_root)
		{
			if (item->GetMax() > 0)
			{
				theTxt = (100* item->GetCurrent()) / item->GetMax();
				theTxt += '%';
			}
			else
				theTxt = "-";
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Right);
		}
		break;

	default:
		break;
	}
}





void CQuotaTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
{
	LHierarchyTable::CalcCellFlagRect(inCell, outRect);
	outRect.right = outRect.left + 16;
	outRect.bottom = outRect.top + 12;
	::OffsetRect(&outRect, 0, -2);
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
	TableIndexT expand_row = 0;
	while(expand_row < mRows)
	{
		TableIndexT	woRow;
		woRow = mCollapsableTree->GetWideOpenIndex(expand_row);
		if (mCollapsableTree->IsCollapsable(woRow))
			DeepExpandRow(woRow);
		expand_row++;
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
void CQuotaTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	LHierarchyTable::AdaptToNewSurroundings();

	// Set image to frame size
	ResizeImageTo(mFrameSize.width, mImageSize.height, true);

	// Name column has variable width
	SetColWidth(mFrameSize.width - 180, 1, 1);

	// Remaining columns have fixed width
	SetColWidth(60, 2, 2);
	SetColWidth(60, 3, 3);
	SetColWidth(60, 4, 4);

} // CQuotaTable::AdaptToNewSurroundings
