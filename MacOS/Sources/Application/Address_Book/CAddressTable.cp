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


// Source for CAddressTable class

#include "CAddressTable.h"

#include "CAdbkAddress.h"
#include "CAddressBook.h"
#include "CAddressBookView.h"
#include "CAddressComparators.h"
#include "CAddressList.h"
#include "CAddressTableAction.h"
#include "CAddressView.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDragIt.h"
#include "CEditAddressDialog.h"
#include "CErrorHandler.h"
#include "CGroup.h"
#include "CMessage.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CResources.h"
#include "CStringUtils.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CUserAction.h"

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S T A B L E
// __________________________________________________________________________________________________

// Static variables

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressTable::CAddressTable()
{
	InitAddressTable();
}

// Default constructor - just do parents' call
CAddressTable::CAddressTable(LStream *inStream)
		: CTableDrag(inStream)

{
	InitAddressTable();
}

// Default destructor
CAddressTable::~CAddressTable()
{
	if (mNewAction || mEditAction)
	{
		PostAction(NULL);
		mNewAction = NULL;
		mEditAction = NULL;
	}
}

// Common init
void CAddressTable::InitAddressTable()
{
	mTableView = NULL;
	mAdbk = NULL;
	mNewAction = NULL;
	mEditAction = NULL;
	mPendingEditAction = NULL;

	mTableGeometry = new CTableRowGeometry(this, 16, 16);
	mTableSelector = new CTableMultiRowSelector(this);

	mDirty = false;

	mLastTyping = 0UL;
	mLastChars[0] = '\0';

	SetRowSelect(true);

}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CAddressTable::FinishCreateSelf()
{
	// Do inherited
	CTableDrag::FinishCreateSelf();

	// Find table view in super view chain
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CAddressBookView*>(super))
		super = super->GetSuperView();
	mTableView = dynamic_cast<CAddressBookView*>(super);

	// Get text traits resource
	SetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, false);

	AddDragFlavor(cDragAddrType);
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

	CContextMenuAttachment::AddUniqueContext(this, 9004, this);
}

// Handle key presses
Boolean CAddressTable::HandleKeyPress(const EventRecord &inKeyEvent)
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
			return CTableDrag::HandleKeyPress(inKeyEvent);
		else
			DoDeleteEntry();
		break;

	default:
		// Allow typing selection from any alphanumeric character - not cmd-xx
		if (!(inKeyEvent.modifiers & cmdKey))
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
		return CTableDrag::HandleKeyPress(inKeyEvent);
	}

	return true;
}

//	Respond to commands
Boolean CAddressTable::ObeyCommand(CommandT inCommand,void *ioParam)
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
			cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
		break;

	case cmd_EditAddress:
	case cmd_ToolbarAddressBookEditBtn:
		DoEditEntry();
		break;

	case cmd_Clear:
	case cmd_DeleteAddress:
	case cmd_ToolbarAddressBookDeleteBtn:
		DoDeleteEntry();
		break;

	case cmd_ActionDeleted:
		if ((CAddressTableNewAction*) ioParam == mNewAction)
			mNewAction = NULL;
		if ((CAddressTableEditAction*) ioParam == mEditAction)
			mEditAction = NULL;
		break;

	case msg_TabSelect:
		if (!IsEnabled() || !IsVisible())
			cmdHandled = false;
		break;

	default:
		cmdHandled = CTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAddressTable::FindCommandStatus(
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
	case cmd_EditAddress:
	case cmd_ToolbarAddressBookEditBtn:
	case cmd_DeleteAddress:
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
			CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;

	default:
		CTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Make it target first
void CAddressTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);
	CTableDrag::ClickSelf(inMouseDown);
}

// Click in the cell
void CAddressTable::ClickCell(
	const STableCell&		inCell,
	const SMouseDownEvent&	inMouseDown)
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

	// Look at user action and determine what to do
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoDoubleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));
	else
		DoSingleClick(inCell.row, CKeyModifiers(inMouseDown.macEvent.modifiers));

	// Carry on to do default action
	CTableDrag::ClickCell(inCell, inMouseDown);
}

// Draw the entries
void CAddressTable::DrawCell(const STableCell& inCell,
								const Rect& inLocalRect)
{
	// Save text & color state in stack objects
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	// Make sure there's a white background for the selection InvertRect
	::EraseRect(&inLocalRect);

#if PP_Target_Carbon
	// Draw selection
	DrawCellSelection(inCell);
#endif

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Get its address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(inCell.row - 1));

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eAddrColumnName:
	case eAddrColumnNameLastFirst:
		theTxt = addr->GetName();
		break;
	
	case eAddrColumnNickName:
		theTxt = addr->GetADL();
		break;
	
	case eAddrColumnEmail:
		theTxt = addr->GetMailAddress();
		break;
	
	case eAddrColumnCompany:
		theTxt = addr->GetCompany();
		break;
	
	case eAddrColumnAddress:
		theTxt = addr->GetAddress(CAdbkAddress::eDefaultAddressType);
		break;
	
	case eAddrColumnPhoneWork:
		theTxt = addr->GetPhone(CAdbkAddress::eWorkPhoneType);
		break;
	
	case eAddrColumnPhoneHome:
		theTxt = addr->GetPhone(CAdbkAddress::eHomePhoneType);
		break;
	
	case eAddrColumnFax:
		theTxt = addr->GetPhone(CAdbkAddress::eFaxType);
		break;
	
	case eAddrColumnURLs:
		theTxt = addr->GetURL();
		break;
	
	case eAddrColumnNotes:
		theTxt = addr->GetNotes();
		break;
	
	default:
		break;
	}

	// Always draw icon in first column
	Rect drawRect = inLocalRect;
	if (inCell.col == 1)
	{
		// Plot the icon
		Rect	iconRect;
		iconRect.left = drawRect.left;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = drawRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		::Ploticns(&iconRect, atNone, ttNone, ICNx_AdbkSearchAddress);
		
		// Adjust text position
		drawRect.left += 14;
	}

	// Draw the string
	::MoveTo(drawRect.left, drawRect.bottom - mTextDescent);
	::DrawClippedStringUTF8(theTxt, drawRect.right - drawRect.left, eDrawString_Left);

}





void CAddressTable::DoSelectionChanged()
{
	CTableDrag::DoSelectionChanged();
	
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
void CAddressTable::DoSingleClick(unsigned long row, const CKeyModifiers& mods)
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
void CAddressTable::DoDoubleClick(unsigned long row, const CKeyModifiers& mods)
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

void CAddressTable::DoPreview()
{
	// Ignore if no preview pane
	if (!mTableView->GetPreview())
		return;

	CAdbkAddress* addr = NULL;

	// Only bother if single selection
	if (IsSingleSelection())
	{
		// Add each selected address
		CAddressList addrs;
		addrs.set_delete_data(false);
		DoToSelection1((DoToSelection1PP) &CAddressTable::AddEntryToList, &addrs);

		// Set to single group only
		if (addrs.size() == 1)
			addr = static_cast<CAdbkAddress*>(addrs.front());
	}

	// Only do preview if there has been a change
	if (addr != mTableView->GetPreview()->GetAddress())
	{
		// Always clear out message preview immediately, otherwise
		// do preview at idle time to prevent re-entrant network calls
		if (addr)
		{
			// Give it to preview (if its NULL the preview will be cleared)
			CAddressPreviewTask* task = new CAddressPreviewTask(mTableView->GetPreview(), mAdbk, addr);
			task->Go();
		}
		else
			// do immediate update of preview
			mTableView->GetPreview()->SetAddress(mAdbk, addr);
	}
}

// Just edit the item
void CAddressTable::DoFullView()
{
	DoEditEntry();
}

// Set the entry list
void CAddressTable::SetAddressBook(CAddressBook* adbk)
{
	// Save address list
	mAdbk = adbk;

	// Calculate number of rows for first time
	ResetTable();
}

// Reset the table from the address list
void CAddressTable::ResetTable()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	TableIndexT	old_rows;
	TableIndexT	old_cols;
	short num_addrs = 0;

	// Add cached mailboxes
	num_addrs = (mAdbk ? mAdbk->GetAddressList()->size() : 0);

	GetTableSize(old_rows, old_cols);

	if (old_rows > num_addrs)
		RemoveRows(old_rows - num_addrs, 1, false);
	else if (old_rows < num_addrs)
		InsertRows(num_addrs - old_rows, 1, NULL, 0, false);

	mTableView->UpdateCaptions();
	Refresh();

} // CAddressTable::ResetTable

// Reset the table from the address list
void CAddressTable::ClearTable()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Remove all rows
	RemoveRows(mRows, 1, true);
	
	// Update captions to empty
	Refresh();
}

// Keep titles in sync
void CAddressTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Do scroll of main table first to avoid double-refresh
	CTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);

	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);
}

// Select from key press
void CAddressTable::DoKeySelection()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	CAdbkAddress *temp;

	// Create new address for typed chars
	switch (static_cast<EAddrColumn>(mTableView->GetSortBy()))
	{
	case eAddrColumnName:
	case eAddrColumnNameLastFirst:
		temp = new CAdbkAddress(NULL, NULL, mLastChars, NULL);
		break;

	case eAddrColumnNickName:
		temp = new CAdbkAddress(NULL, NULL, NULL, mLastChars);
		break;
	
	case eAddrColumnEmail:
		temp = new CAdbkAddress(NULL, mLastChars, NULL, NULL);
		break;

	case eAddrColumnCompany:
		temp = new CAdbkAddress(NULL, NULL, NULL, NULL, mLastChars);
		break;

	default:
		return;
	}

	// Get low bound insert pos
	long select = mAdbk->GetAddressList()->FetchInsertIndexOf(temp);

	// Clip to end of list
	if (select > mAdbk->GetAddressList()->size()) select--;

	STableCell aCell(select, 2);
	UnselectAllCells();
	FakeClickSelect(aCell, false);
	ScrollCellIntoFrame(aCell, true);

	delete temp;

}

// Create new letter from selection
void CAddressTable::CreateNewLetter(bool option_key)
{
	// Create list of selected addresses
	CAddressList list;

	// Copy selection into list for active list only
	AddSelectionToList(&list);

	CReplyChooseDialog::ProcessChoice(&list,
					!(CPreferences::sPrefs->mOptionKeyAddressDialog.GetValue() ^ option_key) && (list.size() > 0));
}

// Create a new address
void CAddressTable::CreateNewEntry()
{
	std::auto_ptr<CAdbkAddress> new_addr(new CAdbkAddress);
	if (CEditAddressDialog::PoseDialog(new_addr.get()))
	{
		// Only add if some text available
		if (!new_addr->IsEmpty())
		{
			// Initiate action - copies new_addr
			mNewAction = new CAddressTableNewAction(this, new_addr.release(), mDirty);
			PostAction(mNewAction);
		}
	}

	SwitchTarget(this);
}

// Edit selected entries
void CAddressTable::DoEditEntry()
{
	// Check that there's a selection first - if not beep and exit
	if (!IsSelectionValid())
	{
		SysBeep(1);
		return;
	}

	// Create new edit action ready to store edits
	mPendingEditAction = new CAddressTableEditAction(this, mDirty);

	// Do edit for each selection
	if (TestSelectionOr((TestSelectionPP) &CAddressTable::EditEntry))
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

// Edit specified address
bool CAddressTable::EditEntry(TableIndexT row)
{
	bool done_edit = false;
	CAdbkAddress* theAddr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	// Copy original address
	std::auto_ptr<CAdbkAddress> copy(new CAdbkAddress(*theAddr));
	if (CEditAddressDialog::PoseDialog(copy.get()))
	{
		// Add info to action
		mPendingEditAction->AddEdit(theAddr, copy.release());
		done_edit = true;
	}

	SwitchTarget(this);

	return done_edit;
}

// Delete selected entries
void CAddressTable::DoDeleteEntry()
{
	// Check that there's a selection first - if not beep and exit
	if (!IsSelectionValid()) {
		SysBeep(1);
		return;
	}

	// Create list to hold deleted items
	CAddressList* addrs = new CAddressList();

	// Add each selected address
	DoToSelection1((DoToSelection1PP) &CAddressTable::AddEntryToList, addrs);

	// Initiate delete action
	PostAction(new CAddressTableDeleteAction(this, addrs, mDirty));

}

// Cut items from table
void CAddressTable::CutItems()
{
	// Copy first
	CopyItems();

	// Delete selection
	DoDeleteEntry();
}

// Copy items from table
void CAddressTable::CopyItems()
{
	// Add all selected basic addresses to text
	LHandleStream basic_txt;
	DoToSelection1((DoToSelection1PP) &CAddressTable::AddAddressText, &basic_txt);

	// Add all selected adbk addresses to text
	LHandleStream adbk_txt;
	DoToSelection1((DoToSelection1PP) &CAddressTable::AddAdbkAddressText, &adbk_txt);

	// Now copy to scrap
	{
		StHandleLocker lock1(basic_txt.GetDataHandle());
		StHandleLocker lock2(adbk_txt.GetDataHandle());

		UScrap::SetData(kScrapFlavorTypeText, basic_txt.GetDataHandle());
		UScrap::SetData(cDragAdbkAddrTextType, adbk_txt.GetDataHandle(), false);
	}
}

// Add address as text to handle
bool CAddressTable::AddAddressText(TableIndexT row, LHandleStream* txt)
{
	// Get selected address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	cdstring addr_text;

	cdstring full_addr = addr->GetFullAddress();

	// Add CR before if multi-copy
	if (txt->GetLength() > 0)
		addr_text += CR;
	addr_text += full_addr;

	txt->WriteBlock(addr_text.c_str(), addr_text.length());

	return true;
}

// Add address as text to handle
bool CAddressTable::AddAdbkAddressText(TableIndexT row, LHandleStream* txt)
{
	// Get selected address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	std::auto_ptr<const char> temp(mAdbk->ExportAddress(addr));

	txt->WriteBlock(temp.get(), ::strlen(temp.get()));

	return true;
}

// Paste items from table
void CAddressTable::PasteItems()
{
	Handle txt = ::NewHandle(0);

	if (UScrap::GetData(cDragAdbkAddrTextType, txt))
	{
		StHandleLocker lock(txt);

		// Start fresh new action
		if (mNewAction || mEditAction)
		{
			PostAction(NULL);
			mNewAction = NULL;
			mEditAction = NULL;
		}

		DropData(cDragAdbkAddrTextType, *txt, ::GetHandleSize(txt));

		// Post action after all items have been added
		if (mNewAction)
			PostAction(mNewAction);
	}
	else if (UScrap::GetData(kScrapFlavorTypeText, txt))
	{
		StHandleLocker lock(txt);

		// Start fresh new action
		if (mNewAction || mEditAction)
		{
			PostAction(NULL);
			mNewAction = NULL;
			mEditAction = NULL;
		}

		DropData(kScrapFlavorTypeText, *txt, ::GetHandleSize(txt));

		// Post action after all items have been added
		if (mNewAction)
			PostAction(mNewAction);
	}

	::DisposeHandle(txt);
}

// Add from list - not unique
void CAddressTable::AddAddressesFromList(CAddressList* addrs)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	bool added = false;

	// Clear selection first - added cells will be selected when added
	UnselectAllCells();

	// Add all addresses
	mAdbk->AddAddress(addrs, true);

	if (added)
	{

		// Reset to include new item
		ResetTable();
		UnselectAllCells();

		// Select added addresses
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			long index = mAdbk->GetAddressList()->FetchIndexOf(*iter);

			if (index)
			{
				STableCell aCell(index, 2);
				SelectCell(aCell);
			}
		}

		// Bring first item into view
		ShowFirstSelection();
		SetUpdateCommandStatus(true);
		mDirty = false;
	}
}

// Change from list
void CAddressTable::ChangeAddressesFromList(CAddressList* old_addrs, CAddressList* new_addrs)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->UpdateAddress(old_addrs, new_addrs);

	// Reset only after all changes have happened
	UnselectAllCells();
	ResetTable();

	SetUpdateCommandStatus(true);
	mDirty = false;
}

// Delete from list
void CAddressTable::RemoveAddressesFromList(CAddressList* addrs)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->RemoveAddress(addrs);

	// Reset only after all changes have happened
	UnselectAllCells();
	ResetTable();

	SetUpdateCommandStatus(true);
	mDirty = false;
}

// Add selected addresses to list
void CAddressTable::AddSelectionToList(CAddressList* list)
{
	// Add all selected address to drag
	DoToSelection1((DoToSelection1PP) &CAddressTable::CopyEntryToList, list);
}

// Copy address to list
bool CAddressTable::CopyEntryToList(TableIndexT row, CAddressList* list)
{
	// Get selected address
	CAdbkAddress* theAddr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	// Copy address
	CAdbkAddress* copy = new CAdbkAddress(*theAddr);

	// Save in selected list if not duplicate
	if (list->InsertUniqueItem(copy))
		return true;
	else
	{
		delete copy;
		return false;
	}
}

// Add address to list
bool CAddressTable::AddEntryToList(TableIndexT row, CAddressList* list)
{
	// Get selected address
	CAdbkAddress* theAddr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(row - 1));

	// Save in selected list if not duplicate
	list->push_back(theAddr);
	return true;
}

// Add address to drag
void CAddressTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& aCell, Rect& dragRect)
{
	// Get the relevant address
	CAdbkAddress* theAddr = static_cast<CAdbkAddress*>(mAdbk->GetAddressList()->at(aCell.row - 1));

	// Add this address to drag
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theAddr, cDragAddrType,
								&theAddr, sizeof(CAdbkAddress*), flavorSenderOnly, true);

	// Promise text to anyone who wants it
	theDragTask->AddFlavorItem(dragRect, (ItemReference) theAddr, kScrapFlavorTypeText, 0L, 0L, 0, false);
}

// Adjust cursor over drop area
bool CAddressTable::IsCopyCursor(DragReference inDragRef)
{
	return true;
}

void CAddressTable::DoDragReceive(DragReference inDragRef)
{
	// Start fresh new action
	if (mNewAction || mEditAction)
	{
		PostAction(NULL);
		mNewAction = NULL;
		mEditAction = NULL;
	}

	try
	{
		// Receive any drag
		LDropArea::DoDragReceive(inDragRef);

		// Post action after all items have been added
		if (mNewAction)
		{
			// Remove highlight frame
			if (mIsHilited)
				UnhiliteDropArea(inDragRef);

			PostAction(mNewAction);

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

// Drop data into table
void CAddressTable::DropData(FlavorType theFlavor, char* drag_data, Size data_size)
{
	bool added = false;
	CAddressList* new_addrs = new CAddressList;

	switch (theFlavor)
	{
	case cDragMsgType:
		CMessage* theMsg = *((CMessage**) drag_data);
		CEnvelope* theEnv = theMsg->GetEnvelope();
		ThrowIfNil_(theEnv);

		// Add From
		if (theEnv->GetFrom()->size())
		{
			CAddress* theAddr = (*theEnv->GetFrom()).front();

			// Check duplicate and smart address
			if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
				!mAdbk->GetAddressList()->IsDuplicate(theAddr))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(*theAddr);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}

		// Add Reply-To
		if (theEnv->GetReplyTo()->size())
		{
			CAddress* theAddr = (*theEnv->GetReplyTo()).front();

			// Check duplicate and smart address
			if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
				!mAdbk->GetAddressList()->IsDuplicate(theAddr))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(*theAddr);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}

		// Add all To
		for(CAddressList::const_iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
		{
			// Check duplicate and smart address
			if (!CPreferences::sPrefs->TestSmartAddress(**iter) &&
				!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}

		// Add all Ccs
		for(CAddressList::const_iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
		{
			// Check duplicate and smart address
			if (!CPreferences::sPrefs->TestSmartAddress(**iter) &&
				!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}
		break;

	case cDragAddrType:
		CAddress* theAddr = *((CAddress**) drag_data);

		// Check duplicate
		if(!mAdbk->GetAddressList()->IsDuplicate(theAddr))
		{
			// Add to list
			CAdbkAddress* copy = new CAdbkAddress(*theAddr);
			added = new_addrs->InsertUniqueItem(copy);
			if (!added)
				delete copy;
		}
		break;

	case cDragAddrGrpType:
		CGroup* theGrp = *((CGroup**) drag_data);

		// Copy all addresses and add to list
		for(short i = 0; i < theGrp->GetAddressList().size(); i++)
		{
			// Check duplicate
			CAdbkAddress* copy = new CAdbkAddress(CAddress(theGrp->GetAddressList().at(i)));
			if (!mAdbk->GetAddressList()->IsDuplicate(copy))
			{
				// Add to list
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
			else
				delete copy;
		}
		break;

	case cDragAdbkAddrTextType:
	{
		// Parse text into list
		CAddressList list;
		unsigned long size = 0;
		char* line_start = drag_data;
		char* line_end = drag_data;
		char* data_end = drag_data + data_size;
		while(line_end < data_end)
		{
			// Skip CR,LF
			while((line_end < data_end) && ((*line_end == '\r') || (*line_end == '\n')))
				line_end++;
			line_start = line_end;
			
			// Skip to line end or end of data
			while((line_end < data_end) && (*line_end != '\r') && (*line_end != '\n'))
				line_end++;
			
			// Import line data
			if (line_start < line_end)
			{
				CAdbkAddress* addr = NULL;
				cdstring temp(line_start, line_end - line_start);
				mAdbk->ImportAddress(temp.c_str_mod(), false, &addr, NULL);
				if (addr)
					list.push_back(addr);
			}
		}

		// Try to add these to table
		for(CAddressList::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			// Check duplicate
			if (!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}
		break;
	}

	case kScrapFlavorTypeText:
	{
		// Parse text into list
		CAddressList list(drag_data, data_size);

		// Try to add these to table
		for(CAddressList::const_iterator iter = list.begin(); iter != list.end(); iter++)
		{
			// Check duplicate
			if (!mAdbk->GetAddressList()->IsDuplicate(*iter))
			{
				// Add to list
				CAdbkAddress* copy = new CAdbkAddress(**iter);
				if (new_addrs->InsertUniqueItem(copy))
					added = true;
				else
					delete copy;
			}
		}
		break;
	}
	}

	if (added)
	{
		// Check for existing action
		if (mNewAction)
		{
			mNewAction->AddAddresses(new_addrs);

			// List items used by action
			new_addrs->clear_without_delete();
			delete new_addrs;
		}
		else
		{
			// Create new action - action owns list
			mNewAction = new CAddressTableNewAction(this, new_addrs, mDirty);
		}
	}
	else
		delete new_addrs;
}

// Other flavor requested by receiver
void CAddressTable::DoDragSendData(
	FlavorType		inFlavor,
	ItemReference	inItemRef,
	DragReference	inDragRef)
{
	if (inFlavor == kScrapFlavorTypeText)
	{
		LHandleStream txt;

		cdstring full_addr = reinterpret_cast<CAdbkAddress*>(inItemRef)->GetFullAddress();

		// Check for multiple items => CR at end
		unsigned short numItems = 0;
		::CountDragItems(inDragRef, &numItems);
		if (numItems > 1)
			full_addr += '\r';

		// Give drag this data
		ThrowIfOSErr_(::SetDragItemFlavorData(inDragRef, inItemRef, kScrapFlavorTypeText, full_addr.c_str(), full_addr.length(), 0L));
	}
	else
		Throw_(badDragFlavorErr);
}
