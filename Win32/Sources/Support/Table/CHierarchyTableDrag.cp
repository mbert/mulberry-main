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


// CHierarchyTableDrag.h

// Header file for hierarchy table class that implements drag and drop extras

#include "CHierarchyTableDrag.h"

#include "CNodeVectorTree.h"
#include "CStringUtils.h"

IMPLEMENT_DYNCREATE(CHierarchyTableDrag, LHierarchyTable)

BEGIN_MESSAGE_MAP(CHierarchyTableDrag, LHierarchyTable)
END_MESSAGE_MAP()

CHierarchyTableDrag::CHierarchyTableDrag()
{
	mKeySelection = false;
	mLastTyping = 0UL;
	mLastChars[0] = '\0';
}

CHierarchyTableDrag::~CHierarchyTableDrag()
{
}

// Remove everything from table
void CHierarchyTableDrag::Clear(void)
{
	static_cast<CNodeVectorTree*>(mCollapsableTree)->Clear();
	CTableDragAndDrop::RemoveRows(mRows, 1, true);
}

Boolean CHierarchyTableDrag::HasSibling(UInt32 inWideOpenIndex)
{
	return static_cast<CNodeVectorTree*>(mCollapsableTree)->HasSibling(inWideOpenIndex);
}

Boolean CHierarchyTableDrag::HasParent(UInt32 inWideOpenIndex)
{
	return static_cast<CNodeVectorTree*>(mCollapsableTree)->HasParent(inWideOpenIndex);
}

Boolean CHierarchyTableDrag::HasChildren(UInt32 inWideOpenIndex)
{
	return static_cast<CNodeVectorTree*>(mCollapsableTree)->HasChildren(inWideOpenIndex);
}

// Handle key down
bool CHierarchyTableDrag::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_LEFT:
	{
		STableCell aCell(0, 0);
		bool did_collapse = false;
		while(GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(aCell.row);
				if (IsCollapsable(woRow) && IsExpanded(woRow))
				{
					if (::GetKeyState(VK_CONTROL) < 0)
						DeepCollapseRow(woRow);
					else
						CollapseRow(woRow);
					RefreshRow(aCell.row);
					did_collapse = true;
				}
			}
		}
		
		if (!did_collapse)
		{
			GetFirstSelection(aCell);
			if (aCell.row)
			{
				// Move up hierarchy to parent
				TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(aCell.row);
				TableIndexT parent = mCollapsableTree->GetParentIndex(woRow);
				
				// Only if parent exists
				if (parent)
				{
					aCell.row = mCollapsableTree->GetExposedIndex(parent);

					// Fake click on parent
					FakeClickSelect(aCell, false);

					// Bring it into view if it went out
					ShowLastSelection();
				}
				else
				{
					// Unselect all and scroll to top
					UnselectAllCells();
					ScrollCellIntoFrame(STableCell(1, 1));
				}
			}
		}
		return true;
	}
	
	case VK_RIGHT:
	{
		STableCell aCell(0, 0);
		while(GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(aCell.row);
				if (IsCollapsable(woRow) && !IsExpanded(woRow))
				{
					if (::GetKeyState(VK_CONTROL) < 0)
						DeepExpandRow(woRow);
					else
						ExpandRow(woRow);
					RefreshRow(aCell.row);
				}
			}
		}
		return true;
	}

	default:
		// Allow typing selection from any alphanumeric character - not cmd-xx
		if (mKeySelection &&
			(::GetKeyState(VK_CONTROL) >= 0) &&
			(::GetKeyState(VK_MENU) >= 0))
		{
			char hit_key = (nChar & 0x000000FF);
			if (::isalpha(hit_key))
			{
				// See if key hit within standard double click time
				unsigned long new_time = clock()/CLOCKS_PER_SEC;
				if (new_time - mLastTyping >= 1)
				{

					// Outside double-click time so new selection
					mLastChars[0] = hit_key;
					mLastChars[1] = 0;
				}
				else
				{
					// Inside double-click time so add char to selection
					short pos = ::strlen(mLastChars);
					if (pos < 31)
					{
						mLastChars[pos++] = hit_key;
						mLastChars[pos] = 0;
					}
				}
				mLastTyping = new_time;

				DoKeySelection();
				return true;
			}
		}
		return LHierarchyTable::HandleKeyDown(nChar, nRepCnt, nFlags);
	}
}

// Handle character
bool CHierarchyTableDrag::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Handle if key selection is active
	if (mKeySelection && (::GetKeyState(VK_CONTROL) >= 0) && (::GetKeyState(VK_MENU) >= 0) && ::isalpha(nChar & 0x000000FF))
		return true;
	else
		return LHierarchyTable::HandleChar(nChar, nRepCnt, nFlags);
}

DROPEFFECT CHierarchyTableDrag::OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Do inherited
	DROPEFFECT result = LHierarchyTable::OnDragEnter(pWnd, pDataObject, dwKeyState, point);

	// Clear expansion array
	mExpandedRows.clear();
	
	return result;
}

void CHierarchyTableDrag::OnDragLeave(CWnd* pWnd)
{
	// Do inherited
	LHierarchyTable::OnDragLeave(pWnd);

	// Collapse all expanded rows in reverse
	for(ulvector::reverse_iterator riter = mExpandedRows.rbegin(); riter != mExpandedRows.rend(); riter++)
		CollapseRow(*riter);
	
	// If anything collapsed, do redraw immediately
	if (mExpandedRows.size())
		RedrawWindow();

}

// Select row after typing
void CHierarchyTableDrag::DoKeySelection()
{
	// Get first selected cell
	STableCell selCell;
	GetFirstSelection(selCell);
	
	// Get its woRow
	TableIndexT	woRow = (selCell.row ? GetWideOpenIndex(selCell.row) : 0);
	
	// Get row to start search
	if (!woRow)
		// Start at the top if none selected
		woRow = 1;
	else if (IsCollapsable(woRow) && IsExpanded(woRow) && HasChildren(woRow))
		// Start at first child if it exists
		woRow++;	
	else if (HasParent(woRow))
		// Go to parent's first child
		woRow = GetParentIndex(woRow) + 1;
	else
		// Start at the top
		woRow = 1;
	
	// Now iterator over all siblings to find the closest match
	TableIndexT selRow = woRow;
	size_t type_len = ::strlen(mLastChars);
	while(woRow)
	{
		selRow = woRow;
		const char* text = GetRowText(woRow);
		
		// Compare upto length of typed text
		int result = (text ? ::strncmpnocase(mLastChars, text, type_len) : 0);
		
		// If less or equal return current
		if (result <= 0)
			break;
		
		// Go to the next sibling
		woRow = static_cast<CNodeVectorTree*>(mCollapsableTree)->GetSiblingIndex(woRow);
	}
	
	// Select the chosen row
	selCell.row = GetExposedIndex(selRow);
	FakeClickSelect(selCell, false);
	ScrollCellIntoFrame(selCell);
}

// Get text for row
const char* CHierarchyTableDrag::GetRowText(UInt32 inWideOpenIndex)
{
	return cdstring::null_str;
}

DROPEFFECT CHierarchyTableDrag::OnDragOver(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	STableCell old_cell = mLastHitCell;

	// Do inherited
	DROPEFFECT result = LHierarchyTable::OnDragOver(pWnd, pDataObject, dwKeyState, point);

	STableCell new_cell = mLastHitCell;
	
	// If not equal reset clock
	if (new_cell.row != old_cell.row)
		mTimeInCell = ::clock();
	else
	{
		// Check time
		if (::clock() - mTimeInCell > 0.5 * CLOCKS_PER_SEC)
		{
			TableIndexT woRow = GetWideOpenIndex(old_cell.row);
			// Check for expandability
			if (CanDropExpand(pDataObject, woRow))
			{
				// Do expand
				DoDropExpand(pDataObject, woRow);
				
				// Cache value for later collapse
				mExpandedRows.push_back(woRow);
			}
		}
	}
	
	return result;
}

void CHierarchyTableDrag::OnDragStartScroll(CWnd* pWnd)
{
	// Just pretend to leave inherited
	LHierarchyTable::OnDragLeave(pWnd);
}

DROPEFFECT CHierarchyTableDrag::OnDragStopScroll(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Just pretend to enter (inherited)
	return LHierarchyTable::OnDragEnter(pWnd, pDataObject, dwKeyState, point);
}

BOOL CHierarchyTableDrag::OnDrop(CWnd* pWnd, COleDataObject* pDataObject, DROPEFFECT dropEffect, CPoint point)
{
	// Must clear selection before drop otherwise cannot drop on target if same table
	LHierarchyTable::OnDragLeave(pWnd);

	return LHierarchyTable::OnDrop(pWnd, pDataObject, dropEffect, point);
}

// Can row expand for drop
bool CHierarchyTableDrag::CanDropExpand(COleDataObject* pDataObject, TableIndexT woRow)
{
	return IsCollapsable(woRow) && !IsExpanded(woRow);
}

// Do row expand for drop
void CHierarchyTableDrag::DoDropExpand(COleDataObject* pDataObject, TableIndexT woRow)
{
	const unsigned long cFlashTickDelay = 5;

	TableIndexT exposed = GetExposedIndex(woRow);

	bool flash_on = (exposed == mLastDropCell.row);

	STableCell cell_on = mLastDropCell;
	STableCell cell_off;

	if (flash_on)
	{
		// Remove hilite
		DrawDropCell(pDataObject, cell_off);
	}

	// Add hilite
	DrawDropCell(pDataObject, cell_on);

	// Remove hilite
	DrawDropCell(pDataObject, cell_off);
	
	// Add hilite
	DrawDropCell(pDataObject, cell_on);

	if (!flash_on)
	{
		// Remove hilite
		DrawDropCell(pDataObject, cell_off);
	}

	// Force expansion and immediate redraw (must unhilite drag then rehilite)
	ExpandRow(woRow);
	RedrawWindow();
}

// Draw drag row frame
void CHierarchyTableDrag::DrawDropCell(COleDataObject* pDataObject, const STableCell& cell)
{
	// Only do if cells are different
	if (mLastDropCell.row == cell.row)
		return;

	// First remove current drag hilite
	if (mLastDropCell.row > 0)
	{
		if (mHasFocus)
		{
			if (!mRowWasSelected)
			{
				UnselectRow(mLastDropCell.row);
				RefreshRow(mLastDropCell.row);
				UpdateWindow();
				if (mLastDropCursor.row == mLastDropCell.row)
					mLastDropCursor.SetCell(0, 0);
			}
		}
		else
		{
			if (!mRowWasSelected)
				UnselectRow(mLastDropCell.row);
			else
				SelectRow(mLastDropCell.row);
			RefreshRow(mLastDropCell.row);
			UpdateWindow();
			if (mLastDropCursor.row == mLastDropCell.row)
				mLastDropCursor.SetCell(0, 0);
		}
	}

	// Then draw new drag hilite
	if (cell.row > 0)
	{
		mRowWasSelected = IsRowSelected(cell.row);
		if (mHasFocus)
			SelectRow(cell.row);
		else
		{
			// Fake focus to get correct selection drawn
			SelectRow(cell.row);
		}
		RefreshRow(cell.row);
		UpdateWindow();
		if (mLastDropCursor.row == mLastDropCell.row)
			mLastDropCursor.SetCell(0, 0);
	}

	// Reset current value
	mLastDropCell = cell;
}
