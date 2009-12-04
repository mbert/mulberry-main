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


// Source for CRulesTable class

#include "CRulesTable.h"

#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CDragIt.h"
#include "CFilterManager.h"
#include "CMailboxInfoTable.h"
#include "CMailboxWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CRulesWindow.h"
#include "CSimpleTitleTable.h"
#include "CSMTPWindow.h"
#include "CStaticText.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"

#include <LCheckBox.h>
#include <LBevelButton.h>

#include <stdio.h>
#include <string.h>

const short cTextIndent = 14;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesTable::CRulesTable()
{
	InitRulesTable();
}

// Default constructor - just do parents' call
CRulesTable::CRulesTable(LStream *inStream)
	: CTableDrag(inStream)
{
	InitRulesTable();
}

// Default destructor
CRulesTable::~CRulesTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CRulesTable::InitRulesTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);

	SetRowSelect(true);
	
	mType = CFilterItem::eLocal;
}

// Get details of sub-panes
void CRulesTable::FinishCreateSelf(void)
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Find window in super view chain
	mWindow = (CRulesWindow*) mSuperView;
	while(mWindow->GetPaneID() != paneid_RulesWindow)
		mWindow = (CRulesWindow*) mWindow->GetSuperView();
	mTitles = (CTitleTable*) mWindow->FindPaneByID(paneid_TitleTable);

	// Get buttons
	mNewBtn = (LBevelButton*) mWindow->FindPaneByID(paneid_RulesBtnNewRule);
	mEditBtn = (LBevelButton*) mWindow->FindPaneByID(paneid_RulesBtnEdit);
	mDeleteBtn = (LBevelButton*) mWindow->FindPaneByID(paneid_RulesBtnDelete);
	mApplyBtn = (LBevelButton*) mWindow->FindPaneByID(paneid_RulesBtnApply);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	SetTable(this, false);
	AddDropFlavor(cDragRulePosType);
	AddDragFlavor(cDragRulePosType);
	AddDragFlavor(cDragRuleType);

	SetDDReadOnly(false);
	SetDropCell(false);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetExternalDrag(false);

	// Set appropriate Drag & Drop inset
	CBetterScrollerX* scroller = dynamic_cast<CBetterScrollerX*>(mPane);
	if (scroller)
	{
		Rect ddInset = {1, 1, 1, 1};
		if (scroller->HasVerticalScrollBar())
			ddInset.right += 15;
		if (scroller->HasHorizontalScrollBar())
			ddInset.bottom += 15;
		SetHiliteInset(ddInset);
	}

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, mWindow, RidL_CRulesWindowBtns);

	// Init columns
	InsertCols(1, 1, NULL, 0, false);
	AdaptToNewSurroundings();
	SetFilterType(mType);
}

void CRulesTable::SetFilterType(CFilterItem::EType type)
{
	mType = type;
	ResetTable();
}

// Handle key presses
Boolean CRulesTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Edit the rule
		case char_Return:
		case char_Enter:
			OnEditRules();
			break;

		// Delete
		case char_Backspace:
		case char_Clear:
			// Special case escape key
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
				return CTableDrag::HandleKeyPress(inKeyEvent);
			else
				OnDeleteRules();
			break;

		default:
			return CTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

//	Respond to commands
Boolean CRulesTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_NewRules:
		OnNewRules();
		break;

	case cmd_EditRules:
		OnEditRules();
		break;

	case cmd_DeleteRules:
		OnDeleteRules();
		break;

	case cmd_ApplyRuleMailbox:
		OnApplyRules();
		break;

	default:
		cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CRulesTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_NewRules:
		outEnabled = true;
		break;

	// These ones must have a selection
	case cmd_EditRules:
	case cmd_DeleteRules:
		outEnabled = IsSelectionValid();
		break;

	// Only if open mailboxes exist (but not SMTP window)
	case cmd_ApplyRuleMailbox:
		{
			cdmutexprotect<CMailboxWindow::CMailboxWindowList>::lock _lock(CMailboxWindow::sMboxWindows);
			outEnabled = (CMailboxWindow::sMboxWindows->size() > 1) ||
							(CMailboxWindow::sMboxWindows->size() == 1) && (CSMTPWindow::sSMTPWindow == NULL);
		}
		break;

	default:
		CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Respond to clicks in the icon buttons
void CRulesTable::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_RulesNewRule:
		OnNewRules();
		break;
	case msg_RulesEdit:
		if (IsTarget())
			OnEditRules();
		break;
	case msg_RulesDelete:
		if (IsTarget())
			OnDeleteRules();
		break;
	case msg_RulesApply:
		OnApplyRules();
		break;

	default:;
	}
}

// Click in the cell
void CRulesTable::ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown)
{
	// Must check whether the current event is still the one we expect to handle.
	// Its possible that a dialog appeared as a result of the initial click, and now
	// the mouse is no longer down. However WaitMouseMoved will always wait for a mouse up
	// before carrying on.
	EventRecord currEvent;
	LEventDispatcher::GetCurrentEvent(currEvent);
	bool event_match = (inMouseDown.macEvent.what == currEvent.what) &&
						(inMouseDown.macEvent.message == currEvent.message) &&
						(inMouseDown.macEvent.when == currEvent.when) &&
						(inMouseDown.macEvent.where.h == currEvent.where.h) &&
						(inMouseDown.macEvent.where.v == currEvent.where.v) &&
						(inMouseDown.macEvent.modifiers == currEvent.modifiers);

	// Check whether D&D available and over a selected cell or not shift or cmd keys
	if (event_match && mAllowDrag &&
		DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey))))
	{

		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		//bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
		//				::WaitMouseMoved(inMouseDown.macEvent.where);
		bool isDrag = ::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{
			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(nil);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(nil);

			return;
		}
	}

	// If multiclick then display message
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		OnEditRules();
}

// Click
void CRulesTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	CTableDrag::ClickSelf(inMouseDown);
}

// Draw the titles
void CRulesTable::DrawCell(const STableCell &inCell,
								const Rect &inLocalRect)
{
	// Save text & color state in stack objects
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Get its rule
	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(inCell.row - 1);

	// Determine which heading it is
	SColumnInfo col_info = mWindow->GetColumnInfo()[inCell.col - 1];

	// Determine which heading it is and draw it
	Rect	iconRect;
	iconRect.left = inLocalRect.left;
	iconRect.right = iconRect.left + 16;
	iconRect.bottom = inLocalRect.bottom - mIconDescent;
	iconRect.top = iconRect.bottom - 16;
	::Ploticns(&iconRect, atNone, ttNone, ICNx_Rule);

	// Get name of item
	theTxt = item->GetName();

	// Use italic for unused rules
	if (!CPreferences::sPrefs->GetFilterManager()->RuleUsed(item))
		::TextFace(italic);

	// Draw the string
	bool strike = false;
	::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);
	::DrawClippedStringUTF8(theTxt, inLocalRect.right - iconRect.right - 2, eDrawString_Left);
}





// Adjust column widths
void CRulesTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	CTableDrag::AdaptToNewSurroundings();

	// Get super frame's width - scroll bar
	SDimension16 my_frame;
	GetFrameSize(my_frame);

	// Set image to super frame size
	ResizeImageTo(my_frame.width, my_frame.height, true);

	// Set column width
	SetColWidth(my_frame.width - 1, 1, 1);
	mTitles->SetColWidth(my_frame.width - 1, 1, 1);

} // CRulesTable::AdaptToNewSurroundings

void CRulesTable::BeTarget()
{
	CTableDrag::BeTarget();
	UpdateButtons();
	mApplyBtn->Enable();
}

void CRulesTable::DontBeTarget()
{
	CTableDrag::DontBeTarget();
	mApplyBtn->Disable();
}

// Update delete buttons
void CRulesTable::UpdateButtons(void)
{
	if (IsSelectionValid())
	{
		mEditBtn->Enable();
		mDeleteBtn->Enable();
	}
	else
	{
		mEditBtn->Disable();
		mDeleteBtn->Disable();
	}
}

// Reset the table from the mboxList
void CRulesTable::ResetTable(void)
{
	TableIndexT	old_rows;
	TableIndexT	old_cols;
	short num_addrs = 0;

	// Add cached mailboxes
	num_addrs = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).size();

	GetTableSize(old_rows, old_cols);

	if (old_rows > num_addrs)
		RemoveRows(old_rows - num_addrs, 1, false);
	else if (old_rows < num_addrs)
		InsertRows(num_addrs - old_rows, 1, NULL, 0, false);

	// Refresh list
	Refresh();
	UpdateButtons();

} // CRulesTable::ResetTable

// Keep titles in sync
void CRulesTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	mTitles->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

#pragma mark ____________________________Drag & Drop

// Add mail message to drag
void CRulesTable::AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect)
{
	// Get the relevant message and envelope
	unsigned long row = aCell.row - 1;

	// Add this message to drag
	theDragTask->AddFlavorItem(dragRect, (ItemReference) row, cDragRulePosType,
								&row, sizeof(unsigned long), flavorSenderOnly, true);

	CFilterItem* item = CPreferences::sPrefs->GetFilterManager()->GetFilters(mType).at(row);
	theDragTask->AddFlavorItem(dragRect, (ItemReference) row, cDragRuleType,
								&item, sizeof(CFilterItem*), 0, false);
}

// Handle multiple items
void CRulesTable::DoDragReceive(DragReference inDragRef)
{
	// Clear drag accumulation objects
	mDragged.clear();
	mDropRow = 0;

	// Do default action
	CTableDrag::DoDragReceive(inDragRef);

	// Move rules to drop location
	CPreferences::sPrefs->GetFilterManager()->MoveRules(mDragged, mDropRow, mType);

	// Refresh list
	Refresh();
	UpdateButtons();
	
}

// Drop data at cell
void CRulesTable::DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell)
{
	// Just add drag items to accumulator
	mDragged.push_back(*reinterpret_cast<unsigned long*>(drag_data));
	mDropRow = beforeCell.row - 1;
}
