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


// Source for CHierarachyTableDrag class

#include "CHierarchyTableDrag.h"

#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CStringUtils.h"
#include "CTableRowGeometry.h"

#include <UTableHelpers.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CHierarchyTableDrag::CHierarchyTableDrag()
#if PP_Target_Carbon
	: CHierarchyTableDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	: CHierarchyTableDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitHierarchyTableDrag();
}

// Constructor from stream
CHierarchyTableDrag::CHierarchyTableDrag(LStream *inStream)
	: LHierarchyTable(inStream),
#if PP_Target_Carbon
	  CHierarchyTableDragAndDrop(UQDGlobals::GetCurrentWindowPort(), this)
#else
	  CHierarchyTableDragAndDrop(UQDGlobals::GetCurrentPort(), this)
#endif
{
	InitHierarchyTableDrag();

	// Replace LNodeArrayTree with CNodeArrayTree
	delete mCollapsableTree;
	mCollapsableTree = new CNodeVectorTree;
}

// Default destructor
CHierarchyTableDrag::~CHierarchyTableDrag()
{
	Broadcast_Message(eBroadcast_TableDeleted, this);
}

// Default constructor
void CHierarchyTableDrag::InitHierarchyTableDrag()
{
	mFirstIndent = 20;
	mLevelIndent = 18;
	mDeferSelectionChanged = 0;
	mSelectionChanged = false;
	mKeySelection = false;
	mLastTyping = 0UL;
	mLastChars[0] = '\0';

	mUseTooltips = false;
	mDrawSelection = true;
	mDrawSelectionActive = false;
	mSelectionIconIndent = true;
	mSelectColumn = 0;
	mHiliteCell = STableCell(0, 0);

	// Always do individual cell hilite
	SetCustomHilite(true);
	
	// Force reconcile of frame and image to always be on
	SetReconcileOverhang(true);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup key navigation
void CHierarchyTableDrag::FinishCreateSelf(void)
{
	// Do inherited
	LHierarchyTable::FinishCreateSelf();

	// Allow extended keyboard actions
	AddAttachment(new LKeyScrollAttachment(this));

}

//	Respond to commands
Boolean CHierarchyTableDrag::ObeyCommand(CommandT inCommand,void *ioParam)
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
void CHierarchyTableDrag::FindCommandStatus(
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
Boolean CHierarchyTableDrag::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Nudge selection up
		case char_UpArrow:
			NudgeSelection(-1, (inKeyEvent.modifiers & shiftKey) != 0);
			break;

		// Nudge selection down
		case char_DownArrow:
			NudgeSelection(1, (inKeyEvent.modifiers & shiftKey) != 0);
			break;

		// Collpase
		case char_LeftArrow:
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
							if (inKeyEvent.modifiers & optionKey)
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
			}
			break;

		// Expand
		case char_RightArrow:
			{
				STableCell aCell(0, 0);
				while(GetNextSelectedCell(aCell))
				{
					if (aCell.col == 1)
					{
						TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(aCell.row);
						if (IsCollapsable(woRow) && !IsExpanded(woRow))
						{
							if (inKeyEvent.modifiers & optionKey)
								DeepExpandRow(woRow);
							else
								ExpandRow(woRow);
							RefreshRow(aCell.row);
						}
					}
				}
			}
			break;

		default:
			// Allow typing selection from any alphanumeric character - not cmd-xx
			if (mKeySelection && !(inKeyEvent.modifiers & cmdKey))
			{
				char hit_key = (inKeyEvent.message & charCodeMask);
				if (::isalnum(hit_key))
				{

					// See if key hit within standard double click time
					unsigned long new_time = TickCount();
					if (new_time - mLastTyping > GetDblTime())
					{

						// Outside double-click time so new selection
						mLastChars[0] = hit_key;
						mLastChars[1] = '\0';
					}
					else
					{
						// Inside double-click time so add char to selection
						short pos = ::strlen(mLastChars);
						if (pos < 31)
						{
							mLastChars[pos++] = hit_key;
							mLastChars[pos] = '\0';
						}
					}
					mLastTyping = new_time;

					DoKeySelection();
				}
			}
			return LCommander::HandleKeyPress(inKeyEvent);
	}

	return true;
}

void CHierarchyTableDrag::BeTarget()
{
	Activate();
	Broadcast_Message(eBroadcast_Activate, this);
}

void CHierarchyTableDrag::DontBeTarget()
{
	Deactivate();
	Broadcast_Message(eBroadcast_Deactivate, this);
}

// Get text for current tooltip cell
void CHierarchyTableDrag::GetTooltipText(cdstring& txt, const STableCell &inCell)
{
	// Sub-classes must override
}

bool CHierarchyTableDrag::SetupHelp(LPane* pane, Point inPortMouse, HMHelpContentPtr ioHelpContent)
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

void CHierarchyTableDrag::SelectionChanged(void)
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

void CHierarchyTableDrag::DoSelectionChanged(void)
{
	// Override to do actual action
	Broadcast_Message(eBroadcast_SelectionChanged, this);
}

// Check for valid selection
bool CHierarchyTableDrag::IsSelectionValid(void)
{
	// Check for valid first selected cell
	STableCell aCell = GetFirstSelectedCell();
	return !aCell.IsNullCell();
}

// ---------------------------------------------------------------------------
//		¥ CellIsSelected
// ---------------------------------------------------------------------------
//	Return whether the specified Cell is part of the current selection

Boolean CHierarchyTableDrag::CellIsSelected(
	const STableCell	&inCell) const
{
	bool	selected = false;

	if (mTableSelector != nil) {
		selected = mTableSelector->CellIsSelected(inCell);
	}

	return selected;
}

// Is a single item only selected
bool CHierarchyTableDrag::IsSingleSelection(void)
{
	// Get first selected cell
	TableIndexT firstRow = GetFirstSelectedRow();
	
	// Must have at least one
	if (firstRow == 0)
		return false;

	// Get last cell
	TableIndexT lastRow = GetLastSelectedRow();

	// Compare first and last and look for differences
	return firstRow == lastRow;
}

// Return first selected cell
void CHierarchyTableDrag::GetFirstSelection(STableCell& firstCell)
{
	// Get first selection
	firstCell = STableCell(0, 0);
	if (mTableSelector != nil) {
		firstCell = mTableSelector->GetFirstSelectedCell();
	}
}

// Get first selected row
TableIndexT	CHierarchyTableDrag::GetFirstSelectedRow()
{
	// Get first selection
	STableCell firstCell;
	GetFirstSelection(firstCell);
	
	return firstCell.row;
}

// Return last selected cell
void CHierarchyTableDrag::GetLastSelection(STableCell& lastCell)
{
	// Get last selection
	lastCell = STableCell(0, 0);
	if (mTableSelector != nil) {
		lastCell = mTableSelector->GetLastSelectedCell();
	}
}

// Get first selected row
TableIndexT	CHierarchyTableDrag::GetLastSelectedRow()
{
	// Get first selection
	STableCell lastCell;
	GetLastSelection(lastCell);
	
	return lastCell.row;
}

// Bring first selected cell into view
void CHierarchyTableDrag::ShowFirstSelection(void)
{
	STableCell	firstCell;

	// Get first selected cell and scroll to it
	GetFirstSelection(firstCell);
	if (IsValidCell(firstCell))
		ScrollCellIntoFrame(firstCell);
}

// Bring last selected cell into view
void CHierarchyTableDrag::ShowLastSelection(void)
{
	STableCell	lastCell;

	// Get first selected cell and scroll to it
	GetLastSelection(lastCell);
	if (IsValidCell(lastCell))
		ScrollCellIntoFrame(lastCell);
}

// Get list of selected rows
void CHierarchyTableDrag::GetSelectedRows(ulvector& sels)
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
void CHierarchyTableDrag::Click(SMouseDownEvent &inMouseDown)
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
		LHierarchyTable::Click(inMouseDown);
}

// Prevent multiple selection changes
Boolean CHierarchyTableDrag::ClickSelect(const STableCell &inCell, const SMouseDownEvent &inMouseDown)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	return LHierarchyTable::ClickSelect(inCell, inMouseDown);
}

// Fake a click in a cell to keep anchor correct
void CHierarchyTableDrag::FakeClickSelect(const STableCell &inCell, bool extend)
{
	SMouseDownEvent mouseEvt;
	mouseEvt.macEvent.modifiers = (extend ? shiftKey : 0);

	ClickSelect(inCell, mouseEvt);
}

// Move selection up or down
void CHierarchyTableDrag::NudgeSelection(short direction, bool extend)
{
	STableCell new_selection;

	// Check for valid selection
	if (IsSelectionValid())
	{
		(direction >= 0) ? GetLastSelection(new_selection) : GetFirstSelection(new_selection);

		// Go to next cell/row/column
		(direction >= 0) ? new_selection.row++ : new_selection.row--;
	}
	else
	{
		new_selection = STableCell(0, 0);
		(direction >= 0) ? GetNextCell(new_selection) : GetPreviousCell(new_selection);
	}

	// Check new cell is valid and select it
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

void CHierarchyTableDrag::SelectRow(TableIndexT row)
{
	STableCell cell(row, 1);
	SelectCell(cell);
}

void CHierarchyTableDrag::UnselectRow(TableIndexT row)
{
	STableCell cell(row, 1);
	UnselectCell(cell);
}

//	Select all Cells in a Table
void CHierarchyTableDrag::SelectAllCells()
{
	// All were selected => unselect all
	if (AllCellsSelected())
		LHierarchyTable::UnselectAllCells();
	else
		LHierarchyTable::SelectAllCells();
}

bool CHierarchyTableDrag::AllCellsSelected() const
{
	// Select or unselect based on current state
	bool all_selected = true;
	STableCell cell(1, 1);
	for(short i = 1; i <= mRows; i++)
	{
		cell.row = i;
		if (!CellIsSelected(cell))
		{
			all_selected = false;
			break;;
		}
	}

	return all_selected;
}

void CHierarchyTableDrag::SelectAllCommandStatus(Boolean &outEnabled, Str255 outName)
{
	outEnabled = (mRows > 0);
	LStr255 txt(STRx_Standards, AllCellsSelected() ? str_UnselectAll : str_SelectAll);
	::PLstrcpy(outName, txt);
}

// Scroll cell into view
void CHierarchyTableDrag::ScrollCellIntoFrame(const STableCell &scrollCell, bool middle)
{
	STableCell  theCell(scrollCell);

	// Adjust for selection mechanism
	theCell.col = 1;

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
void CHierarchyTableDrag::ScrollCellToTop(const STableCell &scrollCell)
{
	STableCell  theCell(scrollCell);

	// Adjust for selection mechanism
	theCell.col = 1;

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
void CHierarchyTableDrag::ScrollToRow(TableIndexT row, bool select, bool clear, EScrollTo scroll_to)
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
		woRow = ((CNodeVectorTree*) mCollapsableTree)->GetSiblingIndex(woRow);
	}
	
	// Select the chosen row
	selCell.row = GetExposedIndex(selRow);
	FakeClickSelect(selCell, false);
	ScrollCellIntoFrame(selCell);
}

void CHierarchyTableDrag::SetDeferSelectionChanged(bool defer, bool do_change)
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

// Do something to each cell in the selection
bool CHierarchyTableDrag::DoToSelection(DoToSelectionPP proc, bool forward)
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
bool CHierarchyTableDrag::DoToSelection1(DoToSelection1PP proc, void* data, bool forward)
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
bool CHierarchyTableDrag::DoToSelection2(DoToSelection2PP proc, void* data1, void* data2, bool forward)
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
bool CHierarchyTableDrag::TestSelectionAnd(TestSelectionPP proc)
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
bool CHierarchyTableDrag::TestSelectionAnd1(TestSelection1PP proc, void* data)
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
bool CHierarchyTableDrag::TestSelectionOr(TestSelectionPP proc)
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
bool CHierarchyTableDrag::TestSelectionOr1(TestSelection1PP proc, void* data)
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
bool CHierarchyTableDrag::CanScrollImageBy(
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
void CHierarchyTableDrag::DrawRow(TableIndexT row)
{
	STableCell drawCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		Rect	cellRect;

		drawCell.col = col;
		GetLocalCellRect(drawCell, cellRect);
		DrawCell(drawCell, cellRect);
	}
}

// Draw whole row
void CHierarchyTableDrag::HiliteRow(TableIndexT row, Boolean inHilite)
{
	STableCell hiliteCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		hiliteCell.col = col;
		HiliteCell(hiliteCell, inHilite);
	}
}

// Draw whole row
void CHierarchyTableDrag::HiliteRowActively(TableIndexT row, Boolean inHilite)
{
	STableCell hiliteCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		hiliteCell.col = col;
		HiliteCellActively(hiliteCell, inHilite);
	}
}

// Draw whole row
void CHierarchyTableDrag::HiliteRowInactively(TableIndexT row, Boolean inHilite)
{
	STableCell hiliteCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++) {
		hiliteCell.col = col;
		HiliteCellInactively(hiliteCell, inHilite);
	}
}

#if PP_Target_Carbon
void CHierarchyTableDrag::DrawSelf()
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

bool CHierarchyTableDrag::DrawCellSelection(const STableCell& inCell)
{
	// If only one column can be selected, ignore the others
	if (!OKToSelect(inCell.col))
		return false;
	
	bool selected = CellIsSelected(inCell) || (mHiliteCell == inCell);
	bool active = IsActive() || mDrawSelectionActive;
	
	Rect cellFrame;

	if (selected && mDrawSelection && GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		// Adjust for twister
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);
		UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

		cellFrame.left += mFirstIndent + nestingLevel * mLevelIndent;
		cellFrame.left += mSelectionIconIndent ? 17 : 0;

		if (active)
		{
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
			::LineTo(cellFrame.right - 2, cellFrame.top);
			::MoveTo(cellFrame.left, cellFrame.bottom - 1);
			::LineTo(cellFrame.right - 2, cellFrame.bottom - 1);

			// Do RHS vert line
			if ((mSelectColumn != 0) || (inCell.col == 1))
			{
				::MoveTo(cellFrame.left, cellFrame.top);
				::LineTo(cellFrame.left, cellFrame.bottom - 1);
			}

			// Do LHS vert line
			if ((mSelectColumn != 0) || (inCell.col == mCols))
			{
				::MoveTo(cellFrame.right - 2, cellFrame.top);
				::LineTo(cellFrame.right - 2, cellFrame.bottom - 1);
			}
		}
	}
	
	return false;
}

//	Draw or undraw active hiliting for a Cell

void CHierarchyTableDrag::HiliteCellActively(const STableCell& inCell, Boolean inHilite)
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
		StValueChanger<bool> _preserve1(mDrawSelection, inHilite);
		StValueChanger<bool> _preserve2(mDrawSelectionActive, true);
		DrawCell(inCell, cellFrame);
	}
}

//	Draw or undraw inactive hiliting for a Cell

void CHierarchyTableDrag::HiliteCellInactively(const STableCell& inCell, Boolean inHilite)
{
	Rect cellFrame;
	if (OKToSelect(inCell.col) && GetLocalCellRect(inCell, cellFrame) && FocusExposed())
	{
		// Deal with background
		ApplyForeAndBackColors();
		::EraseRect(&cellFrame);

		StValueChanger<bool> _preserve(mDrawSelection, inHilite);
		DrawCell(inCell, cellFrame);
	}
}
#endif

// Get rect of row
void CHierarchyTableDrag::GetLocalRowRect(TableIndexT row, Rect& theRect)
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

// Refresh selection
void CHierarchyTableDrag::RefreshSelection(void)
{
	// Refresh every cell in selection
	STableCell aCell(0, 0);

	while(GetNextSelectedCell(aCell))
		RefreshCell(aCell);
}

// Refresh row
void CHierarchyTableDrag::RefreshRow(TableIndexT row)
{
	STableCell aCell(row, 0);

	for(TableIndexT col = 1; col <= mCols; col++)
	{
		aCell.col = col;
		RefreshCell(aCell);
	}
}

// Refresh column
void CHierarchyTableDrag::RefreshCol(TableIndexT col)
{
	STableCell aCell(0, col);

	for(TableIndexT row = 1; row <= mRows; row++)
	{
		aCell.row = row;
		RefreshCell(aCell);
	}
}

void CHierarchyTableDrag::RefreshRowsDownFrom(TableIndexT inWideOpenRow)
{
	// Do default
	LHierarchyTable::RefreshRowsDownFrom(inWideOpenRow);

	// Now refresh visible area below last cell

	// Get bounds of last cell
	STableCell aCell(mRows, mCols);
	SInt32 left, top, right, bottom;
	GetImageCellBounds(aCell, left, top, right, bottom);

	// If bottom is above frame bottom them must refresh frame
	if (ImageRectIntersectsFrame(0, bottom - 1, right, bottom))
	{
		Rect refresh_it;
		refresh_it.left = mFrameLocation.h;
		refresh_it.right = refresh_it.left + mFrameSize.width;
		refresh_it.top = bottom + mImageLocation.v;
		refresh_it.bottom = mFrameLocation.v + mFrameSize.height;

		InvalPortRect(&refresh_it);
	}
}

void CHierarchyTableDrag::RefreshRowsBelow(TableIndexT inWideOpenRow)
{
	// Do default
	LHierarchyTable::RefreshRowsBelow(inWideOpenRow);

	// Now refresh visible area below last cell

	// Get bounds of last cell
	STableCell aCell(mRows, mCols);
	SInt32 left, top, right, bottom;
	GetImageCellBounds(aCell, left, top, right, bottom);

	// If bottom is above frame bottom them must refresh frame
	if (ImageRectIntersectsFrame(0, bottom - 1, right, bottom))
	{
		Rect refresh_it;
		refresh_it.left = mFrameLocation.h;
		refresh_it.right = refresh_it.left + mFrameSize.width;
		refresh_it.top = bottom + mImageLocation.v;
		refresh_it.bottom = mFrameLocation.v + mFrameSize.height;

		InvalPortRect(&refresh_it);
	}
}


void CHierarchyTableDrag::ReconcileFrameAndImage(Boolean inRefresh)
{
	// Always refresh
	LHierarchyTable::ReconcileFrameAndImage(true);

	// Now refresh visible area below last cell

	// Get bounds of last cell
	STableCell aCell(mRows, mCols);
	SInt32 left, top, right, bottom;
	GetImageCellBounds(aCell, left, top, right, bottom);

	// If bottom is above frame bottom them must refresh frame
	if (ImageRectIntersectsFrame(0, bottom - 1, right, bottom))
	{
		Rect refresh_it;
		refresh_it.left = mFrameLocation.h;
		refresh_it.right = refresh_it.left + mFrameSize.width;
		refresh_it.top = bottom + mImageLocation.v;
		refresh_it.bottom = mFrameLocation.v + mFrameSize.height;

		InvalPortRect(&refresh_it);
	}
}

// Remove everything from table
void CHierarchyTableDrag::Clear(void)
{
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

	((CNodeVectorTree*) mCollapsableTree)->Clear();

	AdjustImageSize(false);

}

// Get text for row
const char* CHierarchyTableDrag::GetRowText(UInt32 inWideOpenIndex)
{
	return cdstring::null_str;
}

Boolean CHierarchyTableDrag::HasSibling(UInt32 inWideOpenIndex)
{
	return ((CNodeVectorTree*) mCollapsableTree)->HasSibling(inWideOpenIndex);
}

Boolean CHierarchyTableDrag::HasParent(UInt32 inWideOpenIndex)
{
	return ((CNodeVectorTree*) mCollapsableTree)->HasParent(inWideOpenIndex);
}

Boolean CHierarchyTableDrag::HasChildren(UInt32 inWideOpenIndex)
{
	return ((CNodeVectorTree*) mCollapsableTree)->HasChildren(inWideOpenIndex);
}

// Reset text traits - recal row height
void CHierarchyTableDrag::SetTextTraits(const TextTraitsRecord& aTextTrait)
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
	if (height < 18)
		height = 18;
	SetRowHeight(height, 1, mRows);
	((CTableRowGeometry*) mTableGeometry)->SetDefaultRowHeight(height);
	mTextDescent = finfo.descent + 1;

	// Set icon height
	if (height > 18 + mTextDescent)
		mIconDescent = mTextDescent;
	else
		mIconDescent = height - 18;

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
