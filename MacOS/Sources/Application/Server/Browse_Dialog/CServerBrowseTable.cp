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


// Source for CServerBrowseTable class

#include "CServerBrowseTable.h"

#include "CBrowseMailboxDialog.h"
#include "CMailAccountManager.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CTableMultiRowSelector.h"

#include <LPushButton.h>
#include <LTableSingleRowSelector.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CMboxRef	CServerBrowseTable::sLastCreateBrowsed;						// Last mailbox browsed
CMboxRef	CServerBrowseTable::sLastOpenBrowsed;						// Last mailbox browsed

// Default constructor
CServerBrowseTable::CServerBrowseTable()
{
	InitServerBrowseTable();
}

// Default constructor - just do parents' call
CServerBrowseTable::CServerBrowseTable(LStream *inStream)
	: CServerBrowse(inStream)
{
	InitServerBrowseTable();
}

// Default destructor
CServerBrowseTable::~CServerBrowseTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CServerBrowseTable::InitServerBrowseTable(void)
{
	if (mTableSelector)
		delete mTableSelector;
	mTableSelector = new LTableSingleRowSelector(this);

	// Do not show favourites
	mShowFavourites = false;

	// Do not record expansion
	mRecordExpansion = false;
}

// Get details of sub-panes
void CServerBrowseTable::FinishCreateSelf(void)
{
	// Do inherited
	CServerBrowse::FinishCreateSelf();

	// Find window in super view chain
	CBrowseMailboxDialog* dlog = (CBrowseMailboxDialog*) GetSuperView();
	while((dlog->GetPaneID() != paneid_BrowseMailboxDialog) && (dlog->GetPaneID() != paneid_BrowseCopyMessageDialog))
		dlog = (CBrowseMailboxDialog*) dlog->GetSuperView();

	// Logon button
	mOKButton = (LPushButton*) dlog->FindPaneByID(paneid_BrOKBtn);
	mOKButton->Disable();
	mCreateButton = (LPushButton*) dlog->FindPaneByID(paneid_BrCreateBtn);

	InsertCols(1, 1, NULL, 0, false);

	// Link controls to this table
	UReanimator::LinkListenerToBroadcasters(this, dlog, RidL_CBrowseMailboxDialogBtns);
}

// Respond to clicks in the icon buttons
void CServerBrowseTable::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_BrCreate:
		DoCreateMailbox();
		break;
	}
}

#pragma mark ____________________________Draw & Click

// Click in the cell
void CServerBrowseTable::ClickCell(const STableCell& inCell,
								const SMouseDownEvent& inMouseDown)
{
	if (GetClickCount() == 1)
		// Carry on to do default action
		CServerBrowse::ClickCell(inCell, inMouseDown);

	else if (GetClickCount() == 2)
	{
		if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionMbox))
			BroadcastMessage(msg_OK, nil);
	}
}

// Adjust column widths
void CServerBrowseTable::AdaptToNewSurroundings(void)
{
	// Do inherited call
	CServerBrowse::AdaptToNewSurroundings();

	// Get super frame's width - scroll bar
	SDimension16 super_frame;
	mSuperView->GetFrameSize(super_frame);
	super_frame.width -= 16;

	// Set image to super frame size
	ResizeImageTo(super_frame.width,mImageSize.height,true);

	// Set column width
	SetColWidth(super_frame.width,1,1);

}

// Allow multiple selection
void CServerBrowseTable::SetMultipleSelection(void)
{
	delete mTableSelector;
	mTableSelector = new CTableMultiRowSelector(this);
}

void CServerBrowseTable::DoSelectionChanged()
{
	// Update buttons

	// OK only if mailbox selected
	if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionMbox))
		mOKButton->Enable();
	else
		mOKButton->Disable();

	// Create only if hierarchy selected
	if (!mOpenMode)
	{
		if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionInferiors) && IsSingleSelection() ||
			mSingle && GetSingleServer()->FlatWD())
			mCreateButton->Enable();
		else
			mCreateButton->Disable();
	}
}

// Process mailbox after creation
void CServerBrowseTable::PostCreateAction(CMbox* mbox)
{
	// Select it and broadcast OK message
	unsigned long mbox_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(mbox, true);
	if (!mbox_index)
		return;

	// Must reveal
	RevealRow(mbox_index);
	TableIndexT exposed = GetExposedIndex(mbox_index);

	if (exposed)
	{
		STableCell aCell(exposed, 1);
		FakeClickSelect(aCell, false);
		ScrollCellIntoFrame(aCell);
		BroadcastMessage(msg_OK, nil);
	}
}

// Reset the table from the mboxList
void CServerBrowseTable::ResetTable()
{
	// Do inherited
	CServerBrowse::ResetTable();

	// Now attempt to resolve last selected item and show it
	if (GetLastBrowsed().GetName().empty())
		return;

	// Resolve mailbox
	CMbox* select = GetLastBrowsed().ResolveMbox();

	if (!select)
		return;

	unsigned long woRow = (!mSingle ?
								CMailAccountManager::sMailAccountManager->FetchIndexOf(select, true) :
								GetSingleServer()->FetchIndexOf(select, true)) +
								(mShowFavourites ? CMailAccountManager::sMailAccountManager->FetchServerOffset() : 0);

	RevealRow(woRow);
	TableIndexT exposed = GetExposedIndex(woRow);
	STableCell aCell(exposed, 1);
	FakeClickSelect(aCell, false);
	ScrollCellIntoFrame(aCell);
}

void CServerBrowseTable::SetOpenMode(bool mode)
{
	mOpenMode = mode;

	if (mode)
		mCreateButton->Hide();
}

CMbox* CServerBrowseTable::GetSelectedMbox()
{
	TableIndexT row = GetFirstSelectedRow();
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	if ((GetCellDataType(woRow) == eServerBrowseMbox) && !GetCellMbox(woRow)->IsDirectory())
	{
		CMbox* selected = GetCellMbox(woRow);

		// Save for next time
		GetLastBrowsed() = CMboxRef(selected);

		return selected;
	}
	else
		return nil;
}

// Get all selected mboxes
void CServerBrowseTable::GetSelectedMboxes(CMboxList& mbox_list)
{
	DoToSelection1((DoToSelection1PP) &CServerBrowseTable::AddSelectionToList, &mbox_list);
}