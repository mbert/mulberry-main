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

#include <JTableSelection.h>
#include <jASCIIConstants.h>

#include <sys/time.h>

CHierarchyTableDrag::CHierarchyTableDrag(JXScrollbarSet* scrollbarSet, 
											JXContainer* enclosure,
											const HSizingOption hSizing, 
											const VSizingOption vSizing,
											const JCoordinate x, const JCoordinate y,
											const JCoordinate w, const JCoordinate h)
	: LHierarchyTable(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	// Use clear node tree
	mPreviousDropRow = 0;
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

bool CHierarchyTableDrag::HasSibling(UInt32 inWideOpenIndex)
{
	return static_cast<CNodeVectorTree*>(mCollapsableTree)->HasSibling(inWideOpenIndex);
}

bool CHierarchyTableDrag::HasParent(UInt32 inWideOpenIndex)
{
	return static_cast<CNodeVectorTree*>(mCollapsableTree)->HasParent(inWideOpenIndex);
}

bool CHierarchyTableDrag::HasChildren(UInt32 inWideOpenIndex)
{
	return static_cast<CNodeVectorTree*>(mCollapsableTree)->HasChildren(inWideOpenIndex);
}

// Handle key down
bool CHierarchyTableDrag::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch(key)
	{
	case kJLeftArrow:
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
					if (modifiers.control())
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

				// Only if parent exists, otherwise remove selection
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
	case kJRightArrow:
	{
		STableCell aCell(0, 0);
		while(GetNextSelectedCell(aCell))
		{
			if (aCell.col == 1)
			{
				TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(aCell.row);
				if (IsCollapsable(woRow) && !IsExpanded(woRow))
				{
					if (modifiers.control())
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
		if (mKeySelection && !modifiers.control() && !modifiers.meta())
		{
			if ((key < 256) && isalpha(key))
			{
				// See if key hit within standard double click time
				timeval tv;
				::gettimeofday(&tv, NULL);
				unsigned long new_time = (tv.tv_sec & 0x003FFFFF) * 1000 + tv.tv_usec / 1000;
				if ((new_time - mLastTyping) / 1000 >= 1)
				{

					// Outside double-click time so new selection
					mLastChars[0] = key;
					mLastChars[1] = 0;
				}
				else
				{
					// Inside double-click time so add char to selection
					short pos = ::strlen(mLastChars);
					if (pos < 31)
					{
						mLastChars[pos++] = key;
						mLastChars[pos] = 0;
					}
				}
				mLastTyping = new_time;

				DoKeySelection();
				return true;
			}
		}
		
		return LHierarchyTable::HandleChar(key, modifiers);
	}
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

void CHierarchyTableDrag::HandleDNDEnter()
{
	// Do inherited
	LHierarchyTable::HandleDNDEnter();
	
	// Clear expansion array
	mExpandedRows.clear();
}


void CHierarchyTableDrag::HandleDNDLeave()
{
	
	mPreviousDropRow = 0;
	// Do inherited
	LHierarchyTable::HandleDNDLeave();
	
	// Collapse all expanded rows in reverse
	for(ulvector::reverse_iterator riter = mExpandedRows.rbegin(); riter != mExpandedRows.rend(); riter++)
		CollapseRow(*riter);
	
	// If anything collapsed, do redraw immediately
	if (mExpandedRows.size())
		Redraw();

}

void CHierarchyTableDrag::HandleDNDHere(const JPoint& pt, const JXWidget* source)
{
	STableCell old_cell = mLastHitCell;

	// Do inherited
	LHierarchyTable::HandleDNDHere(pt, source);

	STableCell new_cell = mLastHitCell;
	
	// If not equal reset clock
	if (new_cell.row != old_cell.row)
		mTimeInCell = ::time(NULL);
	else
	{
		// Check time
		if (::difftime(::time(NULL), mTimeInCell) > 1.0)
		{
			TableIndexT woRow = GetWideOpenIndex(old_cell.row);
			// Check for expandability
			if (CanDropExpand(mCurrentDropFlavors, woRow))
			{
				// Do expand
				DoDropExpand(woRow);
				
				// Cache value for later collapse
				mExpandedRows.push_back(woRow);
			}
		}
	}
}

void CHierarchyTableDrag::ClearDropCell(const STableCell& cell)
{
  // Nothing to do here
  // First remove current drag hilite
  if (mLastDropCell.row > 0)
    {
      if (HasFocus())
	{
	  if (!mRowWasSelected)
	    {
	      UnselectRow(mLastDropCell.row);
	    }
	}
      else
	{
	  if (!mRowWasSelected)
	    UnselectRow(mLastDropCell.row);
	  else
	    SelectRow(mLastDropCell.row);
	}
    }
}

// Can row expand for drop
bool CHierarchyTableDrag::CanDropExpand(const JArray<Atom>& typeList,int woRow)
{
	return IsCollapsable(woRow) && !IsExpanded(woRow);
}

// Do row expand for drop
void CHierarchyTableDrag::DoDropExpand(int woRow)
{
	const unsigned long cFlashTickDelay = 5;

	TableIndexT exposed = GetExposedIndex(woRow);

	bool flash_on = (exposed == mLastDropCell.row);

	STableCell cell_on = mLastDropCell;
	STableCell cell_off;

	if (flash_on)
	{
		// Remove hilite
		DrawDropCell(cell_off);
	}

	// Add hilite
	DrawDropCell(cell_on);

	// Remove hilite
	DrawDropCell(cell_off);
	
	// Add hilite
	DrawDropCell(cell_on);

	if (!flash_on)
	{
		// Remove hilite
		DrawDropCell(cell_off);
	}

	// Force expansion and immediate redraw (must unhilite drag then rehilite)
	ExpandRow(woRow);
	Redraw();
}

void CHierarchyTableDrag::DrawDropCell(const STableCell& cell)
{
	// Only do if cells are different
	if (mLastDropCell.row == cell.row)
		return;

	// First remove current drag hilite
	if (mLastDropCell.row > 0)
	{
		if (HasFocus())
		{
			if (!mRowWasSelected)
			{
				UnselectRow(mLastDropCell.row);
				RefreshRow(mLastDropCell.row);
				Redraw();
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
			Redraw();
			if (mLastDropCursor.row == mLastDropCell.row)
				mLastDropCursor.SetCell(0, 0);
		}
	}

	// Then draw new drag hilite
	if (cell.row > 0)
	{
		mRowWasSelected = IsRowSelected(cell.row);
		if (HasFocus())
			SelectRow(cell.row);
		else
		{
			// Fake focus to get correct selection drawn
			SelectRow(cell.row);
		}
		RefreshRow(cell.row);
		Redraw();
		if (mLastDropCursor.row == mLastDropCell.row)
			mLastDropCursor.SetCell(0, 0);
	}

	// Reset current value
	mLastDropCell = cell;
}
