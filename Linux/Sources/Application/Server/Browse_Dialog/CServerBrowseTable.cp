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

#include <JXTextButton.h>
#include <JTableSelection.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

CMboxRef	CServerBrowseTable::sLastCreateBrowsed;						// Last mailbox browsed
CMboxRef	CServerBrowseTable::sLastOpenBrowsed;						// Last mailbox browsed

// Default constructor
CServerBrowseTable::CServerBrowseTable(JXScrollbarSet* scrollbarSet,
										JXContainer* enclosure,
										const HSizingOption hSizing, 
										const VSizingOption vSizing,
										const JCoordinate x, const JCoordinate y,
										const JCoordinate w, const JCoordinate h)
	: CServerBrowse(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)

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
	// Default to single selection
	if (mTableSelector)
		delete mTableSelector;
	mTableSelector = new CTableSingleRowSelector(this);
	
	// Do not show favourites
	mShowFavourites = false;
	
	// Do not record expansion
	mRecordExpansion = false;	
}

void CServerBrowseTable::OnCreate()
{
	// Just do inherited but we need this call since inherited is protected
	CServerBrowse::OnCreate();

	// Always insert a single column
	InsertCols(1, 1);
	SetColWidth(GetApertureWidth(), 1, 1);
}

#pragma mark ____________________________Draw & Click

// Resize columns
void CServerBrowseTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CServerBrowse::ApertureResized(dw, dh);

	// Only the name column exists
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
	}
}

// Double-clicked item
void CServerBrowseTable::LDblClickCell(const STableCell& cell, const JXKeyModifiers& modifiers)
{
	// Check for server
	if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionMbox))
		mDialog->EndDialog(CDialogDirector::kDialogClosed_OK);
}

// Allow multiple selection
void CServerBrowseTable::SetMultipleSelection(void)
{
	delete mTableSelector;
	mTableSelector = new CTableRowSelector(this);
}

void CServerBrowseTable::DoSelectionChanged()
{
	// Update buttons

	// OK only if mailbox selected
	if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionMbox))
		mOKButton->Activate();
	else
		mOKButton->Deactivate();

	// Create only if hierarchy selected
	if (!mOpenMode)
	{
		if (TestSelectionAnd((TestSelectionPP) &CServerBrowseTable::TestSelectionInferiors) && IsSingleSelection() ||
			mSingle && GetSingleServer()->FlatWD())
			mCreateButton->Activate();
		else
			mCreateButton->Deactivate();
	}
}

// Process mailbox after creation
void CServerBrowseTable::PostCreateAction(CMbox* mbox)
{
	// Select it and broadcast OK message
	unsigned long mbox_index = CMailAccountManager::sMailAccountManager->FetchIndexOf(mbox, true);
	if (!mbox_index)
		return;
					
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

	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row);
	if ((GetCellDataType(woRow) == eServerBrowseMbox) && !GetCellMbox(woRow)->IsDirectory())
	{
		CMbox* selected = GetCellMbox(woRow);
		
		// Save for next time
		GetLastBrowsed() = CMboxRef(selected);

		return selected;
	}
	else
		return NULL;
}

// Get all selected mboxes
void CServerBrowseTable::GetSelectedMboxes(CMboxList& mbox_list)
{
	DoToSelection1((DoToSelection1PP) &CServerBrowseTable::AddSelectionToList, &mbox_list);
}
