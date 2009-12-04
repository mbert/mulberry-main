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


// Source for CServerTable class

#include "CServerTable.h"

#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDragIt.h"
#include "CDragTasks.h"
#include "CMailAccountManager.h"
#include "CMailboxInfoTable.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "COptionClick.h"
#include "CServerTitleTable.h"
#include "CServerViewPopup.h"
#include "CServerView.h"
#include "CServerWindow.h"

#include <stdio.h>
#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerTable::CServerTable()
{
	InitServerTable();
}

// Default constructor - just do parents' call
CServerTable::CServerTable(LStream *inStream)
	: CServerBrowse(inStream)
{
	InitServerTable();
}

// Default destructor
CServerTable::~CServerTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CServerTable::InitServerTable(void)
{
}

// Get details of sub-panes
void CServerTable::FinishCreateSelf(void)
{
	// Do inherited
	CServerBrowse::FinishCreateSelf();

	// Find table view in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CServerView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CServerView*>(super);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	// Can drag servers (manager only), mboxes & mboxrefs (manager only)
	AddDragFlavor(cDragWDType);
	AddDragFlavor(cDragMboxType);

	// Can drop servers, mboxes, mboxrefs (manager only) & messages
	AddDropFlavor(cDragServerType);
	AddDropFlavor(cDragWDType);
	AddDropFlavor(cDragMboxType);
	AddDropFlavor(cDragMsgType);

	// Set read only status of Drag and Drop
	SetDDReadOnly(false);
	SetDropCell(true);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetAllowMove(true);
	SetSelfDrag(true);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Make it fit to the superview
	AdaptToNewSurroundings();
}

// Handle key presses
Boolean CServerTable::HandleKeyPress(const EventRecord &inKeyEvent)
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
	// Go to next new mailbox
	case char_Tab:
		SelectNextRecent((inKeyEvent.modifiers & shiftKey) != 0);
		break;

	// Toggle delete
	case char_Backspace:
	case char_Clear:
		// Special case escape key
		if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
			return CServerBrowse::HandleKeyPress(inKeyEvent);
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer))
			DoLogoffServers();
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionHierarchy))
			//DoDeleteHierarchy();
			;	// Do nothing - force use of menu commands
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemove))
			DoRemoveFavourite();
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemoveItems))
			DoRemoveFavouriteItems();
		break;

	default:
		return CServerBrowse::HandleKeyPress(inKeyEvent);
	}

	return true;
}

//	Respond to commands
Boolean CServerTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_ToolbarServerLoginBtn:
		if (!*(long*) ioParam)
		{
			// Look for selected server cell
			if (mManager && !mSingle)
			{
				// Get selected cell
				TableIndexT row = GetFirstSelectedRow();
				TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

				// Logon to appropriate server
				LogonAction(GetCellServer(woRow));
			}

			// Got single server window
			else
				LogonAction(GetSingleServer());
		}
		else
		{
			// Look for selected server cell
			if (mManager && !mSingle)
			{
				// Get selected cell
				TableIndexT row = GetFirstSelectedRow();
				TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

				// Logoff to appropriate server
				LogoffAction(GetCellServer(woRow));
			}

			// Got single server window
			else
				LogoffAction(GetSingleServer());
		}

		// Always reset state
		UpdateState();
		break;

	case cmd_NewLetter:
		DoNewLetter(COptionMenu::sOptionKey);
		break;
	case cmd_ToolbarNewLetter:
		DoNewLetter(false);
		break;
	case cmd_ToolbarNewLetterOption:
		DoNewLetter(true);
		break;

	case cmd_Open:
		// Open a mailbox from its name on this server
		DoChooseMailbox();
		break;

	case cmd_Properties:
	case cmd_ToolbarDetailsBtn:
		if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer))
			DoServerProperties();
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAll))
			DoMailboxProperties();
		break;

	case cmd_CreateMailbox:
	case cmd_ToolbarCreateMailboxBtn:
		DoCreateMailbox();
		break;

	case cmd_RenameMailbox:
		DoRenameMailbox();
		break;

	case cmd_PuntUnseen:
		DoPuntUnseenMailbox();
		break;

	case cmd_DeleteMailbox:
		DoDeleteMailbox();
		break;

	case cmd_CheckMail:
		DoCheckMail();
		break;

	case cmd_ToolbarCheckMailboxBtn:
		CMulberryApp::sApp->ObeyCommand(cmd_AutoCheckMail);
		break;

	case cmd_NewHierarchy:
		DoNewHierarchy();
		break;

	case cmd_EditHierarchy:
		DoRenameHierarchy();
		break;

	case cmd_DeleteHierarchy:
		DoDeleteHierarchy();
		break;

	case cmd_ResetHierarchy:
		DoResetHierarchy();
		break;

	case cmd_RefreshHierarchy:
		DoRefreshHierarchy();
		break;

	case cmd_NewCabinet:
		DoNewFavourite();
		break;

	case cmd_EditCabinet:
		DoRenameFavourite();
		break;

	case cmd_DeleteCabinet:
		DoRemoveFavourite();
		break;

	case cmd_ToolbarCabinetPopup:
		switch(*(long*) ioParam)
		{
		case CServerViewPopup::eServerView_New:
			DoNewFavourite();
			break;
		default:
			SetView((*(long*) ioParam) - CServerViewPopup::eServerView_First);
			break;
		}
		break;

	case cmd_AddWildcardItem:
		DoNewWildcardItem();
		break;

	case cmd_EditWildcardItem:
		DoRenameWildcardItem();
		break;

	case cmd_RemoveItem:
		DoRemoveFavouriteItems();
		break;

	case cmd_SynchroniseMailbox:
		DoSynchroniseMailbox();
		break;

	case cmd_ClearDisconnectMailbox:
		DoClearDisconnectMailbox();
		break;

	case cmd_SubscribeMailbox:
		DoSubscribeMailbox();
		break;

	case cmd_UnsubscribeMailbox:
		DoUnsubscribeMailbox();
		break;

	case cmd_SearchMailbox:
	case cmd_ToolbarSearchMailboxBtn:
		OnSearchMailbox();
		break;

	case cmd_SearchAgainMailbox:
		OnSearchAgainMailbox();
		break;

	default:
		cmdHandled = CServerBrowse::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CServerTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_ToolbarServerLoginBtn:
		if (mManager)
		{
			// Logon button must have single server selected
			if (mSingle || TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer) && IsSingleSelection())
			{
				CMboxProtocol* proto = NULL;

				if (mSingle)
					proto = GetSingleServer();
				else
				{
					TableIndexT row = GetFirstSelectedRow();
					TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
					proto = GetCellServer(woRow);
				}

				outEnabled = true;
				outUsesMark = true;
				outMark = (proto && proto->IsLoggedOn()) ? (UInt16)checkMark : (UInt16)noMark;
				::GetIndString(outName, STRx_Standards, !outMark ? str_Logon : str_Logoff);
			}
			else
			{
				outEnabled = false;
				outUsesMark = false;
				::GetIndString(outName, STRx_Standards, str_Logon);
			}
		}
		else
		{
			outEnabled = true;
			outUsesMark = true;
			outMark = (GetSingleServer() && GetSingleServer()->IsLoggedOn()) ? (UInt16)checkMark : (UInt16)noMark;
			::GetIndString(outName, STRx_Standards, !outMark ? str_Logon : str_Logoff);
		}
		break;

	// These ones must have a logged on IMAPClient
	case cmd_Open:
		outEnabled = (!mSingle ? CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->HasOpenProtocols() :
								 GetSingleServer() && GetSingleServer()->IsLoggedOn());
		break;

	case cmd_CreateMailbox:
	case cmd_ToolbarCreateMailboxBtn:
		outEnabled = CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->HasOpenProtocols();
		break;

	// These ones must have a logged on IMAPClient and a selection
	case cmd_Properties:
	case cmd_ToolbarDetailsBtn:
		outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer) ||
						TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAll) ||
						mSingle && !IsSelectionValid();
		break;

	case cmd_ToolbarCabinetPopup:
	case cmd_ToolbarCheckMailboxBtn:
		outEnabled = true;
		break;

	case cmd_RenameMailbox:
	case cmd_PuntUnseen:
	case cmd_SubscribeMailbox:
	case cmd_UnsubscribeMailbox:
	case cmd_CheckMail:
	case cmd_SearchAgainMailbox:
		// Must be mailboxes on a logged in server
		outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAvailable);
		break;

	case cmd_SynchroniseMailbox:
		outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxDisconnected);
		break;

	case cmd_ClearDisconnectMailbox:
		outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxClearDisconnected);
		break;

	case cmd_DeleteMailbox:
		outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestDSelectionMbox);
		break;

	case cmd_SearchMailbox:
	case cmd_ToolbarSearchMailboxBtn:
		outEnabled = CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->HasOpenProtocols();
		break;

	case cmd_Hierarchy:
	case cmd_NewHierarchy:
		// Always able to do this, even if logged off or no selection
		outEnabled = true;
		break;

	case cmd_EditHierarchy:
	case cmd_DeleteHierarchy:
		outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionHierarchy);
		break;

	case cmd_ResetHierarchy:
		if (IsSelectionValid())
			outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionResetRefresh);
		else
			outEnabled = mSingle;
		break;

	case cmd_RefreshHierarchy:
		if (IsSelectionValid())
			outEnabled = TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionResetRefresh);
		else
			outEnabled = mSingle && GetSingleServer() && GetSingleServer()->FlatWD();
		break;

	case cmd_Cabinets:
	case cmd_NewCabinet:
		outEnabled = mManager;
		break;

	case cmd_EditCabinet:
	case cmd_DeleteCabinet:
		outEnabled = mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemove);
		break;

	case cmd_AddWildcardItem:
		outEnabled = mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteWildcard);
		break;

	case cmd_EditWildcardItem:
		outEnabled = mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteWildcardItems);
		break;

	case cmd_RemoveItem:
		outEnabled = mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemoveItems);
		break;

	default:
		CServerBrowse::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Respond to clicks in the icon buttons
void CServerTable::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch(inMessage)
	{
	case msg_ServerFlatHierarchy:
		SetFlatHierarchy(*(long*) ioParam);
		break;
	}
}

// Click in the cell
void CServerTable::ClickCell(const STableCell& inCell,
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
	if (event_match && (inCell.col == mHierarchyCol) &&
		DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey))) &&
			 ValidDragSelection())
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

			// Now execute the drag task
			CDragTask::Execute();

			// Force refresh of selection
			RefreshSelection();

			return;
		}
	}

	// Look at user action and determine what to do
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoDoubleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));
	else
		DoSingleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));

	// Do inherited
	CServerBrowse::ClickCell(inCell, inMouseDown);
}

void CServerTable::DoSelectionChanged(void)
{
	// Update buttons

	// Flat/hierarchic button if only one hierarchy selected
	// NB This button is always present whether we are in a 1-pane or 3-pane window
	if (IsSingleSelection())
	{

		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

		if ((GetCellDataType(woRow) == eServerBrowseWD) ||
			(GetCellDataType(woRow) == eServerBrowseSubs))
		{
			mTableView->GetHierarchyBtn()->Enable();
			mTableView->GetHierarchyBtn()->StopBroadcasting();
			mTableView->GetHierarchyBtn()->SetValue(GetCellMboxList(woRow)->IsHierarchic());
			mTableView->GetHierarchyBtn()->StartBroadcasting();
		}
		else if (GetCellDataType(woRow) == eServerBrowseMboxRefList)
		{
			mTableView->GetHierarchyBtn()->Enable();
			mTableView->GetHierarchyBtn()->StopBroadcasting();
			mTableView->GetHierarchyBtn()->SetValue(GetCellMboxRefList(woRow)->IsHierarchic());
			mTableView->GetHierarchyBtn()->StartBroadcasting();
		}
		else
			mTableView->GetHierarchyBtn()->Disable();
	}
	else if (mSingle && GetSingleServer()->FlatWD())
	{
		mTableView->GetHierarchyBtn()->Enable();
		mTableView->GetHierarchyBtn()->StopBroadcasting();
		mTableView->GetHierarchyBtn()->SetValue(GetSingleServer()->GetHierarchies().at(1)->IsHierarchic());
		mTableView->GetHierarchyBtn()->StartBroadcasting();
	}
	else
		mTableView->GetHierarchyBtn()->Disable();

	// Do inherited to broadcast change
	CServerBrowse::DoSelectionChanged();
	
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSelection())
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFullView();
}

// Update status items like toolbar
void CServerTable::UpdateState()
{
	// Force toolbar refresh
	mTableView->RefreshToolbar();
}


SColumnInfo	CServerTable::GetColumnInfo(TableIndexT col)
{
	return mTableView->GetColumnInfo()[col - 1];
}

#pragma mark ____________________________Visual State

// Keep titles in sync
void CServerTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CServerBrowse::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

#pragma mark ____________________________Drag & Drop

// Check for valid drag selection
bool CServerTable::ValidDragSelection(void) const
{
	// Loop over all selected cells
	int got_favourite = 0;
	int got_wd = 0;
	int got_server = 0;
	int got_mbox = 0;
	int got_mboxref = 0;
	CMboxProtocol* proto_wd = NULL;
	STableCell selCell(0, 0);
	while(GetNextSelectedCell(selCell))
	{
		unsigned long woRow = GetWideOpenIndex(selCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);

		switch(type)
		{
		case eServerBrowseMbox:
			got_mbox = 1;
			break;
		case eServerBrowseMboxRef:
			got_mboxref = 1;
			break;
		case eServerBrowseWD:
			// Check same server
			if (!got_wd)
			{
				got_wd = 1;
				proto_wd = ResolveCellServer(woRow);
			}
			else if (ResolveCellServer(woRow) != proto_wd)
				return false;
			break;
		case eServerBrowseSubs:
			// Never drag subscribed
			return false;
#if 0
		case eServerBrowseMboxRefList:
			got_favourite = 1;

			// Can only drag non internal
			if (CMailAccountManager::sMailAccountManager->GetFavouriteType(GetCellMboxRefList(woRow)) < CMailAccountManager::eFavouriteOthers)
				return false;
			break;
#endif
		case eServerBrowseServer:
			got_server = 1;
			break;
		default:
			return false;
		}

		// Can only have one type
		if (got_favourite + got_wd + got_server + got_mbox + got_mboxref > 1)
			return false;
	}

	return true;
}

// Add drag cells
void CServerTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& theCell, Rect& dragRect)
{
	// Get the relevant attachment
	unsigned long woRow = GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
	const void* data;
	UInt32 dataSize = sizeof(void*);
	FlavorType flavor;
	FlavorFlags flags = 0;

	// Assume drag only starts when selection is all the same type
	// Thus type of this cell is type of all cells
	EServerBrowseDataType type = GetCellDataType(woRow);

	switch(type)
	{
	case eServerBrowseServer:
		// Dragging server to another location
		data = GetCellServer(woRow);
		flavor = cDragServerType;
		break;
	case eServerBrowseWD:
		// Dragging WD to new location
		data = GetCellMboxList(woRow);
		flavor = cDragWDType;
		flags = flavorSenderOnly;	// Only move WD within its server window
		break;
	case eServerBrowseMbox:
		// Dragging mailbox
		data = GetCellMbox(woRow);
		flavor = cDragMboxType;
		break;
	case eServerBrowseMboxRef:
		// Dragging mailbox ref
		data = GetCellMboxRef(woRow);
		flavor = cDragMboxRefType;
		flags = flavorSenderOnly;	// Only move mbox ref within its server window
		break;
	default:
		// Anything else is illegal!
		return;
	}

	// Add this message to drag
	if (data != NULL)
		theDragTask->AddFlavorItem(dragRect, (ItemReference) data, flavor, &data, dataSize, flags, true);
}

// Can cell expand for drop
bool CServerTable::CanDropExpand(DragReference inDragRef, unsigned long woRow)
{
	// Get flavor for this item
	ItemReference theItemRef;
	FlavorType theFlavor;
	if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
		return false;
	if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
		return false;

	switch(theFlavor)
	{
	case cDragMsgType:
	case cDragMboxType:
	case cDragMboxRefType:
		return CServerBrowse::CanDropExpand(inDragRef, woRow);
	default:
		// Anything else cannot
		return false;
	}
}

// Adjust cursor over drop area
bool CServerTable::IsCopyCursor(DragReference inDragRef)
{
	// Get flavor for this item
	ItemReference theItemRef;
	FlavorType theFlavor;
	if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
		return false;
	if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
		return false;

	switch(theFlavor)
	{
	case cDragMsgType:
	{
		// Toggle based on modifiers
		short mouseModifiers;
		::GetDragModifiers(inDragRef, &mouseModifiers, NULL, NULL);
		bool option_key = mouseModifiers & optionKey;

		return !(CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key);
	}
	case cDragServerType:
	case cDragWDType:
		// Server & WDs always moved
		return false;
	case cDragMboxType:
	case cDragMboxRefType:
		// Mailboxes & references moved if drop at or no drop cell
		if (mLastDropCursor.row || !mLastDropCell.row)
			return false;

		// Must check cell dropped into to see if directory
		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(mLastDropCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);

		switch(type)
		{
		case eServerBrowseMbox:
			return (GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->IsDirectory();
		default:
			return true;
		}

	default:
		// Anything else is an error!
		return false;
	}
}

// Test drag insert cursor
bool CServerTable::IsDropCell(DragReference inDragRef, STableCell theCell)
{
	if (IsValidCell(theCell))
	{
		// Get flavor for this item
		ItemReference theItemRef;
		FlavorType theFlavor;
		Size dataSize = 4;
		void* data = NULL;
		if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
			return false;
		if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
			return false;
		if (::GetFlavorData(inDragRef, theItemRef, theFlavor, &data, &dataSize, 0) != noErr)
			return false;

		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);

		switch(theFlavor)
		{
		case cDragMsgType:
		{
			// Drop into valid mailboxes only
			switch(type)
			{
			case eServerBrowseMbox:
				return (GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->IsDirectory();
			case eServerBrowseMboxRef:
				return (GetCellMboxRef(woRow) != NULL) && !GetCellMboxRef(woRow)->IsDirectory();
			default:
				return false;
			}
		}
		case cDragServerType:
		case cDragWDType:
			// Servers & WDs always moved
			return false;
		case cDragMboxType:
		{
			switch(type)
			{
			case eServerBrowseMbox:
				// Allow drop into any mailbox (directory = move, mbox = copy)
				return true;
			case eServerBrowseSubs:
				// Allow drop if same server
				CMbox* src = (CMbox*) data;
				return (src->GetProtocol() == GetCellMboxList(woRow)->GetProtocol());
			case eServerBrowseMboxRefList:
				// Allow drop into editable lists
				CMailAccountManager::EFavourite fav = CMailAccountManager::sMailAccountManager->GetFavouriteType(GetCellMboxRefList(woRow));
				return (fav != CMailAccountManager::eFavouriteNew);
			default:
				return false;
			}
		}
		case cDragMboxRefType:
		{
			// Drop into any other ref list
			switch(type)
			{
			case eServerBrowseMboxRefList:
				// Allow drop into favourite list
				return true;
			default:
				return false;
			}
		}
		default:
			// Anything else is an error!
			return false;
		}
	}
	else
		return false;
}

// Test drop at cell
bool CServerTable::IsDropAtCell(DragReference inDragRef, STableCell& aCell)
{
	// Adjust for end of table
	int adjust = 0;
	if (aCell.row > mRows)
		adjust = 1;

	if (IsValidCell(aCell) || adjust)
	{
		// Get flavor for this item
		ItemReference theItemRef;
		FlavorType theFlavor;
		Size dataSize = 4;
		void* data = NULL;
		if (::GetDragItemReferenceNumber(inDragRef, 1, &theItemRef) != noErr)
			return false;
		if (::GetFlavorType(inDragRef, theItemRef, 1, &theFlavor) != noErr)
			return false;
		if (::GetFlavorData(inDragRef, theItemRef, theFlavor, &data, &dataSize, 0) != noErr)
			return false;

		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(aCell.row + TABLE_ROW_ADJUST - adjust);
		EServerBrowseDataType type = GetCellDataType(woRow);

		switch(theFlavor)
		{
		case cDragMsgType:
		{
			// Always drop into, never at
			return false;
		}
		case cDragServerType:
			// Must be same window
			if (sTableSource != this)
				return false;

			// Server can only be dropped before/after server in the same window
			if (type == eServerBrowseServer)
				return true;

			// Beyond last row allowed
			if (adjust)
				return true;

			// Check for row immediately after server
			if (woRow > 1)
			{
				EServerBrowseDataType prev_type = GetCellDataType(woRow - 1 + adjust);
				return (prev_type == eServerBrowseServer);
			}
			return false;
		case cDragWDType:
		{
			// Must be same window
			if (sTableSource != this)
				return false;

			CMboxList* src = (CMboxList*) data;

			// Check for same server of previous row
			if (src->GetProtocol() != ResolveCellServer(woRow - 1 + adjust))
				return false;

			// Allow if next cell is WD (or next server)
			if ((type == eServerBrowseWD) || (type == eServerBrowseServer))
				return true;

			// WD can only be dropped at same server in the same window
			return false;
		}
		case cDragMboxType:
		{
			switch(type)
			{
			case eServerBrowseMbox:
				// Allow drop at any mailbox that is not INBOX and not in subscribed hierarchy
				if (IsCellINBOX(woRow))
					return false;
				else
					return (GetCellMboxList(woRow)->GetHierarchyIndex() > 0);
			default:
				return false;
			}
		}
		case cDragMboxRefType:
		{
			// Can only drop into favourites
			return false;
		}
		default:
			// Anything else is an error!
			return false;
		}
	}
	else
		return false;
}

// Drop data into cell
void CServerTable::DropData(FlavorType theFlavor, char* drag_data, Size data_sizel)
{
	switch(theFlavor)
	{
	case cDragServerType:
	{
		// Make this window show this server if not manager
		if (!mManager)
			new CServerDragOpenServerTask(this, *(CMboxProtocol**) drag_data);
		break;
	}
	default:;
	}

}

// Drop data into cell
void CServerTable::DropDataIntoCell(FlavorType theFlavor, char* drag_data,
										Size data_size, const STableCell& theCell)
{
	switch(theFlavor)
	{
	case cDragMsgType:
	{
		CMessage* theMsg = *((CMessage**) drag_data);

		CDragMessagesTask* task = dynamic_cast<CDragMessagesTask*>(CDragTask::GetCurrentDragTask());
		if (task == NULL)
		{
			// Determine mbox to use for copy
			CMbox* to = NULL;
			TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
			if (GetCellDataType(woRow) == eServerBrowseMbox)
				to = GetCellMbox(woRow);
			else if (GetCellDataType(woRow) == eServerBrowseMboxRef)
				to = GetCellMboxRef(woRow)->ResolveMbox(true);

			CMbox* from = theMsg->GetMbox();

			if (to != NULL)
				task = new CDragMessagesTask(to, from);
		}
		
		if (task != NULL)
			task->AddMessage(theMsg);
			
		break;
	}
	case cDragWDType:
	{
		break;
	}
	case cDragMboxType:
	{
		CMbox* mbox = *(CMbox**) drag_data;

		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);

		switch(type)
		{
		case eServerBrowseMbox:
			{
				CDragMailboxIntoMailboxTask* task = dynamic_cast<CDragMailboxIntoMailboxTask*>(CDragTask::GetCurrentDragTask());
				if (task == NULL)
				{
					CMbox* destination = GetCellMbox(woRow);

					if (destination != NULL)
						task = new CDragMailboxIntoMailboxTask(destination);
				}
				if (task != NULL)
					task->AddMbox(mbox);
			}
			break;
		case eServerBrowseMboxRefList:
			{
				CDragMailboxToCabinetTask* task = dynamic_cast<CDragMailboxToCabinetTask*>(CDragTask::GetCurrentDragTask());
				if (task == NULL)
				{
					CMboxRefList* ref_list = GetCellMboxRefList(woRow);

					task = new CDragMailboxToCabinetTask(ref_list);
				}
				task->AddMbox(mbox);
			}
			break;
		case eServerBrowseSubs:
			{
				CDragMailboxSubscribeTask* task = dynamic_cast<CDragMailboxSubscribeTask*>(CDragTask::GetCurrentDragTask());
				if (task == NULL)
					task = new CDragMailboxSubscribeTask;
				task->AddMbox(mbox);
			}
			break;
		default:;
		}

		break;
	}
	case cDragMboxRefType:
	{
		CMboxRef* mboxref = *(CMboxRef**) drag_data;

		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);

		switch(type)
		{
		case eServerBrowseMboxRefList:
			{
				CDragMailboxRefToCabinetTask* task = dynamic_cast<CDragMailboxRefToCabinetTask*>(CDragTask::GetCurrentDragTask());
				if (!task)
				{
					CMboxRefList* ref_list = GetCellMboxRefList(woRow);

					task = new CDragMailboxRefToCabinetTask(ref_list);
				}
				task->AddMboxRef(mboxref);
			}
			break;
		default:;
		}

		break;
	}
	default:;
	}

}

// Drop data at cell
void CServerTable::DropDataAtCell(FlavorType theFlavor,
										char* drag_data,
										Size data_size,
										const STableCell& beforeCell)
{
	// Adjust for end of table
	int adjust = 0;
	if (beforeCell.row > mRows)
		adjust = 1;

	// Get drop cell type
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(beforeCell.row + TABLE_ROW_ADJUST - adjust);
	EServerBrowseDataType type = GetCellDataType(woRow);

	switch(theFlavor)
	{
	case cDragServerType:
	{
		CDragServerTask* task = dynamic_cast<CDragServerTask*>(CDragTask::GetCurrentDragTask());
		if (!task)
		{
			long dropat = -1L;
			if (adjust)
				dropat = CMailAccountManager::sMailAccountManager->GetProtocolCount();
			else
			{
				CMboxProtocol* above = GetCellServer(woRow);
				if (type == eServerBrowseServer)
					dropat = CMailAccountManager::sMailAccountManager->GetProtocolIndex(above);
				else
					dropat = CMailAccountManager::sMailAccountManager->GetProtocolIndex(above) + 1;
			}
			task = new CDragServerTask(dropat);
		}

		CMboxProtocol* moved = *(CMboxProtocol**) drag_data;
		long old_index = CMailAccountManager::sMailAccountManager->GetProtocolIndex(moved);
		task->AddIndex(old_index);
		break;
	}

	case cDragWDType:
	{
		CMboxList* list = *(CMboxList**) drag_data;

		switch(type)
		{
		case eServerBrowseWD:
		case eServerBrowseServer:
		{
			CDragWDTask* task = dynamic_cast<CDragWDTask*>(CDragTask::GetCurrentDragTask());
			if (!task)
			{
				long dropat = -1L;
				if (!adjust && (type == eServerBrowseWD))
				{
					CMboxList* drop_list = GetCellMboxList(woRow);
					unsigned long old_index = list->GetHierarchyIndex();
					dropat = (drop_list != list) ? drop_list->GetHierarchyIndex() : old_index;
				}
				else
					dropat = list->GetProtocol()->GetHierarchies().size();

				task = new CDragWDTask(dropat);
			}
			task->AddMboxList(list);
			break;
		}
		default:;
		}

		break;
	}
	case cDragMboxType:
	{
		CMbox* mbox = *(CMbox**) drag_data;

		switch(type)
		{
		case eServerBrowseMbox:
			{
				CDragMailboxAtMailboxTask* task = dynamic_cast<CDragMailboxAtMailboxTask*>(CDragTask::GetCurrentDragTask());
				if (task == NULL)
				{
					CMbox* destination = GetCellMbox(woRow);
					if (destination != NULL)
					{
						CMboxList* list = GetCellMboxList(woRow);
						CMbox* parent = (CMbox*) list->GetParentNode(destination);

						task = new CDragMailboxAtMailboxTask(parent, destination);
					}
				}
				if (task != NULL)
					task->AddMbox(mbox);
			}
			break;
		default:;
		}

		break;
	}
	default:;
	}

}
