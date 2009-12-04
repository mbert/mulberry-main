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


// Source for CTableDrag class

#include "CTableDrag.h"

#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableRowGeometry.h"

#include <UMemoryMgr.h>

// __________________________________________________________________________________________________
// C L A S S __ C T A B L E V I E W
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTableDrag::CTableDrag()
#if PP_Target_Carbon
	: CTableDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	: CTableDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitTableDrag();
}

//	Construct from input parameters

CTableDrag::CTableDrag(
	const SPaneInfo	&inPaneInfo,
	const SViewInfo	&inViewInfo)
	: LTableView(inPaneInfo, inViewInfo),
#if PP_Target_Carbon
	  CTableDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	  CTableDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitTableDrag();
}

// Constructor from stream
CTableDrag::CTableDrag(LStream *inStream)
	: LTableView(inStream),
#if PP_Target_Carbon
	  CTableDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	  CTableDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitTableDrag();
}

// Default destructor
CTableDrag::~CTableDrag()
{
	Broadcast_Message(eBroadcast_TableDeleted, this);
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Default constructor
void CTableDrag::InitTableDrag()
{
	mRowSelect = false;
	mColSelect = false;
	mDeferSelectionChanged = 0;
	mSelectionChanged = false;

	mUseTooltips = false;
	mDrawSelection = true;
	mSelectColumn = 0;
	mHiliteCell = STableCell(0, 0);

	// Always do individual cell hilite
	SetCustomHilite(true);
}

// Setup key navigation
void CTableDrag::FinishCreateSelf(void)
{
	// Do inherited
	LTableView::FinishCreateSelf();

	// Allow extended keyboard actions
	AddAttachment(new LKeyScrollAttachment(this));

}

//	Respond to commands
Boolean CTableDrag::ObeyCommand(CommandT inCommand,void *ioParam)
{
	Boolean	cmdHandled = true;

	switch (inCommand)
	{
	case msg_TabSelect:
		break;

	case cmd_SelectAll:
		SelectAllCells();
		break;

	default:
		cmdHandled = LCommander::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CTableDrag::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_SelectAll:			// Check if any addresses are present
		SelectAllCommandStatus(outEnabled, outName);
		break;

	default:
		LCommander::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

// Handle key presses
Boolean CTableDrag::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
	case char_LeftArrow:
		if (!mRowSelect)
			NudgeSelection(-1, (inKeyEvent.modifiers & shiftKey) != 0);
		break;

	case char_RightArrow:
		if (!mRowSelect)
			NudgeSelection(1, (inKeyEvent.modifiers & shiftKey) != 0);
		break;

	// Nudge selection up
	case char_UpArrow:
		if (!mColSelect)
			NudgeSelection(-1, (inKeyEvent.modifiers & shiftKey) != 0, mRowSelect);
		break;

	// Nudge selection down
	case char_DownArrow:
		if (!mColSelect)
			NudgeSelection(1, (inKeyEvent.modifiers & shiftKey) != 0, mRowSelect);
		break;

	default:
		return LCommander::HandleKeyPress(inKeyEvent);
	}

	return true;
}

void CTableDrag::BeTarget()
{
	Activate();
	Broadcast_Message(eBroadcast_Activate, this);
}

void CTableDrag::DontBeTarget()
{
	Deactivate();
	Broadcast_Message(eBroadcast_Deactivate, this);
}

// Get text for current tooltip cell
void CTableDrag::GetTooltipText(cdstring& txt, const STableCell &inCell)
{
	// Sub-classes must override
}

bool CTableDrag::SetupHelp(LPane* pane, Point inPortMouse, HMHelpContentPtr ioHelpContent)
{
	// Special beahviour if handling tooltips only
	if (CPreferences::sPrefs->mDetailedTooltips.GetValue() && mUseTooltips)
	{
		Point localPt = inPortMouse;
		PortToLocalPoint(localPt);

		STableCell hitCell = STableCell(0, 0);
		SPoint32 imagePt;

		LocalToImagePoint(localPt, imagePt);

		if (GetCellHitBy(imagePt, hitCell) && !hitCell.IsNullCell())
		{
			// Get cell frame
			Rect cellRect;
			GetLocalCellRect(hitCell, cellRect);
			LocalToPortPoint( topLeft(cellRect) );
			LocalToPortPoint( botRight(cellRect) );
			PortToGlobalPoint( topLeft(cellRect) );
			PortToGlobalPoint( botRight(cellRect) );

			// Get the help text
			cdstring txt;
			GetTooltipText(txt, hitCell);
			SetTagText(txt);
			
			ioHelpContent->absHotRect = cellRect;
			ioHelpContent->tagSide = kHMDefaultSide;
			ioHelpContent->content[kHMMinimumContentIndex].contentType = kHMCFStringContent;
			ioHelpContent->content[kHMMinimumContentIndex].u.tagCFString = GetHelpContent();; 
			ioHelpContent->content[kHMMaximumContentIndex].contentType = kHMNoContent;
			
			return true;
		}
	}

	return false;
}

void CTableDrag::GetLocalCellRectAlways(const STableCell& inCell, Rect& outCellRect) const
{
	SInt32	cellLeft, cellTop, cellRight, cellBottom;
	mTableGeometry->GetImageCellBounds(inCell, cellLeft, cellTop, cellRight, cellBottom);

	SPoint32	imagePt;
	imagePt.h = cellLeft;
	imagePt.v = cellTop;
	ImageToLocalPoint(imagePt, topLeft(outCellRect));
	outCellRect.right  = (SInt16) (outCellRect.left + (cellRight - cellLeft));
	outCellRect.bottom = (SInt16) (outCellRect.top + (cellBottom - cellTop));
}

void CTableDrag::SelectionChanged(void)
{
	// Only if processing selection changed
	if (!mDeferSelectionChanged)
	{
		DoSelectionChanged();
 		LCommander::SetUpdateCommandStatus(true);
 	}
	else
		// Set this flag to force selection change when defer is turned off
		mSelectionChanged = true;
}

void CTableDrag::DoSelectionChanged(void)
{
	// Override to do actual change actions
	Broadcast_Message(eBroadcast_SelectionChanged, this);
}

// Check for valid selection
bool CTableDrag::IsSelectionValid(void)
{
	// Check for valid first selected cell
	STableCell aCell = GetFirstSelectedCell();
	return !aCell.IsNullCell();
}

// Is a single item only selected
bool CTableDrag::IsSingleSelection(void)
{
	// Get first selected cell
	STableCell firstCell = GetFirstSelectedCell();
	
	// Must have at least one
	if (firstCell.IsNullCell())
		return false;

	// Get last cell
	STableCell lastCell(0, 0);
	if (mTableSelector != nil) {
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
void CTableDrag::GetFirstSelection(STableCell& firstCell)
{
	// Get first selection
	firstCell = STableCell(0, 0);
	if (mTableSelector != nil) {
		firstCell = mTableSelector->GetFirstSelectedCell();
	}
}

// Get first selected row
TableIndexT	CTableDrag::GetFirstSelectedRow()
{
	// Get first selection
	STableCell firstCell;
	GetFirstSelection(firstCell);
	
	return firstCell.row;
}

// Return last selected cell
void CTableDrag::GetLastSelection(STableCell& lastCell)
{
	// Get last selection
	lastCell = STableCell(0, 0);
	if (mTableSelector != nil) {
		lastCell = mTableSelector->GetLastSelectedCell();
	}
}

// Get first selected row
TableIndexT	CTableDrag::GetLastSelectedRow()
{
	// Get first selection
	STableCell lastCell;
	GetLastSelection(lastCell);
	
	return lastCell.row;
}

STableCell CTableDrag::GetFirstFullyVisibleCell() const
{
	Rect	frame = mRevealedRect;		// Get Revealed Rect
	PortToLocalPoint(topLeft(frame));	//   in Local coords
	PortToLocalPoint(botRight(frame));

	STableCell	cell, topLeftCell, botRightCell;
	FetchIntersectingCells(frame, topLeftCell, botRightCell);

	// Adjust top one if partially visible
	Rect cellRect;
	GetLocalCellRect(topLeftCell, cellRect);
	if ((cellRect.top < frame.top) && (topLeftCell.row < mRows))
		topLeftCell.row++;
	return topLeftCell;
}

STableCell CTableDrag::GetLastFullyVisibleCell() const
{
	Rect	frame = mRevealedRect;		// Get Revealed Rect
	PortToLocalPoint(topLeft(frame));	//   in Local coords
	PortToLocalPoint(botRight(frame));

	STableCell	cell, topLeftCell, botRightCell;
	FetchIntersectingCells(frame, topLeftCell, botRightCell);

	// Adjust bottom one if partially visible
	Rect cellRect;
	GetLocalCellRect(botRightCell, cellRect);
	if ((cellRect.bottom > frame.bottom) && (botRightCell.row > 1))
		botRightCell.row--;
	return botRightCell;
}

// Bring first selected cell into view
void CTableDrag::ShowFirstSelection(void)
{
	STableCell	firstCell;

	// Get first selected cell and scroll to it
	GetFirstSelection(firstCell);
	if (IsValidCell(firstCell))
		ScrollCellIntoFrame(firstCell);
}

// Bring last selected cell into view
void CTableDrag::ShowLastSelection(void)
{
	STableCell	lastCell;

	// Get first selected cell and scroll to it
	GetLastSelection(lastCell);
	if (IsValidCell(lastCell))
		ScrollCellIntoFrame(lastCell);
}

// Get list of selected rows
void CTableDrag::GetSelectedRows(ulvector& sels)
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

// Allow background window D&D
void CTableDrag::Click(SMouseDownEvent &inMouseDown)
{
	if (inMouseDown.delaySelect && DragAndDropIsPresent() && mAllowDrag)
	{
		// In order to support dragging from an inactive window,
		// we must explicitly test for delaySelect and the
		// presence of Drag and Drop.

		// Convert to a local point.
		PortToLocalPoint(inMouseDown.whereLocal);

		// Execute click attachments.
		if (ExecuteAttachments(msg_Click, &inMouseDown))
		{
			// Handle the actual click event.
			ClickSelf(inMouseDown);
		}

	}
	else
		// Call inherited for default behavior.
		LTableView::Click( inMouseDown );
}

// Prevent multiple selection changes
Boolean CTableDrag::ClickSelect(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	return LTableView::ClickSelect(inCell, inMouseDown);
}

// Fake a click in a cell to keep anchor correct
void CTableDrag::FakeClickSelect(const STableCell &inCell, bool extend)
{
	SMouseDownEvent mouseEvt;
	mouseEvt.macEvent.modifiers = (extend ? shiftKey : 0);

	ClickSelect(inCell, mouseEvt);
}

// Move selection up or down
void CTableDrag::NudgeSelection(short direction, bool extend, bool primary)
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
			(direction >= 0) ? new_selection.row++ : new_selection.row--;
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

//	Select all Cells in a Table
void CTableDrag::SelectAllCells()
{
	// All selected => unselect all
	if (AllCellsSelected())
		LTableView::UnselectAllCells();
	else
		LTableView::SelectAllCells();
}

bool CTableDrag::AllCellsSelected() const
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

void CTableDrag::SelectAllCommandStatus(Boolean &outEnabled, Str255 outName)
{
	outEnabled = (mRows > 0);
	LStr255 txt(STRx_Standards, AllCellsSelected() ? str_UnselectAll : str_SelectAll);
	::PLstrcpy(outName, txt);
}

void CTableDrag::SelectRow(TableIndexT row)
{
	STableCell cell(row, 1);
	SelectCell(cell);
}

void CTableDrag::UnselectRow(TableIndexT row)
{
	STableCell cell(row, 1);
	UnselectCell(cell);
}

// Test row for selection
bool CTableDrag::IsRowSelected(TableIndexT row) const
{
	STableCell cell(row, 1);
	return CellIsSelected(cell);
}

// Scroll cell into view
void CTableDrag::ScrollCellIntoFrame(const STableCell &scrollCell, bool middle)
{
	STableCell  theCell(scrollCell);

	// Adjust for selection mechanism
	if (mRowSelect)
		theCell.col = 1;
	else if (mColSelect)
		theCell.row = 1;

	if (middle)
	{
		// Adjustment is half frame height
		unsigned long adjust_halfway = mFrameSize.height / 2;

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

		Rect	frame;					// Get Frame in Image coords
		CalcLocalFrameRect(frame);
		SPoint32	frameTopLeft, frameBotRight;
		LocalToImagePoint(topLeft(frame), frameTopLeft);
		LocalToImagePoint(botRight(frame), frameBotRight);

		SInt32	horizScroll = 0;		// Find horizontal scroll amount
		if (cellRight > frameBotRight.h) {
											// Cell is to the right of frame
			horizScroll = cellRight - frameBotRight.h;
		}

		if ((cellLeft - horizScroll) < frameTopLeft.h) {
											// Cell is to the left of frame
											//   or Cell is wider than frame
			horizScroll = cellLeft - frameTopLeft.h;
		}

		SInt32	vertScroll = 0;			// Find vertical scroll amount
		if (cellBottom > frameBotRight.v) {
											// Cell is below bottom of frame
			vertScroll = (cellBottom + adjust_halfway) - frameBotRight.v;
		}

		if ((cellTop - vertScroll) < frameTopLeft.v) {
											// Cell is above top of frame
											//   or Cell is taller than frame
			vertScroll = (cellTop - adjust_halfway) - frameTopLeft.v;
		}

		ScrollPinnedImageBy(horizScroll, vertScroll, Refresh_Yes);
	}
	else
		LTableView::ScrollCellIntoFrame(theCell);
}

// Scroll cell into view
void CTableDrag::ScrollCellToTop(const STableCell &scrollCell)
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

	Rect	frame;					// Get Frame in Image coords
	CalcLocalFrameRect(frame);
	SPoint32	frameTopLeft, frameBotRight;
	LocalToImagePoint(topLeft(frame), frameTopLeft);
	LocalToImagePoint(botRight(frame), frameBotRight);

	SInt32	horizScroll = 0;		// Find horizontal scroll amount
	if (cellRight > frameBotRight.h) {
										// Cell is to the right of frame
		horizScroll = cellRight - frameBotRight.h;
	}

	if ((cellLeft - horizScroll) < frameTopLeft.h) {
										// Cell is to the left of frame
										//   or Cell is wider than frame
		horizScroll = cellLeft - frameTopLeft.h;
	}

	SInt32	vertScroll = 0;			// Find vertical scroll amount
	if (cellTop != frameTopLeft.v) {
		vertScroll = cellTop - frameTopLeft.v;
	}

	ScrollPinnedImageBy(horizScroll, vertScroll, Refresh_Yes);
}

// Scroll to row doing selection etc
void CTableDrag::ScrollToRow(TableIndexT row, bool select, bool clear, EScrollTo scroll_to)
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

void CTableDrag::SetDeferSelectionChanged(bool defer, bool do_change)
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

// Do something to each cell in the table
bool CTableDrag::DoToRows(DoToSelectionPP proc, bool forward)
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

// Do something to each cell in the table with some data
bool CTableDrag::DoToRows1(DoToSelection1PP proc, void* data, bool forward)
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

// Do something to each cell in the selection by row
bool CTableDrag::DoToSelection(DoToSelectionPP proc, bool forward)
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
bool CTableDrag::DoToSelection1(DoToSelection1PP proc, void* data, bool forward)
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
bool CTableDrag::DoToSelection2(DoToSelection2PP proc, void* data1, void* data2, bool forward)
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
bool CTableDrag::TestSelectionAnd(TestSelectionPP proc)
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
bool CTableDrag::TestSelectionAnd1(TestSelection1PP proc, void* data)
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
bool CTableDrag::TestSelectionOr(TestSelectionPP proc)
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
bool CTableDrag::TestSelectionOr1(TestSelection1PP proc, void* data)
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

// Determine whether pinned scroll will occur
bool CTableDrag::CanScrollImageBy(
	SInt32		inLeftDelta,			// Pixels to scroll horizontally
	SInt32		inTopDelta)				// Pixels to scroll vertically
{
	SInt32	rightBy = inLeftDelta;
	SInt32	downBy = inTopDelta;

	if (rightBy != 0) {				// Check horizontal position
		SInt32	tryLeft = mImageLocation.h - rightBy;

		if (tryLeft > mFrameLocation.h) {
				// New position would put Image past left of Frame

			if (mImageLocation.h <= mFrameLocation.h) {
					// Scroll to align left sides of Image and Frame
				rightBy = mImageLocation.h - mFrameLocation.h;

			} else if (rightBy < 0) {
					// Image is already past left of Frame.
					// Don't scroll left any farther.
				rightBy = 0;
			}

		} else if ((tryLeft + mImageSize.width) <
						(mFrameLocation.h + mFrameSize.width)) {
				// New position would put Image past right of Frame

			if ((mImageLocation.h + mImageSize.width) >=
					(mFrameLocation.h + mFrameSize.width)) {
					// Scroll to align right sides of Image and Frame
				rightBy = mImageLocation.h - mFrameLocation.h
							+ mImageSize.width - mFrameSize.width;

			} else if (rightBy > 0) {
					// Image is already past right of Frame.
					// Don't scroll right any farther.
				rightBy = 0;
			}
		}
	}

	if (downBy != 0) {				// Check vertical position
		SInt32	tryDown = mImageLocation.v - downBy;

		if (tryDown > mFrameLocation.v) {
				// New position would put Image past top of Frame

			if (mImageLocation.v <= mFrameLocation.v) {
					// Scroll to align top sides of Image and Frame
				downBy = mImageLocation.v - mFrameLocation.v;

			} else if (downBy < 0) {
					// Image is already past top of Frame.
					// Don't scroll up any farther.
				downBy = 0;
			}

		} else if ((tryDown + mImageSize.height) <
						(mFrameLocation.v + mFrameSize.height)) {
				// New position would put Image past bottom of Frame

			if ((mImageLocation.v + mImageSize.height) >=
					(mFrameLocation.v + mFrameSize.height)) {
					// Scroll to align bottom sides of Image and Frame
				downBy = mImageLocation.v - mFrameLocation.v
							+ mImageSize.height - mFrameSize.height;

			} else if (downBy > 0) {
					// Image is already past bottom of Frame.
					// Don't scroll down any farther.
				downBy = 0;
			}
		}
	}

	bool	scrolled = (rightBy != 0) || (downBy != 0);

	return scrolled;
}

// Draw whole row
void CTableDrag::DrawRow(TableIndexT row)
{
	STableCell drawCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		Rect	cellRect;

		drawCell.col = col;
		GetLocalCellRect(drawCell, cellRect);
		DrawCell(drawCell, cellRect);
	}
}

// Draw whole col
void CTableDrag::DrawColumn(TableIndexT col)
{
	STableCell drawCell(0, col);

	for(TableIndexT row = 1; row <= mCols; row++) {
		Rect	cellRect;

		drawCell.row = row;
		GetLocalCellRect(drawCell, cellRect);
		DrawCell(drawCell, cellRect);
	}
}

// Draw whole row
void CTableDrag::HiliteRow(TableIndexT row, Boolean inHilite)
{
	STableCell hiliteCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		hiliteCell.col = col;
		HiliteCell(hiliteCell, inHilite);
	}
}

// Draw whole col
void CTableDrag::HiliteColumn(TableIndexT col, Boolean inHilite)
{
	STableCell hiliteCell(0, col);

	for(TableIndexT row = 1; row <= mCols; row++) {
		hiliteCell.row = row;
		HiliteCell(hiliteCell, inHilite);
	}
}

#if PP_Target_Carbon

void CTableDrag::DrawSelf()
{
	DrawBackground();

		// Determine cells that need updating. Rather than checking
		// on a cell by cell basis, we just see which cells intersect
		// the bounding box of the update region. This is relatively
		// fast, but may result in unnecessary cell updates for
		// non-rectangular update regions.

	Rect	updateRect;
	{
		StRegion	localUpdateRgn( GetLocalUpdateRgn(), false );
		localUpdateRgn.GetBounds(updateRect);
	}
	
	STableCell	topLeftCell, botRightCell;
	FetchIntersectingCells(updateRect, topLeftCell, botRightCell);
	
	DrawCellRange(topLeftCell, botRightCell);

	// Don't need this as selecion is drawn by cells
	// HiliteSelection(IsActive(), true);
}

bool CTableDrag::DrawCellSelection(const STableCell& inCell)
{
	// If only one column can be selected, ignore the others
	if (!OKToSelect(inCell.col))
		return false;
	
	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = IsActive();
	
	Rect cellFrame;

	if (selected && mDrawSelection && GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		if (active)
		{
			StColorState saveColors;
			StColorState::Normalize();

			RGBColor selectColor;
			::GetThemeBrushAsColor(/*kThemeBrushPrimaryHighlightColor*/ -3, GetBitDepth(), GetBitDepth() > 1, &selectColor);

			::RGBBackColor(&selectColor);
			::EraseRect(&cellFrame);
			
			return true;
		}
		else if (!active)
		{
			StColorState saveColors;
			StColorState::Normalize();

			RGBColor selectColor;
			::GetThemeBrushAsColor(/*kThemeBrushPrimaryHighlightColor*/ -3, GetBitDepth(), GetBitDepth() > 1, &selectColor);

			::RGBForeColor(&selectColor);

			// Do horiz lines
			::MoveTo(cellFrame.left, cellFrame.top);
			::LineTo(cellFrame.right - 1, cellFrame.top);
			::MoveTo(cellFrame.left, cellFrame.bottom - 1);
			::LineTo(cellFrame.right - 1, cellFrame.bottom - 1);

			// Do RHS vert line
			if (inCell.col == 1)
			{
				::MoveTo(cellFrame.left, cellFrame.top);
				::LineTo(cellFrame.left, cellFrame.bottom - 1);
			}

			// Do LHS vert line
			if (inCell.col == mCols)
			{
				::MoveTo(cellFrame.right - 1, cellFrame.top);
				::LineTo(cellFrame.right - 1, cellFrame.bottom - 1);
			}
		}
	}
			
	return false;
}

//	Draw or undraw active hiliting for a Cell

void CTableDrag::HiliteCellActively(const STableCell& inCell, Boolean inHilite)
{
	Rect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		// Deal with background
		ApplyForeAndBackColors();
		::EraseRect(&cellFrame);

		// Set the hilite cell to this one if it needs to be hilited
		// This takes care of hiliting a cell that is not selected, which can happen
		// during drag and drop hiliting
		StValueChanger<STableCell> _preserve_cell(mHiliteCell, inHilite ? inCell : STableCell(0, 0));
		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		
		// If there are no sub-panes, just draw cell directly for faster response
		if (GetSubPanes().GetCount() == 0)
			DrawCell(inCell, cellFrame);
		else
		{
			// Do full draw of the cell area only - this will also draw intersecting sub-panes
			LocalToPortPoint(topLeft(cellFrame));
			LocalToPortPoint(botRight(cellFrame));
			
			StRegion rgn(cellFrame);
			Draw(rgn);
		}
	}
}

//	Draw or undraw inactive hiliting for a Cell

void CTableDrag::HiliteCellInactively(const STableCell& inCell, Boolean inHilite)
{
	Rect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		// Deal with background
		ApplyForeAndBackColors();
		::EraseRect(&cellFrame);

		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		
		// If there are no sub-panes, just draw cell directly for faster response
		if (GetSubPanes().GetCount() == 0)
			DrawCell(inCell, cellFrame);
		else
		{
			// Do full draw of the cell area only - this will also draw intersecting sub-panes
			LocalToPortPoint(topLeft(cellFrame));
			LocalToPortPoint(botRight(cellFrame));
			
			StRegion rgn(cellFrame);
			Draw(rgn);
		}
	}
}
#endif

// Get rect of row
void CTableDrag::GetLocalRowRect(TableIndexT row, Rect& theRect)
{
	STableCell aCell(row, 1);

	// Get rect for row (scan columns for visible rows)
	while(!GetLocalCellRect(aCell, theRect) && (aCell.col <= mCols))
		aCell.col++;
	Rect endRect;
	aCell.col = mCols;
	while(!GetLocalCellRect(aCell, endRect) && (aCell.col >= 1))
		aCell.col--;
	theRect.right = endRect.right;
}

// Get rect of column
void CTableDrag::GetLocalColRect(TableIndexT col, Rect& theRect)
{
	STableCell aCell(1, col);

	// Get rect for row
	GetLocalCellRect(aCell, theRect);
	Rect endRect;
	aCell.row = mRows;
	GetLocalCellRect(aCell, endRect);
	theRect.bottom = endRect.bottom;
}

// Refresh selection
void CTableDrag::RefreshSelection(void)
{
	// Refresh every cell in selection
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
		RefreshCell(aCell);
}

// Refresh row
void CTableDrag::RefreshRow(TableIndexT row)
{
	STableCell aCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++)
	{
		aCell.col = col;
		RefreshCell(aCell);
	}
}

// Refresh column
void CTableDrag::RefreshCol(TableIndexT col)
{
	STableCell aCell(0, col);

	for(TableIndexT row = 1; row <= mRows; row++)
	{
		aCell.row = row;
		RefreshCell(aCell);
	}
}

// Reset text traits - recal row height
void CTableDrag::SetTextTraits(ResIDT txtr)
{

	UTextTraits::LoadTextTraits(txtr, mTextTraits);
	SetTextTraits(mTextTraits);
}

// Reset text traits - recal row height
void CTableDrag::SetTextTraits(const TextTraitsRecord& aTextTrait)
{
	mTextTraits = aTextTrait;

	// Save current port and set to table's
	GrafPtr save;
	::GetPort(&save);
	EstablishPort();

	// Set table's port to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Get font info
	FontInfo finfo;
	::GetFontInfo(&finfo);

	// Set new row height
	UInt16 old_row_height = (mRows ? GetRowHeight(1) : 0);
	short height = finfo.ascent + finfo.descent + 4;
	if (height < 16)
		height = 16;
	SetRowHeight(height, 1, mRows);
	((CTableRowGeometry*) mTableGeometry)->SetDefaultRowHeight(height);
	mTextDescent = finfo.descent + 1;

	// Set icon height
	if (height > 17 + mTextDescent)
		mIconDescent = mTextDescent;
	else
		mIconDescent = height - 17;

	// Readjust text if smaller than row height
	if (height != finfo.ascent + finfo.descent + 4)
		mTextDescent = mIconDescent + 8 - finfo.ascent/2;

	// Restore old port
	::SetPort(save);

	// Force image size change
	if (mRows)
		ResizeImageBy(0, mRows * (GetRowHeight(1) - old_row_height), true);

	// Force redraw
	Refresh();
}
