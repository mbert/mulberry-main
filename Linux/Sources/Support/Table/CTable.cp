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


#include "CTable.h"

#include "CCommands.h"
#include "CFocusBorder.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"

#include "StPenState.h"
#include "StValueChanger.h"

#include <UNX_UTableHelpers.h>

#include "lround.h"

#include <JPainter.h>
#include <JXColormap.h>
#include <JXFontManager.h>
#include <JXImage.h>
#include <JXScrollbarSet.h>
#include <JXWindow.h>
#include <JXWindowPainter.h>
#include <jASCIIConstants.h>
#include <jXGlobals.h>
#include <jXKeysym.h>

#include <algorithm>

CTable::CTable(JXScrollbarSet* scrollbarSet, 
				 JXContainer* enclosure,
				 const HSizingOption hSizing, 
				 const VSizingOption vSizing,
				 const JCoordinate x, const JCoordinate y,
				 const JCoordinate w, const JCoordinate h)
  : CCommander(enclosure),
  	LTableView(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h),
	CContextMenu(this),
	mDeferSelectionChanged(false), mSelectionChanged(false)
{
	mSbs = scrollbarSet;

	mTooltipCell = STableCell(0, 0);
	mUseTooltips = false;
	mHasTooltip = false;
	mChanging = 0;
	mRowSelect = false;
	mColSelect = false;
	mIsDropTarget = false;
	mDrawSelection = true;
	mSelectColumn = 0;
	mHiliteCell = STableCell(0, 0);
	mDeferSelectionChanged = 0;
	mSelectionChanged = false;
	
	mFontInfo.fontname = JGetDefaultFontName();
	mFontInfo.size = 10;
}

void CTable::OnCreate()
{
	ResetFont(CPreferences::sPrefs->mListTextFontInfo.GetValue());
	
	// Always fit to enclosure horizontally, and move bottom
	// edge to enclosure's bottom if the vertical scaling is elastic.
	// This fitting assumes thatthe top-left corner is already properly set.

	const JRect frame      = GetFrame();
	const JPoint oldPt     = frame.topLeft();
	const JRect enclBounds = (GetEnclosure())->GetBounds();

	JCoordinate dw=0, dh=0;
	dw = enclBounds.width() - frame.width();
	if (GetVSizing() == kVElastic)
	{
		dh = enclBounds.height() - frame.height() - oldPt.y;
	}

	AdjustSize(dw,dh);
}

// ---------------------------------------------------------------------------
//		 ~CTable
// ---------------------------------------------------------------------------
//	Destructor

CTable::~CTable()
{
	Broadcast_Message(eBroadcast_TableDeleted, this);
}

void CTable::ResetFont(const SFontInfo& finfo)
{
	mFontInfo = finfo;

	// Calculate origins
	// Get current DC and calc font metrics
	JCoordinate ascent, descent;
	JFontStyle fstyle;
	JSize height = GetFontManager()->GetLineHeight(mFontInfo.fontname, mFontInfo.size, fstyle, &ascent, &descent);

	// Set new row height
	if (height < 17)
		height = 17;
	mTextOrigin = 0;

	// Set icon height
	mIconOrigin = height/2 - 8;
	if (mIconOrigin < 0)
		mIconOrigin = 0;

	SetRowHeight(height, 1, mRows);
}

void CTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Get the current relative scroll positions
	double scroll_vert = 0.0;

	{
		double max_val = GetBoundsHeight() - (GetApertureHeight() - dh);
		double now_val = GetApertureGlobal().top - GetBoundsGlobal().top;
		if (max_val > 0.0)
			scroll_vert = now_val / max_val;
	}

	LTableView::ApertureResized(dw, dh);
	
	if (scroll_vert)
	{
		double max_val = GetBoundsHeight() - GetApertureHeight();
		double now_val = scroll_vert * max_val;
		if (max_val > 0.0)
		{
			long v_new = ::lround(now_val);

			ScrollTo(0, v_new);
		}
	}
}

// Handle key down
void CTable::HandleKeyPress(const int key, const JXKeyModifiers& modifiers)
{
	StDeferSelectionChanged _defer(this);	// Must protect against stack unwind
	
	LTableView::HandleKeyPress(key, modifiers);
}

bool CTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	switch(key)
	{
	case kJLeftArrow:
		if (!mRowSelect)
		{
			NudgeSelection(-1, modifiers.shift());
			return true;
		}
	case kJRightArrow:
		if (!mRowSelect)
		{
			NudgeSelection(1, modifiers.shift());
			return true;
		}
	case kJUpArrow:
		if (!mColSelect)
		{
			NudgeSelection(-1, modifiers.shift(), mRowSelect);
			return true;
		}
	case kJDownArrow:
		if (!mColSelect)
		{
			NudgeSelection(1, modifiers.shift(), mRowSelect);
			return true;
		}

	case XK_End:
	case XK_KP_End:
		// Special case row/column table
		if (mRowSelect)
		{
			ScrollPinnedImageBy(0, INT_MAX/2, true);
			if ((mRows != 0) && (mCols != 0))
			{
				ClickSelect(STableCell(mRows, 1), modifiers);
			}
			return true;
		}
		else if (mColSelect)
		{
			ScrollPinnedImageBy(INT_MAX/2, 0, true);
			if ((mRows != 0) && (mCols != 0))
			{
				ClickSelect(STableCell(1, mCols), modifiers);
			}
			return true;
		}
		break;

	default:;
	}

	return CCommander::HandleChar(key, modifiers);
}

//#pragma mark -
void CTable::HandleMouseDown(const JPoint& pt, const JXMouseButton button, const JSize clickCount, 
				      			const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	// Hide tooltip
	if (mHasTooltip)
	{
		ClearHint();
		mHasTooltip = false;
		mTooltipCell.SetCell(0, 0);
	}
	
	LTableView::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
	
	if ((clickCount == 1) && (button == kJXRightButton))
		ContextEvent(pt, buttonStates, modifiers);
}

void CTable::Activate()
{
	// Do inherited
	LTableView::Activate();
}

void CTable::Deactivate()
{
	// Hide tooltip
	if (mHasTooltip)
	{
		ClearHint();
		mHasTooltip = false;
		mTooltipCell.SetCell(0, 0);
	}
	
	// Do inherited
	LTableView::Deactivate();
}

void CTable::HandleFocusEvent()
{
	// Do inherited
	LTableView::HandleFocusEvent();

	// Make it the commander target
	SetTarget(this);

	Broadcast_Message(eBroadcast_Activate, this);
	
	// If there is a focus border, tell it to focus
	const JXContainer* parent = GetEnclosure();
	while((parent != NULL) && (dynamic_cast<const CFocusBorder*>(parent) == NULL))
		parent = parent->GetEnclosure();
	const CFocusBorder* focus = dynamic_cast<const CFocusBorder*>(parent);
	if (focus)
		const_cast<CFocusBorder*>(focus)->SetFocus(true);
}

void CTable::HandleUnfocusEvent()
{
	// Do inherited
	LTableView::HandleUnfocusEvent();

	// Remove the commander target
	//SetTarget(GetSuperCommander());

	Broadcast_Message(eBroadcast_Deactivate, this);
	
	// If there is a focus border, tell it to unfocus
	const JXContainer* parent = GetEnclosure();
	while((parent != NULL) && (dynamic_cast<const CFocusBorder*>(parent) == NULL))
		parent = parent->GetEnclosure();
	const CFocusBorder* focus = dynamic_cast<const CFocusBorder*>(parent);
	if (focus)
		const_cast<CFocusBorder*>(focus)->SetFocus(false);
}

void CTable::HandleWindowFocusEvent()
{
	// Do inherited
	LTableView::HandleWindowFocusEvent();

	// Make it the commander target
	SetTarget(this);

	Broadcast_Message(eBroadcast_Activate, this);
	
	// If there is a focus border, tell it to focus
	const JXContainer* parent = GetEnclosure();
	while((parent != NULL) && (dynamic_cast<const CFocusBorder*>(parent) == NULL))
		parent = parent->GetEnclosure();
	const CFocusBorder* focus = dynamic_cast<const CFocusBorder*>(parent);
	if (focus)
		const_cast<CFocusBorder*>(focus)->SetFocus(true);
}

void CTable::HandleWindowUnfocusEvent()
{
	// Do inherited
	LTableView::HandleWindowUnfocusEvent();
}

void CTable::Receive (JBroadcaster* sender, const Message& message) 
{
	// Check for context menu
	if (!ReceiveMenu(sender, message))
		// Do default action
		LTableView::Receive(sender, message);
}

// Get text for current tooltip cell
void CTable::GetTooltipText(cdstring& txt)
{
	// Only if valid cell
	if (!IsValidCell(mTooltipCell))
	{
		txt = cdstring::null_str;
		return;
	}

	// Get using current cell
	GetTooltipText(txt, mTooltipCell);
}

// Get text for current tooltip cell
void CTable::GetTooltipText(cdstring& txt, const STableCell& cell)
{
	// Sub-classes must override
}

void CTable::HandleMouseHere(const JPoint& pt, const JXKeyModifiers& modifiers)
{
	// Special beahviour if handling tooltips only
	if (CPreferences::sPrefs->mDetailedTooltips.GetValue() && mUseTooltips)
	{
		STableCell hitCell;
		JPoint imagePt = pt;
		GetCellHitBy(imagePt, hitCell);

		// Do tooltip
		bool display_tooltip = false;
		if (mTooltipCell.IsNullCell() && !hitCell.IsNullCell())
		{
			// Save the current element
			mTooltipCell = hitCell;
			display_tooltip = true;
		}
		else if (!mTooltipCell.IsNullCell() && hitCell.IsNullCell())
		{
			// Clear the current element
			mTooltipCell.SetCell(0, 0);
			
			// Hide the tooltip
			ClearHint();
			mHasTooltip = false;
		}
		else if (mTooltipCell != hitCell)
		{
			// Save the current element
			mTooltipCell = hitCell;
			display_tooltip = true;
			
			// Hide the tooltip
			ClearHint();
			mHasTooltip = false;
		}
		
		// Initiate display if requested
		if (display_tooltip)
		{
			// Get height of current row
			JCoordinate rowHeight = GetRowHeight(mTooltipCell.row);

			// Point to position and add cell height or at least cursor height
			JPoint pos(pt);
			pos.y += std::max(rowHeight, (JCoordinate) 18);

			// Determine suitable rect
			JRect rect;
			GetImageCellBounds(mTooltipCell, rect);

			// Start a tooltip (no text for now, tooltip will request text when it shows itself)
			cdstring txt;
			GetTooltipText(txt);
			SetHint(txt.c_str(), &rect);
			mHasTooltip = true;
		}
	}
	
	LTableView::HandleMouseHere(pt, modifiers);
}

void CTable::HandleMouseLeave()
{
	// Hide tooltip window
	if (mHasTooltip)
	{
		ClearHint();
		mHasTooltip = false;
		mTooltipCell.SetCell(0, 0);
	}
	
	// Do inherited
	LTableView::HandleMouseLeave();
}

// Prevent multiple selection changes
bool CTable::ClickSelect(const STableCell &inCell, const JXKeyModifiers& modifiers)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	return LTableView::ClickSelect(inCell, modifiers);
}

// Fake a click in a cell to keep anchor correct
void CTable::FakeClickSelect(const STableCell &inCell, bool extend)
{
	JXKeyModifiers mods(GetDisplay());
	if (extend)
		mods.SetState(kJXShiftKeyIndex, kTrue);
	ClickSelect(inCell, mods);
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

void CTable::SelectRow(TableIndexT row)
{
	// Prevent selection flashes
	StDeferSelectionChanged _defer(this);

	STableCell cell(row, 1);
	SelectCell(cell);
}

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

bool CTable::IsSelectionValid() const
{
	// Check for valid first selected cell
	STableCell aCell = GetFirstSelectedCell();
	return !aCell.IsNullCell();
}

bool CTable::IsSingleSelection() const
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

void CTable::GetSelectedRows(ulvector &sels) const
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

void CTable::SelectAllCells()
{
	// Prevent selection flashes
	StDeferSelectionChanged _defer(this);

	// All selected => unselect all
	if (AllCellsSelected())
		LTableView::UnselectAllCells();
	else
		LTableView::SelectAllCells();
}

void CTable::UnselectAllCells()
{
	// Prevent selection flashes
	StDeferSelectionChanged _defer(this);

	LTableView::UnselectAllCells();
}

bool CTable::AllCellsSelected() const
{
	// Select or unselect based on current state
	bool all_selected = true;
	STableCell cell(1, 1);
	if (mRowSelect)
	{
		for(TableIndexT i = 1; i <= mRows; i++)
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
		for(TableIndexT i = 1; i <= mCols; i++)
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
		for(TableIndexT i = 1; i <= mCols; i++)
		{
			for(TableIndexT j = 1; j <= mRows; j++)
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
		JRect	image_frame = GetAperture();					// Get image frame of aperture

		// Adjustment is half frame height
		unsigned long adjust_halfway = image_frame.height() / 2;

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

		JPoint	frameTopLeft = image_frame.topLeft();
		JPoint  frameBotRight = image_frame.bottomRight();

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

	JRect	image_frame = GetAperture();					// Get Frame in Image coords
	JPoint	frameTopLeft = image_frame.topLeft();
	JPoint  frameBotRight = image_frame.bottomRight();

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

void CTable::GetLocalCellRectAlways(const STableCell& inCell, JRect& outCellRect) const
{
	JRect imageRect;
	mTableGeometry->GetImageCellBounds(inCell, imageRect);
	ImageToLocalRect(imageRect, outCellRect);
}

// Get rect of row
void CTable::GetLocalRowRect(TableIndexT row, JRect& theRect)
{
	STableCell aCell(row, 1);

	// Get rect for row (scan columns for visible rows)
	while(!GetLocalCellRect(aCell, theRect) && (aCell.col <= mCols))
		aCell.col++;
	JRect endRect;
	aCell.col = mCols;
	while(!GetLocalCellRect(aCell, endRect) && (aCell.col >= 1))
		aCell.col--;
	theRect.right = endRect.right;
}

// Get rect of column
void CTable::GetLocalColRect(TableIndexT col, JRect& theRect)
{
	STableCell aCell(1, col);

	// Get rect for row
	GetLocalCellRect(aCell, theRect);
	JRect endRect;
	aCell.row = mRows;
	GetLocalCellRect(aCell, endRect);
	theRect.bottom = endRect.bottom;
}

void CTable::RefreshSelection()
{
	// Refresh every cell in selection
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
		RefreshCell(aCell);
}

void CTable::RefreshRow(TableIndexT row)
{
	STableCell aCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++)
	{
		aCell.col = col;
		RefreshCell(aCell);
	}
}

void CTable::RefreshCol(TableIndexT col)
{
	STableCell aCell(0, col);

	for(TableIndexT row = 1; row <= mRows; row++)
	{
		aCell.row = row;
		RefreshCell(aCell);
	}
}

void CTable::DrawBorder(JXWindowPainter& p, const JRect& frame)
{
	// Do default draw for D&D only
	if (IsDNDTarget() && (GetBorderWidth() == 0))
	{
		JRect r   = frame;		// adjust for X centering thickness around line
		r.top    += kJXDefaultBorderWidth/2;
		r.left   += kJXDefaultBorderWidth/2;
		r.bottom -= (kJXDefaultBorderWidth-1)/2;
		r.right  -= (kJXDefaultBorderWidth-1)/2;

		p.SetLineWidth(kJXDefaultBorderWidth);
		p.SetPenColor((p.GetColormap())->GetDefaultDNDBorderColor());
		p.JPainter::Rect(r);
		p.SetLineWidth(1);
	}
	else
		LTableView::DrawBorder(p, frame);
}

void CTable::Draw(JXWindowPainter& p, const JRect& rect)
{
	// Set text font before drawing any cells
	JFontStyle style;
	p.SetFont(mFontInfo.fontname, mFontInfo.size, style);
	
	LTableView::Draw(p, rect);
}

bool CTable::DrawCellSelection(JPainter* pDC, const STableCell& inCell)
{
	// If only one column can be selected, ignore the others
	if (!OKToSelect(inCell.col))
		return false;
	
	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = (HasFocus() || mIsDropTarget);
	
	JRect cellFrame;

	if (selected && mDrawSelection && GetLocalCellRect(inCell, cellFrame))
	{
		if (active)
		{
			StPenState save(pDC);
			pDC->SetPenColor(GetColormap()->GetDefaultSelectionColor());
			pDC->SetFilling(kTrue);
			pDC->RectInside(cellFrame);
			
			return true;
		}
		else
		{
			StPenState save(pDC);
			pDC->SetPenColor(GetColormap()->GetDefaultSelectionColor());
			pDC->SetFilling(kFalse);

			if (mRowSelect)
			{
				int adjust = (inCell.col == mCols) ? 1 : 0;

				pDC->Line(cellFrame.left, cellFrame.top, cellFrame.right - adjust, cellFrame.top);
				pDC->Line(cellFrame.right - adjust, cellFrame.bottom - 1, cellFrame.left, cellFrame.bottom - 1);
				if (inCell.col == 1)
				{
					pDC->Line(cellFrame.left, cellFrame.bottom - 1, cellFrame.left, cellFrame.top);
				}
				else if (inCell.col == mCols)
				{
					pDC->Line(cellFrame.right - adjust, cellFrame.top, cellFrame.right - adjust, cellFrame.bottom - 1);
				}
			}
			else if (mColSelect)
			{
				int adjust = (inCell.row == mRows) ? 1 : 0;
				pDC->Line(cellFrame.left, cellFrame.bottom - adjust, cellFrame.left, cellFrame.top);
				pDC->Line(cellFrame.right - 1, cellFrame.top, cellFrame.right - 1, cellFrame.bottom - adjust);
				if (inCell.row == 1)
				{
					pDC->Line(cellFrame.left, cellFrame.top, cellFrame.right - 1, cellFrame.top);
				}
				else if (inCell.row == mRows)
				{
					pDC->Line(cellFrame.right - 1, cellFrame.bottom - adjust, cellFrame.left, cellFrame.bottom - adjust);
				}
			}
			else
			{
				pDC->RectInside(cellFrame);
			}
		}
	}
			
	return false;
}

//	Draw or undraw active hiliting for a Cell
void CTable::HiliteCellActively(const STableCell& inCell, bool inHilite)
{
	JRect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame))
	{
		// Set the hilite cell to this one if it needs to be hilited
		// This takes care of hiliting a cell that is not selected, which can happen
		// during drag and drop hiliting
		StValueChanger<STableCell> _preserve_cell(mHiliteCell, inHilite ? inCell : STableCell(0, 0));
		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		
		{
			// Do full draw of the cell area only - this will also draw intersecting sub-panes
			RefreshCell(inCell);
		}
	}
}

//	Draw or undraw inactive hiliting for a Cell
void CTable::HiliteCellInactively(const STableCell& inCell, bool inHilite)
{
	JRect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame))
	{
		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		{
			// Do full draw of the cell area only - this will also draw intersecting sub-panes
			RefreshCell(inCell);
		}
	}
}

unsigned long CTable::GetCellBackground(const STableCell& inCell) const
{
	unsigned long bkgnd = 0x00FFFFFF;
	if (CellIsSelected(inCell) && IsActive() && HasFocus())
	{
		JColorIndex bkgnd_index = GetColormap()->GetDefaultSelectionColor();
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}
	
	return bkgnd;
}

// Delete all selected rows
void CTable::DeleteSelection()
{
	DoToSelection((DoToSelectionPP) &CTable::DeleteRow, false);
}

bool CTable::DeleteRow(TableIndexT row)
{
	RemoveRows(1, row, true);
	return true;
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

void CTable::OnUpdateSelection(CCmdUI* cmdui)
{
	cmdui->Enable(IsSelectionValid());
}

void CTable::OnUpdateSelectAll(CCmdUI* cmdui)
{
	cmdui->Enable(GetItemCount() > 0);

	cdstring txt;
	txt.FromResource(AllCellsSelected() ? IDS_UNSELECTALL_CMD_TEXT : IDS_SELECTALL_CMD_TEXT);
	cmdui->SetText(txt);
}

void CTable::OnEditSelectAll()
{
	StDeferSelectionChanged defer(this);

	SelectAllCells();
}

void CTable::OnEditClear(void)
{
	DeleteSelection();
}

void CTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	// Always allow select all
	switch(cmd)
	{
	case CCommand::eEditSelectAll:
		// Enable/change name
		OnUpdateSelectAll(cmdui);
		return;
	default:;
	}

	// Call inherited
	CCommander::UpdateCommand(cmd, cmdui);
}

bool CTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	// Always do select all
	switch(cmd)
	{
	case CCommand::eEditSelectAll:
		OnEditSelectAll();
		return true;
	default:;
	}

	// Call inherited
	return CCommander::ObeyCommand(cmd, menu);
}
