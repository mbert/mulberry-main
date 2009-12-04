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

#include "CAddressBook.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#include "CAddressList.h"
#include "CAddressView.h"
#include "CClipboard.h"
#include "CCommands.h"
#include "CEditGroupDialog.h"
#include "CErrorHandler.h"
#include "CGroup.h"
#include "CGroupList.h"
#include "CGroupTableAction.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CTaskClasses.h"
#include "CUserAction.h"

#include <UNX_LTableArrayStorage.h>

#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXTextMenu.h>
#include <JTableSelection.h>
#include <jASCIIConstants.h>
#include <jXKeysym.h>

#include <algorithm>
#include <memory>

/////////////////////////////////////////////////////////////////////////////
// CGroupTable

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTable::CGroupTable(JXScrollbarSet* scrollbarSet, 
							JXContainer* enclosure,
							const HSizingOption hSizing, 
							const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);

	mAdbk = NULL;
	mTableView = NULL;
	mEditAction = NULL;
	mReplaceAction = NULL;

	mLastTyping = 0UL;
	mLastChars[0] = 0;

	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	mTableStorage = new LTableArrayStorage(this, sizeof(SGroupTableItem));

	SetRowSelect(true);
}

// Default destructor
CGroupTable::~CGroupTable()
{
	if (mEditAction || mReplaceAction)
	{
		mTableView->GetUndoer()->PostAction(NULL);
		mEditAction = NULL;
		mReplaceAction = NULL;
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CGroupTable::OnCreate()
{
	// Find table view in super view chain
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CAddressBookView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CAddressBookView*>(dynamic_cast<const CAddressBookView*>(parent));

	CHierarchyTableDrag::OnCreate();

	// Get keys but not Tab which is used to shift focus
	WantInput(kTrue);
	SetKeySelection(true);

	// Set Drag & Drop info
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDragFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());

	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAddrList);
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDropFlavor(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	AddDropFlavor(GetDisplay()->GetSelectionManager()->GetTextXAtom());

	SetReadOnly(false);
	SetDropCell(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetAllowMove(false);

	// Context menu
	CreateContextMenu(CMainMenu::eContextAdbkGroup);
}

// Resize columns
void CGroupTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	CHierarchyTableDrag::ApertureResized(dw, dh);

	// Single column always fills table width
	if (mCols)
	{
		JCoordinate cw = GetApertureWidth();
		if (cw < 32)
			cw = 32;
		SetColWidth(cw, 1, 1);
	}
}

// Set dirty flag
void CGroupTable::SetDirty(bool dirty)
{
	mDirty = dirty;
}

// Is it dirty
bool CGroupTable::IsDirty()
{
	return mDirty;
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
void CGroupTable::DoSingleClick(TableIndexT row, const CKeyModifiers& mods)
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
void CGroupTable::DoDoubleClick(TableIndexT row, const CKeyModifiers& mods)
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
TableIndexT CGroupTable::InsertGroup(TableIndexT pos, CGroup* grp)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Find position of previous group
	TableIndexT woRow;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (GetNestingLevel(woRow) == 0)
			pos--;
		if (pos < 0) break;
	}

	SGroupTableItem item(true, grp);
	TableIndexT newRow = InsertSiblingRows(1, woRow, &item, sizeof(SGroupTableItem), true, false);

	// Add all addresses in group
	for(unsigned long i = 0; i < grp->GetAddressList().size(); i++)
	{
		SGroupTableItem addr_item(false, grp);
		AddLastChildRow(newRow, &addr_item, sizeof(SGroupTableItem), false, false);
	}
	DeepCollapseRow(newRow);
	
	return GetExposedIndex(newRow);
}

// Delete group from display
void CGroupTable::DeleteGroup(TableIndexT pos)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Find position of group
	TableIndexT woRow;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (GetNestingLevel(woRow) == 0)
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
		TableIndexT woRow;
		for(woRow = 0; woRow <= mCollapsableTree->CountNodes(); woRow++)
		{
			if (GetNestingLevel(woRow) == 0)
				pos--;
			if (pos < 1) break;
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
void CGroupTable::InsertedAddress(CGroup* grp, int num)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	short pos = mAdbk->GetGroupList()->FetchIndexOf(grp);

	// Find position of group
	TableIndexT woRow;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (GetNestingLevel(woRow) == 0)
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
void CGroupTable::DeletedAddress(CGroup* grp, int num)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	short pos = mAdbk->GetGroupList()->FetchIndexOf(grp);

	// Find position of group
	UInt32 woRow;
	for(woRow = 0; woRow < mCollapsableTree->CountNodes(); woRow++)
	{
		if (GetNestingLevel(woRow) == 0)
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
	RemoveAllRows(true);

	// Add all groups
	if (mAdbk)
	{
		UInt32 last_grp_row = 0;
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

	mTableView->UpdateCaptions();
	Refresh();
}

// Reset the table from the address list
void CGroupTable::ClearTable()
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Remove all rows
	Clear();
}

void CGroupTable::OnNewMessage(void)
{
	CreateNewLetter(GetDisplay()->GetLatestKeyModifiers().meta());
}

void CGroupTable::OnNewGroup(void)
{
	CreateNewEntry();
}

void CGroupTable::OnEditGroup(void)
{
	DoEditEntry();
}

void CGroupTable::OnDeleteGroup(void)
{
	DoDeleteEntry();
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
	// Create new group and add to list
	CGroup* new_grp = new CGroup;

	// Let DialogHandler process events
	if (CEditGroupDialog::PoseDialog(new_grp))
	{
		// Only add if some text available
		if (!new_grp->IsEmpty())
		{
			// Initiate action
			mTableView->GetUndoer()->PostAction(new CGroupTableNewAction(this, new_grp, IsDirty()));
		}
		else
			delete new_grp;
	}
	else
		delete new_grp;
}

// Edit selected entries
void CGroupTable::DoEditEntry()
{
	// Create new edit action ready to store edits (force previous one to be committed)
	CGroupTableEditAction* oldEditAction = mEditAction;
	mEditAction = new CGroupTableEditAction(this, IsDirty());

	// Do edit for each selection
	if (TestSelectionOr((TestSelectionPP) &CGroupTable::EditEntry))

		// Initiate action - will commit previous
		mTableView->GetUndoer()->PostAction(mEditAction);

	else
	{
		// Delete attempted action and restore previous
		delete mEditAction;
		mEditAction = oldEditAction;
	}
}

// Edit specified group
bool CGroupTable::EditEntry(TableIndexT row)
{
	bool done_edit = false;

	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Copy original address
	CGroup* copy = new CGroup(*item.mGroup);

	// Only edit groups
	if (item.mIsGroup)
	{
		// Let DialogHandler process events
		if (CEditGroupDialog::PoseDialog(copy))
		{
			// Add info to action
			mEditAction->AddEdit(item.mGroup, copy);
			done_edit = true;
		}
		else
		{
			delete copy;
			done_edit = false;
		}
	}
	
	return done_edit;
}

// Delete selected entries
void CGroupTable::DoDeleteEntry()
{
	// Determine whether to do groups or address only
	if (TestSelectionOr((TestSelectionPP) &CGroupTable::TestSelectionGroup))
	{
		// Create list to hold deleted items
		CGroupList* grps = new CGroupList();

		// Add each selected group
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupToList, grps);

		// Initiate delete action if some selected
		if (grps->size() > 0)
			mTableView->GetUndoer()->PostAction(new CGroupTableDeleteAction(this, grps, IsDirty()));
		else
			delete grps;
	}
	else
	{
		CGroup* aGrp = NULL;
	
		if (TestSelectionAnd1((TestSelection1PP) &CGroupTable::TestSameGroup, &aGrp) && aGrp)
		{
			// Copy original group
			CGroup* copy = new CGroup(*aGrp);

			// Delete selected addresses (in reverse)
			STableCell	delCell(0, 0);
			while (GetPreviousSelectedCell(delCell))
			{
				// Get its info
				TableIndexT woRow = GetWideOpenIndex(delCell.row);
				
				STableCell	woCell(woRow, delCell.col);
				SGroupTableItem item;
				UInt32 dataSize = sizeof(SGroupTableItem);
				GetCellData(woCell, &item, dataSize);

				// Determine position in group
				short pos = woRow - GetParentIndex(woRow) - 1;

				// Delete address
				copy->GetAddressList().erase(copy->GetAddressList().begin() + pos);
			}

			// Initiate delete action
			mReplaceAction = new CGroupTableReplaceAction(this, aGrp, copy, IsDirty());
			mTableView->GetUndoer()->PostAction(mReplaceAction);
		}
		else if (aGrp)
			CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::DeleteAddressSameGroup");
	}
}

// Add from list - not unique
void CGroupTable::AddGroupsFromList(CGroupList* grps)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Add all groups
	mAdbk->AddGroup(grps, true);

	SelectGroups(grps);
	
	mTableView->UpdateCaptions();
	SetDirty(false);
}

// Change from list
void CGroupTable::ChangeGroupsFromList(CGroupList* old_grps, CGroupList* new_grps)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->UpdateGroup(old_grps, new_grps);

	// Reset only after all changes have happened
	UnselectAllCells();
	Refresh();	
	mTableView->UpdateCaptions();

	SetDirty(false);
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
	SetDirty(false);
}

// Delete from list
void CGroupTable::SelectGroups(CGroupList* grps)
{
	// Prevent selection changes
	StDeferSelectionChanged _defer(this);

	// Remove existing selection
	UnselectAllCells();

	// Loop over all rows selecting from list
	for(TableIndexT row = 1; row <= mRows; row++)
	{
		TableIndexT woRow = GetWideOpenIndex(row);

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
	SetDirty(false);
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
	TableIndexT	woRow = GetWideOpenIndex(row);
	
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
		short pos = woRow - GetParentIndex(woRow) - 1;

		// Copy and save in selected list
		list->push_back(new CAddress(item.mGroup->GetAddressList().at(pos)));
	}
	
	return true;
}		

// Add to text
bool CGroupTable::AddToText(TableIndexT row, cdstring* txt)
{
	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Check for group
	if (item.mIsGroup)
	{
	
		// Add all addresses in this group
		for(unsigned long i = 0; i < item.mGroup->GetAddressList().size(); i++)
		{
			*txt += item.mGroup->GetAddressList().at(i);
			*txt += os_endl;
		}
	}
	else
	{

		// Determine position in group
		UInt32 pos = woRow - GetParentIndex(woRow) - 1;

		// Copy and save in selected list
		*txt += item.mGroup->GetAddressList().at(pos);
		*txt += os_endl;
	}
	
	return true;
}		

// Add group to list
bool CGroupTable::AddGroupToList(TableIndexT row, CGroupList* list)
{
	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Check for group and attempt unique insert
	if (item.mIsGroup)
		return list->InsertUniqueItem(item.mGroup);
	
	return false;
}		

// Test for selected item group
bool CGroupTable::TestSelectionGroup(TableIndexT row)
{
	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// This is deleted
	return item.mIsGroup;
}

// Test for selected items in same group
bool CGroupTable::TestSameGroup(TableIndexT row, CGroup** test)
{
	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// This is deleted
	return *test ? (item.mGroup == *test) : ((*test = item.mGroup) != NULL);
}

// Common updaters
void CGroupTable::OnUpdateEditPaste(JXTextMenu* menu, JIndex item)
{
	// Check for text in clipboard
	menu->EnableItem(item);
}

// Command handlers
void CGroupTable::OnEditCut()
{
	// Copy first
	OnEditCopy();
	
	// Delete selection
	DoDeleteEntry();
}

void CGroupTable::OnEditCopy()
{
	cdstring txt;

	// Add all selected groups to text
	DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupText, &txt);

	// Now copy to scrap
	CClipboard::CopyToSecondaryClipboard(GetDisplay(), txt);
}

// Add groups as text to handle
bool CGroupTable::AddGroupText(TableIndexT row , cdstring* txt)
{
	// Determine whether to do groups or address only
	bool got_grp = TestSelectionOr((TestSelectionPP) &CGroupTable::TestSelectionGroup);

	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Do not do mixture - copy only groups or only addresses
	if (got_grp && !item.mIsGroup) return false;

	// Check for single address or group
	if (got_grp)
	{
		CGroup* theGrp = item.mGroup;

		for(unsigned long i = 0; i < theGrp->GetAddressList().size(); i++)
		{
			cdstring& aStr = theGrp->GetAddressList().at(i);
			
			// Add CR before if multi-copy
			if (txt->length())
				*txt += "\r\n";
			
			*txt += aStr;
		}
	}
	else
	{
		// Determine position in group
		UInt32 pos = woRow - GetParentIndex(woRow) - 1;

		// Get address
		CGroup* theGrp = item.mGroup;
		cdstring& aStr = theGrp->GetAddressList().at(pos);

		// Add CR before if multi-copy
		if (txt->length())
			*txt += os_endl;
		
		*txt += aStr;
	}
	
	return true;
}		

void CGroupTable::OnEditPaste()
{
	cdstring txt;
	CClipboard::GetSecondaryClipboard(GetDisplay(), txt);
	if (txt.length())
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddAddressTextToGroup, txt);
}

// Add address as text to handle
bool CGroupTable::AddAddressTextToGroup(TableIndexT row, char* txt)
{
	// Get its info
	TableIndexT	woRow = GetWideOpenIndex(row);
	
	STableCell	woCell(woRow, 1);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	// Check for group
	if (item.mIsGroup)
		DropDataIntoCell(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom(), (unsigned char*) txt, ::strlen(txt), STableCell(row, 1));
	
	return true;
}		

void CGroupTable::ActionDeleted()
{
	if (mEditAction)
		mEditAction = NULL;
	if (mReplaceAction)
		mReplaceAction = NULL;
}

#pragma mark ____________________________________Keyboard/Mouse

// Double-clicked item
void CGroupTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	CHierarchyTableDrag::LClickCell(inCell, modifiers);
	DoSingleClick(inCell.row, CKeyModifiers(modifiers));
}

// Double-clicked item
void CGroupTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoDoubleClick(inCell.row, CKeyModifiers(modifiers));
}

// Handle key down
bool CGroupTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Look for preview/full view based on key stroke

	// Determine whether preview is triggered
	CKeyModifiers mods(modifiers);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == key) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == key) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	switch(key)
	{
	case kJDeleteKey:
	case kJForwardDeleteKey:
		DoDeleteEntry();
		return true;

	case '\t':
		//mTableView->FocusAddress();
		//mTableView->SetFocus();
		return true;

	default:
		// Did not handle key
		return CHierarchyTableDrag::HandleChar(key, modifiers);
	}
}

// Select from key press
void CGroupTable::DoKeySelection()
{
	// Create new address for typed chars
	CGroup* temp = new CGroup(mLastChars);
	
	// Get low bound insert pos
	long select = mAdbk->GetGroupList()->FetchInsertIndexOf(temp);
	
	// Clip to end of list
	if (select > mAdbk->GetGroupList()->size()) select--;

	// Find exposed position of group
	UInt32 pos;
	for(pos = 0; pos < mCollapsableTree->CountNodes(); pos++)
	{
		if (GetNestingLevel(pos) == 0)
			select--;
		if (select < 0) break;
	}

	STableCell aCell(GetExposedIndex(pos), 2);
	UnselectAllCells();
	FakeClickSelect(aCell, false);
	ScrollCellIntoFrame(aCell);

	delete temp;

}

bool CGroupTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnNewMessage();
		return true;

	case CCommand::eEditCut:
		OnEditCut();
		return true;
	case CCommand::eEditCopy:
		OnEditCopy();
		return true;
	case CCommand::eEditPaste:
		OnEditPaste();
		return true;

	case CCommand::eAddressEdit:
	case CCommand::eToolbarAddressBookEditBtn:
		DoEditEntry();
		return true;

	case CCommand::eAddressDelete:
	case CCommand::eToolbarAddressBookDeleteBtn:
		DoDeleteEntry();
		return true;

	default:;
	}

	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}

void CGroupTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eEditCut:
	case CCommand::eEditCopy:
	case CCommand::eAddressEdit:
	case CCommand::eToolbarAddressBookEditBtn:
	case CCommand::eAddressDelete:
	case CCommand::eToolbarAddressBookDeleteBtn:
		OnUpdateSelection(cmdui);
		return;

	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
	case CCommand::eEditPaste:
		OnUpdateAlways(cmdui);
		return;
	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}

// Draw the titles
void CGroupTable::DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect)
{
	TableIndexT woRow = GetWideOpenIndex(inCell.row);

	STableCell	woCell(woRow, inCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	unsigned long bkgnd = 0x00FFFFFF;
	if (item.mIsGroup && CellIsSelected(inCell) && IsActive() && HasFocus())
	{
		JColorIndex bkgnd_index = GetColormap()->GetDefaultSelectionColor();
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	JXImage* icon;
	const char* content;
	if (item.mIsGroup)
	{
		CGroup* grp = item.mGroup;
		icon = CIconLoader::GetIcon(IDI_GROUP_FLAG, this, 16, bkgnd);
		content = grp->GetName();
	}
	else
	{
		CGroup* grp = item.mGroup;
		icon = CIconLoader::GetIcon(IDI_ADDRESS_FLAG, this, 16, bkgnd);

		// Determine position in group
		UInt32 pos = woRow - GetParentIndex(woRow) - 1;

		// Determine name to use
		content = grp->GetAddressList().at(pos);
	}

	DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, icon);
}

#pragma mark ____________________________________Drag&Drop

// Limit flavors based on selection
bool CGroupTable::GetSelectionAtom(CFlavorsList& atoms)
{
	// Determine whether to do groups or address only
	bool all_grp = TestSelectionAnd((TestSelectionPP) &CGroupTable::TestSelectionGroup);

	// If all groups allow group list drag
	if (all_grp)
		atoms.push_back(CMulberryApp::sFlavorGrpList);
	else
		atoms.push_back(CMulberryApp::sFlavorAddrList);

	// Always do text
	atoms.push_back(GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom());
	atoms.push_back(GetDisplay()->GetSelectionManager()->GetTextXAtom());

	return true;
}

// Determine if drop into cell
bool CGroupTable::IsDropCell(JArray<Atom>& typeList, const STableCell& cell)
{
	if (IsValidCell(cell))
	{
		TableIndexT woRow = GetWideOpenIndex(cell.row);
		STableCell woCell(woRow, cell.col);
		SGroupTableItem item;
		UInt32 dataSize = sizeof(SGroupTableItem);
		GetCellData(woCell, &item, dataSize);
		return item.mIsGroup;
	}
	else
		return false;
}

bool CGroupTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	if  (type == CMulberryApp::sFlavorGrpList)
	{
		// Create list to hold dragged items
		std::auto_ptr<CGroupList> grps(new CGroupList);
		grps->set_delete_data(false);

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupToList, grps.get());
		seldata->SetData(type, reinterpret_cast<unsigned char*>(grps.release()), sizeof(CGroupList*));
		rendered = true;
	}

	else if  (type == CMulberryApp::sFlavorAddrList)
	{
		// Create list to hold deleted items
		std::auto_ptr<CAddressList> addrs(new CAddressList);
		addrs->set_delete_data(false);

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddAddressesToList, addrs.get());
		seldata->SetData(type, reinterpret_cast<unsigned char*>(addrs.release()), sizeof(CAddressList*));
		rendered = true;
	}

	else if ((type == GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom()) ||
			 (type == GetDisplay()->GetSelectionManager()->GetTextXAtom()))
	{
		cdstring txt;

		// Add all selected address to text
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddToText, &txt);

		// Add all selected address to text
		unsigned long dataLength = txt.length() + 1;
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			::memcpy(data, txt.c_str(), dataLength);
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}
	
	return rendered;
}

// Drop data into whole table
bool CGroupTable::DropData(Atom inFlavor, unsigned char* drag_data, unsigned long data_size)
{
	if (inFlavor == CMulberryApp::sFlavorGrpList)
	{
		CGroupList* grps = reinterpret_cast<CGroupList*>(drag_data);
		CGroupTableNewAction* add_action = NULL;
		for(CGroupList::const_iterator iter = grps->begin(); iter != grps->end(); iter++)
		{
			// Does it already exist?
			if (!mAdbk->GetGroupList()->IsDuplicate(*iter))
			{
				CGroup* copy = new CGroup(**iter);
				
				// Check for existing action
				if (add_action)
					add_action->AddGroup(copy);
				else
					// Create new action - action owns list
					add_action = new CGroupTableNewAction(this, copy, IsDirty());
			}
		}

		// Post action after all added
		mTableView->GetUndoer()->PostAction(add_action);
	}
	
	return true;
}

// Drop data into cell
bool CGroupTable::DropDataIntoCell(Atom theFlavor,
										unsigned char* drag_data,
										unsigned long data_size, const STableCell& inCell)
{
	bool added = false;

	// Only allow drag to group
	TableIndexT	woRow = GetWideOpenIndex(inCell.row);

	STableCell	woCell(woRow, inCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);
	if (!item.mIsGroup)
		return false;

	CGroup* old_grp = item.mGroup;

	// Create new group
	CGroup* new_grp = new CGroup(*old_grp);

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			CEnvelope* theEnv = (*iter)->GetEnvelope();

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
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorAddrList)
	{
		CAddressList* addrs = reinterpret_cast<CAddressList*>(drag_data);
		for(CAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
		{
			// Add to list
			cdstring txt = (*iter)->GetFullAddress();
			new_grp->AddAddress(txt);
			added = true;
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorGrpList)
	{
		unsigned long count = *((unsigned long*) drag_data);
		drag_data += sizeof(unsigned long);
		for(unsigned long i = 0; i < count; i++)
		{
			CGroup* theGrp = ((CGroup**) drag_data)[i];

			// Add group to new group
			new_grp->AddAddress(theGrp->GetNickName());
			added = true;
		}
	}
	else if ((theFlavor == GetDisplay()->GetSelectionManager()->GetMimePlainTextXAtom()) ||
			 (theFlavor == GetDisplay()->GetSelectionManager()->GetTextXAtom()))
	{
		// Parse text into list
		CAddressList list(reinterpret_cast<const char*>(drag_data), data_size);
		
		// Try to add these to table
		for(CAddressList::iterator iter = list.begin(); iter != list.end(); iter++)
		{				
			// Now add to group
			cdstring txt = (*iter)->GetFullAddress();
			new_grp->AddAddress(txt);
			added = true;
		}
	}

	// If there was a change reset both tables
	if (added)
	{
		// Check for existing action and force close
		if (mReplaceAction)
			mTableView->GetUndoer()->PostAction(NULL);

		// Create new action and post
		mReplaceAction = new CGroupTableReplaceAction(this, old_grp, new_grp, IsDirty());
		mTableView->GetUndoer()->PostAction(mReplaceAction);
	}
	else
		delete new_grp;
	
	return added;
}
