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


// Source for CGroupTable class

#include "CGroupTable.h"

#include "CAddress.h"
#include "CAddressBook.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAddressTable.h"
#include "CAddressView.h"
#include "CBalloonDialog.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDragIt.h"
#include "CEditGroupDialog.h"
#include "CEnvelope.h"
#include "CErrorHandler.h"
#include "CGroup.h"
#include "CGroupList.h"
#include "CGroupTableAction.h"
#include "CMessage.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CResources.h"
#include "CStringUtils.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CTaskClasses.h"
#include "CUserAction.h"

#include <LNodeArrayTree.h>
#include <LTableArrayStorage.h>


// __________________________________________________________________________________________________
// C L A S S __ C G R O U P T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTable::CGroupTable()
{
	InitGroupTable();
}

// Default constructor - just do parents' call
CGroupTable::CGroupTable(LStream *inStream)
		: CHierarchyTableDrag(inStream)

{
	InitGroupTable();
}

// Default destructor
CGroupTable::~CGroupTable()
{
	if (mNewAction || mEditAction || mReplaceAction)
	{
		PostAction(NULL);
		mNewAction = NULL;
		mEditAction = NULL;
		mReplaceAction = NULL;
	}
}

// Common init
void CGroupTable::InitGroupTable()
{
	mAdbk = NULL;
	mNewAction = NULL;
	mEditAction = NULL;
	mPendingEditAction = NULL;
	mReplaceAction = NULL;

	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(SGroupTableItem));

	mDirty = false;

}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CGroupTable::FinishCreateSelf()
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Find table view in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CAddressBookView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CAddressBookView*>(super);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	AddDragFlavor(cDragAddrGrpType);
	AddDragFlavor(kScrapFlavorTypeText);

	AddDropFlavor(cDragAddrType);
	AddDropFlavor(cDragAddrGrpType);
	AddDropFlavor(cDragMsgType);
	AddDropFlavor(kScrapFlavorTypeText);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(false);

	// Make it fit to the superview
	InsertCols(1, 1, NULL, 0, false);
	AdaptToNewSurroundings();

	CContextMenuAttachment::AddUniqueContext(this, 9005, this);
}

//	Respond to commands
Boolean CGroupTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Cut:
		CutItems();
		break;

	case cmd_Copy:
		CopyItems();
		break;

	case cmd_Paste:
		PasteItems();
		break;

	case cmd_NewLetter:
	case cmd_ToolbarNewLetter:
	case cmd_ToolbarNewLetterOption:
		if (IsSelectionValid())
			CreateNewLetter(inCommand == cmd_ToolbarNewLetterOption);
		else
			cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
		break;

	case cmd_EditGroup:
	case cmd_ToolbarAddressBookEditBtn:
		DoEditEntry();
		break;

	case cmd_Clear:
	case cmd_DeleteGroup:
	case cmd_ToolbarAddressBookDeleteBtn:
		DoDeleteEntry();
		break;

	case cmd_ActionDeleted:
		if ((CGroupTableNewAction*) ioParam == mNewAction)
			mNewAction = NULL;
		else if ((CGroupTableEditAction*) ioParam == mEditAction)
			mEditAction = NULL;
		else if ((CGroupTableReplaceAction*) ioParam == mReplaceAction)
			mReplaceAction = NULL;
		break;

	case msg_TabSelect:
		if (!IsEnabled() || !IsVisible())
			cmdHandled = false;
		break;

	default:
		cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CGroupTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_Cut:				// Cut, Copy, and Clear enabled
	case cmd_Copy:				//   if something is selected
	case cmd_Clear:
	case cmd_EditGroup:
	case cmd_ToolbarAddressBookEditBtn:
	case cmd_DeleteGroup:
	case cmd_ToolbarAddressBookDeleteBtn:
		outEnabled = IsSelectionValid();
		break;

	case cmd_Paste:				// Check if TEXT is in the Scrap
		outEnabled = UScrap::HasData(kScrapFlavorTypeText);
		break;

	case cmd_NewLetter:
	case cmd_ToolbarNewLetter:
	case cmd_ToolbarNewLetterOption:
		if (IsSelectionValid())
			outEnabled = true;
		else
			CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;

	default:
		CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Handle key presses
Boolean CGroupTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	// Look for preview/full view based on key stroke
	char key_press = (inKeyEvent.message & charCodeMask);
	if (key_press == char_Enter)
		key_press = char_Return;

	// Determine whether preview is triggered
	CKeyModifiers mods(inKeyEvent.modifiers);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == key_press) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key_press) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	switch (inKeyEvent.message & charCodeMask)
	{
	// Delete
	case char_Backspace:
	case char_Clear:
		// Special case escape key
		if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
			return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
		else
			DoDeleteEntry();
		break;

	default:
		// Allow typing selection from any alphanumeric character - not cmd-xx
		if (!(inKeyEvent.modifiers & cmdKey))
		{
			char hit_key = (inKeyEvent.message & charCodeMask);
			if (((hit_key >= 'A') && (hit_key <= 'Z')) ||
				((hit_key >= 'a') && (hit_key <= 'z')) ||
				((hit_key >= '0') && (hit_key <= '9')))
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
		return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

// Allow twist
void CGroupTable::ClickCell(
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
			 !(inMouseDown.macEvent.modifiers & cmdKey))))
	{
		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
						::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag)
		{
			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(NULL);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(NULL);
			return;
		}
	}

	// Look at user action and determine what to do
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoDoubleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));
	else
		DoSingleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));

	// Carry on to do default action
	CHierarchyTableDrag::ClickCell(inCell, inMouseDown);
}

// Make it target first
void CGroupTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	StColorState::Normalize();
	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Draw the titles
void CGroupTable::DrawCell(const STableCell &inCell,
								const Rect &inLocalRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	STableCell	woCell(woRow, inCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

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

	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
	Rect iconRect;
	iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
	iconRect.right = iconRect.left + 16;
	iconRect.bottom = inLocalRect.bottom - mIconDescent;
	iconRect.top = iconRect.bottom - 16;
	if (item.mIsGroup)
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, ICNx_AdbkGroup);

	// Move to origin for text
	::MoveTo(iconRect.right - 2, inLocalRect.bottom - mTextDescent);

	if (item.mIsGroup)
	{
		CGroup* grp = item.mGroup;
		theTxt = grp->GetName();
	}
	else
	{
		CGroup* grp = item.mGroup;

		// Determine position in group
		short pos = woRow - mCollapsableTree->GetParentIndex(woRow) - 1;

		// Determine name to use
		theTxt = grp->GetAddressList().at(pos);
	}

	// Clip to cell frame & table frame
	Rect clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper,&inLocalRect,&clipper);
	StClipRgnState	clip(clipper);

	// Draw the string
	short width = inLocalRect.right - iconRect.right - 2;
	::DrawClippedStringUTF8(theTxt, width, eDrawString_Left);
}





void CGroupTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
{
	LHierarchyTable::CalcCellFlagRect(inCell, outRect);
	outRect.right = outRect.left + 16;
	outRect.bottom = outRect.top + 12;
}

// Make sure columns are set
void CGroupTable::AdaptToNewSurroundings()
{
	// Do inherited call
	CHierarchyTableDrag::AdaptToNewSurroundings();

	// Get super frame's width - scroll bar
	SDimension16 super_frame;
	mSuperView->GetFrameSize(super_frame);
	super_frame.width -= 16;

	// Set column width
	SetColWidth(super_frame.width - 1, 1, 1);

}

void CGroupTable::DoSelectionChanged()
{
	CHierarchyTableDrag::DoSelectionChanged();
	
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSelection())
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFullView();
}

// Handle single click
void CGroupTable::DoSingleClick(unsigned long row, const CKeyModifiers& mods)
{
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSingleClick() &&
		(preview.GetSingleClickModifiers() == mods))
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSingleClick() &&
		(fullview.GetSingleClickModifiers() == mods))
		DoFullView();
}

// Handle double click
void CGroupTable::DoDoubleClick(unsigned long row, const CKeyModifiers& mods)
{
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetDoubleClick() &&
		(preview.GetDoubleClickModifiers() == mods))
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetDoubleClick() &&
		(fullview.GetDoubleClickModifiers() == mods))
		DoFullView();
}

void CGroupTable::DoPreview()
{
	// Ignore if no preview pane
	if (!mTableView->GetPreview())
		return;

	CGroup* grp = NULL;

	// Determine whether to do groups or address only
	if (TestSelectionOr((TestSelectionPP) &CGroupTable::TestSelectionGroup))
	{
		// Create list to hold preview items (make sure list does not delete them when it is destroyed)
		CGroupList grps;
		grps.set_delete_data(false);

		// Add each selected group
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupToList, &grps);

		// Set to single group only
		if (grps.size() == 1)
			grp = grps.front();
	}

	// Only do preview if there has been a change
	if (grp != mTableView->GetPreview()->GetGroup())
	{
		// Always clear out message preview immediately, otherwise
		// do preview at idle time to prevent re-entrant network calls
		if (grp)
		{
			// Give it to preview (if its NULL the preview will be cleared)
			CGroupPreviewTask* task = new CGroupPreviewTask(mTableView->GetPreview(), mAdbk, grp);
			task->Go();
		}
		else
			// do immediate update of preview
			mTableView->GetPreview()->SetGroup(mAdbk, grp);
	}
}

// Just edit the item
void CGroupTable::DoFullView()
{
	DoEditEntry();
}

// Set the entry list
void CGroupTable::SetAddressBook(CAddressBook* adbk)
{
	// Cache list
	mAdbk = adbk;
	if (mAdbk)
		mAdbk->GetGroupList()->SetComparator(new CGroupComparator());

	// Force reset
	ResetTable();
}

// Insert group into display
TableIndexT CGroupTable::InsertGroup(short pos, CGroup* grp)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Find position of previous group
	TableIndexT woRow = 0;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (mCollapsableTree->GetNestingLevel(woRow) == 0)
			pos--;
		if (pos < 0) break;
	}

	SGroupTableItem item(true, grp);
	TableIndexT newRow = InsertSiblingRows(1, woRow, &item, sizeof(SGroupTableItem), true, false);

	// Add all addresses in group
	for(short i = 0; i < grp->GetAddressList().size(); i++)
	{
		SGroupTableItem addr_item(false, grp);
		AddLastChildRow(newRow, &addr_item, sizeof(SGroupTableItem), false, false);
	}
	DeepCollapseRow(newRow);

	return mCollapsableTree->GetExposedIndex(newRow);
}

// Delete group from display
void CGroupTable::DeleteGroup(short pos)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Find position of group
	TableIndexT woRow = 0;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (mCollapsableTree->GetNestingLevel(woRow) == 0)
			pos--;
		if (pos < 0) break;
	}

	RemoveRows(1, woRow, false);
}

// Replace one group with another
void CGroupTable::ReplaceGroup(CGroup* old_grp, CGroup* new_grp)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Get change
	short diff = new_grp->GetAddressList().size() - old_grp->GetAddressList().size();

	// Do replace
	CGroupList::iterator replace = find(mAdbk->GetGroupList()->begin(), mAdbk->GetGroupList()->end(), old_grp);

	if (replace < mAdbk->GetGroupList()->end())
	{
		*replace = new_grp;

		// Force change in adbk
		mAdbk->UpdateGroup(new_grp);

		short pos = mAdbk->GetGroupList()->FetchIndexOf(new_grp);

		// Find position of group
		TableIndexT woRow = 0;
		for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
		{
			if (mCollapsableTree->GetNestingLevel(woRow) == 0)
				pos--;
			if (pos < 0) break;
		}

		// Replace group item in table
		SGroupTableItem grp_item(true, new_grp);
		STableCell aCell(woRow, 1);
		SetCellData(aCell, &grp_item, sizeof(SGroupTableItem));

		// Change address rows
		SGroupTableItem addr_item(false, new_grp);
		short num = (diff > 0) ? diff : -diff;
		while(num--)
		{
			if (diff > 0)
				AddLastChildRow(woRow, &addr_item, sizeof(SGroupTableItem), false, false);
			else
				RemoveRows(1, woRow+1, false);
		}

		// Refill address rows with new group
		for(short i = 0; i < new_grp->GetAddressList().size(); i++)
		{
			SGroupTableItem addr_item(false, new_grp);
			STableCell aCell(woRow + 1 + i, 1);
			SetCellData(aCell, &addr_item, sizeof(SGroupTableItem));
		}

	}
}

// Insert address into display
void CGroupTable::InsertedAddress(CGroup* grp, short num)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	short pos = mAdbk->GetGroupList()->FetchIndexOf(grp);

	// Find position of group
	TableIndexT woRow = 0;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (mCollapsableTree->GetNestingLevel(woRow) == 0)
			pos--;
		if (pos < 0) break;
	}

	// Replace group item
	SGroupTableItem grp_item(true, grp);
	STableCell aCell(woRow + 1, 1);
	SetCellData(aCell, &grp_item, sizeof(SGroupTableItem));

	// Add required number of new addresses
	SGroupTableItem addr_item(false, grp);
	while(num--)
		AddLastChildRow(woRow, &addr_item, sizeof(SGroupTableItem), false, false);

	// Refill address rows with new group
	for(short i = 0; i < grp->GetAddressList().size(); i++)
	{
		SGroupTableItem addr_item(false, grp);
		STableCell aCell(woRow + 1 + i, 1);
		SetCellData(aCell, &addr_item, sizeof(SGroupTableItem));
	}

}

// Delete address from display
void CGroupTable::DeletedAddress(CGroup* grp, short num)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	short pos = mAdbk->GetGroupList()->FetchIndexOf(grp);

	// Find position of group
	TableIndexT woRow = 0;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (mCollapsableTree->GetNestingLevel(woRow) == 0)
			pos--;
		if (pos < 0) break;
	}

	// Replace group item
	SGroupTableItem grp_item(true, grp);
	STableCell aCell(woRow + 1, 1);
	SetCellData(aCell, &grp_item, sizeof(SGroupTableItem));

	// Remove number of required rows
	while(num--)
		RemoveRows(1, woRow+1, false);

	// Refill address rows with new group
	for(short i = 0; i < grp->GetAddressList().size(); i++)
	{
		SGroupTableItem addr_item(false, grp);
		STableCell aCell(woRow + 1 + i, 1);
		SetCellData(aCell, &addr_item, sizeof(SGroupTableItem));
	}

}

// Keep cached list in sync
void CGroupTable::ResetTable()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Delete all existing rows
	while (mRows)
		RemoveRows(1, mRows, false);

	// Add all groups
	if (mAdbk)
	{
		short last_grp_row = 0;
		for(CGroupList::iterator iter = mAdbk->GetGroupList()->begin(); iter != mAdbk->GetGroupList()->end(); iter++)
		{
			// Insert group item
			SGroupTableItem grp_item(true, *iter);
			last_grp_row = InsertSiblingRows(1, last_grp_row, &grp_item, sizeof(SGroupTableItem), true, false);

			// Add all addresses in group
			for(short i = 0; i < (*iter)->GetAddressList().size(); i++)
			{
				SGroupTableItem addr_item(false, *iter);
				AddLastChildRow(last_grp_row, &addr_item, sizeof(SGroupTableItem), false, false);
			}
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
}

// Reset the table from the address list
void CGroupTable::ClearTable()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Remove all rows
	Clear();
	
	// Update captions to empty
	Refresh();
}

// Select from key press
void CGroupTable::DoKeySelection()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Create new address for typed chars
	CGroup* temp = new CGroup(mLastChars);

	long select = mAdbk->GetGroupList()->FetchInsertIndexOf(temp);

	// Clip to end of list
	if (select > mAdbk->GetGroupList()->size()) select--;

	// Find exposed position of group
	short pos = 0;
	for(pos = 0; pos < mCollapsableTree->CountNodes(); pos++)
	{
		if (mCollapsableTree->GetNestingLevel(pos) == 0)
			select--;
		if (select < 0) break;
	}

	STableCell aCell(GetExposedIndex(pos), 2);
	UnselectAllCells();
	FakeClickSelect(aCell, false);
	ScrollCellIntoFrame(aCell);

	delete temp;

}

// Create new letter from selection
void CGroupTable::CreateNewLetter(bool option_key)
{
	// Create list of selected addresses
	CAddressList list;

	// Copy selection into list for active list only
	AddSelectionToList(&list);

	CReplyChooseDialog::ProcessChoice(&list,
					!(CPreferences::sPrefs->mOptionKeyAddressDialog.GetValue() ^ option_key) && (list.size() > 0));
}

// Create a new group
void CGroupTable::CreateNewEntry()
{
	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_EditGroup, this);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true) {
			MessageT	hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{

				// Create new group and add to list
				CGroup* new_grp = new CGroup();

				// Get strings and update group if changed
				((CEditGroupDialog*) theHandler.GetDialog())->GetFields(new_grp);

				// Only add if some text available
				if (!new_grp->IsEmpty())
				{

					// Initiate action
					mNewAction = new CGroupTableNewAction(this, new_grp, mDirty);
					PostAction(mNewAction);
				}
				else
					delete new_grp;
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				break;
			}
		}

	}

	SwitchTarget(this);
}

// Edit selected entries
void CGroupTable::DoEditEntry()
{
	// Check that there's a selection first - if not beep and exit
	if (!IsSelectionValid()) {
		SysBeep(1);
		return;
	}

	// Create new edit action ready to store edits (force previous one to be committed)
	mPendingEditAction = new CGroupTableEditAction(this, mDirty);

	// Do edit for each selection
	if (TestSelectionOr((TestSelectionPP) &CGroupTable::EditEntry))
	{
		// Initiate action - will commit previous
		mEditAction = mPendingEditAction;
		mPendingEditAction = NULL;
		PostAction(mEditAction);
	}
	else
	{
		// Delete attempted action and restore previous
		delete mPendingEditAction;
		mPendingEditAction = NULL;
	}
}

// Edit specified group
bool CGroupTable::EditEntry(TableIndexT row)
{
	bool done_edit = false;

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Copy original address
	CGroup* copy = new CGroup(*item.mGroup);

	// Only edit groups
	if (item.mIsGroup)
	{
		// Create the dialog
		CBalloonDialog	theHandler(paneid_EditGroup, this);

		((CEditGroupDialog*) theHandler.GetDialog())->SetFields(copy);
		theHandler.StartDialog();

		// Let DialogHandler process events
		while (true)
		{
			MessageT hitMessage = theHandler.DoDialog();

			if (hitMessage == msg_OK)
			{
				// Get strings and update address if changed
				done_edit = ((CEditGroupDialog*) theHandler.GetDialog())->GetFields(copy);

				// Add info to action
				mPendingEditAction->AddEdit(item.mGroup, copy);
				break;
			}
			else if (hitMessage == msg_Cancel)
			{
				delete copy;
				done_edit = false;
				break;
			}
		}
	}

	SwitchTarget(this);

	return done_edit;
}

// Delete selected entries
void CGroupTable::DoDeleteEntry()
{
	// Check that there's a selection first - if not beep and exit
	if (!IsSelectionValid()) {
		SysBeep(1);
		return;
	}

	// Determine whether to do groups or address only
	if (TestSelectionOr((TestSelectionPP) &CGroupTable::TestSelectionGroup))
	{
		// Create list to hold deleted items
		CGroupList* grps = new CGroupList();

		// Add each selected group
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupToList, grps);

		// Initiate delete action
		PostAction(new CGroupTableDeleteAction(this, grps, mDirty));
	}
	else
	{
		// Loop over all selected cells
		STableCell	aCell(0, 0);
		CGroup* aGrp = NULL;
		bool same = true;
		while (GetNextSelectedCell(aCell))
		{
			if ((aCell.col == 1) && !TestCellSameGroup(aCell.row, &aGrp))
			{
				same = false;
				break;
			}
		}

		if (same)
		{
			// Copy original group
			CGroup* copy = new CGroup(*aGrp);

			// Delete selected addresses (in reverse)
			STableCell	delCell(0, 0);
			while (GetPreviousSelectedCell(delCell))
			{
				// Get its info
				TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(delCell.row);

				STableCell	woCell(woRow, delCell.col);
				SGroupTableItem item;
				UInt32 dataSize = sizeof(SGroupTableItem);
				GetCellData(woCell, &item, dataSize);

				// Determine position in group
				short pos = woRow - mCollapsableTree->GetParentIndex(woRow) - 1;

				// Delete address
				copy->GetAddressList().erase(copy->GetAddressList().begin() + pos);
			}

			// Initiate delete action
			mReplaceAction = new CGroupTableReplaceAction(this, aGrp, copy, mDirty);
			PostAction(mReplaceAction);
		}
		else
			CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::DeleteAddressSameGroup");
	}

}

// Cut items from table
void CGroupTable::CutItems()
{
	// Copy first
	CopyItems();

	// Delete selection
	DoDeleteEntry();
}

// Copy items from table
void CGroupTable::CopyItems()
{
	LHandleStream txt;

	// Add all selected address to text
	DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupText, &txt);

	// Now copy to scrap
	{
		StHandleLocker lock(txt.GetDataHandle());

		UScrap::SetData(kScrapFlavorTypeText, txt.GetDataHandle());
	}
}

// Add groups as text to handle
bool CGroupTable::AddGroupText(TableIndexT row, LHandleStream* txt)
{
	// Determine whether to do groups or address only
	mDragGroup = TestSelectionOr((TestSelectionPP) &CGroupTable::TestSelectionGroup);

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Do not do mixture - copy only groups or only addresses
	if (mDragGroup && !item.mIsGroup) return false;

	// Check for single address or group
	if (mDragGroup)
	{
		CGroup* theGrp = item.mGroup;

		for(short i = 0; i < theGrp->GetAddressList().size(); i++)
		{
			cdstring& aStr = theGrp->GetAddressList().at(i);

			// Add CR before if multi-copy
			if (txt->GetLength() > 0)
				txt->WriteBlock(CR, 1);

			txt->WriteBlock(aStr.c_str(), aStr.length());
		}

	}
	else
	{
		// Determine position in group
		short pos = woRow - mCollapsableTree->GetParentIndex(woRow) - 1;

		// Get address
		CGroup* theGrp = item.mGroup;
		cdstring& aStr = theGrp->GetAddressList().at(pos);

		// Add CR before if multi-copy
		if (txt->GetLength() > 0)
			txt->WriteBlock(CR, 1);

		txt->WriteBlock(aStr.c_str(), aStr.length());
	}

	return true;
}

// Paste items from table
void CGroupTable::PasteItems()
{
	StHandleBlock txt(0L);

	if (UScrap::GetData(kScrapFlavorTypeText, txt))
	{
		StHandleLocker lock(txt);

		// Start fresh new/edit actions
		if (mNewAction || mEditAction || mReplaceAction)
		{
			PostAction(NULL);
			mNewAction = NULL;
			mEditAction = NULL;
			mReplaceAction = NULL;
		}

		// Add all addresses to group
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddAddressTextToGroup, &txt);

		// Post action after all items have been added
		if (mNewAction || mReplaceAction)
			PostAction((mNewAction ? (LAction*) mNewAction : (LAction*) mReplaceAction));
	}
}

// Add address as text to handle
bool CGroupTable::AddAddressTextToGroup(TableIndexT row, StHandleBlock* txt)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Check for group
	if (item.mIsGroup)
	{
		STableCell aCell(row, 1);
		DropDataIntoCell(kScrapFlavorTypeText, **txt, ::GetHandleSize(*txt), aCell);
	}

	return true;
}

// Add from list - not unique
void CGroupTable::AddGroupsFromList(CGroupList* grps)
{
	// Add all groups
	mAdbk->AddGroup(grps, true);

	SelectGroups(grps);

	mTableView->UpdateCaptions();

	SetUpdateCommandStatus(true);
	mDirty = false;
}

// Change from list
void CGroupTable::ChangeGroupsFromList(CGroupList* old_grps, CGroupList* new_grps)
{
	// Delete from list
	mAdbk->UpdateGroup(old_grps, new_grps);

	// Reset only after all changes have happened
	UnselectAllCells();
	Refresh();
	mTableView->UpdateCaptions();

	SetUpdateCommandStatus(true);
	mDirty = false;
}

// Delete from list
void CGroupTable::RemoveGroupsFromList(CGroupList* grps)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->RemoveGroup(grps);

	// Reset only after all changes have happened
	UnselectAllCells();
	Refresh();
	mTableView->UpdateCaptions();

	SetUpdateCommandStatus(true);
	mDirty = false;
}

// Delete from list
void CGroupTable::SelectGroups(CGroupList* grps)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Remove existing selection
	UnselectAllCells();

	// Loop over all rows selecting from list
	for(short row = 1; row <= mRows; row++)
	{
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

		STableCell	woCell(woRow, 1);
		SGroupTableItem item;
		UInt32 dataSize = sizeof(SGroupTableItem);
		GetCellData(woCell, &item, dataSize);

		if (item.mIsGroup)
		{
			CGroup* grp = item.mGroup;
			CGroupList::const_iterator found = std::find(grps->begin(), grps->end(), grp);
			if (found != grps->end())
				SelectCell(STableCell(row, 1));
		}
	}

	// Bring first item into view
	ShowFirstSelection();

	// Reset only after all changes have happened
	SetUpdateCommandStatus(true);
}

// Add selected addresses to list
void CGroupTable::AddSelectionToList(CAddressList* list)
{
	// Add all selected address to drag
	DoToSelection1((DoToSelection1PP) &CGroupTable::AddAddressesToList, list);
}

// Add group to list
bool CGroupTable::AddAddressesToList(TableIndexT row, CAddressList* list)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Check for group
	if (item.mIsGroup)
	{

		// Add all addresses in this group
		for(short i = 0; i < item.mGroup->GetAddressList().size(); i++)
		{
			cdstring& aStr = item.mGroup->GetAddressList().at(i);
			list->push_back(new CAddress(aStr));
		}
	}
	else
	{

		// Determine position in group
		short pos = woRow - mCollapsableTree->GetParentIndex(woRow) - 1;

		// Copy and save in selected list
		list->push_back(new CAddress(item.mGroup->GetAddressList().at(pos)));
	}

	return true;
}

// Add group to list
bool CGroupTable::AddGroupToList(TableIndexT row, CGroupList* list)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Check for group and attempt unique insert
	if (item.mIsGroup)
		return list->InsertUniqueItem(item.mGroup);

	return false;
}

// Add items to drag
void CGroupTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& aCell, Rect& dragRect)
{
	// Determine whether to do groups or address only
	mDragGroup = TestSelectionOr((TestSelectionPP) &CGroupTable::TestSelectionGroup);

	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(aCell.row);

	STableCell	woCell(woRow, aCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Do not do mixture - drag only groups or only addresses
	if (mDragGroup && !item.mIsGroup) return;

	// Add this item to drag
	ItemReference ref;
	if (item.mIsGroup)
	{
		ref = (ItemReference) item.mGroup;
		FlavorType type = cDragAddrGrpType;
		void* data = (void*) ref;
		Size data_size = sizeof(CGroup*);
		theDragTask->AddFlavorItem(dragRect, ref, type, &data, data_size, flavorSenderOnly, true);

		// Cannot drag own group to self
		SetSelfDrag(false);
	}
	else
	{
		ref =  (ItemReference) woRow;

		// Can drag own addresses to self
		SetSelfDrag(true);
	}

	// Promise text to anyone who wants it
	theDragTask->AddFlavorItem(dragRect, ref, kScrapFlavorTypeText, 0L, 0L, 0, !item.mIsGroup);
}

// Test for selected item group
bool CGroupTable::TestSelectionGroup(TableIndexT row)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// This is deleted
	return item.mIsGroup;
}

// Test for selected items in same group
bool CGroupTable::TestCellSameGroup(TableIndexT row, CGroup** test)
{
	// Get its info
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row);

	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// This is deleted
	return *test ? (item.mGroup == *test) : ((*test = item.mGroup) != NULL);
}

// Determine scroll mode
void CGroupTable::EnterDropArea(DragReference inDragRef, Boolean inDragHasLeftSender)
{
	bool got_group = false;

	DragAttributes	dragAttrs;
	::GetDragAttributes(inDragRef, &dragAttrs);

	UInt16	itemCount;				// Number of Items in Drag
	::CountDragItems(inDragRef, &itemCount);

	for (UInt16 item = 1; item <= itemCount; item++)
	{
		ItemReference	itemRef;
		::GetDragItemReferenceNumber(inDragRef, item, &itemRef);

		// Check for group in this drag - NB only groups allowed in multiple item drag
		FlavorFlags theFlags;
		if (::GetFlavorFlags(inDragRef, itemRef, cDragAddrGrpType, &theFlags) == noErr)
		{
			got_group = true;
			break;
		}
	}

	// Do not allow scroll if dragging groups
	SetDropCell(!got_group || !inDragHasLeftSender);

	// Do inherited
	CHierarchyTableDragAndDrop::EnterDropArea(inDragRef, inDragHasLeftSender);
}

// Adjust cursor over drop area
bool CGroupTable::IsCopyCursor(DragReference inDragRef)
{
	return true;
}

void CGroupTable::DoDragReceive(DragReference inDragRef)
{
	// Start fresh new/edit actions
	if (mNewAction || mEditAction || mReplaceAction)
	{
		PostAction(NULL);
		mNewAction = NULL;
		mEditAction = NULL;
		mReplaceAction = NULL;
	}

	try
	{
		// Receive any drag
		LDropArea::DoDragReceive(inDragRef);

		// Post action after all items have been added
		if (mNewAction || mReplaceAction)
		{
			// Remove highlight frame
			if (mIsHilited)
				UnhiliteDropArea(inDragRef);

			PostAction((mNewAction ? (LAction*) mNewAction : (LAction*) mReplaceAction));

			// Now add hilite again
			if (mIsHilited)
				HiliteDropArea(inDragRef);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Have to throw up since adbk window may have been destroyed if network failed
		CLOG_LOGRETHROW;
		throw;
	}

	// Always reset table
	ResetTable();
}

// Drop data into whole table
void CGroupTable::DropData(FlavorType inFlavor, char* drag_data, Size data_size)
{
	switch (inFlavor) {

		case cDragAddrGrpType:
			CGroup* theGrp = *((CGroup**) drag_data);

			// Does it already exist?
			if (!mAdbk->GetGroupList()->IsDuplicate(theGrp))
			{
				// Check for existing action
				CGroup* copy = new CGroup(*theGrp);
				if (mNewAction)
					mNewAction->AddGroup(copy);
				else
				{
					// Create new action - action owns list
					mNewAction = new CGroupTableNewAction(this, copy, mDirty);
				}
			}
			break;
	}
}

// Drop data into cell
void CGroupTable::DropDataIntoCell(FlavorType inFlavor, char* drag_data,
											Size data_size, const STableCell& inCell)
{
	bool added = false;

	// Only allow drag to group
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);

	STableCell	woCell(woRow, inCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);
	ThrowIf_(!item.mIsGroup);

	// Get the group for this cell
	CGroup* old_grp = item.mGroup;

	// Create new group if not dropped before
	CGroup* new_grp;
	if (!mReplaceAction)
		new_grp = new CGroup(*old_grp);
	else
		new_grp = mReplaceAction->GetNewGroup();

	switch (inFlavor)
	{

		case cDragMsgType:
			CMessage* theMsg = *((CMessage**) drag_data);
			CEnvelope* theEnv = theMsg->GetEnvelope();
			ThrowIfNil_(theEnv);
			CAddress* theAddr;

			// Add From
			if (theEnv->GetFrom()->size())
			{
				// Add to list
				cdstring txt = theEnv->GetFrom()->front()->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}

			// Add Reply-To
			if (theEnv->GetReplyTo()->size())
			{
				// Add to list
				cdstring txt = theEnv->GetReplyTo()->front()->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}

			// Add all To: except me
			for(CAddressList::iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				// Add to list
				cdstring txt = (*iter)->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}

			// Add all Ccs except me
			for(CAddressList::iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				// Add to list
				cdstring txt = (*iter)->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}

			break;

		case cDragAddrType:
			theAddr = *((CAddress**) drag_data);
			{
				// Add to list
				cdstring txt = theAddr->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}
			break;

		case cDragAddrGrpType:
			CGroup* theGrp = *((CGroup**) drag_data);

			// Add group to new group
			new_grp->AddAddress(theGrp->GetNickName());
			added = true;
			break;

		case kScrapFlavorTypeText:

			// Parse text into list
			CAddressList list(drag_data, data_size);

			// Try to add these to table
			for(CAddressList::iterator iter = list.begin(); iter != list.end(); iter++)
			{
				// Now add to group
				cdstring txt = (*iter)->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}
			break;
	}

	// If there was a change reset both tables
	if (added)
	{
		if (!mReplaceAction)
			mReplaceAction = new CGroupTableReplaceAction(this, old_grp, new_grp, mDirty);
	}
	else if (!mReplaceAction)
		delete new_grp;
}

// Other flavor requested by receiver
void CGroupTable::DoDragSendData(
	FlavorType		inFlavor,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
	if (inFlavor == kScrapFlavorTypeText)
	{

		// Check for single address or group
		if (mDragGroup)
		{
			cdstrvect& addr_list = ((CGroup*) inItemRef)->GetAddressList();

			cdstring txt;
			bool first = true;

			for(short i = 0; i < addr_list.size(); i++)
			{
				txt += addr_list.at(i);
				txt += '\r';
			}

			// Give drag this data
			ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeText, txt.c_str(), txt.length(), 0L));
		}
		else
		{
			// Get info for item
			TableIndexT woRow = (TableIndexT) inItemRef;

			STableCell	woCell(woRow, 1);
			SGroupTableItem item;
			UInt32 dataSize = sizeof(SGroupTableItem);
			GetCellData(woCell, &item, dataSize);

			// Determine position in group
			short pos = woRow - mCollapsableTree->GetParentIndex(woRow) - 1;

			cdstring txt = item.mGroup->GetAddressList().at(pos);

			// Check for multiple items => CR at end
			unsigned short numItems = 0;
			::CountDragItems(inDragRef, &numItems);
			if (numItems > 1)
				txt += '\r';

			// Give drag this data
			ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeText, txt.c_str(), txt.length(), 0L));
		}
	}
	else
		Throw_(badDragFlavorErr);
}

// Test drag insert cursor
bool CGroupTable::IsDropCell(DragReference inDragRef, STableCell theCell)
{
	TableIndexT woRow = GetWideOpenIndex(theCell.row);
	STableCell woCell(woRow, theCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);
	return IsValidCell(theCell) && item.mIsGroup;
}
