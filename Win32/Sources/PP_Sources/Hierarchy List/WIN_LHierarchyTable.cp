/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


#include <WIN_LHierarchyTable.h>
#include <WIN_LCollapsableTree.h>

#include "CNodeVectorTree.h"

#include "CDrawUtils.h"
#include "CIconLoader.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"

#include "cdustring.h"

#include <WIN_UTableHelpers.h>

//#include <algobase.h>

const int cHorizIndent = 18;

const int cLineLeft = 7;
const int cLineTop = 7;

IMPLEMENT_DYNCREATE(LHierarchyTable, CTableDragAndDrop)

BEGIN_MESSAGE_MAP(LHierarchyTable, CTableDragAndDrop)
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//		¥ LHierarchyTable
// ---------------------------------------------------------------------------
//	Default constructor

LHierarchyTable::LHierarchyTable()
{
	mCollapsableTree = nil;
	SetRect(&mFlagRect, 2, 2, 18, 14);

	mCollapsableTree = new CNodeVectorTree;

	mHierarchyCol = 1;
}


// ---------------------------------------------------------------------------
//		¥ ~LHierarchyTable
// ---------------------------------------------------------------------------
//	Destructor

LHierarchyTable::~LHierarchyTable()
{
	delete mCollapsableTree;
}

#pragma mark -

// ---------------------------------------------------------------------------
//	¥ GetWideOpenTableSize											  [public]
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
//	¥ SetCollapsableTree											  [public]
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
	
	RedrawWindow(NULL, NULL, RDW_INVALIDATE);								// Redraw entire Table
}

#pragma mark -

// ---------------------------------------------------------------------------
//		¥ InsertRows
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
	Boolean		inRefresh)
{
	InsertSiblingRows(inHowMany, inAfterRow, inDataPtr, inDataSize,
			false, inRefresh);
}


// ---------------------------------------------------------------------------
//		¥ InsertSiblingRows
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
	Boolean		inCollapsable,
	Boolean		inRefresh)
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
//		¥ InsertChildRows
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
	Boolean		inCollapsable,
	Boolean		inRefresh)
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
//		¥ AddLastChildRow
// ---------------------------------------------------------------------------
//	Add one row as the last child of inParentRow
//
//	Returns the wide open index where the row was inserted

UInt32
LHierarchyTable::AddLastChildRow(
	UInt32		inParentRow,			// WideOpen Index
	const void	*inDataPtr,
	UInt32		inDataSize,
	Boolean		inCollapsable,
	Boolean		inRefresh)
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
//		¥ RemoveRows
// ---------------------------------------------------------------------------
//	Delete rows from a HierarchyTable.
//
//	All rows must have the same parent

void
LHierarchyTable::RemoveRows(
	UInt32		inHowMany,
	UInt32		inFromRow,				// WideOpen Index
	Boolean		inRefresh)

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
	Boolean		inRefresh)
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
			RedrawWindow(NULL, NULL, RDW_INVALIDATE);				//   the entire Pane
		}
	}
}

#pragma mark -

// ---------------------------------------------------------------------------
//		¥ CollapseRow
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
//		¥ DeepCollapseRow
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
//		¥ ExpandRow
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
//		¥ DeepExpandRow
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
//		¥ RevealRow
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

#pragma mark -

// ---------------------------------------------------------------------------
//		¥ RefreshRowsDownFrom
// ---------------------------------------------------------------------------
//	Force a redraw at the next update event of all the rows in the Table
//	from the specified row down to the last row

void
LHierarchyTable::RefreshRowsDownFrom(
	UInt32	inWideOpenRow)
{
	Boolean		exposed = true;
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
//		¥ RefreshRowsBelow
// ---------------------------------------------------------------------------
//	Force a redraw at the next update event of all the rows in the Table
//	below (NOT including) the specified row

void
LHierarchyTable::RefreshRowsBelow(
	UInt32	inWideOpenRow)
{
	Boolean		exposed = true;
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
//		¥ ConcealRowsBelow
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
//		¥ RevealRowsBelow
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

#pragma mark -

// ---------------------------------------------------------------------------
//		¥ ClickSelf
// ---------------------------------------------------------------------------
//	Handle a mouse click within a HierarchyTable

// Double-clicked item
void LHierarchyTable::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// Look for click in hierarchy button
	STableCell	hitCell;
	CPoint		imagePt;

	LocalToImagePoint(point, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		// Get rect of hierarchy item
		CRect localRect;
		GetLocalCellRect(hitCell.row, localRect);
		
		// Adjust to hierarchy column
		for(int i = 1; i <= mHierarchyCol; i++)
			localRect.left += GetColWidth(i);
		localRect.right = localRect.left + GetColWidth(mHierarchyCol);

		// Process click in hierarchy column
		if ((hitCell.col == mHierarchyCol) && LClickHierarchyRow(hitCell.row, point, localRect))
			return;
	}

	// Do default processing
	CTableDragAndDrop::OnLButtonDblClk(nFlags, point);
}

// Clicked somewhere
void LHierarchyTable::OnLButtonDown(UINT nFlags, CPoint point)
{
	// Look for click in hierarchy button
	STableCell	hitCell;
	CPoint		imagePt;

	LocalToImagePoint(point, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		// Get rect of hierarchy item
		CRect localRect;
		GetLocalCellRect(hitCell, localRect);
		
		// Process click in hierarchy column
		if ((hitCell.col == mHierarchyCol) && LClickHierarchyRow(hitCell.row, point, localRect))
		{
			// Set focus to table (not done unless we explicitly do it here as processing of
			// click stops at thgis point)
			SetFocus();
			return;
		}
	}

	// Do default processing
	CTableDragAndDrop::OnLButtonDown(nFlags, point);
}

Boolean
LHierarchyTable::LClickHierarchyRow(UInt32 inAtRow, CPoint point, const CRect& inLocalRect)
{
	StDeferSelectionChanged defer(this);	// Must protect against stack unwind
	bool result = false;

									// Click is inside hitCell
									// Check if click is inside DropFlag
	UInt32	woRow = mCollapsableTree->GetWideOpenIndex(inAtRow);
	UInt32 nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Get rect for toggle button
	CRect nested = inLocalRect;
	nested.left = inLocalRect.left + nestingLevel*cHorizIndent;
	
	// Adjust for size of flag
	nested.left += 3;
	nested.top += 3 + mIconOrigin;
	nested.right = nested.left + 9;
	nested.bottom = nested.top + 9;
		
	if (mCollapsableTree->IsCollapsable(woRow) &&
		nested.PtInRect(point))
	{
									// Click is inside DropFlag
		Boolean	expanded = mCollapsableTree->IsExpanded(woRow);
		if ((::GetKeyState(VK_CONTROL) < 0)) {
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
int LHierarchyTable::DrawHierarchyRow(CDC* pDC, UInt32 inAtRow,
											const CRect& inLocalRect,
											const char* title,
											UINT iconID,
											bool clip_center)
{
	HICON hIcon = CIconLoader::GetIcon(pDC, iconID, 16);
	return DrawHierarchyRow(pDC, inAtRow, inLocalRect, title, hIcon, clip_center);
}

// Draw hierarchy element at specific row
int LHierarchyTable::DrawHierarchyRow(CDC* pDC, UInt32 inAtRow,
											const CRect& inLocalRect,
											const char* title,
											HICON hIcon,
											bool clip_center)
{
	STableCell inCell(inAtRow, mHierarchyCol);

	// Save current drawing state
	StDCState save(pDC);

	UInt32 woRow = mCollapsableTree->GetWideOpenIndex(inAtRow + TABLE_ROW_ADJUST);
	UInt32 nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Draw icon
	CRect nested = inLocalRect;
	nested.left = inLocalRect.left + (nestingLevel + 1)*cHorizIndent;
	if (hIcon != NULL)
		CIconLoader::DrawIcon(pDC, nested.left, nested.top + mIconOrigin, hIcon, 16);
		
	// Bump past icon (if it exists) or if hierarchic
	if (hIcon != NULL)
		nested.left += cHorizIndent;

	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = (mHasFocus || mIsDropTarget);

	// Calc string area
	cdustring utf16(title);
	SIZE size;
	::GetTextExtentPoint32W(*pDC, utf16, utf16.length(), &size);

	CRect str_area = nested;
	str_area.right = std::min(str_area.left + size.cx + 1, str_area.right);

	if (selected && mDrawSelection)
	{
		if (active)
		{
			pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
			
		   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
			pDC->ExtTextOut(str_area.left, str_area.top, ETO_OPAQUE, str_area, _T(""), 0, nil);
		}
		else
		{
			CRect box(str_area);
			box.DeflateRect(-1, 0);
			CBrush framer(::GetSysColor(COLOR_HIGHLIGHT));
			pDC->FrameRect(box, &framer);
		}
	}

	int text_start = nested.left;

	::DrawClippedStringUTF8(pDC, title, CPoint(nested.left, nested.top + mTextOrigin),
							nested, eDrawString_Left, clip_center ? eClipString_Center : eClipString_Right);
	pDC->SetBkColor(::GetSysColor(COLOR_WINDOW));
	
	// Set up dotted pen
	CBrush brush(RGB(0x44, 0x44, 0x44));
	LOGBRUSH lb;
	brush.GetLogBrush(&lb);
	CPen temp;
	if (::GetVersion() < 0x80000000)
		temp.CreatePen(PS_COSMETIC | PS_ALTERNATE, 1, &lb);
	else
		temp.CreatePen(PS_SOLID, 0, RGB(0x44, 0x44, 0x44));
	CPen* old_pen = (CPen*) pDC->SelectObject(&temp);

	// Always have horiz line
	nested.left = inLocalRect.left + nestingLevel*cHorizIndent;
	pDC->MoveTo(nested.left + cLineLeft, nested.top + mIconOrigin + cLineTop);
	pDC->LineTo(nested.left + cHorizIndent, nested.top + mIconOrigin + cLineTop);
	
	// If its a child
	if (mCollapsableTree->HasParent(woRow) || (woRow > 1))
	{
		pDC->MoveTo(nested.left + cLineLeft, nested.top - 1);
		pDC->LineTo(nested.left + cLineLeft, nested.top + mIconOrigin + cLineTop);
	}
	
	// If sibling
	if (mCollapsableTree->HasSibling(woRow))
	{
		pDC->MoveTo(nested.left + cLineLeft, nested.top + mIconOrigin + cLineTop);
		pDC->LineTo(nested.left + cLineLeft, nested.bottom);
	}
	
	// If its a parent draw icon
	if (mCollapsableTree->IsCollapsable(woRow))
	{
		CIconLoader::DrawIcon(pDC, nested.left, nested.top + mIconOrigin, mCollapsableTree->IsExpanded(woRow) ? IDI_HIERDOWN : IDI_HIERUP, 16);
	}
	
	// Iterate over nesting levels and look for parents with siblings
	while(nestingLevel--)
	{
		nested.left -= cHorizIndent;
		woRow = mCollapsableTree->GetParentIndex(woRow);
		
		// Look for sibling and draw line
		if (mCollapsableTree->HasSibling(woRow))
		{
			pDC->MoveTo(nested.left + cLineLeft, nested.top - 1);
			pDC->LineTo(nested.left + cLineLeft, nested.bottom);
		}
	}

	// Select old pen before stack pen goes out of scope
	pDC->SelectObject(old_pen);
	
	return text_start;
}
