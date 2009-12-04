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


// Source for CScriptsTable class

#include "CScriptsTable.h"

#include "CBetterScrollerX.h"
#include "CContextMenu.h"
#include "CDragIt.h"
#include "CErrorHandler.h"
#include "CFilterManager.h"
#include "CMulberryCommon.h"
#include "CNodeVectorTree.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CRulesTable.h"
#include "CRulesWindow.h"
#include "CSimpleTitleTable.h"
#include "CStringUtils.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CTitleTable.h"

#include <LBevelButton.h>
#include <LTableArrayStorage.h>


// __________________________________________________________________________________________________
// C L A S S __ C G R O U P T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CScriptsTable::CScriptsTable()
{
	InitScriptsTable();
}

// Default constructor - just do parents' call
CScriptsTable::CScriptsTable(LStream *inStream)
		: CHierarchyTableDrag(inStream)

{
	InitScriptsTable();
}

// Default destructor
CScriptsTable::~CScriptsTable()
{
}

// Common init
void CScriptsTable::InitScriptsTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(void*));

	mType = CFilterItem::eLocal;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CScriptsTable::FinishCreateSelf(void)
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Find window in super view chain
	LWindow* aWindow = (LWindow*) mSuperView;
	while(aWindow->GetPaneID() != paneid_RulesWindow)
		aWindow = (LWindow*) aWindow->GetSuperView();

	mTitles = (CTitleTable*) aWindow->FindPaneByID(paneid_RulesScriptsTitles);

	// Get buttons
	mNewBtn = (LBevelButton*) aWindow->FindPaneByID(paneid_RulesBtnNewScript);
	mEditBtn = (LBevelButton*) aWindow->FindPaneByID(paneid_RulesBtnEdit);
	mDeleteBtn = (LBevelButton*) aWindow->FindPaneByID(paneid_RulesBtnDelete);
	mGenerateBtn = (LBevelButton*) aWindow->FindPaneByID(paneid_RulesBtnGenerateScript);

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	AddDragFlavor(cDragRuleType);
	AddDropFlavor(cDragRuleType);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(false);

	// Init columns
	InsertCols(1, 1, NULL, 0, false);
	AdaptToNewSurroundings();
	SetFilterType(mType);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, aWindow, RidL_CRulesWindowBtns);
}

void CScriptsTable::SetFilterType(CFilterItem::EType type)
{
	mType = type;

	if (mType == CFilterItem::eLocal)
		mTitles->SetTitleInfo(false, false, "UI::Titles::Targets", 1, 0);
	else
		mTitles->SetTitleInfo(false, false, "UI::Titles::Scripts", 1, 0);

	ResetTable();
}

// Respond to clicks in the icon buttons
void CScriptsTable::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_RulesNewTarget:
	case msg_RulesNewScript:
		OnNewScript();
		break;
	case msg_RulesEdit:
		if (IsTarget())
			OnEditScript();
		break;
	case msg_RulesDelete:
		if (IsTarget())
			OnDeleteScript();
		break;
	case msg_RulesGenerateScript:
		OnGenerateScript();
		break;

	default:;
	}
}

// Handle key presses
Boolean CScriptsTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{

		// Edit the address
		case char_Return:
		case char_Enter:
			OnEditScript();
			break;

		// Delete
		case char_Backspace:
		case char_Clear:
			// Special case escape key
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
				return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
			else
				OnDeleteScript();
			break;

		default:
			return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

// Allow twist
void CScriptsTable::ClickCell(
	const STableCell		&inCell,
	const SMouseDownEvent	&inMouseDown)
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
	if (event_match && DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey)))) {

		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
						::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag) {

			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(NULL);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(NULL);
			return;
		}
	}

	// If multiclick then edit entry
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		OnEditScript();
	else
		// Carry on to do default action
		CHierarchyTableDrag::ClickCell(inCell, inMouseDown);
}

// Make it target first
void CScriptsTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	StColorState::Normalize();
	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Draw the titles
void CScriptsTable::DrawCell(const STableCell &inCell,
								const Rect &inLocalRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;
	CFilterItem* filter = NULL;

	STableCell	woCell(woRow, inCell.col);
	UInt32 dataSize = sizeof(void*);
	GetCellData(woCell, is_script ? (void*) &script : (void*) &filter, dataSize);

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	::EraseRect(&inLocalRect);

	DrawDropFlag(inCell, woRow);

#if PP_Target_Carbon
	// Draw selection
	bool selected_state = DrawCellSelection(inCell);
#else
	bool selected_state = false;
#endif

	Rect iconRect;
	iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
	iconRect.right = iconRect.left + 16;
	iconRect.bottom = inLocalRect.bottom - mIconDescent;
	iconRect.top = iconRect.bottom - 16;
	if (mType == CFilterItem::eLocal)
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, is_script ? ICNx_Target : ICNx_Rule);
	else
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, is_script ? ICNx_Script : ICNx_Rule);

	// Move to origin for text
	::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);

	theTxt = is_script ? script->GetName() : filter->GetName();

	// Clip to cell frame & table frame
	Rect clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper,&inLocalRect,&clipper);
	StClipRgnState	clip(clipper);

	// Use italic for disabled scripts
	if (script && !script->IsEnabled())
		::TextFace(italic);

	// Draw the string
	short width = inLocalRect.right - iconRect.right - 2;
	::DrawClippedStringUTF8(theTxt, width, eDrawString_Left);
}





// Make sure columns are set
void CScriptsTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	CHierarchyTableDrag::AdaptToNewSurroundings();

	// Get super frame's width - scroll bar
	SDimension16 my_frame;
	GetFrameSize(my_frame);

	// Set image to super frame size
	ResizeImageTo(my_frame.width, my_frame.height, true);

	// Set column width
	SetColWidth(my_frame.width - 1, 1, 1);
	mTitles->SetColWidth(my_frame.width - 1, 1, 1);
}

void CScriptsTable::BeTarget()
{
	CHierarchyTableDrag::BeTarget();
	UpdateButtons();
}

void CScriptsTable::DontBeTarget()
{
	CHierarchyTableDrag::DontBeTarget();
	mGenerateBtn->Disable();
}

// Update delete buttons
void CScriptsTable::UpdateButtons(void)
{
	if (IsSelectionValid())
	{
		mEditBtn->Enable();
		mDeleteBtn->Enable();
		
		if ((mType == CFilterItem::eSIEVE) &&
			IsSingleSelection() &&
			TestSelectionOr((TestSelectionPP) &CScriptsTable::TestSelectionScript))
			mGenerateBtn->Enable();
		else
			mGenerateBtn->Disable();
	}
	else
	{
		mEditBtn->Disable();
		mDeleteBtn->Disable();
		mGenerateBtn->Disable();
	}
}

// Keep cached list in sync
void CScriptsTable::ResetTable(void)
{
	// Delete all existing rows
	while (mRows)
		RemoveRows(1, mRows, false);

	// Add all scripts
	short last_grp_row = 0;
	if (mType == CFilterItem::eLocal)
	{
		for(CTargetItemList::iterator iter1 = CPreferences::sPrefs->GetFilterManager()->GetTargets(mType).begin();
				iter1 != CPreferences::sPrefs->GetFilterManager()->GetTargets(mType).end(); iter1++)
		{
			// Insert script item
			last_grp_row = InsertSiblingRows(1, last_grp_row, &(*iter1), sizeof(CFilterScript*), true, false);

			// Add all rules in script
			for(CFilterItemList::iterator iter2 = (*iter1)->GetFilters().begin(); iter2 != (*iter1)->GetFilters().end(); iter2++)
				AddLastChildRow(last_grp_row, &(*iter2), sizeof(CFilterItem*), false, false);
		}
	}
	else
	{
		for(CFilterScriptList::iterator iter1 = CPreferences::sPrefs->GetFilterManager()->GetScripts(mType).begin();
				iter1 != CPreferences::sPrefs->GetFilterManager()->GetScripts(mType).end(); iter1++)
		{
			// Insert script item
			last_grp_row = InsertSiblingRows(1, last_grp_row, &(*iter1), sizeof(CFilterScript*), true, false);

			// Add all rules in script
			for(CFilterItemList::iterator iter2 = (*iter1)->GetFilters().begin(); iter2 != (*iter1)->GetFilters().end(); iter2++)
				AddLastChildRow(last_grp_row, &(*iter2), sizeof(CFilterItem*), false, false);
		}
	}

	// Collapse all rows
	TableIndexT collapse_row = 0;
	while(collapse_row < mRows)
	{
		TableIndexT	woRow;
		woRow = mCollapsableTree->GetWideOpenIndex(collapse_row);
		if (mCollapsableTree->IsCollapsable(woRow))
			DeepCollapseRow(woRow);
		collapse_row++;
	}

	Refresh();
	UpdateButtons();
}

// Add mail message to drag
void CScriptsTable::AddCellToDrag(CDragIt* theDragTask,
										const STableCell& aCell,
										Rect& dragRect)
{
}

// Adjust cursor over drop area
bool CScriptsTable::IsCopyCursor(DragReference inDragRef)
{
	return true;
}

// Test drag insert cursor
bool CScriptsTable::IsDropCell(DragReference inDragRef, STableCell theCell)
{
	TableIndexT woRow = GetWideOpenIndex(theCell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	return IsValidCell(theCell) && (nestingLevel == 0);
}

// Test drop at cell
bool CScriptsTable::IsDropAtCell(DragReference inDragRef, STableCell& aCell)
{
	TableIndexT woRow = GetWideOpenIndex(aCell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	return IsValidCell(aCell) && (nestingLevel == 1);
}

// Drop data into cell
void CScriptsTable::DropDataIntoCell(FlavorType inFlavor, char* drag_data,
											Size data_size, const STableCell& inCell)
{
	bool added = false;

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	bool is_script = nestingLevel == 0;
	CFilterScript* script = NULL;

	// Only if script
	ThrowIf_(!is_script);

	// Get data item
	STableCell	woCell(woRow, inCell.col);
	UInt32 dataSize = sizeof(CFilterScript*);
	GetCellData(woCell, &script, dataSize);

	switch (inFlavor)
	{
	case cDragRuleType:
		{
			CFilterItem* filter = *((CFilterItem**) drag_data);
			added = script->AddFilter(filter);
			if (added)
				AddLastChildRow(woRow, &filter, sizeof(CFilterItem*), false, true);
			break;
		}
	}
}

// Drop data at cell
void CScriptsTable::DropDataAtCell(FlavorType theFlavor, char* drag_data,
									Size data_size, const STableCell& beforeCell)
{
}
