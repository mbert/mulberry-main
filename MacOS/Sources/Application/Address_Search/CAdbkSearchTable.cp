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


// Source for CAdbkSearchTable class

#include "CAdbkSearchTable.h"

#include "CAdbkSearchTitleTable.h"
#include "CAdbkSearchWindow.h"
#include "CAdbkServerPopup.h"
#include "CBetterScrollerX.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CDragIt.h"
#include "CEditAddressDialog.h"
#include "CErrorHandler.h"
#include "CLog.h"
#include "COptionClick.h"
#include "CReplyChooseDialog.h"
#include "CResources.h"
#include "CTableMultiRowSelector.h"
#include "CTableRowGeometry.h"
#include "CTextFieldX.h"
#include "CWaitCursor.h"

#include <LDropFlag.h>
#include <LBevelButton.h>
#include <LPopupButton.h>
#include <LNodeArrayTree.h>

#include <UGAColorRamp.h>


#include <stdio.h>
#include <string.h>

const short cTextIndent = 16;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchTable::CAdbkSearchTable()
{
	InitAdbkSearchTable();
}

// Default constructor - just do parents' call
CAdbkSearchTable::CAdbkSearchTable(LStream *inStream)
	: CHierarchyTableDrag(inStream)
{
	InitAdbkSearchTable();
}

// Default destructor
CAdbkSearchTable::~CAdbkSearchTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CAdbkSearchTable::InitAdbkSearchTable(void)
{
	mTableGeometry = new CTableRowGeometry(this, mFrameSize.width, 18);
	mTableSelector = new CTableMultiRowSelector(this);

	mLastParent = 0;
}

// Get details of sub-panes
void CAdbkSearchTable::FinishCreateSelf(void)
{
	// Do inherited
	CHierarchyTableDrag::FinishCreateSelf();

	// Find window in super view chain
	mWindow = (CAdbkSearchWindow*) mSuperView;
	while(mWindow->GetPaneID() != paneid_AdbkSearchWindow)
		mWindow = (CAdbkSearchWindow*) mWindow->GetSuperView();

	// Link controls to this table
	UReanimator::LinkListenerToBroadcasters(this, mWindow, RidL_CAdbkSearchTableBtns);

	// Get lookup items
	mMethod = (LPopupButton*) mWindow->FindPaneByID(paneid_AdbkSearchMethod);
	mServer = (CAdbkServerPopup*) mWindow->FindPaneByID(paneid_AdbkSearchServer);
	mServer->SyncMenu();
	mLookup = (CTextFieldX*) mWindow->FindPaneByID(paneid_AdbkSearchField);
	mLookup->AddListener(this);
	mLookup->SetBroadcastReturn(true);
	mMatch = (LPopupButton*) mWindow->FindPaneByID(paneid_AdbkSearchMatch);
	mMatch->SetValue(CAdbkAddress::eMatchAnywhere);
	mCriteria = (LPopupButton*) mWindow->FindPaneByID(paneid_AdbkSearchPopup);

	mLDAPView = (LView*) mWindow->FindPaneByID(paneid_AdbkSearchLDAPView);
	mAdbkView = (LView*) mWindow->FindPaneByID(paneid_AdbkSearchAdbkView);

	LBevelButton* aBtn = (LBevelButton*) mWindow->FindPaneByID(paneid_AdbkSearchMessageBtn);
	aBtn->AddAttachment(new COptionClick);

	// Set Drag & Drop pane to scroller
	mPane = GetSuperView();

	// Set Drag & Drop info
	SetTable(this, true);

	AddDragFlavor(cDragAddrType);
	AddDragFlavor(kScrapFlavorTypeText);

	// Set appropriate Drag & Drop inset
	Rect ddInset = {1, 1, 1, 1};
	if (((CBetterScrollerX*) mPane)->HasVerticalScrollBar())
		ddInset.right += 15;
	if (((CBetterScrollerX*) mPane)->HasHorizontalScrollBar())
		ddInset.bottom += 15;
	SetHiliteInset(ddInset);

	// Set read only status of Drag and Drop
	SetDDReadOnly(true);

	// Make it fit to the superview
	InsertCols(1, 1, nil, 0, false);
	AdaptToNewSurroundings();

	// Force update of table now
	ResetTable();
}

// Handle key presses
Boolean CAdbkSearchTable::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Edit the address
		case char_Return:
		case char_Enter:
			DoEditEntry();
			break;

		// Delete
		case char_Backspace:
		case char_Clear:
			// Special case escape key
			if ((inKeyEvent.message & keyCodeMask) == vkey_Escape)
				return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
			else
				DoDeleteAddress();
			break;

		default:
			return CHierarchyTableDrag::HandleKeyPress(inKeyEvent);
			break;
	}

	return true;
}

//	Respond to commands
Boolean CAdbkSearchTable::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
	case cmd_NewLetter:
		if (IsSelectionValid())
			CreateNewLetter(false);
		else
			cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
		break;

	default:
		cmdHandled = CHierarchyTableDrag::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAdbkSearchTable::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_NewLetter:
		if (IsSelectionValid())
			outEnabled = true;

		// Else fall through to app
		else
			CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;

	default:
		CHierarchyTableDrag::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Respond to clicks in the icon buttons
void CAdbkSearchTable::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_AdbkSearchMethod:
		switch(*(long*) ioParam)
		{
		case menu_AddrSearchMethodMulberry:
			//InitAccountMenu(CINETAccount::eLocal);
			mLDAPView->Hide();
			mAdbkView->Show();
			for(int i = 1; i < CAdbkAddress::eNumberFields; i++)
				::EnableItem(mCriteria->GetMacMenuH(), i + 1);
			break;

		case menu_AddrSearchMethodLDAP:
			mServer->InitAccountMenu(CINETAccount::eLDAP);
			mLDAPView->Show();
			mAdbkView->Hide();
			for(int i = 1; i < CAdbkAddress::eNumberFields; i++)
				::EnableItem(mCriteria->GetMacMenuH(), i + 1);
			break;
		}
		break;

	case paneid_AdbkSearchField:
	case msg_AdbkSearchBtn:
		DoSearch();
		break;
	case msg_AdbkSearchClearBtn:
		DoClear();
		break;
	case msg_AdbkSearchMessageBtn:
		CreateNewLetter(COptionClick::sOptionKey);
		break;
	default:;
	}
}

// Test for selected addr
bool CAdbkSearchTable::TestSelectionAddr(TableIndexT row)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	return (nestingLevel > 0);
}

// Click in the cell
void CAdbkSearchTable::ClickCell(const STableCell& inCell,
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
	if (event_match && DragAndDropIsPresent() &&
		(CellIsSelected(inCell) ||
			(!(inMouseDown.macEvent.modifiers & shiftKey) &&
			 !(inMouseDown.macEvent.modifiers & cmdKey))) &&
		TestSelectionOr((TestSelectionPP) &CAdbkSearchTable::TestSelectionAddr)) {

		// Track item long enough to distinguish between a click to
		// select, and the beginning of a drag
		bool isDrag = !CContextMenuProcessAttachment::ProcessingContextMenu() &&
						::WaitMouseMoved(inMouseDown.macEvent.where);

		// Now do drag
		if (isDrag) {

			// If we leave the window, the drag manager will be changing thePort,
			// so we'll make sure thePort remains properly set.
			OutOfFocus(nil);
			FocusDraw();
			OSErr err = CreateDragEvent(inMouseDown);
			OutOfFocus(nil);
			return;
		}
	}

	// If multiclick then edit entry
	if ((GetClickCount() > 1) && !inMouseDown.delaySelect)
		DoEditEntry();

	// Carry on to do default action
	CHierarchyTableDrag::ClickCell(inCell, inMouseDown);
}

// Click
void CAdbkSearchTable::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	SwitchTarget(this);

	// Must set background color to grey if tracking drop flag
	StColorState	saveColors;

	STableCell	hitCell;
	SPoint32	imagePt;

	LocalToImagePoint(inMouseDown.whereLocal, imagePt);

	if (GetCellHitBy(imagePt, hitCell))
	{
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(hitCell.row + TABLE_ROW_ADJUST);
		UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);
		if (!nestingLevel)
		{
			RGBColor temp = UGAColorRamp::GetColor(colorRamp_Gray2);
			::RGBBackColor(&temp);
		}
	}

	CHierarchyTableDrag::ClickSelf(inMouseDown);
}

// Draw the titles
void CAdbkSearchTable::DrawCell(const STableCell &inCell,
								const Rect &inLocalRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);

	void* data = GetCellData(inCell.row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Save text state in stack object
	StTextState		textState;
	StColorState	saveColors;
	StColorPenState::Normalize();
	cdstring		theTxt;

	// Set to required text
	UTextTraits::SetPortTextTraits(&mTextTraits);

	// Clip to cell frame & table frame
	Rect	clipper = mRevealedRect;
	PortToLocalPoint(topLeft(clipper));
	PortToLocalPoint(botRight(clipper));
	::SectRect(&clipper, &inLocalRect, &clipper);
	StClipRgnState	clip(clipper);

	// Erase to ensure drag hightlight is overwritten
	if (!nestingLevel)
	{
		RGBColor temp = UGAColorRamp::GetColor(colorRamp_Gray2);
		::RGBBackColor(&temp);
	}
	::EraseRect(&inLocalRect);

	// Determine which heading it is
	SColumnInfo col_info = mWindow->GetColumnInfo()[inCell.col - 1];


	// Always draw hierarchy & icon in first column
	if (inCell.col == 1)
	{
		DrawDropFlag(inCell, woRow);

#if PP_Target_Carbon
		// Draw selection
		bool selected_state = DrawCellSelection(inCell);
#else
		bool selected_state = false;
#endif

		Rect	iconRect;
		iconRect.left = inLocalRect.left + mFirstIndent + nestingLevel * mLevelIndent;
		iconRect.right = iconRect.left + 16;
		iconRect.bottom = inLocalRect.bottom - mIconDescent;
		iconRect.top = iconRect.bottom - 16;
		::Ploticns(&iconRect, atNone, selected_state ? ttSelected : ttNone, (nestingLevel ? ICNx_AdbkSearchAddress : ICNx_AdbkSearched));
	}

	// Draw white line after drop flag
	if (!nestingLevel)
	{
		StColorState	saveColors2;
		RGBColor temp = UGAColorRamp::GetWhiteColor();
		::RGBForeColor(&temp);
		::MoveTo(inLocalRect.left, inLocalRect.top);
		::LineTo(inLocalRect.right - 1, inLocalRect.top);
	}

	if (nestingLevel)
	{
		switch(col_info.column_type)
		{
		case eAddrColumnName:
		case eAddrColumnNameLastFirst:
			theTxt = static_cast<CAdbkAddress*>(data)->GetName();
			break;

		case eAddrColumnNickName:
			theTxt = static_cast<CAdbkAddress*>(data)->GetADL();
			break;

		case eAddrColumnEmail:
			theTxt = static_cast<CAdbkAddress*>(data)->GetMailAddress();
			break;

		case eAddrColumnCompany:
			theTxt = static_cast<CAdbkAddress*>(data)->GetCompany();
			break;

		case eAddrColumnAddress:
			theTxt = static_cast<CAdbkAddress*>(data)->GetAddress(CAdbkAddress::eDefaultAddressType);
			break;

		case eAddrColumnPhoneWork:
			theTxt = static_cast<CAdbkAddress*>(data)->GetPhone(CAdbkAddress::eWorkPhoneType);
			break;

		case eAddrColumnPhoneHome:
			theTxt = static_cast<CAdbkAddress*>(data)->GetPhone(CAdbkAddress::eHomePhoneType);
			break;

		case eAddrColumnFax:
			theTxt = static_cast<CAdbkAddress*>(data)->GetPhone(CAdbkAddress::eFaxType);
			break;

		case eAddrColumnURLs:
			theTxt = static_cast<CAdbkAddress*>(data)->GetURL();
			break;

		case eAddrColumnNotes:
			theTxt = static_cast<CAdbkAddress*>(data)->GetNotes();
			break;

		default:
			break;
		}
		if (inCell.col == 1)
		{
			::MoveTo(inLocalRect.left + cTextIndent + mFirstIndent + nestingLevel * mLevelIndent, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - (inLocalRect.left + cTextIndent + mFirstIndent + nestingLevel * mLevelIndent), eDrawString_Left);
		}
		else
		{
			::MoveTo(inLocalRect.left, inLocalRect.bottom - mTextDescent);
			::DrawClippedStringUTF8(theTxt, inLocalRect.right - inLocalRect.left, eDrawString_Left);
		}
	}
	else if (inCell.col == 1)
	{
		theTxt = static_cast<CAddressSearchResult*>(data)->first;
		::TextFace(bold);
		::MoveTo(inLocalRect.left + cTextIndent + mFirstIndent + nestingLevel * mLevelIndent, inLocalRect.bottom - mTextDescent);
		::DrawClippedStringUTF8(theTxt, inLocalRect.right - (inLocalRect.left + cTextIndent + mFirstIndent + nestingLevel * mLevelIndent), eDrawString_Left);
	}
}





void CAdbkSearchTable::CalcCellFlagRect(const STableCell &inCell, Rect &outRect)
{
	if (inCell.col == 1)
	{
		LHierarchyTable::CalcCellFlagRect(inCell, outRect);
		outRect.right = outRect.left + 16;
		outRect.bottom = outRect.top + 12;
		::OffsetRect(&outRect, 0, 2);
	}
	else
		::SetRect(&outRect, 0, 0, 0, 0);
}

// Create new letter from selection
void CAdbkSearchTable::CreateNewLetter(bool option_key)
{
	// Create list of selected addresses
	CAddressList list;

	// Copy selection into list for active list only
	AddSelectionToList(&list);

	CReplyChooseDialog::ProcessChoice(&list,
					!(CPreferences::sPrefs->mOptionKeyAddressDialog.GetValue() ^ option_key) && (list.size() > 0));
}

// Search
void CAdbkSearchTable::DoSearch(void)
{
	switch(mMethod->GetValue())
	{
	case menu_AddrSearchMethodMulberry:
		{
			// Check that search address books exist
			if (!CAddressBookManager::sAddressBookManager->CanSearch())
			{
				// Display warning
				CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::NoSearch");
				return;
			}

			// Get details from controls
			cdstring lookup_name = mLookup->GetText();

			CAdbkAddress::EAddressField field = (CAdbkAddress::EAddressField) (mCriteria->GetValue() - 1);
			cdstring field_name = ::GetPopupMenuItemTextUTF8(mCriteria);

			try
			{
				// Do a search
				CAddressBookManager::sAddressBookManager->SearchAddress(lookup_name, (CAdbkAddress::EAddressMatch) mMatch->GetValue(), field, field_name);

				// Append last item to table
				AppendItem(CAddressBookManager::sAddressBookManager->GetSearchResultList().back());
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Do not throw up
			}
		}
		break;

	case menu_AddrSearchMethodLDAP:
		{
			// Get details from controls
			cdstring lookup_name = mLookup->GetText();

			CAdbkAddress::EAddressField field = (CAdbkAddress::EAddressField) (mCriteria->GetValue() - 1);

			try
			{
				// Do a search
				CAddressBookManager::sAddressBookManager->SearchAddress(CAddressBookManager::eLDAPLookup, false, lookup_name, (CAdbkAddress::EAddressMatch) mMatch->GetValue(), field);


				// Append last item to table
				AppendItem(CAddressBookManager::sAddressBookManager->GetSearchResultList().back());
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Do not throw up
			}
		}
		break;
	}
}

// Clear
void CAdbkSearchTable::DoClear(void)
{
	// Clear all results and force table reset
	CAddressBookManager::sAddressBookManager->ClearSearch();
	ResetTable();
}

// Edit selected entries
void CAdbkSearchTable::DoEditEntry(void)
{
	// Check that there's a selection first - if not beep and exit
	if (!IsSelectionValid())
	{
		SysBeep(1);
		return;
	}

	DoToSelection((DoToSelectionPP) &CAdbkSearchTable::EditEntry);
}

// Edit specified address
bool CAdbkSearchTable::EditEntry(TableIndexT row)
{
	bool done_edit = false;
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	if (nestingLevel)
	{
		// Copy original address
		CAdbkAddress* data = static_cast<CAdbkAddress*>(GetCellData(row));

		CEditAddressDialog::PoseDialog(data, false);

		SwitchTarget(this);
	}

	return done_edit;
}

// Delete address books
void CAdbkSearchTable::DoDeleteAddress(void)
{
	// Delete each selected item in reverse to stay in sync
	DoToSelection((DoToSelectionPP) &CAdbkSearchTable::DeleteAddress, false);
}

// Delete specified address book
bool CAdbkSearchTable::DeleteAddress(TableIndexT row)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	void* data = GetCellData(row);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Delete whole item or single address
	if (nestingLevel)
	{
		// Get parent item
		TableIndexT parentRow = GetParentIndex(woRow);
		void* item = mData.at(parentRow - 1);
		CAddressBookManager::sAddressBookManager->ClearSearchItemAddress((CAddressSearchResult*) item, static_cast<CAdbkAddress*>(data));
		RemoveRows(1, woRow, true);
		mData.erase(mData.begin() + (woRow - 1));
		if (woRow < mLastParent)
			mLastParent--;
	}
	else
	{
		size_t count = 1 + ((CAddressSearchResult*) data)->second->size();
		CAddressBookManager::sAddressBookManager->ClearSearchItem((CAddressSearchResult*) data);
		RemoveRows(1, woRow, true);
		mData.erase(mData.begin() + (woRow - 1), mData.begin() + (woRow - 1 + count));
		if (woRow < mLastParent)
			mLastParent -= count;
		else if (woRow == mLastParent)
		{
			mLastParent = GetParentIndex(mCollapsableTree->CountNodes());
		}
	}

	return true;
}

// Keep titles in sync
void CAdbkSearchTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, Boolean inRefresh)
{
	// Find titles in owner chain
	((CAdbkSearchTitleTable*) mWindow->FindPaneByID(paneid_TitleTable))->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Reset the table from the mboxList
void CAdbkSearchTable::ResetTable(void)
{
	// Start cursor for busy operation
	CWaitCursor wait;

	// Delete all existing rows
	Clear();
	mData.clear();

	// Get list from manager
	const CAddressSearchResultList& results = CAddressBookManager::sAddressBookManager->GetSearchResultList();

	// Add each list entry
	mLastParent = 0;
	for(CAddressSearchResultList::const_iterator iter = results.begin(); iter != results.end(); iter++)
	{
		TableIndexT parent_row;
		mLastParent = InsertSiblingRows(1, mLastParent, &(*iter), sizeof(void*), true, false);
		mData.push_back(*iter);
		parent_row = mLastParent;
		AddList((*iter)->second, parent_row);
		CollapseRow(mLastParent);
	}

	// Refresh list
	Refresh();

} // CAdbkSearchTable::ResetTable

// Add an item to end of list
void CAdbkSearchTable::AppendItem(const CAddressSearchResult* item)
{
	// Insert each address as child
	mLastParent = InsertSiblingRows(1, mLastParent, &item, sizeof(void*), true, true);
	mData.push_back((void*) item);
	TableIndexT parent_row = mLastParent;
	AddList(item->second, parent_row);
	ExpandRow(mLastParent);
	RefreshRowsBelow(mLastParent);
}

// Add a node to the list
void CAdbkSearchTable::AddList(const CAddressList* list, TableIndexT& row)
{
	// Insert each address as child
	for(CAddressList::const_iterator iter = list->begin(); iter != list->end(); iter++)
	{
		AddLastChildRow(row, &(*iter), sizeof(CAdbkAddress*), false, false);
		mData.push_back(*iter);
	}
}

// Get the selected adbk
void* CAdbkSearchTable::GetCellData(TableIndexT row)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	return mData.at(woRow - 1);
}

// Add selected addresses to list
void CAdbkSearchTable::AddSelectionToList(CAddressList* list)
{
	// Add all selected address to drag
	DoToSelection1((DoToSelection1PP) &CAdbkSearchTable::CopyEntryToList, list);
}

// Copy address to list
bool CAdbkSearchTable::CopyEntryToList(TableIndexT row, CAddressList* list)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Only if address
	if (!nestingLevel)
		return false;

	// Get the relevant address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(GetCellData(row));

	// Copy address
	CAdbkAddress* copy = new CAdbkAddress(*addr);

	// Save in selected list if not duplicate
	if (list->InsertUniqueItem(copy))
		return true;
	else
	{
		delete copy;
		copy = nil;
		return false;
	}
}

// Add address to drag
void CAdbkSearchTable::AddCellToDrag(CDragIt* theDragTask, const STableCell& aCell, Rect& dragRect)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(aCell.row + TABLE_ROW_ADJUST);
	UInt32	nestingLevel = mCollapsableTree->GetNestingLevel(woRow);

	// Only if address
	if (!nestingLevel)
		return;

	// Get the relevant address
	CAdbkAddress* addr = static_cast<CAdbkAddress*>(GetCellData(aCell.row));

	// Add this address to drag
	theDragTask->AddFlavorItem(dragRect, (ItemReference) addr, cDragAddrType,
								&addr, sizeof(CAdbkAddress*), flavorSenderOnly, true);

	// Promise text to anyone who wants it
	theDragTask->AddFlavorItem(dragRect, (ItemReference) addr, kScrapFlavorTypeText, 0L, 0L, 0, false);
}

// Other flavor requested by receiver
void CAdbkSearchTable::DoDragSendData(
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
