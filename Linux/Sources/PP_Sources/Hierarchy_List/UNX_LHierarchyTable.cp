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


#include <UNX_LHierarchyTable.h>
#include <UNX_LCollapsableTree.h>

#include "CNodeVectorTree.h"

#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

#include "cdustring.h"

#include "StPenState.h"

#include <UNX_UTableHelpers.h>

#include <JPainter.h>
#include <JXColormap.h>
#include <JXImage.h>

#include <algorithm>
#include "HResourceMap.h"

const int cHorizIndent = 18;

const int cLineLeft = 7;
const int cLineTop = 7;

CIconArray LHierarchyTable::sIcons;

// ---------------------------------------------------------------------------
//		 LHierarchyTable
// ---------------------------------------------------------------------------
//	Default constructor

LHierarchyTable::LHierarchyTable(JXScrollbarSet* scrollbarSet, 
									JXContainer* enclosure,
									const HSizingOption hSizing, 
									const VSizingOption vSizing,
									const JCoordinate x, const JCoordinate y,
									const JCoordinate w, const JCoordinate h) :
	CTableDragAndDrop(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mCollapsableTree = nil;
	//SetRect(&mFlagRect, 2, 2, 18, 14);

	mCollapsableTree = new CNodeVectorTree;

	mHierarchyCol = 1;

  // Load icons
  if (!sIcons.size())
    {
      JXImage* icon = iconFromResource(IDI_TWISTUP, this, 16, 0x00FFFFFF);
      sIcons.push_back(icon);
      icon = iconFromResource(IDI_TWISTDOWN, this, 16, 0x00FFFFFF);
      sIcons.push_back(icon);
    }
}


// ---------------------------------------------------------------------------
//		 ~LHierarchyTable
// ---------------------------------------------------------------------------
//	Destructor

LHierarchyTable::~LHierarchyTable()
{
	delete mCollapsableTree;
}

//#pragma mark -

// ---------------------------------------------------------------------------
//	 GetWideOpenTableSize											  [public]
// ---------------------------------------------------------------------------
//	Pass back the number of rows and columns of the wide open Table
//	(Table with all rows fully expanded).

void
LHierarchyTable::GetWideOpenTableSize(
	UInt32	&outRows) const
{
	outRows = mCollapsableTree->CountNodes();
}


// ---------------------------------------------------------------------------
//	 SetCollapsableTree											  [public]
// ---------------------------------------------------------------------------
//	Set a new CollapsableTree for a HierarchyTable
//
//	Use to restore the state of a HierarchyTable. Make sure to set the
//	TableStorage BEFORE calling this function, and be sure that the Tree
//	has the same number of Nodes as rows of data in the Storage.

void
LHierarchyTable::SetCollapsableTree(
	LCollapsableTree	*inTree)
{
	delete mCollapsableTree;
	
	mCollapsableTree = inTree;
	
	UnselectAllCells();						// Clear selection
	
	mTableGeometry->RemoveRows(mRows, 1);	// Adjust Geometry to match
	mRows = inTree->CountExposedNodes();	//   exposed nodes of Tree
	mTableGeometry->InsertRows(mRows, 0);
	
	Refresh();								// Redraw entire Table
}

//#pragma mark -

// ---------------------------------------------------------------------------
//		 InsertRows
// ---------------------------------------------------------------------------
//	Add rows to a HierarchyTable.
//
//	With a hierarchy, new rows can be inserted as either the siblings
//	or the children of the "after" row. The InsertSiblingRows() and
//	InsertChildRows() funnctions handle these two cases. However, this
//	function, inherited from LTableView, must be overridden to choose
//	between these two alternatives.
//
//	For simplicity, we choose to implement this function as the
//	equivalent of InsertSiblingRows() with the new rows not being
//	collapsable.

void
LHierarchyTable::InsertRows(
	UInt32		inHowMany,
	UInt32		inAfterRow,				// WideOpen Index
	const void	*inDataPtr,
	UInt32		inDataSize,
	bool		inRefresh)
{
	InsertSiblingRows(inHowMany, inAfterRow, inDataPtr, inDataSize,
			false, inRefresh);
}


// ---------------------------------------------------------------------------
//		 InsertSiblingRows
// ---------------------------------------------------------------------------
//	Add rows to a HierarchyTable.
//
//	Rows are added within the same parent as inAfterRow (as siblings of
//	inAfterRow).
//
//	Returns the wide open index where the first row was inserted

UInt32
LHierarchyTable::InsertSiblingRows(
	UInt32		inHowMany,
	UInt32		inAfterRow,				// WideOpen Index
	const void	*inDataPtr,
	UInt32		inDataSize,
	bool		inCollapsable,
	bool		inRefresh)
{
	UInt32		lastWideOpen = mCollapsableTree->CountNodes();
	if (inAfterRow > lastWideOpen) {	// If inAfterIndex is too big,
		inAfterRow = lastWideOpen;		//   insert after last row
	}

		// Inserts Rows into CollapsableTree and store the data
		// for the new Rows

	UInt32		insertAtRow = mCollapsableTree->InsertSiblingNodes(inHowMany,
								inAfterRow, inCollapsable);
	
	if (mTableStorage != nil)
	{
		mTableStorage->InsertRows(inHowMany, insertAtRow - 1,
							inDataPtr, inDataSize);
	}
	
		// If inserted rows are exposed, we need to adjust the
		// visual display, which involves the Table Geometry,
		// Selection, and Image size.
	
	TableIndexT	exInsert = mCollapsableTree->GetExposedIndex(insertAtRow);
	if (exInsert > 0) {
		mRows += inHowMany;
		mTableGeometry->InsertRows(inHowMany, exInsert - 1);

		if (mTableSelector != nil) {
			mTableSelector->InsertRows(inHowMany, exInsert - 1);
		}

		AdjustImageSize(false);

		if (inRefresh) {
			RefreshRowsDownFrom(insertAtRow);
		}
	}
	
	return insertAtRow;
}


// ---------------------------------------------------------------------------
//		 InsertChildRows
// ---------------------------------------------------------------------------
//	Add rows to a HierarchyTable.
//
//	Rows are added as the first children of inParentRow
//
//	Returns the wide open index where the first row was inserted (which will
//	always be (inParentRow + 1) if insertion succeeds).

UInt32
LHierarchyTable::InsertChildRows(
	UInt32		inHowMany,
	UInt32		inParentRow,			// WideOpen Index
	const void	*inDataPtr,
	UInt32		inDataSize,
	bool		inCollapsable,
	bool		inRefresh)
{
		// If Row is not collapsable, adding children will make it
		// collapsable. If so, we'll need to refresh the Parent
		// row in addition to all the rows below it
		
	UInt32	refreshBelowRow = inParentRow + 1;
	if (!mCollapsableTree->IsCollapsable(inParentRow)) {
		refreshBelowRow -= 1;
	}

		// Inserts Rows into CollapsableTree and store the data
		// for the new Rows

	mCollapsableTree->InsertChildNodes(inHowMany, inParentRow, inCollapsable);
	
	if (mTableStorage != nil) {
		mTableStorage->InsertRows(inHowMany, inParentRow,
									inDataPtr, inDataSize);
	}
	
		// If inserted rows are exposed, we need to adjust the
		// visual display, which involves the Table Geometry,
		// Selection, and Image size.
	
	TableIndexT	exParent = mCollapsableTree->GetExposedIndex(inParentRow);
	if ((exParent > 0) && mCollapsableTree->IsExpanded(inParentRow)) {
		mRows += inHowMany;
		mTableGeometry->InsertRows(inHowMany, exParent);

		if (mTableSelector != nil) {
			mTableSelector->InsertRows(inHowMany, exParent);
		}

		AdjustImageSize(false);

		if (inRefresh) {
			RefreshRowsDownFrom(refreshBelowRow);
		}
	}
	
	return (inParentRow + 1);
}


// ---------------------------------------------------------------------------
//		 AddLastChildRow
// ---------------------------------------------------------------------------
//	Add one row as the last child of inParentRow
//
//	Returns the wide open index where the row was inserted

UInt32
LHierarchyTable::AddLastChildRow(
	UInt32		inParentRow,			// WideOpen Index
	const void	*inDataPtr,
	UInt32		inDataSize,
	bool		inCollapsable,
	bool		inRefresh)
{
		// If Row is not collapsable, adding children will make it
		// collapsable. If so, we'll need to refresh the Parent
		// row in addition to all the rows below it.
		
	UInt32	refreshAdjustment = 0;
	if (!mCollapsableTree->IsCollapsable(inParentRow)) {
		refreshAdjustment = -1;			// Parent was not collapsable. It will
	}									//   always be one row above the
										//   inserted row since it does not
										//   have any other children.

		// Insert Row into CollapsableTree and store its data

	UInt32	woInsertAt = mCollapsableTree->AddLastChildNode(inParentRow,
															inCollapsable);
	UInt32	woInsertAfter = woInsertAt - 1;
	
	if (mTableStorage != nil) {
		mTableStorage->InsertRows(1, woInsertAfter, inDataPtr, inDataSize);
	}
	
		// If inserted row is exposed, we need to adjust the
		// visual display, which involves the Table Geometry,
		// Selection, and Image size.
	
	TableIndexT	exInsertAt = mCollapsableTree->GetExposedIndex(woInsertAt);
	if (exInsertAt > 0) {
		mRows += 1;
		mTableGeometry->InsertRows(1, exInsertAt - 1);

		if (mTableSelector != nil) {
			mTableSelector->InsertRows(1, exInsertAt - 1);
		}

		AdjustImageSize(false);

		if (inRefresh) {
			RefreshRowsDownFrom(woInsertAt - refreshAdjustment);
		}
	}
	
	return woInsertAt;
}


// ---------------------------------------------------------------------------
//		 RemoveRows
// ---------------------------------------------------------------------------
//	Delete rows from a HierarchyTable.
//
//	All rows must have the same parent

void
LHierarchyTable::RemoveRows(
	UInt32		inHowMany,
	UInt32		inFromRow,				// WideOpen Index
	bool		inRefresh)

{
	if (inHowMany != 1) {
		//SignalPStr_("\pCan only remove one row at a time from a HierarchyTable");
		return;
	}
	
	TableIndexT	exposedRow = mCollapsableTree->GetExposedIndex(inFromRow);
	
		// Remove Row from CollapsableTree and delete the data
		// for the removed rows
	
	UInt32	totalRemoved, exposedRemoved;
	mCollapsableTree->RemoveNode(inFromRow, totalRemoved, exposedRemoved);
		
	if (mTableStorage != nil) {
		mTableStorage->RemoveRows(totalRemoved, inFromRow);
	}
	
		// Adjust the visual display of the Table if we removed
		// any exposed rows
	
	if (exposedRemoved > 0) {
		if (inRefresh) {				// Redraw cells below removed rows
			STableCell	topLeftCell(exposedRow, 1);
			STableCell	botRightCell(mRows, mCols);

			RefreshCellRange(topLeftCell, botRightCell);
		}

		mRows -= exposedRemoved;
		mTableGeometry->RemoveRows(exposedRemoved, exposedRow);

		if (mTableSelector != nil) {
			mTableSelector->RemoveRows(exposedRemoved, exposedRow);
		}

		AdjustImageSize(false);
	}
}


void
LHierarchyTable::RemoveAllRows(
	bool		inRefresh)
{
	if (mRows > 0) {
	
			// Set internal row count to zero. The Geometry, Storage,
			// or Selector may query the table while removing entries
			// and they expect the row count to be already adjusted.

		TableIndexT		revealedRows = mRows;
		mRows = 0;
		
			// The Geometry and Selector deal with revealed rows,
			// so we remove all revealed rows from them. Storage
			// has data for all cells, even those that are collapsed,
			// so we remove data for the number of "wide open" rows.
		
		mTableGeometry->RemoveRows(revealedRows, 1);
		
		if (mTableStorage != nil) {
			mTableStorage->RemoveRows(mCollapsableTree->CountNodes(), 1);
		}
		
		if (mTableSelector != nil) {
			mTableSelector->RemoveRows(revealedRows, 1);
		}

		delete mCollapsableTree;	// Make an empty tree
		mCollapsableTree = new CNodeVectorTree;
		
		if (inRefresh) {			// All rows removed, so refresh
			Refresh();				//   the entire Pane
		}
	}
}

//#pragma mark -

// ---------------------------------------------------------------------------
//		 CollapseRow
// ---------------------------------------------------------------------------
//	Collapse the specified Row, visually hiding its child rows

void
LHierarchyTable::CollapseRow(
	UInt32	inWideOpenRow)
{
	if (!mCollapsableTree->IsCollapsable(inWideOpenRow)) {
		//SignalPStr_("\pCan't collapse an uncollapsable row");
		return;
	}
								// Collapse within Hierarchy storage
	UInt32	rowsConcealed = mCollapsableTree->CollapseNode(inWideOpenRow);
	
	if (rowsConcealed > 0) {
		ConcealRowsBelow(rowsConcealed, inWideOpenRow);
	}
}


// ---------------------------------------------------------------------------
//		 DeepCollapseRow
// ---------------------------------------------------------------------------
//	Collapse the specified Row and all its child rows

void
LHierarchyTable::DeepCollapseRow(
	UInt32	inWideOpenRow)
{
	if (!mCollapsableTree->IsCollapsable(inWideOpenRow)) {
		//SignalPStr_("\pCan't collapse an uncollapsable row");
		return;
	}
								// Collapse within Hierarchy storage
	UInt32	rowsConcealed = mCollapsableTree->DeepCollapseNode(inWideOpenRow);
	
	if (rowsConcealed > 0) {
		ConcealRowsBelow(rowsConcealed, inWideOpenRow);
	}
}


// ---------------------------------------------------------------------------
//		 ExpandRow
// ---------------------------------------------------------------------------
//	Expand the specified Row, visually revealing its child rows

void
LHierarchyTable::ExpandRow(
	UInt32	inWideOpenRow)
{
	if (!mCollapsableTree->IsCollapsable(inWideOpenRow)) {
		//SignalPStr_("\pCan't expand an uncollapsable row");
		return;
	}
	
	UInt32	rowsRevealed = mCollapsableTree->ExpandNode(inWideOpenRow);
	
	if (rowsRevealed > 0) {
		RevealRowsBelow(rowsRevealed, inWideOpenRow);
	}
}


// ---------------------------------------------------------------------------
//		 DeepExpandRow
// ---------------------------------------------------------------------------
//	Expand the specified Row and all its child rows

void
LHierarchyTable::DeepExpandRow(
	UInt32	inWideOpenRow)
{
	if (!mCollapsableTree->IsCollapsable(inWideOpenRow)) {
		//SignalPStr_("\pCan't expand an uncollapsable row");
		return;
	}
	
	UInt32	rowsRevealed = mCollapsableTree->DeepExpandNode(inWideOpenRow);
	
	if (rowsRevealed > 0) {
		RevealRowsBelow(rowsRevealed, inWideOpenRow);
	}
}


// ---------------------------------------------------------------------------
//		 RevealRow
// ---------------------------------------------------------------------------
//	Make the specified row visible within the Table by expanding all
//	of its parent rows

void
LHierarchyTable::RevealRow(
	UInt32	inWideOpenRow)
{
	if (mCollapsableTree->GetExposedIndex(inWideOpenRow) == 0) {
											// Row is not currently exposed
											// Expand all its ancestors
		TableIndexT	woIndex = inWideOpenRow;
		TableIndexT	lastExpanded;
		while (true) {
			woIndex = mCollapsableTree->GetParentIndex(woIndex);

			if (woIndex > 0) {
				ExpandRow(woIndex);
				lastExpanded = woIndex;
			} else {
				break;
			}
		}

		RefreshRowsDownFrom(lastExpanded);
	}
}

//#pragma mark -

// ---------------------------------------------------------------------------
//		 RefreshRowsDownFrom
// ---------------------------------------------------------------------------
//	Force a redraw at the next update event of all the rows in the Table
//	from the specified row down to the last row

void
LHierarchyTable::RefreshRowsDownFrom(
	UInt32	inWideOpenRow)
{
	bool		exposed = true;
	UInt32		exRow = 0;
	
	if (inWideOpenRow > 0) {
		exRow = mCollapsableTree->GetExposedIndex(inWideOpenRow);
		exposed = (exRow > 0);
	}
	
	if (exposed) {
		STableCell	topLeftCell(exRow, 1);
		STableCell	botRightCell(mRows, mCols);
		RefreshCellRange(topLeftCell, botRightCell);
	}
}


// ---------------------------------------------------------------------------
//		 RefreshRowsBelow
// ---------------------------------------------------------------------------
//	Force a redraw at the next update event of all the rows in the Table
//	below (NOT including) the specified row

void
LHierarchyTable::RefreshRowsBelow(
	UInt32	inWideOpenRow)
{
	bool		exposed = true;
	UInt32	exRow = 0;
	
	if (inWideOpenRow > 0) {
		exRow = mCollapsableTree->GetExposedIndex(inWideOpenRow);
		exposed = (exRow > 0);
	}
	
	if (exposed) {
		STableCell	topLeftCell(exRow + 1, 1);
		STableCell	botRightCell(mRows, mCols);
		RefreshCellRange(topLeftCell, botRightCell);
	}
}


// ---------------------------------------------------------------------------
//		 ConcealRowsBelow
// ---------------------------------------------------------------------------
//	Conceal rows below the specified row
//
//	Concealing visually removes the rows from the Table, but does not
//	remove the data for the Cells in the rows from mTableStorage

void
LHierarchyTable::ConcealRowsBelow(
	UInt32		inHowMany,
	UInt32	inWideOpenRow)
{
	RefreshRowsBelow(inWideOpenRow);

	mRows -= inHowMany;

	TableIndexT	exRow = mCollapsableTree->GetExposedIndex(inWideOpenRow);
	mTableGeometry->RemoveRows(inHowMany, exRow + 1);

	if (mTableSelector != nil) {
		mTableSelector->RemoveRows(inHowMany, exRow + 1);
	}

	AdjustImageSize(false);
}


// ---------------------------------------------------------------------------
//		 RevealRowsBelow
// ---------------------------------------------------------------------------
//	Reveal rows below the specified row
//
//	Revealing visually adds the rows to the Table, but the data for
//	the Cells in the rows is already in mTableStorage

void
LHierarchyTable::RevealRowsBelow(
	UInt32		inHowMany,
	UInt32	inWideOpenRow)
{
	mRows += inHowMany;
	TableIndexT	exRow = mCollapsableTree->GetExposedIndex(inWideOpenRow);

	mTableGeometry->InsertRows(inHowMany, exRow);

	if (mTableSelector != nil) {
		mTableSelector->InsertRows(inHowMany, exRow);
	}

	AdjustImageSize(false);

	RefreshRowsBelow(inWideOpenRow);
}

//#pragma mark -

void LHierarchyTable::HandleMouseDown(const JPoint& pt, 
				      const JXMouseButton button,
				      const JSize clickCount, 
				      const JXButtonStates& buttonStates,
				      const JXKeyModifiers& modifiers)
{
	JPoint		imagePt = pt;
	JPoint		localPt;
	ImageToLocalPoint(imagePt, localPt);

	// We handle a click in their hierarchy column no matter how many 
	// clicks it is.  Otherwise we call the single or double click in cell
	if (button == kJXButton1)
	{
		STableCell	hitCell;

		// if a single click or double click in the hierarchy column
		if (GetCellHitBy(imagePt, hitCell) && (hitCell.col == mHierarchyCol))
		{
			JRect localRect;
			GetLocalCellRect(hitCell, localRect);
			JPoint ptG = JXContainer::LocalToGlobal(pt);
			if (LClickHierarchyRow(hitCell.row, localPt, localRect, modifiers))
			{
				// Readjust back to local co-ords which may have changed due to collapse/expand
				const_cast<JPoint&>(pt) = JXContainer::GlobalToLocal(ptG);
				startPoint = pt;
				return;
			}
		}

		// It's a left click
		startPoint = pt;
	}

	CTableDragAndDrop::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

bool
LHierarchyTable::LClickHierarchyRow(UInt32 inAtRow, JPoint point, const JRect& inLocalRect)
{
	JXKeyModifiers nomods(GetDisplay());
	return LClickHierarchyRow(inAtRow, point, inLocalRect, nomods);
}

bool
LHierarchyTable::LClickHierarchyRow(UInt32 inAtRow, JPoint point, const JRect& inLocalRect, const JXKeyModifiers& modifiers)
{
	StDeferSelectionChanged defer(this);	// Must protect against stack unwind
	bool result = false;

									// Click is inside hitCell
									// Check if click is inside DropFlag
	UInt32	woRow = mCollapsableTree->GetWideOpenIndex(inAtRow);
	UInt32 nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Get rect for toggle button
	JRect nested = inLocalRect;
	
	// Adjust for size of flag
	nested.left += 3;
	nested.top += 3 + mIconOrigin;
	nested.right = nested.left + 9;
	nested.bottom = nested.top + 9;
		
	if (mCollapsableTree->IsCollapsable(woRow) &&
		nested.Contains(point))
	{
									// Click is inside DropFlag
		bool	expanded = mCollapsableTree->IsExpanded(woRow);
		if (modifiers.control()) {
								// ControlKey down means to do
								//   a deep collapse/expand						
			if (expanded) {
				DeepCollapseRow(woRow);
			} else {
				DeepExpandRow(woRow);
			}
		
		} else {				// Shallow collapse/expand
			if (expanded) {
				CollapseRow(woRow);
			} else {
				ExpandRow(woRow);
			}
		}

		RefreshRow(inAtRow);

		result = true;
	}
	
	return result;
}

// Draw hierarchy element at specific row
int LHierarchyTable::DrawHierarchyRow(JPainter* pDC, UInt32 inAtRow,
											const JRect& inLocalRect,
											const char* title,
										    JXImage* icon,
											bool clip_center)
{
	STableCell inCell(inAtRow, mHierarchyCol);

	// Save current drawing state
	StPenState saveDC(pDC);

	UInt32 woRow = mCollapsableTree->GetWideOpenIndex(inAtRow + TABLE_ROW_ADJUST);
	UInt32 nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Draw icon
	JRect nested = inLocalRect;
	nested.left = inLocalRect.left + (nestingLevel + 1)*cHorizIndent;
	if (icon)
		pDC->Image(*icon, icon->GetBounds(), nested.left, nested.top + mIconOrigin);
		
	// Bump past icon (if it exists) or if hierarchic
	if (icon != NULL)
		nested.left += cHorizIndent;

	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = (HasFocus() || mIsDropTarget);

	// Calc string area
	cdustring utf16(title);
	unsigned long size = pDC->GetStringWidth(title);

	JRect str_area = nested;
	str_area.right = std::min(str_area.left + size + 1, (unsigned long) str_area.right);

	if (selected && mDrawSelection)
	{
		if (active)
		{
			StPenState save(pDC);
			pDC->SetPenColor(GetColormap()->GetDefaultSelectionColor());
			pDC->SetFilling(kTrue);
			pDC->RectInside(str_area);
		}
		else
		{
			StPenState save(pDC);
			pDC->SetPenColor(GetColormap()->GetDefaultSelectionColor());
			pDC->RectInside(str_area);
		}
	}

	int text_start = nested.left;

	::DrawClippedStringUTF8(pDC, title, JPoint(nested.left, nested.top + mTextOrigin),
							nested, eDrawString_Left, clip_center ? eClipString_Center : eClipString_Right);
	
	// If its a parent draw icon
	if (mCollapsableTree->IsCollapsable(woRow))
	{
		nested.left = inLocalRect.left;
		pDC->Image(*sIcons[mCollapsableTree->IsExpanded(woRow) ? eHierDown : eHierUp], sIcons[mCollapsableTree->IsExpanded(woRow) ? eHierDown : eHierUp]->GetBounds(), nested.left, nested.top + mIconOrigin);
	}
	
	return text_start;
}
