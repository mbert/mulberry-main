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
#include "CTableRowSelector.h"

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CMboxRef	CServerBrowseTable::sLastCreateBrowsed;						// Last mailbox browsed
CMboxRef	CServerBrowseTable::sLastOpenBrowsed;						// Last mailbox browsed


IMPLEMENT_DYNCREATE(CServerBrowseTable, CServerBrowse)

BEGIN_MESSAGE_MAP(CServerBrowseTable, CServerBrowse)
	ON_COMMAND(IDM_MAILBOX_CREATE, OnCreateMailbox)

	ON_WM_SIZE()
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARCREATEMAILBOXBTN, OnCreateMailbox)
END_MESSAGE_MAP()

// Default constructor
CServerBrowseTable::CServerBrowseTable()
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
	mTableSelector = new CTableSingleRowSelector(this);

	// Do not show favourites
	mShowFavourites = false;
	
	// Do not record expansion
	mRecordExpansion = false;

	// Allow multiple selections
	ModifyStyle(0, LVS_SINGLESEL);
}

// Resize columns
void CServerBrowseTable::OnSize(UINT nType, int cx, int cy)
{
	CServerBrowse::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Name column: variable
	if (cy)
		SetColWidth(cx, 1, 1);
}

#pragma mark ____________________________Draw & Click

// Double-clicked item
void CServerBrowseTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	// Check for server
	if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionMbox))
	{
		mDialog->UpdateData(true);
		mDialog->EndDialog(IDOK);
	}
}

// Turn on multiple selection
void CServerBrowseTable::SetMultipleSelection()
{
	delete mTableSelector;
	mTableSelector = new CTableRowSelector(this);
}

void CServerBrowseTable::SelectionChanged(void)
{
	// Update buttons

	// OK only if mailbox selected
	if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionMbox))
		mOKButton->EnableWindow(true);
	else
		mOKButton->EnableWindow(false);

	// Create only if hierarchy selected
	if (!mOpenMode)
	{
		if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionInferiors) && IsSingleSelection() ||
			mSingle && GetSingleServer()->FlatWD())
			mCreateButton->EnableWindow(true);
		else
			mCreateButton->EnableWindow(false);
	}

	// Do inherited
	CServerBrowse::SelectionChanged();
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
		UnselectAllCells();
		FakeClickSelect(aCell, false);
		ScrollCellIntoFrame(aCell);
	}
}

// Reset the table from the mboxList
void CServerBrowseTable::ResetTable(void)
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
		mCreateButton->ShowWindow(SW_HIDE);
}

CMbox* CServerBrowseTable::GetSelectedMbox(void)
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

// Create Mailbox
void CServerBrowseTable::OnCreateMailbox(void)
{
	DoCreateMailbox(true);
}
