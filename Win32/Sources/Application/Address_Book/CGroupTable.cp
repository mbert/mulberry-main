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
#include "CDrawUtils.h"
#include "CEditGroupDialog.h"
#include "CErrorHandler.h"
#include "CGroup.h"
#include "CGroupList.h"
#include "CGroupTableAction.h"
#include "CIconLoader.h"
#include "CLocalAddressBook.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CSDIFrame.h"
#include "CTableRowGeometry.h"
#include "CTableRowSelector.h"
#include "CTaskClasses.h"
#include "CUserAction.h"

#include <WIN_LTableArrayStorage.h>

/////////////////////////////////////////////////////////////////////////////
// CGroupTable

IMPLEMENT_DYNCREATE(CGroupTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CGroupTable, CHierarchyTableDrag)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnNewMessage)
	ON_COMMAND(IDC_ADDRESS_NEWMSG, OnNewMessage)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CUT, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_CUT, OnEditCut)
	ON_UPDATE_COMMAND_UI(ID_EDIT_COPY, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_UPDATE_COMMAND_UI(ID_EDIT_PASTE, OnUpdateEditPaste)
	ON_COMMAND(ID_EDIT_PASTE, OnEditPaste)
	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateSelection)
	ON_COMMAND(ID_EDIT_CLEAR, DeleteSelection)
	ON_COMMAND(IDC_ActionDeleted, OnCmdActionDeleted)
	ON_COMMAND(IDC_ActionDeleted, OnCmdActionDeleted)

	ON_UPDATE_COMMAND_UI(IDC_ADDRESS_EDIT, OnUpdateSelection)
	ON_COMMAND(IDC_ADDRESS_EDIT, OnEditGroup)

	ON_UPDATE_COMMAND_UI(IDC_ADDRESS_DELETE, OnUpdateSelection)
	ON_COMMAND(IDC_ADDRESS_DELETE, OnDeleteGroup)

	ON_WM_CREATE()
	ON_WM_SIZE()
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnNewMessage)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnNewMessageOption)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARADDRESSBOOKEDITBTN, OnUpdateSelection)
	ON_COMMAND(IDC_TOOLBARADDRESSBOOKEDITBTN, OnEditGroup)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARADDRESSBOOKDELETEBTN, OnUpdateSelection)
	ON_COMMAND(IDC_TOOLBARADDRESSBOOKDELETEBTN, OnDeleteGroup)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CGroupTable::CGroupTable()
{
	mTableView = NULL;
	mParentView = NULL;
	mEditAction = NULL;
	mReplaceAction = NULL;
	mDirty = false;

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

int CGroupTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDragFlavor(CF_UNICODETEXT);

	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAddrList);
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDropFlavor(CF_UNICODETEXT);

	SetReadOnly(false);
	SetDropCell(true);
	SetAllowDrag(true);
	SetSelfDrag(true);
	SetAllowMove(false);

	// Find table view in super view chain
	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CAddressBookView*>(parent))
		parent = parent->GetParent();
	mTableView = dynamic_cast<CAddressBookView*>(parent);

	return 0;
}

// Resize columns
void CGroupTable::OnSize(UINT nType, int cx, int cy)
{
	CHierarchyTableDrag::OnSize(nType, cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Single column = whole width
	if (cy)
		SetColWidth(cx, 1, 1);
}

// Set dirty flag
void  CGroupTable::SetDirty(bool dirty)
{
	mDirty = dirty;
}

// Is it dirty
bool  CGroupTable::IsDirty(void)
{
	return dynamic_cast<CLocalAddressBook*>(mAdbk) && mDirty;
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
	// Prevent selection changes while doing multi-remove
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
	for(short i = 0; i < grp->GetAddressList().size(); i++)
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
	// Prevent selection changes while doing multi-remove
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
	// Prevent selection changes while doing multi-remove
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
		for(woRow = 1; woRow <= mCollapsableTree->CountNodes(); woRow++)
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
	// Prevent selection changes while doing multi-remove
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
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	short pos = mAdbk->GetGroupList()->FetchIndexOf(grp);

	// Find position of group
	int woRow;
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
void CGroupTable::ResetTable(void)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	// Delete all existing rows
	RemoveAllRows(true);

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

	mTableView->UpdateCaptions();
	RedrawWindow();
}

// Reset the table from the address list
void CGroupTable::ClearTable()
{
	// Remove all rows
	Clear();
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

void CGroupTable::OnNewMessage(void)
{
	CreateNewLetter(false);
}

void CGroupTable::OnNewMessageOption(void)
{
	CreateNewLetter(true);
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
void CGroupTable::CreateNewEntry(void)
{
	{
		// Create the dialog
		CEditGroupDialog dlog(CSDIFrame::GetAppTopWindow());

		// Let DialogHandler process events
		if (dlog.DoModal() == IDOK)
		{
			// Create new group and add to list
			CGroup* new_grp = new CGroup();

			// Get strings and update group if changed
			dlog.GetFields(new_grp);

			// Only add if some text available
			if (!new_grp->IsEmpty())
			{
				// Initiate action
				mTableView->GetUndoer()->PostAction(new CGroupTableNewAction(this, new_grp, IsDirty()));
			}
			else
				delete new_grp;
		}
	}
	
	SetFocus();
}

// Edit selected entries
void CGroupTable::DoEditEntry(void)
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
	
	SetFocus();
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
		// Create the dialog
		CEditGroupDialog dlog(CSDIFrame::GetAppTopWindow());
	
		dlog.SetFields(copy);
	
		// Let DialogHandler process events
		if (dlog.DoModal() == IDOK)
		{
			// Get strings and update address if changed
			done_edit = dlog.GetFields(copy);

			// Add info to action
			mEditAction->AddEdit(item.mGroup, copy);
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
void CGroupTable::DoDeleteEntry(void)
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
	// Add all groups
	mAdbk->AddGroup(grps, true);

	SelectGroups(grps);
	
	mTableView->UpdateCaptions();
	SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
}

// Change from list
void CGroupTable::ChangeGroupsFromList(CGroupList* old_grps, CGroupList* new_grps)
{
	// Delete from list
	mAdbk->UpdateGroup(old_grps, new_grps);

	// Reset only after all changes have happened
	UnselectAllCells();
	RedrawWindow();	
	mTableView->UpdateCaptions();

	SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
}

// Delete from list
void CGroupTable::RemoveGroupsFromList(CGroupList* grps)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Delete from list
	mAdbk->RemoveGroup(grps);

	// Reset only after all changes have happened
	UnselectAllCells();
	RedrawWindow();	
	mTableView->UpdateCaptions();
	SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
}

// Delete from list
void CGroupTable::SelectGroups(CGroupList* grps)
{
	// Prevent selection changes while doing multi-remove
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
			CGroupList::const_iterator found = ::find(grps->begin(), grps->end(), grp);
			if (found != grps->end())
				SelectCell(STableCell(row, 1));
		}
	}

	// Bring first item into view
	ShowFirstSelection();

	// Reset only after all changes have happened
	SetDirty(dynamic_cast<CLocalAddressBook*>(mAdbk));
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
		for(short i = 0; i < item.mGroup->GetAddressList().size(); i++)
		{
			*txt += item.mGroup->GetAddressList().at(i);
			*txt += "\r\n";
		}
	}
	else
	{

		// Determine position in group
		short pos = woRow - GetParentIndex(woRow) - 1;

		// Copy and save in selected list
		*txt += item.mGroup->GetAddressList().at(pos);
		*txt += "\r\n";
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
void CGroupTable::OnUpdateEditPaste(CCmdUI* pCmdUI)
{
	// Check for text in clipboard
	pCmdUI->Enable(::IsClipboardFormatAvailable(CF_UNICODETEXT));
}

// Command handlers
void CGroupTable::OnEditCut(void)
{
	// Copy first
	OnEditCopy();
	
	// Delete selection
	DeleteSelection();
}

void CGroupTable::OnEditCopy(void)
{
	// Add all selected groups to text
	cdstring txt;
	DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupText, &txt);

	// Convert to utf16
	cdustring utf16(txt);

	// Now copy to scrap
	if (!::OpenClipboard(*this))
		return;
	::EmptyClipboard();

	// Allocate global memory for the text
	HANDLE hglb = ::GlobalAlloc(GMEM_DDESHARE, (utf16.length() + 1) * sizeof(unichar_t));
	if (hglb)
	{
		// Copy to global after lock
		unichar_t* lptstr = (unichar_t*) ::GlobalLock(hglb);
		::unistrcpy(lptstr, utf16);
		::GlobalUnlock(hglb);
		
		::SetClipboardData(CF_UNICODETEXT, hglb);
	}
	::CloseClipboard();
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

		for(short i = 0; i < theGrp->GetAddressList().size(); i++)
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
		short pos = woRow - GetParentIndex(woRow) - 1;

		// Get address
		CGroup* theGrp = item.mGroup;
		cdstring& aStr = theGrp->GetAddressList().at(pos);

		// Add CR before if multi-copy
		if (txt->length())
			*txt += "\r\n";
		
		*txt += aStr;
	}
	
	return true;
}		

void CGroupTable::OnEditPaste(void)
{
	if (!::OpenClipboard(*this))
		return;
	HANDLE hglb = ::GetClipboardData(CF_UNICODETEXT);
	if (hglb)
	{
		LPWSTR lptstr = (LPWSTR) ::GlobalLock(hglb);

		// Convert to utf8
		cdstring txt(lptstr);

		// Add all addresses to each group
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddAddressTextToGroup, txt);

		::GlobalUnlock(hglb);
	}
	::CloseClipboard();
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
	{
		cdustring utf16(txt);
		DropDataIntoCell(CF_UNICODETEXT, (char*)utf16.c_str(), ::unistrlen(utf16) * sizeof(unichar_t), STableCell(row, 1));
	}
	
	return true;
}		

void CGroupTable::OnCmdActionDeleted(void)
{
	if (mEditAction)
		mEditAction = NULL;
	if (mReplaceAction)
		mReplaceAction = NULL;
}

#pragma mark ____________________________________Keyboard/Mouse

// Clicked item
void CGroupTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	CHierarchyTableDrag::LClickCell(inCell, nFlags);
	DoSingleClick(inCell.row, CKeyModifiers(0));
}

// Double-clicked item
void CGroupTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoDoubleClick(inCell.row, CKeyModifiers(0));
}

// Handle key down
bool CGroupTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Look for preview/full view based on key stroke

	// Determine whether preview is triggered
	CKeyModifiers mods(0);
	const CUserAction& preview = mTableView->GetPreviewAction();
	if ((preview.GetKey() == nChar) &&
		(preview.GetKeyModifiers() == mods))
	{
		DoPreview();
		return true;
	}

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if ((fullview.GetKey() == nChar) &&
		(fullview.GetKeyModifiers() == mods))
	{
		DoFullView();
		return true;
	}

	switch(nChar)
	{
	case VK_BACK:
	case VK_DELETE:
		DoDeleteEntry();
		break;

	case VK_TAB:
		mTableView->FocusAddress();
		mTableView->SetFocus();
		break;

	default:
		// Allow typing selection from any alphanumeric character - not cmd-xx
		if ((::GetKeyState(VK_CONTROL) >= 0) &&
			(::GetKeyState(VK_MENU) >= 0))
		{
			char hit_key = (nChar & 0x000000FF);
			if (((hit_key >= 'A') && (hit_key <= 'Z')) ||
				((hit_key >= 'a') && (hit_key <= 'z')) ||
				((hit_key >= '0') && (hit_key <= '9')))
				{
				
				// See if key hit within standard double click time
				unsigned long new_time = clock()/CLOCKS_PER_SEC;
				if (new_time - mLastTyping >= 1)
				{
				
					// Outside double-click time so new selection
					mLastChars[0] = hit_key;
					mLastChars[1] = 0;
				}
				else
				{
					// Inside double-click time so add char to selection
					short pos = ::strlen(mLastChars);
					if (pos < 31)
					{
						mLastChars[pos++] = hit_key;
						mLastChars[pos] = 0;
					}
				}
				mLastTyping = new_time;
				
				DoKeySelection();
			}
			else
				// Did not handle key
				return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
		}
		else
			// Did not handle key
			return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
	}

	// Handled key
	return true;
}

// Select from key press
void CGroupTable::DoKeySelection(void)
{
	// Prevent selection changes while doing multi-remove
	StDeferSelectionChanged _defer(this);

	// Create new address for typed chars
	CGroup* temp = new CGroup(mLastChars);
	
	// Get low bound insert pos
	long select = mAdbk->GetGroupList()->FetchInsertIndexOf(temp);
	
	// Clip to end of list
	if (select > mAdbk->GetGroupList()->size()) select--;

	// Find exposed position of group
	int pos;
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

// Draw the titles
void CGroupTable::DrawCell(CDC* pDC, const STableCell& inCell, const CRect& inLocalRect)
{
	StDCState save(pDC);

	TableIndexT woRow = GetWideOpenIndex(inCell.row);

	STableCell	woCell(woRow, inCell.col);
	SGroupTableItem item;
	UInt32 dataSize = sizeof(SGroupTableItem);
	GetCellData(woCell, &item, dataSize);

	UINT iconID;
	const char* content;
	if (item.mIsGroup)
	{
		CGroup* grp = item.mGroup;
		iconID = IDI_GROUP_FLAG;
		content = grp->GetName();
	}
	else
	{
		CGroup* grp = item.mGroup;
		iconID = IDI_ADDRESS_FLAG;

		// Determine position in group
		short pos = woRow - GetParentIndex(woRow) - 1;

		// Determine name to use
		content = grp->GetAddressList().at(pos);
	}

	DrawHierarchyRow(pDC, inCell.row, inLocalRect, content, iconID);

}

#pragma mark ____________________________________Drag&Drop

// Limit flavors based on selection
void CGroupTable::SetDragFlavors(TableIndexT row)
{
	// Determine whether to do groups or address only
	bool all_grp = TestSelectionAnd((TestSelectionPP) &CGroupTable::TestSelectionGroup);

	// If all groups allow group list drag
	if (all_grp)
		mDrag.DelayRenderData(CMulberryApp::sFlavorGrpList);
	else
		mDrag.DelayRenderData(CMulberryApp::sFlavorAddrList);

	// Always do text
	mDrag.DelayRenderData(CF_UNICODETEXT);

}

// Prevent cell drop when group
bool CGroupTable::ItemIsAcceptable(COleDataObject* pDataObject)
{
	// Do not allow scroll if dragging groups
	SetDropCell(!pDataObject->IsDataAvailable(CMulberryApp::sFlavorGrpList) || (sTableDragSource == this));

	// Do inherited
	return CHierarchyTableDrag::ItemIsAcceptable(pDataObject);
}

// Determine if drop into cell
bool CGroupTable::IsDropCell(COleDataObject* pDataObject, const STableCell& cell)
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

BOOL CGroupTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  (lpFormatEtc->cfFormat == CMulberryApp::sFlavorGrpList)
	{
		// Create list to hold dragged items
		CGroupList grps;

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddGroupToList, &grps);
		
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, grps.size() * sizeof(CGroup*) + sizeof(int));
		if (*phGlobal)
		{
			// Copy to global after lock
			CGroup** pGrp = (CGroup**) ::GlobalLock(*phGlobal);
			*((int*) pGrp) = grps.size();
			pGrp += sizeof(int);
			for(CGroupList::iterator iter = grps.begin(); iter != grps.end(); iter++)
				*pGrp++ = *iter;
			//::memcpy(pGrp, grps.begin(), grps.size() * sizeof(CGroup*));
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
		
		// Do not delete originals
		grps.clear_without_delete();
	}

	else if  (lpFormatEtc->cfFormat == CMulberryApp::sFlavorAddrList)
	{
		// Create list to hold dragged items
		CAddressList addrs;

		// Add each selected address
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddAddressesToList, &addrs);
		
		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, addrs.size() * sizeof(CAddress*) + sizeof(int));
		if (*phGlobal)
		{
			// Copy to global after lock
			CAddress** pAddr = (CAddress**) ::GlobalLock(*phGlobal);
			*((int*) pAddr) = addrs.size();
			pAddr += sizeof(int);
			for(CAddressList::iterator iter = addrs.begin(); iter != addrs.end(); iter++)
				*pAddr++ = *iter;
			//::memcpy(pAddr, addrs.begin(), addrs.size() * sizeof(CAddress*));
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
		
		// Do not delete originals <- do this as address are all copies
		addrs.clear_without_delete();
	}

	else if (lpFormatEtc->cfFormat == CF_UNICODETEXT)
	{
		// Add all selected address to text
		cdstring txt;
		DoToSelection1((DoToSelection1PP) &CGroupTable::AddToText, &txt);

		// Convert to utf16
		cdustring utf16(txt);

		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, (utf16.length() + 1) * sizeof(unichar_t));
		if (*phGlobal)
		{
			// Copy to global after lock
			unichar_t* lptstr = (unichar_t*) ::GlobalLock(*phGlobal);
			::unistrcpy(lptstr, utf16);
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
	}
	
	return rendered;
}

// Drop data into whole table
bool CGroupTable::DropData(unsigned int inFlavor, char* drag_data, unsigned long data_size)
{
	if (inFlavor == CMulberryApp::sFlavorGrpList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		
		CGroupTableNewAction* add_action = NULL;

		for(int i = 0; i < count; i++)
		{
			CGroup* theGrp = ((CGroup**) drag_data)[i];

			// Does it already exist?
			if (!mAdbk->GetGroupList()->IsDuplicate(theGrp))
			{
				CGroup* copy = new CGroup(*theGrp);
				
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
bool CGroupTable::DropDataIntoCell(unsigned int theFlavor, char* drag_data,
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
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CMessage* theMsg = ((CMessage**) drag_data)[i];
			CEnvelope* theEnv = theMsg->GetEnvelope();

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
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CAddress* theAddr = ((CAddress**) drag_data)[i];
			{
				// Add to list
				cdstring txt = theAddr->GetFullAddress();
				new_grp->AddAddress(txt);
				added = true;
			}
		}
	}
	else if (theFlavor == CMulberryApp::sFlavorGrpList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int*);
		for(int i = 0; i < count; i++)
		{
			CGroup* theGrp = ((CGroup**) drag_data)[i];

			// Add group to new group
			new_grp->AddAddress(theGrp->GetNickName());
			added = true;
		}
	}
	else if (theFlavor == CF_UNICODETEXT)
	{
		// Parse text into list
		cdustring utf16((unichar_t*)drag_data, data_size / sizeof(unichar_t));
		cdstring utf8 = utf16.ToUTF8();
		CAddressList list(utf8, utf8.length());
		
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
