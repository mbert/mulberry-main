/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// Source for CAdbkManagerTable class

#include "CAdbkManagerTable.h"

#include "CAdbkManagerWindow.h"
#include "CAdbkPropDialog.h"
#include "CAdbkProtocol.h"
#include "CAdbkSearchWindow.h"
#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "CAddressBookView.h"
#include "CAddressBookWindow.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CBalloonDialog.h"
#include "CCommands.h"
#endif
#include "CCreateAdbkDialog.h"
#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CResources.h"
#endif
#include "CPreferences.h"
#include "CTaskClasses.h"
#if __dest_os != __win32_os
#include "CWaitCursor.h"
#endif

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"
#include "CTableMultiRowSelector.h"

#include <LDropFlag.h>
#include <LNodeArrayTree.h>
#include <LTableArrayStorage.h>
#else
#include "StValueChanger.h"
#endif

#include <stdio.h>
#include <string.h>

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// O T H E R  M E T H O D S ____________________________________________________________________________

// Set the address book manager
void CAdbkManagerTable::SetManager(CAddressBookManager* manager)
{
	// Save server
	mManager = manager;
	
	if (mManager)
	{
		mManager->Add_Listener(this);

		// Make sure its open so items will actually be displayed
		mTableView->SetOpen();

		// Calculate number of rows for first time
		ResetTable();
	}
}

// Reset the table
void CAdbkManagerTable::ResetTable()
{
	// Start cursor for busy operation
	CWaitCursor wait;

	// Delete all existing rows
	Clear();
	mHierarchyCol = 1;
	mData.clear();

	// Only if we have the manager
	if (!mManager)
		return;

	// Get list from manager
	const CAddressBookList& nodes = mManager->GetNodes();

	// Add each node
	TableIndexT row = 0;
	TableIndexT exp_row = 1;
	for(CAddressBookList::const_iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		AddNode(*iter, row, false);

		// Listen to each protocol
		CAdbkProtocol* proto = (*iter)->GetProtocol();
		if (proto)
			proto->Add_Listener(this);
		
		// Bump up exposed row counter
		exp_row++;
	}

	// Always expand a single account
	if (exp_row == 2)
		ExpandRow(1);

	// Refresh list
	FRAMEWORK_REFRESH_WINDOW(this)
	
	// Force toolbar update
	mTableView->RefreshToolbar();

}

// Reset the table from the mboxList
void CAdbkManagerTable::ClearTable()
{
	// Delete all existing rows
	Clear();
	mData.clear();

	// Refresh list
	FRAMEWORK_REFRESH_WINDOW(this)
	
	// Force toolbar update
	mTableView->RefreshToolbar();
}

void CAdbkManagerTable::ListenTo_Message(long msg, void* param)
{
	// Only do these if the parent view is open
	if (!mTableView->IsOpen())
		return;

	switch (msg)
	{
	case CAddressBookManager::eBroadcast_NewAdbkAccount:
		AddProtocol(static_cast<CAdbkProtocol*>(param));
		break;
	case CAddressBookManager::eBroadcast_RemoveAdbkAccount:
		RemoveProtocol(static_cast<CAdbkProtocol*>(param));
		break;
	case CAdbkProtocol::eBroadcast_ClearList:
		ClearProtocol(static_cast<CAdbkProtocol*>(param));
		break;
	case CAdbkProtocol::eBroadcast_RefreshList:
		RefreshProtocol(static_cast<CAdbkProtocol*>(param));
		break;
	case CINETProtocol::eBroadcast_Logon:
		SwitchProtocol(static_cast<CAdbkProtocol*>(param));
		break;
	case CINETProtocol::eBroadcast_Logoff:
		SwitchProtocol(static_cast<CAdbkProtocol*>(param));
		break;
	case CAdbkProtocol::eBroadcast_ClearSubList:
		ClearSubList(static_cast<CAddressBook*>(param));
		break;
	case CAdbkProtocol::eBroadcast_RefreshSubList:
		RefreshSubList(static_cast<CAddressBook*>(param));
		break;
	case CAddressBookManager::eBroadcast_InsertNode:
		InsertNode(static_cast<CAddressBook*>(param));
		break;
	case CAddressBookManager::eBroadcast_RemoveNode:
	case CAddressBookManager::eBroadcast_DeleteNode:
		DeleteNode(static_cast<CAddressBook*>(param));
		break;
	case CAdbkProtocol::eBroadcast_RefreshNode:
		RefreshNode(static_cast<CAddressBook*>(param));
		break;
	}
}

void CAdbkManagerTable::DoSelectionChanged()
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
void CAdbkManagerTable::DoSingleClick(unsigned long row, const CKeyModifiers& mods)
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
void CAdbkManagerTable::DoDoubleClick(unsigned long row, const CKeyModifiers& mods)
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

void CAdbkManagerTable::DoPreview()
{
	PreviewAddressBook();
}

void CAdbkManagerTable::DoPreview(CAddressBook* adbk)
{
	PreviewAddressBook(adbk);
}

// Just edit the item
void CAdbkManagerTable::DoFullView()
{
	OnOpenAddressBook();
}

// Do preview of address book
void CAdbkManagerTable::PreviewAddressBook(bool clear)
{
	// Ignore if no preview pane
	if (!mTableView->GetPreview())
		return;

	CAddressBook* adbk = NULL;
	ulvector sels;
	if (!clear)
	{
		// Add selection to list
		CAddressBookList selected;
		selected.set_delete_data(false);
		if (DoToSelection1((DoToSelection1PP) &CAdbkManagerTable::AddSelectionToList, &selected) && (selected.size() == 1))
		{
			adbk = selected.front();
			if (!adbk->IsAdbk())
				adbk = NULL;
		}
	}
	
	// Do preview of specific address book only if it exists or its being cleared
	if (adbk || clear)
		PreviewAddressBook(adbk, clear);
}

// Do preview of address book
void CAdbkManagerTable::PreviewAddressBook(CAddressBook* adbk, bool clear)
{
	// Ignore if no preview pane
	if (!mTableView->GetPreview())
		return;

	bool changed = clear;
	ulvector sels;
	if (!clear)
	{
		// Check whether different
		CAddressBook* old_adbk = mTableView->GetPreview()->GetAddressBook();
		cdstring old_url = old_adbk ? old_adbk->GetAccountName() : cdstring::null_str;
		if ((adbk ? adbk->GetAccountName() : cdstring::null_str) != old_url)
		{
			// Set flag to force update
			changed = true;
		}
	}

	// Only do preview if there has been a change
	if (changed)
	{
		// Always clear out message preview immediately, otherwise
		// do preview at idle time to prevent re-entrant network calls
		if (adbk)
		{
			// Give it to preview (if its NULL the preview will be cleared)
			CAddressBookPreviewTask* task = new CAddressBookPreviewTask(mTableView->GetPreview(), adbk);
			task->Go();
		}
		else
			// do immediate update of preview
			mTableView->GetPreview()->SetAddressBook(adbk);
	}
}

// Get properties of mailbox
void CAdbkManagerTable::OnAddressBookProperties(void)
{
	CAddressBookList selected;
	selected.set_delete_data(false);

	try
	{
		// Add selection to list
		if (DoToSelection1((DoToSelection1PP) &CAdbkManagerTable::AddSelectionToList, &selected))
		{
			// Check all addressbooks first
			for(CAddressBookList::iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				long new_msgs = 0;

				// IMPORTANT: if the connection is lost the addressbooks are removed by the protocol
				// being logged off. Must protect against this case
				const CAdbkProtocol* proto = (*iter)->GetProtocol();

				// Must not allow one failure to stop others

				// Get extension info
				try
				{
					(*iter)->CheckMyRights();
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Throw up if proto has died
					if (!proto->IsLoggedOn())
					{
						CLOG_LOGRETHROW;
						throw;
					}
				}

				try
				{
					(*iter)->CheckACLs();
				}
				catch (...)
				{
					CLOG_LOGCATCH(...);

					// Throw up if proto has died
					if (!proto->IsLoggedOn())
					{
						CLOG_LOGRETHROW;
						throw;
					}
				}
			}

#if __dest_os == __mac_os || __dest_os == __mac_os_x
			// Create the dialog
			CBalloonDialog	theHandler(paneid_AdbkPropDialog, this);
			CAdbkPropDialog* prop_dlog = (CAdbkPropDialog*) theHandler.GetDialog();
			prop_dlog->SetAdbkList(&selected);
			long set = 1;
			prop_dlog->ListenToMessage(msg_SetPropAdbkGeneral, &set);
			theHandler.StartDialog();

			// Let DialogHandler process events
			while (true)
			{
				MessageT	hitMessage = theHandler.DoDialog();

				if (hitMessage == msg_OK)
				{
					break;
				}
				else if (hitMessage == msg_Cancel)
				{
					break;
				}
			}
#elif __dest_os == __win32_os
			// Create the dialog
			CAdbkPropDialog dlog(this);
			dlog.SetAdbkList(&selected);

			// Let DialogHandler process events
			dlog.DoModal();
#elif __dest_os == __linux_os
			CAdbkPropDialog::PoseDialog(&selected);
#else
#error __dest_os
#endif
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Catch errors to prevent further processing
	}

	// Always refresh
	RefreshSelection();

#if __dest_os == __win32_os
	// Set focus back to table
	SetFocus();
#endif

#if __dest_os == __linux_os
	Focus();
#endif

} // CAdbkManagerTable::DoAddressBookProperties

void CAdbkManagerTable::OnLogin()
{
	// Must have single selected protocol
	if (!IsSingleSelection())
		return;
	
	TableIndexT row = GetFirstSelectedRow();
	CAddressBook* adbk = GetCellNode(row);
	
	if (!adbk->IsProtocol())
		return;
	
	// Policy:
	//
	// 1.	Local protocols are always logged in
	// 2.	Protocols that cannot disconnect
	// 2.1	can change login state only when global state is connected
	// 2.2	are always logged out when global state is disconnected
	// 3.	Protocols that can disconnect
	// 3.1	can change login/disconnect state when global state is connected
	// 3.1	are always logged in when global state is disconnected

	// 1. (as above)
	if (adbk->GetProtocol()->IsOffline() && !adbk->GetProtocol()->IsDisconnected())
	{
		// Ignore - should already be in logged in state
		return;
	}

	// 2. (as above)
	else if (!adbk->GetProtocol()->CanDisconnect())
	{
		// 2.1 (as above)
		if (CConnectionManager::sConnectionManager.IsConnected())
		{
			// Toggle login state
			if (adbk->GetProtocol()->IsLoggedOn())
			{
				mManager->StopProtocol(adbk->GetProtocol());
			}
			else
			{
				mManager->StartProtocol(adbk->GetProtocol());
			}
		}
		// 2.2 (as above)
		else
		{
			// Ignore - should already be in logged out state
			return;
		}
	}
	
	// 3. (as above)
	else
	{
		// 3.1 (as above)
		if (CConnectionManager::sConnectionManager.IsConnected())
		{
			// Toggle force disconnect state
			adbk->GetProtocol()->ForceDisconnect(!adbk->GetProtocol()->IsForceDisconnect());
		}
		// 3.2 (as above)
		else
		{
			// Ignore - should already be in logged in state
			return;
		}
	}

	// Reset all views
	//CCalendarView::ResetAll();
}

// New address book
void CAdbkManagerTable::OnNewAddressBook(void)
{
	CCreateAdbkDialog::SCreateAdbk create;
	create.open_on_startup = true;
	create.use_nicknames = true;
	create.use_search = true;

	CAdbkProtocol* proto = NULL;
	CAddressBook* node = NULL;
	CAddressBook* result = NULL;
	bool insert_delim = false;

	// Determine initial creation type
	if (IsSelectionValid())
	{
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row);

		// First check logged in state of server - may come back as NULL if cabinet selected
		CAdbkProtocol* server = GetCellAdbkProtocol(row);
		node = GetCellNode(row);

		// If logged in then use current selection as reference point
		if (server && server->IsLoggedOn())
		{
			proto = server;
			create.account = proto->GetAccountName();

			if (node->IsProtocol())
			{
				create.use_wd = false;
			}
			else if (node->IsDirectory())
			{
				create.use_wd = true;
				create.parent = node->GetName();
			}
			else
			{
				create.use_wd = false;
			}
		}
		else
		{
			// Force generic create
			create.use_wd = false;
		}
	}

	try
	{
		if (CCreateAdbkDialog::PoseDialog(&create))
		{
			cdstring new_name;
			if (create.use_wd)
			{
				new_name = create.parent;
				if (node->GetProtocol()->GetDirDelim())
					new_name += node->GetProtocol()->GetDirDelim();
				new_name += create.name;
			}
			else
				new_name = create.name;

			// Check and get proto from dialog
			if (!proto)
				proto = mManager->GetProtocol(create.account);
			if (!proto)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			CAddressBook* adbk = NULL;

			// Create address book
			adbk = mManager->NewAddressBook(proto, new_name, create.directory);

			if (adbk && !create.directory)
			{
				// Set chosen flags
				adbk->SetFlags(CAddressBook::eOpenOnStart, create.open_on_startup);
				adbk->SetFlags(CAddressBook::eLookup, create.use_nicknames);
				adbk->SetFlags(CAddressBook::eSearch, create.use_search);

				// Change prefs list
				mManager->SyncAddressBook(adbk, true);
				CPreferences::sPrefs->ChangeAddressBookOpenOnStart(adbk, create.open_on_startup);
				CPreferences::sPrefs->ChangeAddressBookLookup(adbk, create.use_nicknames);
				CPreferences::sPrefs->ChangeAddressBookSearch(adbk, create.use_search);
			}
			
			result = adbk;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Open address book
void CAdbkManagerTable::OnOpenAddressBook(void)
{
	// Display each selected address book
	DoToSelection((DoToSelectionPP) &CAdbkManagerTable::OpenAddressBook);
}

// Display a specified address book
bool CAdbkManagerTable::OpenAddressBook(TableIndexT row)
{
	// Get node
	CAddressBook* adbk = GetCellNode(row);

	// Must be a real address book
	if (!adbk->IsAdbk())
		return false;
	
	// Open it
	return CAddressBookWindow::OpenWindow(adbk);
}

// Rename address books
void CAdbkManagerTable::OnRenameAddressBook(void)
{
	// Display each selected message
	DoToSelection((DoToSelectionPP) &CAdbkManagerTable::RenameAddressBook);
}

// Rename specified address book
bool CAdbkManagerTable::RenameAddressBook(TableIndexT row)
{
	bool failagain = false;
	CAddressBook* adbk = GetCellNode(row);

	if (adbk == NULL)
		return false;

	do
	{
		failagain = false;

		// Get a new name for the mailbox (use old name as starter)
		cdstring new_name;
		new_name = adbk->GetName();
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		if (CGetStringDialog::PoseDialog("Alerts::Adbk::Rename", new_name))
#else
		if (CGetStringDialog::PoseDialog("Alerts::Adbk::RenameTitle", "Alerts::Adbk::Rename", new_name))
#endif
		{
			try
			{
				mManager->RenameAddressBook(adbk, new_name);
				RefreshRow(row);
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				failagain = true;
			}
		}
	}
	while (failagain);

	return false;
}

// Delete address books
void CAdbkManagerTable::OnDeleteAddressBook(void)
{
	// Check that this is what we want to do
	try
	{
		// Get all selected mailboxes
		CAddressBookList selected;
		selected.set_delete_data(false);
		if (DoToSelection1((DoToSelection1PP) &CAdbkManagerTable::AddSelectionToList, &selected))
		{
			// Generate string for list of items being deleted
			cdstring deletes;
			for(CAddressBookList::const_iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				if (!deletes.empty())
					deletes += ", ";
				deletes += (*iter)->GetAccountName(mManager->HasMultipleProtocols());
			}
			
			// Now ask user about delete, providing list of items being deleted
			if (CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Adbk::ReallyDelete", deletes) == CErrorHandler::Ok)
			{
				// Delete all mboxes in reverse
				for(CAddressBookList::reverse_iterator riter = selected.rbegin(); riter != selected.rend(); riter++)
					mManager->DeleteAddressBook(*riter);
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Search address books
void CAdbkManagerTable::OnSearchAddressBook(void)
{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	CMulberryApp::sApp->ObeyCommand(cmd_SearchAddressBook);
#elif __dest_os == __win32_os || __dest_os == __linux_os
	CMulberryApp::sApp->OnAppSearchAddress();
#else
#error __dest_os
#endif
}

// Refresh servers
void CAdbkManagerTable::OnRefreshAddressBook(void)
{
	// Only if we have a single selection
	if (!IsSingleSelection())
		return;
	
	// Get node
	TableIndexT row = GetFirstSelectedRow();
	CAddressBook* adbk = GetCellNode(row);

	// If the node is an ordinary calendar then refresh is done on the parent
	if (adbk->IsAdbk())
		adbk = adbk->GetParent();
	
	// Refresh based on type
	if (adbk->IsProtocol())
		adbk->GetProtocol()->RefreshList();
	else if (adbk->IsDirectory())
		adbk->GetProtocol()->LoadSubList(adbk, false);
	
}

// Synchronise address book
void CAdbkManagerTable::OnSynchroniseAddressBook(void)
{
	// Display each selected message
	DoToSelection((DoToSelectionPP) &CAdbkManagerTable::SynchroniseAddressBook);
}

// Refresh specified server
bool CAdbkManagerTable::SynchroniseAddressBook(TableIndexT row)
{
	CAddressBook* adbk = GetCellNode(row);

	try
	{
		adbk->Synchronise(false);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	RefreshRow(row);

	return false;
}

// Clear Disconnected address book
void CAdbkManagerTable::OnClearDisconnectAddressBook(void)
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Adbk::ReallyClearDisconnect") == CErrorHandler::Ok)
	{
		// Display each selected message
		DoToSelection((DoToSelectionPP) &CAdbkManagerTable::ClearDisconnectAddressBook);
	}
}

// Clear Disconnected specified address book
bool CAdbkManagerTable::ClearDisconnectAddressBook(TableIndexT row)
{
	CAddressBook* adbk = GetCellNode(row);

	try
	{
		// Must close the address book if its open in disconnected mode
		if (adbk->GetProtocol()->IsDisconnected() && adbk->IsOpen())
			adbk->Close();

		// Now clear out disconnected cache
		adbk->ClearDisconnect();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

	RefreshRow(row);

	return false;
}

// Add selected mboxes to list
bool CAdbkManagerTable::AddSelectionToList(TableIndexT row, CAddressBookList* list)
{
	CAddressBook* adbk = GetCellNode(row);

	// Determine delete mailbox
	list->push_back(adbk);

	return true;
}

// Remove rows and adjust parts
#if __dest_os == __mac_os_x
void CAdbkManagerTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, Boolean inRefresh)
#else
void CAdbkManagerTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh)
#endif
{
	// Count number to remove (this + descendents)
	UInt32 remove_count = CountAllDescendents(inFromRow) + 1;

	// Do standard removal
	CHierarchyTableDrag::RemoveRows(inHowMany, inFromRow, inRefresh);
	
	// Remove our data using count
	mData.erase(mData.begin() + (inFromRow - 1), mData.begin() + (inFromRow - 1 + remove_count));
}

// Get the selected adbk
CAddressBook* CAdbkManagerTable::GetCellNode(TableIndexT row, bool worow) const
{
	TableIndexT	woRow = (worow ? row : mCollapsableTree->GetWideOpenIndex(row));
	return mData[woRow - 1];
}

// Get the requested calendar protocol
CAdbkProtocol* CAdbkManagerTable::GetCellAdbkProtocol(TableIndexT row) const
{
	CAddressBook* node = GetCellNode(row);
	return node->GetProtocol();
}

// Add a node to the list
void CAdbkManagerTable::AddNode(CAddressBook* node, TableIndexT& row, bool child, bool refresh)
{
	TableIndexT parent_row;

	if (child)
	{
		parent_row = AddLastChildRow(row, &node, sizeof(CAddressBook*), node->IsDirectory() && (!node->IsAdbk() || node->HasInferiors()), refresh);
	}
	else
	{
		parent_row = row = InsertSiblingRows(1, row, &node, sizeof(CAddressBook*), node->IsDirectory() && (!node->IsAdbk() || node->HasInferiors()), refresh);
	}
	mData.insert(mData.begin() + (parent_row - 1), node);

	// Insert children
	AddChildren(node, parent_row, refresh);

	// Expand if previously expanded (do this AFTER adding all the children)
	ExpandRestore(parent_row);
}

// Add child nodes to the list
void CAdbkManagerTable::AddChildren(const CAddressBook* node, TableIndexT& parent_row, bool refresh)
{
	// Insert children
	if (node->HasInferiors())
	{
		for(CAddressBookList::const_iterator iter = node->GetChildren()->begin(); iter != node->GetChildren()->end(); iter++)
			AddNode(*iter, parent_row, true, refresh);
	}
}

// Remove child nodes from the list
void CAdbkManagerTable::RemoveChildren(TableIndexT& parent_row, bool refresh)
{
	// Sledge-hammer approach!
	{
		// Prevent window update during changes
		StDeferTableAdjustment changing(this);

		while(CountAllDescendents(parent_row))
		{
			RemoveRows(1, parent_row + 1, false);
		}
	}

	if (refresh)
		RefreshRowsBelow(parent_row);
}

// Add protocol at end of list
void CAdbkManagerTable::AddProtocol(CAdbkProtocol* proto)
{
	// Get list from manager
	const CAddressBookList& adbks = mManager->GetNodes();

	// Get last top-level sibling row
	TableIndexT row = 0;
	AddNode(adbks.back(), row, true, true);
	
	proto->Add_Listener(this);
}

// Insert protocol somewhere in the middle
void CAdbkManagerTable::InsertProtocol(CAdbkProtocol* proto)
{
	// For now do complete reset
	ResetTable();
}

// Remove protocol from list
void CAdbkManagerTable::RemoveProtocol(CAdbkProtocol* proto)
{
	unsigned long pos;
	if (mManager->FindProtocol(proto, pos))
	{
		// Remove this and all children
		TableIndexT temp = pos;
		RemoveRows(1, temp, true);
	}
}

// Clear protocol list
void CAdbkManagerTable::ClearProtocol(CAdbkProtocol* proto)
{
	unsigned long pos;
	if (mManager->FindProtocol(proto, pos))
	{
		// Remove all children
		TableIndexT temp = pos;
		RemoveChildren(temp, true);
	}
}

// Add protocol children from list
void CAdbkManagerTable::RefreshProtocol(CAdbkProtocol* proto)
{
	unsigned long pos;
	if (mManager->FindProtocol(proto, pos))
	{
		// Remove this and all children
		TableIndexT woRow = pos;
		if (!HasChildren(woRow))
		{
			TableIndexT row = GetExposedIndex(woRow);
			if (row)
			{
				CAddressBook* adbk = GetCellNode(row);
				AddChildren(adbk, woRow, true);
				RefreshRow(row);
			}
		}
	}
}

// Remove protocol children from list
void CAdbkManagerTable::SwitchProtocol(CAdbkProtocol* proto)
{
	// Address book objects remain after logoff
	unsigned long pos;
	if (mManager->FindProtocol(proto, pos))
	{
		// Refresh this and all children
		TableIndexT woRow = pos;

		bool		exposed = true;
		TableIndexT	exRow = 0;

		if (woRow > 0)
		{
			exRow = GetExposedIndex(woRow);
			exposed = (exRow > 0);
		}

		if (exposed)
		{
			STableCell	topLeftCell(exRow, 1);
			STableCell	botRightCell(exRow + CountExposedDescendents(woRow), mCols);
			RefreshCellRange(topLeftCell, botRightCell);
		}
	}
}

// Insert a node to the list
// NB these will always be child nodes (i.e. there will always be a parent)
void CAdbkManagerTable::InsertNode(CAddressBook* adbk)
{
	// Find its wide-open row location
	
	// Iterate back over parents accumulating offsets into parents
	uint32_t parent_row = 0;
	uint32_t sibling_row = 0;
	adbk->GetInsertRows(parent_row, sibling_row);
	
	// Now insert using child/sibling switch
	if (sibling_row == 0)
	{
		TableIndexT worow = InsertChildRows(1, parent_row, &adbk, sizeof(CAddressBook*), adbk->IsDirectory() && (!adbk->IsAdbk() || adbk->HasInferiors()), true);
		mData.insert(mData.begin() + (worow - 1), adbk);
	}
	else
	{
		TableIndexT worow = InsertSiblingRows(1, sibling_row, &adbk, sizeof(CAddressBook*), adbk->IsDirectory() && (!adbk->IsAdbk() || adbk->HasInferiors()), true);
		mData.insert(mData.begin() + (worow - 1), adbk);
	}
}

// Delete a node from the list
void CAdbkManagerTable::DeleteNode(CAddressBook* adbk)
{
	// Find its wide-open row location
	uint32_t woRow = adbk->GetRow();
	RemoveRows(1, woRow, true);
}

#pragma mark ____________________________Hierarchy Related

// Create new hierarchy
void CAdbkManagerTable::OnNewHierarchy(void)
{
	// New hierarchy dialog
	cdstring new_name;
	
	if (CGetStringDialog::PoseDialog("Alerts::Server::NewHierarchy", new_name))
	{
		//StValueChanger<bool> change(mListChanging, true);
		//list->GetProtocol()->RenameWD(wd_index, new_name);
	}
}

// Rename hierarchy
void CAdbkManagerTable::OnRenameHierarchy(void)
{
	// Add selection to list
	CAddressBookList selected;
	selected.set_delete_data(false);
	if (DoToSelection1((DoToSelection1PP) &CAdbkManagerTable::AddSelectionToList, &selected))
	{
		// Rename (do in reverse)
		for(CAddressBookList::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
		{
			CAddressBook* adbk = static_cast<CAddressBook*>(*iter);
			
			// Rename it
			cdstring new_name = adbk->GetName();
			
			if (CGetStringDialog::PoseDialog("Alerts::Server::NewHierarchy", new_name))
			{
				//StValueChanger<bool> change(mListChanging, true);
				//list->GetProtocol()->RenameWD(wd_index, new_name);
			}
			else
				// Break out of entire loop if user cancels
				break;
		}
	}
	
	RefreshSelection();
}

// Delete hierarchy
void CAdbkManagerTable::OnDeleteHierarchy(void)
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteHierarchy") == CErrorHandler::Ok)
	{
		// Prevent flashes during multiple selection changes
		StDeferSelectionChanged noSelChange(this);
		
		//		CServerNodeArray selected;
		//		
		//		// Add selection to list
		//		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
		//		{
		//			// Remove wds from account (do in reverse)
		//			for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
		//			{
		//				CMboxList* list = (CMboxList*) (*iter).mData;
		//				long wd_index = list->GetHierarchyIndex();
		//				if (wd_index >= 1)
		//					list->GetProtocol()->RemoveWD(wd_index);
		//			}
		//		}
	}
}

// Refresh a node from the list
void CAdbkManagerTable::RefreshNode(CAddressBook* adbk)
{
	// Find its wide-open row location
	uint32_t woRow = adbk->GetRow();
	TableIndexT exp_row = GetExposedIndex(woRow);
	if (exp_row)
		RefreshRow(exp_row);
}

void CAdbkManagerTable::ClearSubList(CAddressBook* adbk)
{
	// Find its wide-open row location
	TableIndexT woRow = adbk->GetRow();
	RemoveChildren(woRow, true);
}

void CAdbkManagerTable::RefreshSubList(CAddressBook* adbk)
{
	// Find its wide-open row location
	TableIndexT woRow = adbk->GetRow();
	AddChildren(adbk, woRow, true);
}

#pragma mark ____________________________Tests

// Test for selected servers only
bool CAdbkManagerTable::TestSelectionServer(TableIndexT row)
{
	CAddressBook* adbk = GetCellNode(row, false);
	return adbk->IsProtocol();
}

// Test for selected adbk
bool CAdbkManagerTable::TestSelectionAdbk(TableIndexT row)
{
	// This is an adbk
	CAddressBook* adbk = GetCellNode(row, false);
	return !adbk->IsProtocol();
}

// Test for selected adbk
bool CAdbkManagerTable::TestSelectionAdbkDisconnected(TableIndexT row)
{
	// This is an adbk
	CAddressBook* adbk = GetCellNode(row, false);
	if (!adbk->IsProtocol())
	{
		return adbk->GetProtocol()->CanDisconnect() && !adbk->GetProtocol()->IsDisconnected();
	}
	
	return false;
}

// Test for selected adbk
bool CAdbkManagerTable::TestSelectionAdbkClearDisconnected(TableIndexT row)
{
	// This is an adbk
	CAddressBook* adbk = GetCellNode(row, false);
	if (!adbk->IsProtocol())
	{
		return adbk->GetProtocol()->CanDisconnect();
	}
	
	return false;
}

// Test for selected hierarchies only
bool CAdbkManagerTable::TestSelectionHierarchy(TableIndexT row)
{
	// This is deleted
	CAddressBook* adbk = GetCellNode(row, false);
	return adbk->IsDisplayHierarchy();
}


#pragma mark ____________________________Expand/collapse

void CAdbkManagerTable::CollapseRow(UInt32 inWideOpenRow)
{
	CHierarchyTableDrag::CollapseRow(inWideOpenRow);

	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, false);
}

void CAdbkManagerTable::DeepCollapseRow(UInt32 inWideOpenRow)
{
	CHierarchyTableDrag::DeepCollapseRow(inWideOpenRow);

	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, false);
}

void CAdbkManagerTable::ExpandRow(UInt32 inWideOpenRow)
{
	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, true);

	// Force hierarchy descovery
	ExpandAction(inWideOpenRow, false);

	CHierarchyTableDrag::ExpandRow(inWideOpenRow);
}

void CAdbkManagerTable::DeepExpandRow(UInt32 inWideOpenRow)
{
	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, true);

	// Force hierarchy descovery
	ExpandAction(inWideOpenRow, true);

	CHierarchyTableDrag::DeepExpandRow(inWideOpenRow);
}

void CAdbkManagerTable::ProcessExpansion(UInt32 inWideOpenRow, bool expand)
{
	CAddressBook* adbk = GetCellNode(inWideOpenRow, true);
	
	// Add or remove from expansion preference
	if (expand)
	{
		CPreferences::sPrefs->mExpandedAdbks.Value().insert(adbk->IsProtocol() ? adbk->GetName() : adbk->GetAccountName());
	}
	else
	{
		CPreferences::sPrefs->mExpandedAdbks.Value().erase(adbk->IsProtocol() ? adbk->GetName() : adbk->GetAccountName());
	}
	CPreferences::sPrefs->mExpandedAdbks.SetDirty();
}

void CAdbkManagerTable::ExpandRestore(TableIndexT worow)
{
	// Do expansion if expand state is in prefs - bypass the prefs change
	CAddressBook* adbk = GetCellNode(worow, true);
	bool do_expand = CPreferences::sPrefs->mExpandedAdbks.GetValue().count(adbk->IsProtocol() ? adbk->GetName() : adbk->GetAccountName()) != 0;
	if (do_expand)
	{
		// Force hierarchy descovery
		ExpandAction(worow, false);

		// Do visual expand
		CHierarchyTableDrag::ExpandRow(worow);

		TableIndexT exp_row = GetExposedIndex(worow);
		if (exp_row)
			RefreshRow(exp_row);
	}
}

void CAdbkManagerTable::ExpandAction(TableIndexT worow, bool deep)
{
	// Force hierarchy descovery
	CAddressBook* adbk = GetCellNode(worow, true);
	if (!adbk->HasExpanded())
	{
		adbk->GetProtocol()->LoadSubList(adbk, deep);
	}
}
