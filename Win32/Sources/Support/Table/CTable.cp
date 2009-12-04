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


// Source for CTable class


#include "CTable.h"

#include "CGrayBackground.h"
#include "CDrawUtils.h"
#include "CFontCache.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"

#include "StValueChanger.h"

#include <WIN_UTableHelpers.h>

/////////////////////////////////////////////////////////////////////////////
// CTable

IMPLEMENT_DYNCREATE(CTable, LTableView)

BEGIN_MESSAGE_MAP(CTable, LTableView)
	ON_UPDATE_COMMAND_UI(ID_EDIT_SELECT_ALL, OnUpdateSelectAll)
	ON_COMMAND(ID_EDIT_SELECT_ALL, OnEditSelectAll)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)

	ON_WM_KEYDOWN()
	ON_WM_CHAR()
	ON_WM_CONTEXTMENU()
	
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTable::CTable()
{
	mContextMenuID = 0;
	mContextView = NULL;
	mTextHeight = 16;
	mTextOrigin = 1;
	mIconOrigin = 0;
	mHasFocus = false;
	mChanging = 0;
	mRowSelect = false;
	mColSelect = false;
	mDrawSelection = true;
	mSelectColumn = 0;
	mHiliteCell = STableCell(0, 0);
	mDeferSelectionChanged = 0;
	mSelectionChanged = false;
	mIsDropTarget = false;

	mTooltipCell.SetCell(0, 0);
}

// Default destructor
CTable::~CTable()
{
	Broadcast_Message(eBroadcast_TableDeleted, this);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Handle keyboard equiv for context menu
BOOL CTable::PreTranslateMessage(MSG* pMsg)
{
	// Shift+F10: show pop-up menu.
	if (((((pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN) &&
		(pMsg->wParam == VK_F10) && (GetKeyState(VK_SHIFT) & ~1)) != 0) ||
		(pMsg->message == WM_CONTEXTMENU)) &&
		mContextMenuID)
	{
		// Calculate hit point
		CRect rect;
		GetClientRect(rect);
		ClientToScreen(rect);
		CPoint point = rect.TopLeft();
		point.Offset(5, 5);
			
		// Force its view to be active and focus
		//mContextView->GetParentFrame()->SetActiveView(mContextView);
		SetFocus();

		// Do context menu
		OnContextMenu(NULL, point);
		return TRUE;
	}

	return LTableView::PreTranslateMessage(pMsg);
}

// Display and track context menu
void CTable::OnContextMenu(CWnd* wnd, CPoint point)
{
	if (mContextMenuID)
		HandleContextMenu(wnd, point);
}

// Display and track context menu
void CTable::HandleContextMenu(CWnd*, CPoint point)
{
	// Must update the selection state before doing the popup
	// The selection updating has been deferred at this point
	DoSelectionChanged();

	CMenu menu;
	VERIFY(menu.LoadMenu(mContextMenuID));
	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT(pPopup != NULL);
	CWnd* pWndPopupOwner = this;
	while (pWndPopupOwner->GetStyle() & WS_CHILD)
		pWndPopupOwner = pWndPopupOwner->GetParent();
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, pWndPopupOwner);
}

int CTable::GetSortBy() const
{
	return -1;
}

// Get text for current tooltip cell
void CTable::GetTooltipText(cdstring& txt)
{
	// Only if valid cell
	if (mTooltipCell.IsNullCell())
		return;

	// Get using current cell
	GetTooltipText(txt, mTooltipCell);
}

// Get text for current tooltip cell
void CTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	// Sub-classes must override
}

// Draw whole row
void CTable::HiliteRow(TableIndexT row, bool inHilite)
{
	STableCell hiliteCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		hiliteCell.col = col;
		HiliteCell(hiliteCell, inHilite);
	}
}

// Draw whole col
void CTable::HiliteColumn(TableIndexT col, bool inHilite)
{
	STableCell hiliteCell(0, col);

	for(TableIndexT row = 1; row <= mCols; row++) {
		hiliteCell.row = row;
		HiliteCell(hiliteCell, inHilite);
	}
}

void CTable::GetLocalCellRectAlways(const STableCell& inCell, CRect& outCellRect) const
{
	CRect imageRect;
	mTableGeometry->GetImageCellBounds(inCell, imageRect);
	ImageToLocalRect(imageRect, outCellRect);
}

// Get rect of row
void CTable::GetLocalRowRect(TableIndexT row, CRect& theRect)
{
	STableCell aCell(row, 1);

	// Get rect for row (scan columns for visible rows)
	while(!GetLocalCellRect(aCell, theRect) && (aCell.col <= mCols))
		aCell.col++;
	CRect endRect;
	aCell.col = mCols;
	while(!GetLocalCellRect(aCell, endRect) && (aCell.col >= 1))
		aCell.col--;
	theRect.right = endRect.right;
}

// Get rect of column
void CTable::GetLocalColRect(TableIndexT col, CRect& theRect)
{
	STableCell aCell(1, col);

	// Get rect for row
	GetLocalCellRect(aCell, theRect);
	CRect endRect;
	aCell.row = mRows;
	GetLocalCellRect(aCell, endRect);
	theRect.bottom = endRect.bottom;
}

// Refresh selection
void CTable::RefreshSelection()
{
	// Refresh every cell in selection
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
		RefreshCell(aCell);
}

// Refresh row
void CTable::RefreshRow(TableIndexT row)
{
	STableCell aCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++)
	{
		aCell.col = col;
		RefreshCell(aCell);
	}
}

// Refresh column
void CTable::RefreshCol(TableIndexT col)
{
	STableCell aCell(0, col);

	for(TableIndexT row = 1; row <= mRows; row++)
	{
		aCell.row = row;
		RefreshCell(aCell);
	}
}

// Delete all selected rows
void CTable::DeleteSelection()
{
	DoToSelection((DoToSelectionPP) &CTable::DeleteRow, false);
}

// DeleteRow
bool CTable::DeleteRow(TableIndexT row)
{
	RemoveRows(1, row, false);
	return true;
}

// Scroll cell into view
void CTable::ScrollCellIntoFrame(const STableCell &scrollCell, bool middle)
{
	STableCell  theCell(scrollCell);

	// Adjust for selection mechanism
	if (mRowSelect)
		theCell.col = 1;
	else if (mColSelect)
		theCell.row = 1;

	if (middle)
	{
		CRect frame;
		GetClientRect(frame);

		// Adjustment is half frame height
		unsigned long adjust_halfway = frame.Height() / 2;

		// This is code copied direct from LTableView
		// Its been modified so that the scrolled to cell is also placed as
		// close to the center of the visible cells as possible

		SInt32	cellLeft, cellTop, cellRight, cellBottom;
		mTableGeometry->GetImageCellBounds(theCell, cellLeft, cellTop,
								cellRight, cellBottom);

		if (ImagePointIsInFrame(cellLeft, cellTop) &&
			ImagePointIsInFrame(cellRight, cellBottom)) {

			return;						// Entire Cell is already within Frame
		}

		CPoint	frameTopLeft, frameBotRight;
		LocalToImagePoint(frame.TopLeft(), frameTopLeft);
		LocalToImagePoint(frame.BottomRight(), frameBotRight);

		SInt32	horizScroll = 0;		// Find horizontal scroll amount
		if (cellRight > frameBotRight.x) {
											// Cell is to the right of frame
			horizScroll = cellRight - frameBotRight.x;
		}

		if ((cellLeft - horizScroll) < frameTopLeft.x) {
											// Cell is to the left of frame
											//   or Cell is wider than frame
			horizScroll = cellLeft - frameTopLeft.x;
		}

		SInt32	vertScroll = 0;			// Find vertical scroll amount
		if (cellBottom > frameBotRight.y) {
											// Cell is below bottom of frame
			vertScroll = (cellBottom + adjust_halfway) - frameBotRight.y;
		}

		if ((cellTop - vertScroll) < frameTopLeft.y) {
											// Cell is above top of frame
											//   or Cell is taller than frame
			vertScroll = (cellTop - adjust_halfway) - frameTopLeft.y;
		}

		ScrollPinnedImageBy(horizScroll, vertScroll, true);
	}
	else
		LTableView::ScrollCellIntoFrame(theCell);
}

// Scroll cell into view
void CTable::ScrollCellToTop(const STableCell &scrollCell)
{
	STableCell  theCell(scrollCell);

	// Adjust for selection mechanism
	if (mRowSelect)
		theCell.col = 1;
	else if (mColSelect)
		theCell.row = 1;

	// This is code copied direct from LTableView

	SInt32	cellLeft, cellTop, cellRight, cellBottom;
	mTableGeometry->GetImageCellBounds(theCell, cellLeft, cellTop,
							cellRight, cellBottom);

	CRect frame;
	GetClientRect(frame);
	CPoint	frameTopLeft, frameBotRight;
	LocalToImagePoint(frame.TopLeft(), frameTopLeft);
	LocalToImagePoint(frame.BottomRight(), frameBotRight);

	SInt32	horizScroll = 0;		// Find horizontal scroll amount
	if (cellRight > frameBotRight.x) {
										// Cell is to the right of frame
		horizScroll = cellRight - frameBotRight.x;
	}

	if ((cellLeft - horizScroll) < frameTopLeft.x) {
										// Cell is to the left of frame
										//   or Cell is wider than frame
		horizScroll = cellLeft - frameTopLeft.x;
	}

	SInt32	vertScroll = 0;			// Find vertical scroll amount
	if (cellTop != frameTopLeft.y) {
		vertScroll = cellTop - frameTopLeft.y;
	}

	ScrollPinnedImageBy(horizScroll, vertScroll, true);
}

// Scroll to row doing selection etc
void CTable::ScrollToRow(TableIndexT row, bool select, bool clear, EScrollTo scroll_to)
{
	// Only if row available
	if (row > mRows)
		return;

	STableCell aCell(row, 1);

	if (clear)
		UnselectAllCells();
	if (select)
		FakeClickSelect(aCell, false);

	switch(scroll_to)
	{
	case eScroll_Top:
		ScrollCellToTop(aCell);
		break;
	case eScroll_Center:
		ScrollCellIntoFrame(aCell, true);
		break;
	case eScroll_Bottom:
		ScrollCellIntoFrame(aCell);
		break;
	}
}

#pragma mark ____________________________________Selections

// Select all cells
void CTable::SelectAllCells()
{
	// All selected => unselect all
	if (AllCellsSelected())
		LTableView::UnselectAllCells();
	else
		LTableView::SelectAllCells();
}

bool CTable::AllCellsSelected() const
{
	// Select or unselect based on current state
	bool all_selected = true;
	STableCell cell(1, 1);
	if (mRowSelect)
	{
		for(short i = 1; i <= mRows; i++)
		{
			cell.row = i;
			if (!CellIsSelected(cell))
			{
				all_selected = false;
				break;
			}
		}
	}
	else if (mColSelect)
	{
		for(short i = 1; i <= mCols; i++)
		{
			cell.col = i;
			if (!CellIsSelected(cell))
			{
				all_selected = false;
				break;
			}
		}
	}
	else
	{
		for(short i = 1; i <= mCols; i++)
		{
			for(short j = 1; j <= mRows; j++)
			{
				cell.row = j;
				cell.col = i;
				if (!CellIsSelected(cell))
				{
					all_selected = false;
					break;
				}
			}
		}
	}

	return all_selected;
}

// Prevent multiple selection changes
bool CTable::ClickSelect(const STableCell &inCell, UINT nFLags)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	return LTableView::ClickSelect(inCell, nFLags);
}

// Fake a click in a cell to keep anchor correct
void CTable::FakeClickSelect(const STableCell &inCell, bool extend)
{
	ClickSelect(inCell, extend ? MK_SHIFT : 0);
}

// Move selection up or down by specified amount
void CTable::NudgeSelection(short direction, bool extend, bool primary)
{
	STableCell new_selection;

	// Check for valid selection
	if (IsSelectionValid())
	{
		(direction >= 0) ? GetLastSelection(new_selection) : GetFirstSelection(new_selection);

		// Go to next cell/row/column
		if (!mRowSelect && !mColSelect)
		{
			if (primary)
				(direction >= 0) ? GetNextCell(new_selection) : GetPreviousCell(new_selection);
			else
				(direction >= 0) ? new_selection.row++ : new_selection.row--;
		}
		else if (mRowSelect)
		{
			if (extend)
			{
				STableCell anchor = GetSelectionAnchor();
				STableCell boat = GetSelectionBoat();
				
				if (direction >= 0)
				{
					if (boat.row >= anchor.row)
					{
						if (boat.row < mRows)
						{
							boat.row++;
							FakeClickSelect(boat, extend);
						}
					}
					else
					{
						UnselectCell(boat);
					}
				}
				else
				{
					if (boat.row <= anchor.row)
					{
						if (boat.row > 1)
						{
							boat.row--;
							FakeClickSelect(boat, extend);
						}
					}
					else
					{
						UnselectCell(boat);
					}
				}

				// Bring it into view if it went out
				if (boat.row >= anchor.row)
					ShowLastSelection();
				else
					ShowFirstSelection();
				return;
			}
			else
				(direction >= 0) ? new_selection.row++ : new_selection.row--;
		}
		else
			(direction >= 0) ? new_selection.col++ : new_selection.col--;
	}
	else
	{
		new_selection = STableCell(0, 0);
		(direction >= 0) ? GetNextCell(new_selection) : GetPreviousCell(new_selection);

	}

	// Check new cell is valid and select it with fake click
	if (IsValidCell(new_selection))
	{
		FakeClickSelect(new_selection, extend);

		// Bring it into view if it went out
		if (direction >= 0)
			ShowLastSelection();
		else
			ShowFirstSelection();
	}
}

// Select a row
void CTable::SelectRow(TableIndexT row)
{
	STableCell cell(row, 1);
	SelectCell(cell);
}

// Unselect a row
void CTable::UnselectRow(TableIndexT row)
{
	STableCell cell(row, 1);
	UnselectCell(cell);
}

// Test row for selection
bool CTable::IsRowSelected(TableIndexT row) const
{
	STableCell cell(row, 1);
	return CellIsSelected(cell);
}

// Check for valid selection
bool CTable::IsSelectionValid() const
{
	// Check for valid first selected cell
	STableCell aCell = GetFirstSelectedCell();
	return !aCell.IsNullCell();
}

// Is a single item only selected
bool CTable::IsSingleSelection(void) const
{
	// Get first selected cell
	STableCell firstCell = GetFirstSelectedCell();
	
	// Must have at least one
	if (firstCell.IsNullCell())
		return false;

	// Get last cell
	STableCell lastCell(0, 0);
	if (mTableSelector != NULL) {
		lastCell = mTableSelector->GetLastSelectedCell();
	}

	// Compare first and last and look for differences
	if (!mRowSelect && !mColSelect)
		return firstCell == lastCell;
	else if (mRowSelect)
		return firstCell.row == lastCell.row;
	else
		return firstCell.col == lastCell.col;
}

// Return first selected cell
void CTable::GetFirstSelection(STableCell& firstCell) const
{
	// Get first selection
	firstCell = STableCell(0, 0);
	if (mTableSelector != NULL) {
		firstCell = mTableSelector->GetFirstSelectedCell();
	}
}

// Get first selected row
TableIndexT	CTable::GetFirstSelectedRow() const
{
	// Get first selection
	STableCell firstCell;
	GetFirstSelection(firstCell);
	
	return firstCell.row;
}

// Return last selected cell
void CTable::GetLastSelection(STableCell& lastCell) const
{
	// Get last selection
	lastCell = STableCell(0, 0);
	if (mTableSelector != NULL) {
		lastCell = mTableSelector->GetLastSelectedCell();
	}
}

// Get first selected row
TableIndexT	CTable::GetLastSelectedRow() const
{
	// Get first selection
	STableCell lastCell;
	GetLastSelection(lastCell);
	
	return lastCell.row;
}

// Bring first selected row into view
void CTable::ShowFirstSelection(void)
{
	STableCell	firstCell;

	// Get first selected cell and scroll to it
	GetFirstSelection(firstCell);
	if (IsValidCell(firstCell))
		ScrollCellIntoFrame(firstCell);
}

// Bring last selected row into view
void CTable::ShowLastSelection(void)
{
	STableCell	lastCell;

	// Get first selected cell and scroll to it
	GetLastSelection(lastCell);
	if (IsValidCell(lastCell))
		ScrollCellIntoFrame(lastCell);
}

// Get list of selected rows
void CTable::GetSelectedRows(ulvector& sels) const
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return;

	// Do this twice - once to count selection (so we can reserve space)
	//				 - a second time to actually add th rows

	unsigned long ctr = 0;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(1, 1);
	for(aCell.row = firstRow; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
			ctr++;
	}

	// Reserve space
	sels.reserve(ctr);

	// Iterate from first to last finding all in the range that are selected
	for(aCell.row = firstRow; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
			sels.push_back(aCell.row);
	}
}

#pragma mark ____________________________________Cell Actions

// Do something to each cell in the selection
bool CTable::DoToRows(DoToSelectionPP proc, bool forward)
{
	bool result = false;

	// Loop over all cells
	STableCell	aCell(0, 0);
	while (forward ? GetNextCell(aCell) : GetPreviousCell(aCell))
	{

		// Do proc based on rows only
		if (mRowSelect && (aCell.col == 1))
			result = (this->*proc)(aCell.row) || result;
	}

	return result;
}

// Do something to each cell in the selection with some edata
bool CTable::DoToRows1(DoToSelection1PP proc, void* data, bool forward)
{
	bool result = false;

	// Loop over all cells
	STableCell	aCell(0, 0);
	while (forward ? GetNextCell(aCell) : GetPreviousCell(aCell))
	{
		// Do proc based on rows only
		if (mRowSelect && (aCell.col == 1))
			result = (this->*proc)(aCell.row, data) || result;
	}

	return result;
}

// Do something to each cell in the selection
bool CTable::DoToSelection(DoToSelectionPP proc, bool forward)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool result = false;
	if (forward)
	{
		// Iterate from first to last finding all in the range that are selected
		STableCell aCell(firstRow, 1);
		for(; aCell.row <= lastRow; aCell.row++)
		{
			if (CellIsSelected(aCell))
				result = (this->*proc)(aCell.row) || result;
		}
	}
	else
	{
		// Iterate from last to first finding all in the range that are selected
		STableCell aCell(lastRow, 1);
		for(; aCell.row >= firstRow; aCell.row--)
		{
			if (CellIsSelected(aCell))
				result = (this->*proc)(aCell.row) || result;
		}
	}

	return result;
}

// Do something to each cell in the selection with some edata
bool CTable::DoToSelection1(DoToSelection1PP proc, void* data, bool forward)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool result = false;
	if (forward)
	{
		// Iterate from first to last finding all in the range that are selected
		STableCell aCell(firstRow, 1);
		for(; aCell.row <= lastRow; aCell.row++)
		{
			if (CellIsSelected(aCell))
				result = (this->*proc)(aCell.row, data) || result;
		}
	}
	else
	{
		// Iterate from last to first finding all in the range that are selected
		STableCell aCell(lastRow, 1);
		for(; aCell.row >= firstRow; aCell.row--)
		{
			if (CellIsSelected(aCell))
				result = (this->*proc)(aCell.row, data) || result;
		}
	}

	return result;
}

// Do something to each cell in the selection with some edata
bool CTable::DoToSelection2(DoToSelection2PP proc, void* data1, void* data2, bool forward)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool result = false;
	if (forward)
	{
		// Iterate from first to last finding all in the range that are selected
		STableCell aCell(firstRow, 1);
		for(; aCell.row <= lastRow; aCell.row++)
		{
			if (CellIsSelected(aCell))
				result = (this->*proc)(aCell.row, data1, data2) || result;
		}
	}
	else
	{
		// Iterate from last to first finding all in the range that are selected
		STableCell aCell(lastRow, 1);
		for(; aCell.row >= firstRow; aCell.row--)
		{
			if (CellIsSelected(aCell))
				result = (this->*proc)(aCell.row, data1, data2) || result;
		}
	}

	return result;
}

// Test each cell in the selection using logical AND
bool CTable::TestSelectionAnd(TestSelectionPP proc)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = true;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
			test = (this->*proc)(aCell.row) && test;
	}

	return test;
}

// Test each cell in the selection using logical AND
bool CTable::TestSelectionAnd1(TestSelection1PP proc, void* data)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = true;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
			test = (this->*proc)(aCell.row, data) && test;
	}

	return test;
}

// Test each cell in the selection using logical OR
bool CTable::TestSelectionOr(TestSelectionPP proc)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = false;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
			test = (this->*proc)(aCell.row) || test;
	}

	return test;
}

// Test each cell in the selection using logical OR
bool CTable::TestSelectionOr1(TestSelection1PP proc, void* data)
{
	// Get first selected row (must be non-zero for a valid selection)
	TableIndexT firstRow = GetFirstSelectedRow();
	if (firstRow == 0)
		return false;

	// Get last selected row
	TableIndexT lastRow = GetLastSelectedRow();

	bool test = false;

	// Iterate from first to last finding all in the range that are selected
	STableCell aCell(firstRow, 1);
	for(; aCell.row <= lastRow; aCell.row++)
	{
		if (CellIsSelected(aCell))
			test = (this->*proc)(aCell.row, data) || test;
	}

	return test;
}

// Change font in each row
void CTable::ResetFont(CFont* pFont)
{
	// Set font in Wnd - no redraw yet
	SetFont(pFont, false);
	
	// Get current DC and calc font metrics
	TEXTMETRIC tm;
	{
		CDC* pDC = GetDC();
		CFont* old_font = pDC->SelectObject(pFont);
		pDC->GetTextMetrics(&tm);
		pDC->SelectObject(old_font);
		ReleaseDC(pDC);
	}
	
	// Set new row height
	short height = tm.tmAscent + tm.tmDescent + 2;
	if (height < 16)
		height = 16;
	mTextOrigin = 1;

	// Set icon height
	mIconOrigin = height/2 - 8;
	if (mIconOrigin < 0)
		mIconOrigin = 0;

	SetRowHeight(height, 1, mRows);
	mTextHeight = height;
}

// Change font in each row
void CTable::Changing(bool changing)
{
	int old = mChanging;

	if (changing)
	{
		mChanging++;
		if (!old)
			SetRedraw(false);
	}
	else
	{
		if (mChanging) mChanging--;
		if (!mChanging)
		{
			SetRedraw(true);
			Invalidate();
		}
	}
}

#pragma mark ____________________________________Command Updaters

void CTable::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

void CTable::OnUpdateNever(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(false);	// Never
}

void CTable::OnUpdateSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(IsSelectionValid());	// Only if selection
}

void CTable::OnUpdateSelectAll(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mRows > 0);	// Only if selection

	CString txt;
	txt.LoadString(AllCellsSelected() ? IDS_UNSELECTALL_CMD_TEXT : IDS_SELECTALL_CMD_TEXT);
	::OnUpdateMenuTitle(pCmdUI, txt);
}

#pragma mark ____________________________________Command Handlers

BOOL CTable::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Prevent re-entrancy
	CCommanderProtect::StCommandProtect _protect(mCmdProtect, nID, nCode);
	if (_protect.AlreadyProcessing())
		return false;

	// Do default processing
	if (LTableView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;
	
	// Try super commander if any
	else if (GetSuperCommander())
		return GetSuperCommander()->HandleCmdMsg(nID, nCode, pExtra, pHandlerInfo);
	
	// No one wants it
	else
		return false;
}

void CTable::OnEditSelectAll(void)
{
	StDeferSelectionChanged defer(this);

	SelectAllCells();
}

void CTable::OnEditClear(void)
{
	DeleteSelection();
}

#pragma mark ____________________________________Keyboard/Mouse

// Handle key down
void CTable::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	StDeferSelectionChanged defer(this);	// Must protect against stack unwind
	
	if (!HandleKeyDown(nChar, nRepCnt, nFlags))
		// Do inherited action
		LTableView::OnKeyDown(nChar, nRepCnt, nFlags);
}

bool CTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Handle arrow navigation
	switch(nChar)
	{
	case VK_LEFT:
		if (!mRowSelect)
		{
			NudgeSelection(-1, ::GetKeyState(VK_SHIFT) < 0);
			return true;
		}
	case VK_RIGHT:
		if (!mRowSelect)
		{
			NudgeSelection(1, ::GetKeyState(VK_SHIFT) < 0);
			return true;
		}
	case VK_UP:
		if (!mColSelect)
		{
			NudgeSelection(-1, ::GetKeyState(VK_SHIFT) < 0, mRowSelect);
			return true;
		}
	case VK_DOWN:
		if (!mColSelect)
		{
			NudgeSelection(1, ::GetKeyState(VK_SHIFT) < 0, mRowSelect);
			return true;
		}
	case VK_END:
		// Special case row/column table
		if (mRowSelect)
		{
			ScrollPinnedImageBy(0, INT_MAX/2, true);
			if ((mRows != 0) && (mCols != 0))
			{
				ClickSelect(STableCell(mRows, 1), (::GetKeyState(VK_SHIFT) < 0) ? MK_SHIFT : 0);
			}
			return true;
		}
		else if (mColSelect)
		{
			ScrollPinnedImageBy(INT_MAX/2, 0, true);
			if ((mRows != 0) && (mCols != 0))
			{
				ClickSelect(STableCell(1, mCols), (::GetKeyState(VK_SHIFT) < 0) ? MK_SHIFT : 0);
			}
			return true;
		}
		break;
	default:;
	}

	return false;
}

// Handle character
void CTable::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	StDeferSelectionChanged defer(this);	// Must protect against stack unwind
	
	if (!HandleChar(nChar, nRepCnt, nFlags))
		// Do inherited action
		LTableView::OnChar(nChar, nRepCnt, nFlags);
}

void CTable::OnSetFocus(CWnd* pOldWnd)
{
	// Change state and redraw selection highlight
	mHasFocus = true;
	RefreshSelection();

	LTableView::OnSetFocus(pOldWnd);

	// Make it the commander target
	SetTarget(this);

	// Check whether parent chain contains a focus border
	CWnd* parent = GetParent();
	while(parent != NULL)
	{
		CGrayBackground* focus = dynamic_cast<CGrayBackground*>(parent);
		if ((focus != NULL) && (focus->CanFocus()))
		{
			focus->SetBorderFocus(true);
			break;
		}
		parent = parent->GetParent();
	}

	Broadcast_Message(eBroadcast_Activate, this);

	// Do this after the activation broadcast to prevent old view from
	// actually being re-focussed
	if (mContextView)
		mContextView->GetParentFrame()->SetActiveView(mContextView);
}

void CTable::OnKillFocus(CWnd* pNewWnd)
{
	// Change state and redraw selection highlight
	mHasFocus = false;
	RefreshSelection();

	LTableView::OnKillFocus(pNewWnd);

	// Remove the commander target
	//SetTarget(GetSuperCommander());

	// Check whether parent chain contains a focus border
	CWnd* parent = GetParent();
	while(parent != NULL)
	{
		CGrayBackground* focus = dynamic_cast<CGrayBackground*>(parent);
		if ((focus != NULL) && (focus->CanFocus()))
		{
			focus->SetBorderFocus(false);
			break;
		}
		parent = parent->GetParent();
	}

	Broadcast_Message(eBroadcast_Deactivate, this);
}

void CTable::SetDeferSelectionChanged(bool defer, bool do_change)
{
	if (defer)
	{
		if (!mDeferSelectionChanged++)
			mSelectionChanged = false;
	}
	else
	{
		if (mDeferSelectionChanged)
			mDeferSelectionChanged--;
		if (!mDeferSelectionChanged && mSelectionChanged && do_change)
			SelectionChanged();
	}
}

void CTable::SelectionChanged()
{
	// Only do if not deferred and not a drop target
	if (!mDeferSelectionChanged)
	{
		if (!mIsDropTarget)
			DoSelectionChanged();
	}
	else
		// Set this flag to force selection change when defer is turned off
		mSelectionChanged = true;
}

void CTable::DoSelectionChanged()
{
	// Let sub-classes deal with this
	Broadcast_Message(eBroadcast_SelectionChanged, this);
}

#pragma mark ____________________________________Drawing

bool CTable::DrawCellSelection(CDC* pDC, const STableCell& inCell)
{
	// If only one column can be selected, ignore the others
	if (!OKToSelect(inCell.col))
		return false;
	
	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = (mHasFocus || mIsDropTarget);
	
	CRect cellFrame;

	if (selected && mDrawSelection && GetLocalCellRect(inCell, cellFrame))
	{
		if (active)
		{
			pDC->SetTextColor(::GetSysColor(COLOR_HIGHLIGHTTEXT));
			pDC->SetBkColor(::GetSysColor(COLOR_HIGHLIGHT));
			
		   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
			pDC->ExtTextOut(cellFrame.left, cellFrame.top, ETO_OPAQUE, cellFrame, _T(""), 0, nil);
			
			return true;
		}
		else
		{
			if (mRowSelect)
			{
				int adjust = (inCell.col == mCols) ? 1 : 0;
				CPen temp_pen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));
				CPen* old_pen = pDC->SelectObject(&temp_pen);
				pDC->MoveTo(cellFrame.left, cellFrame.top);
				pDC->LineTo(cellFrame.right - adjust, cellFrame.top);
				pDC->MoveTo(cellFrame.right - adjust, cellFrame.bottom - 1);
				pDC->LineTo(cellFrame.left, cellFrame.bottom - 1);
				if (inCell.col == 1)
				{
					pDC->MoveTo(cellFrame.left, cellFrame.bottom - 1);
					pDC->LineTo(cellFrame.left, cellFrame.top);
				}
				else if (inCell.col == mCols)
				{
					pDC->MoveTo(cellFrame.right - adjust, cellFrame.top);
					pDC->LineTo(cellFrame.right - adjust, cellFrame.bottom - 1);
				}
				pDC->SelectObject(old_pen);
			}
			else if (mColSelect)
			{
				int adjust = (inCell.row == mRows) ? 1 : 0;
				CPen temp_pen(PS_SOLID, 1, ::GetSysColor(COLOR_HIGHLIGHT));
				CPen* old_pen = pDC->SelectObject(&temp_pen);
				pDC->MoveTo(cellFrame.left, cellFrame.bottom - adjust);
				pDC->LineTo(cellFrame.left, cellFrame.top);
				pDC->MoveTo(cellFrame.right - 1, cellFrame.top);
				pDC->LineTo(cellFrame.right - 1, cellFrame.bottom - adjust);
				if (inCell.row == 1)
				{
					pDC->MoveTo(cellFrame.left, cellFrame.top);
					pDC->LineTo(cellFrame.right - 1, cellFrame.top);
				}
				else if (inCell.row == mRows)
				{
					pDC->MoveTo(cellFrame.right - 1, cellFrame.bottom - adjust);
					pDC->LineTo(cellFrame.left, cellFrame.bottom - adjust);
				}
				pDC->SelectObject(old_pen);
			}
			else
			{
				CBrush framer(::GetSysColor(COLOR_HIGHLIGHT));
				pDC->FrameRect(cellFrame, &framer);
			}
		}
	}
			
	return false;
}

//	Draw or undraw active hiliting for a Cell
void CTable::HiliteCellActively(const STableCell& inCell, bool inHilite)
{
	CRect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame))
	{
#if 0
		// Setup drawing
		CPaintDC dc(this);
		dc.SetBkColor(::GetSysColor(COLOR_WINDOW));
		CFont* old_font = dc.SelectObject(CFontCache::GetListFont());

		// Deal with background
		DrawBackground(&dc, cellFrame);
#endif

		// Set the hilite cell to this one if it needs to be hilited
		// This takes care of hiliting a cell that is not selected, which can happen
		// during drag and drop hiliting
		StValueChanger<STableCell> _preserve_cell(mHiliteCell, inHilite ? inCell : STableCell(0, 0));
		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		
#if 0
		// If there are no sub-panes, just draw cell directly for faster response
		HWND hWndChild = ::GetWindow(GetSafeHwnd(), GW_CHILD);
		if (hWndChild == NULL)
			DrawCell(&dc, inCell, cellFrame);
		else
#endif
		{
			// Do full draw of the cell area only - this will also draw intersecting sub-panes
			RedrawWindow(cellFrame);
		}
	}
}

//	Draw or undraw inactive hiliting for a Cell
void CTable::HiliteCellInactively(const STableCell& inCell, bool inHilite)
{
	CRect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame))
	{
#if 0
		// Setup drawing
		CPaintDC dc(this);
		dc.SetBkColor(::GetSysColor(COLOR_WINDOW));
		CFont* old_font = dc.SelectObject(CFontCache::GetListFont());

		// Deal with background
		DrawBackground(&dc, cellFrame);
#endif

		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		
#if 0
		// If there are no sub-panes, just draw cell directly for faster response
		HWND hWndChild = ::GetWindow(GetSafeHwnd(), GW_CHILD);
		if (hWndChild == NULL)
			DrawCell(&dc, inCell, cellFrame);
		else
#endif
		{
			// Do full draw of the cell area only - this will also draw intersecting sub-panes
			RedrawWindow(cellFrame);
		}
	}
}

#pragma mark ____________________________________Tooltips

int CTable::OnToolHitTest(CPoint point, TOOLINFO* pTI) const
{
	// Do tooltips only if user wants them
	if (CPreferences::sPrefs->mDetailedTooltips.GetValue())
	{
		CRect rect;
		GetClientRect(rect);
		if (rect.PtInRect(point) && GetItemCount())
		{
			CPoint imagePt;
			LocalToImagePoint(point, imagePt);
			if (GetCellHitBy(imagePt, mTooltipCell))
			{
				// Get cell rectangle
				CRect cellRect;
				GetLocalCellRect(mTooltipCell, cellRect);
				
				// Hits against child windows always center the tip
				if (pTI != NULL && pTI->cbSize >= sizeof(AFX_OLDTOOLINFO))
				{
					// Setup the TOOLINFO structure
					pTI->hwnd = m_hWnd;
					pTI->uId = ((mTooltipCell.row & 0xFFFF) << 16) | (mTooltipCell.col & 0xFFFF);
					pTI->lpszText = LPSTR_TEXTCALLBACK;
					pTI->rect = cellRect;
					return pTI->uId;
				}
			}
		}
	}
	
	// Not found
	mTooltipCell.SetCell(0, 0);
	return -1;
}

BOOL CTable::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
	// Need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	UINT nID = pNMHDR->idFrom;

	if (nID == 0)
		return false;

	::SendMessage(pNMHDR->hwndFrom, TTM_SETMAXTIPWIDTH, 0, 300);

	// Get text from element
	cdstring text;
	GetTooltipText(text);

#ifdef _UNICODE
	mTooltipText = cdustring(text);
#else
	mTooltipText = text;
#endif
	
#ifdef _UNICODE
	pTTTW->lpszText = mTooltipText.c_str_mod();
#else
	pTTTA->lpszText = mTooltipText.c_str_mod();
#endif
	*pResult = 0;

	return TRUE;    // message was handled
}

