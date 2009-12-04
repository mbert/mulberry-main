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

#include "CCalendarStoreTable.h"

#include "CActionManager.h"
#include "CCalendarPropDialog.h"
#include "CCalendarStoreView.h"
#include "CCalendarView.h"
#include "CCalendarWindow.h"
#include "CCreateCalendarDialog.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CPreferences.h"
#if __dest_os != __win32_os
#include "CWaitCursor.h"
#endif
#include "CXStringResources.h"

#include "CCalendarStoreNode.h"
#include "CCalendarStoreManager.h"
#include "CCalendarStoreWebcal.h"

#include "CICalendar.h"
#include "CITIPProcessor.h"

#pragma mark -

void CCalendarStoreTable::SetManager(calstore::CCalendarStoreManager* manager)
{
	if (manager)
	{
		manager->Add_Listener(this);

		// Make sure its open so items will actually be displayed
		mTableView->SetOpen();

		// Calculate number of rows for first time
		ResetTable();
	}
}

void CCalendarStoreTable::ResetTable()
{
	// Start cursor for busy operation
	CWaitCursor wait;

	// Delete all existing rows
	Clear();
	mData.clear();

	// Only if manager exists
	if (calstore::CCalendarStoreManager::sCalendarStoreManager == NULL)
		return;

	// Get list from manager
	const calstore::CCalendarStoreNodeList& nodes = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNodes();

	// Add each node
	TableIndexT row = 0;
	TableIndexT exp_row = 1;
	for(calstore::CCalendarStoreNodeList::const_iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		AddNode(*iter, row, false);

		// Listen to each protocol
		calstore::CCalendarProtocol* proto = (*iter)->GetProtocol();
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

void CCalendarStoreTable::ClearTable()
{
	// Delete all existing rows
	Clear();
	mData.clear();

	// Refresh list
	FRAMEWORK_REFRESH_WINDOW(this)
	
	// Force toolbar update
	mTableView->RefreshToolbar();
}

void CCalendarStoreTable::ListenTo_Message(long msg, void* param)
{
	// Only do these if the parent view is open
	if (!mTableView->IsOpen())
		return;

	switch (msg)
	{
	case calstore::CCalendarStoreManager::eBroadcast_NewCalendarAccount:
		AddProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarStoreManager::eBroadcast_InsertCalendarAccount:
		InsertProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarStoreManager::eBroadcast_RemoveCalendarAccount:
		RemoveProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_ClearList:
		ClearProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_RefreshList:
		RefreshProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_Logon:
		SwitchProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_Logoff:
		SwitchProtocol(static_cast<calstore::CCalendarProtocol*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_ClearSubList:
		ClearSubList(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_RefreshSubList:
		RefreshSubList(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	case calstore::CCalendarStoreManager::eBroadcast_InsertNode:
		InsertNode(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	case calstore::CCalendarStoreManager::eBroadcast_RemoveNode:
	case calstore::CCalendarStoreManager::eBroadcast_DeleteNode:
		DeleteNode(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	case calstore::CCalendarProtocol::eBroadcast_RefreshNode:
	case calstore::CCalendarStoreManager::eBroadcast_SubscribeNode:
	case calstore::CCalendarStoreManager::eBroadcast_UnsubscribeNode:
		RefreshNode(static_cast<calstore::CCalendarStoreNode*>(param));
		break;
	}
}

void CCalendarStoreTable::DoSelectionChanged()
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
void CCalendarStoreTable::DoSingleClick(unsigned long row, const CKeyModifiers& mods)
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
void CCalendarStoreTable::DoDoubleClick(unsigned long row, const CKeyModifiers& mods)
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

void CCalendarStoreTable::DoPreview()
{
	PreviewCalendar();
}

void CCalendarStoreTable::DoPreview(calstore::CCalendarStoreNode* node)
{
	PreviewCalendar(node);
}

// Just open the item
void CCalendarStoreTable::DoFullView()
{
	OnOpenCalendar();
}

// Do preview of calendar
void CCalendarStoreTable::PreviewCalendar(bool clear)
{
	
}

// Do preview of calendar
void CCalendarStoreTable::PreviewCalendar(calstore::CCalendarStoreNode* node, bool clear)
{
	
}

void CCalendarStoreTable::OnImport()
{
	calstore::CCalendarStoreNode* node = NULL;
	bool merge = false;

	// If there is a single selection allow merge to that
	if (IsSingleSelection() && TestSelectionAnd((TestSelectionPP) &CCalendarStoreTable::TestSelectionRealCalendar))
	{
		TableIndexT row = GetFirstSelectedRow();
		calstore::CCalendarStoreNode* selected = GetCellNode(row);
		
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ImportCalendarMergeExisting",
																		"ErrorDialog::Btn::ImportCalendarReplaceExisting",
																		"ErrorDialog::Btn::ImportCalendarCreate",
																		"ErrorDialog::Btn::Cancel",
																		"ErrorDialog::Text::ImportCalendarSelection", 4,
																		selected->GetName());
		// Cancel
		if (result == CErrorDialog::eBtn4)
			return;
		
		// Create
		else if (result == CErrorDialog::eBtn3)
		{
			// Do create
			node = DoCreateCalendar();
			if (node == NULL)
				return;
		}
		
		// Replace
		else if (result == CErrorDialog::eBtn2)
		{
			node = selected;
			merge = false;
		}
		
		// Merge
		else if (result == CErrorDialog::eBtn1)
		{
			node = selected;
			merge = true;
		}
	}
	
	// Users chooses to create or merge with active item
	else
	{
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ImportCalendarChoose",
																		"ErrorDialog::Btn::ImportCalendarCreate",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::ImportCalendarNoSelection", 3);
		// Cancel
		if (result == CErrorDialog::eBtn3)
			return;
		
		// Create
		else if (result == CErrorDialog::eBtn2)
		{
			// Do create
			node = DoCreateCalendar();
			if (node == NULL)
				return;
		}
		
		// Choose
		else if (result == CErrorDialog::eBtn1)
		{
			iCal::CICalendar* cal = calstore::CCalendarStoreManager::sCalendarStoreManager->PickCalendar(NULL);
			if (cal == NULL)
				return;
			node = const_cast<calstore::CCalendarStoreNode*>(calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(cal));
			if (node == NULL)
				return;
			merge = true;
		}
	}
	
	// Do actual import
	DoImportCalendar(node, merge);
}

void CCalendarStoreTable::OnExport()
{
	// Do to each selected calendar
	DoToSelection((DoToSelectionPP) &CCalendarStoreTable::ExportCalendar);
}

// Get properties of server
void CCalendarStoreTable::OnServerProperties(void)
{
	// Get selected cell
	TableIndexT row = GetFirstSelectedRow();
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	// Create the dialog
	CCalendarPropDialog::PoseDialog(node->GetProtocol());
}

// Get properties of mailbox
void CCalendarStoreTable::OnCalendarProperties(void)
{
	try
	{
		// Add selection to list
		calstore::CCalendarStoreNodeList selected;
		selected.set_delete_data(false);
		if (DoToSelection1((DoToSelection1PP) &CCalendarStoreTable::AddSelectionToList, &selected))
		{
			// Check all calendars first
			for(calstore::CCalendarStoreNodeList::iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				long new_msgs = 0;

				// Get extension info if server is logged in
				calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(*iter);
				if (cal->GetProtocol()->IsLoggedOn())
				{
					// Must not allow one failure to stop others
					try
					{
						cal->CheckMyRights();
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);

					}

					try
					{
						cal->CheckACLs();
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);

					}
				}
			}

			// Create the dialog
			CCalendarPropDialog::PoseDialog(&selected);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Catch errors to prevent further processing
	}

	// Must refresh for change
	RefreshSelection();
}

void CCalendarStoreTable::OnLogin()
{
	// Must have single selected protocol
	if (!IsSingleSelection())
		return;
	
	TableIndexT row = GetFirstSelectedRow();
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	
	if (!node->IsProtocol())
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
	if (node->GetProtocol()->IsOffline() && !node->GetProtocol()->IsDisconnected())
	{
		// Ignore - should already be in logged in state
		return;
	}

	// 2. (as above)
	else if (!node->GetProtocol()->CanDisconnect())
	{
		// 2.1 (as above)
		if (CConnectionManager::sConnectionManager.IsConnected())
		{
			// Toggle login state
			if (node->GetProtocol()->IsLoggedOn())
			{
				calstore::CCalendarStoreManager::sCalendarStoreManager->StopProtocol(node->GetProtocol());
			}
			else
			{
				calstore::CCalendarStoreManager::sCalendarStoreManager->StartProtocol(node->GetProtocol());
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
			node->GetProtocol()->ForceDisconnect(!node->GetProtocol()->IsForceDisconnect());
		}
		// 3.2 (as above)
		else
		{
			// Ignore - should already be in logged in state
			return;
		}
	}

	// Reset all views
	CCalendarView::ResetAll();
}

void CCalendarStoreTable::OnNewCalendar()
{
	DoCreateCalendar();
}

// Create & open a new mailbox
calstore::CCalendarStoreNode* CCalendarStoreTable::DoCreateCalendar()
{
	CCreateCalendarDialog::SCreateCalendar	create;
	calstore::CCalendarProtocol* proto = NULL;
	calstore::CCalendarStoreNode* node = NULL;
	calstore::CCalendarStoreNode* result = NULL;
	bool insert_delim = false;

	// Determine initial creation type
	if (IsSelectionValid())
	{
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row);

		// First check logged in state of server - may come back as NULL if cabinet selected
		calstore::CCalendarProtocol* server = GetCellCalendarProtocol(row);
		node = GetCellNode(row);

		// If logged in then use current selection as reference point
		if (server && server->IsLoggedOn())
		{
			proto = server;
			create.account = proto->GetAccountName();
			create.account_type = proto->GetAccountType();

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

	// Get mailbox name and create & open the mailbox
	try
	{
		if (CCreateCalendarDialog::PoseDialog(create))
		{
			cdstring new_name;
			if (create.use_wd)
			{
				new_name = create.parent;
				if (node->GetProtocol()->GetDirDelim())
					new_name += node->GetProtocol()->GetDirDelim();
				new_name += create.new_name;
			}
			else
				new_name = create.new_name;

			// Check and get proto from dialog
			if (!proto)
				proto = calstore::CCalendarStoreManager::sCalendarStoreManager->GetProtocol(create.account);
			if (!proto)
			{
				CLOG_LOGTHROW(CGeneralException, -1);
				throw CGeneralException(-1);
			}

			// Check for duplicate
			const calstore::CCalendarStoreNode* cal = proto->GetNode(new_name);

			if (cal != NULL)
				// Error
				CErrorHandler::PutStopAlertRsrc("CCalendarStoreTable::DuplicateCreate");
			else
			{
				// Create and open it
				calstore::CCalendarStoreNode* new_node = calstore::CCalendarStoreManager::sCalendarStoreManager->NewCalendar(proto, new_name, create.directory);
				
				// Now do subscription
				if (create.subscribe && !create.directory && (new_node != NULL))
				{
					calstore::CCalendarStoreManager::sCalendarStoreManager->SubscribeNode(new_node, true);
				}
				
				result = new_node;
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do nothing if it fails
		// lower handlers display dialog
	}

	return result;
}

void CCalendarStoreTable::OnNewWebCalendar()
{
	// Must be logged in to webcal account
	if ((calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
		calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected())
		return;

	// Get URL from user
	cdstring url;
#if __dest_os == __mac_os_x
	if (CGetStringDialog::PoseDialog("CCalendarStoreTable::NewWebCalendar", url))
#else
	if (CGetStringDialog::PoseDialog("CCalendarStoreTable::NewWebCalendar", "CCalendarStoreTable::NewWebCalendar", url))
#endif
	{
		// Check for duplicate
		const calstore::CCalendarStoreNode* cal = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNodeByRemoteURL(url);

		if (cal != NULL)
			// Error
			CErrorHandler::PutStopAlertRsrcStr("CCalendarStoreTable::DuplicateWebCalendar", url);
		else
			calstore::CCalendarStoreManager::sCalendarStoreManager->NewWebCalendar(url);
	}
}

// Refresh selected items
void CCalendarStoreTable::OnRefreshWebCalendar()
{
	// Must be logged in to webcal account
	if ((calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
		calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected())
		return;

	DoToSelection((DoToSelectionPP) &CCalendarStoreTable::RefreshWebCalendar);
}

// Refresh specified item
bool CCalendarStoreTable::RefreshWebCalendar(TableIndexT row)
{
	// Must have web calendar
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	if (!node->GetProtocol()->IsWebCalendar())
		return false;
	
	calstore::CCalendarStoreManager::StNodeActivate _activate(node);
	if (node->GetCalendar() != NULL)
		node->GetProtocol()->SubscribeFullCalendar(*node, *node->GetCalendar());

	return true;
}

// Upload selected items
void CCalendarStoreTable::OnUploadWebCalendar()
{
	// Must be logged in to webcal account
	if ((calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol() != NULL) &&
		calstore::CCalendarStoreManager::sCalendarStoreManager->GetWebCalendarProtocol()->IsDisconnected())
		return;

	DoToSelection((DoToSelectionPP) &CCalendarStoreTable::UploadWebCalendar);
}

// Upload specified item
bool CCalendarStoreTable::UploadWebCalendar(TableIndexT row)
{
	// Must have web calendar
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	if (!node->GetProtocol()->IsWebCalendar())
		return false;
	
	calstore::CCalendarStoreManager::StNodeActivate _activate(node);
	if (node->GetCalendar() != NULL)
		node->GetProtocol()->PublishFullCalendar(*node, *node->GetCalendar());

	return true;
}

// Open selected items
void CCalendarStoreTable::OnOpenCalendar()
{
	DoToSelection((DoToSelectionPP) &CCalendarStoreTable::OpenCalendar);
}

// Open specified item
bool CCalendarStoreTable::OpenCalendar(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	
	// Must be a real calendar
	if (!node->IsViewableCalendar())
		return false;
	
	// See if it already exists
	CCalendarWindow* found = CCalendarWindow::FindWindow(node);
	if (found != NULL)
		FRAMEWORK_WINDOW_TO_TOP(found)
	else
		CCalendarWindow::MakeWindow(node);
	return true;
}

void CCalendarStoreTable::OnRenameCalendar()
{
	// Rename each selected item
	DoToSelection((DoToSelectionPP) &CCalendarStoreTable::RenameCalendar);
}

// Rename specified item
bool CCalendarStoreTable::RenameCalendar(TableIndexT row)
{
	// Get calendar for hit cell
	bool failagain = false;
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node == NULL)
		return false;

	do
	{
		failagain = false;

		// Get name from user
		cdstring name(node->GetName());
#if __dest_os == __mac_os_x
		if (CGetStringDialog::PoseDialog("CCalendarStoreTable::RenameCalendarName", name))
#else
		if (CGetStringDialog::PoseDialog("CCalendarStoreTable::RenameCalendarName", "CCalendarStoreTable::RenameCalendarName", name))
#endif
		{
			try
			{
				calstore::CCalendarStoreManager::sCalendarStoreManager->RenameCalendar(node, name);
				RefreshRow(row);
			}
			catch(...)
			{
				CLOG_LOGCATCH(...);

				failagain = true;
			}
		}
	}
	while (failagain);

	return true;
}

void CCalendarStoreTable::OnDeleteCalendar()
{
	// Check that this is what we want to do
	try
	{
		// Get all selected mailboxes
		calstore::CCalendarStoreNodeList selected;
		selected.set_delete_data(false);
		if (DoToSelection1((DoToSelection1PP) &CCalendarStoreTable::AddSelectionToList, &selected))
		{
			// Generate string for list of items being deleted
			cdstring deletes;
			for(calstore::CCalendarStoreNodeList::const_iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				if (!deletes.empty())
					deletes += ", ";
				deletes += (*iter)->GetAccountName(calstore::CCalendarStoreManager::sCalendarStoreManager->HasMultipleProtocols());
			}
			
			// Now ask user about delete, providing list of items being deleted
			if (CErrorHandler::PutCautionAlertRsrcStr(true, "CCalendarStoreTable::ReallyDeleteCalendar", deletes) == CErrorHandler::Ok)
			{
				// Delete all mboxes in reverse
				for(calstore::CCalendarStoreNodeList::reverse_iterator riter = selected.rbegin(); riter != selected.rend(); riter++)
					calstore::CCalendarStoreManager::sCalendarStoreManager->DeleteCalendar(*riter);
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

void CCalendarStoreTable::OnFreeBusyCalendar()
{
	// Send selected items
	calstore::CCalendarStoreFreeBusyList selected;
	if (DoToSelection1((DoToSelection1PP) &CCalendarStoreTable::FreeBusyCalendar, &selected))
	{
		//CCalendarWindow::CreateFreeBusyWindow(selected);
	}
}

// Send specified item
bool CCalendarStoreTable::FreeBusyCalendar(TableIndexT row, calstore::CCalendarStoreFreeBusyList* list)
{
	// Get calendar for hit cell
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node == NULL)
		return false;

	// May need to activate if not already
	calstore::CCalendarStoreManager::StNodeActivate _activate(node);
	
	// Write calendar to file
	iCal::CICalendar* cal = node->GetCalendar();
	if (cal != NULL)
	{
		iCal::CICalendarDateTime today = iCal::CICalendarDateTime::GetToday();
		today.SetDateOnly(false);
		iCal::CICalendarDateTime tomorrow = today;
		tomorrow.OffsetDay(1);
		
		iCal::CICalendarPeriod period(today, tomorrow);
		calstore::CCalendarStoreFreeBusy items(node->GetDisplayShortName());
		cal->GetFreeBusy(period, items.GetPeriods());
		list->push_back(items);
	}
	
		
	return true;
}

void CCalendarStoreTable::OnSendCalendar()
{
	// Send selected items
	DoToSelection((DoToSelectionPP) &CCalendarStoreTable::SendCalendar);
}

// Send specified item
bool CCalendarStoreTable::SendCalendar(TableIndexT row)
{
	// Get calendar for hit cell
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	if (node == NULL)
		return false;

	// May need to activate if not already
	calstore::CCalendarStoreManager::StNodeActivate _activate(node);
	
	// Write calendar to file
	iCal::CICalendar* cal = node->GetCalendar();
	if (cal != NULL)
	{
		// Get iTIP PUBLISH component for this event
		CDataAttachment* attach;
		iCal::CITIPProcessor::SendCalendar(*cal, attach);
		
		// Create a new draft with this component as a text/calendar part
		CActionManager::NewDraftiTIP(cdstring::null_str, cdstring::null_str, attach, NULL);
	}
		
	return true;
}

void CCalendarStoreTable::OnCheckCalendar()
{
	// If no selection, check all subscribed calendars
	if (!IsSelectionValid())
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
		for(iCal::CICalendarList::const_iterator iter = cals.begin(); iter != cals.end(); iter++)
		{
			calstore::CCalendarStoreNode* node = const_cast<calstore::CCalendarStoreNode*>(calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(*iter));
			node->GetProtocol()->CheckCalendar(*node, *node->GetCalendar());
		}
	}
	else
	{
		DoToSelection((DoToSelectionPP) &CCalendarStoreTable::CheckCalendar);
	}

	// Reset all views
	CCalendarView::ResetAll();
}

bool CCalendarStoreTable::CheckCalendar(TableIndexT row)
{
	// Get calendar for hit cell
	calstore::CCalendarStoreNode* node = GetCellNode(row);

	// Only do those that are active calendars
	if ((node == NULL) || !node->IsViewableCalendar() || !node->IsActive())
		return false;

	if (node->GetCalendar() != NULL)
		return node->GetProtocol()->CheckCalendar(*node, *node->GetCalendar());
		
	return false;
}

void CCalendarStoreTable::OnRefreshList()
{
	// Only if we have a single selection
	if (!IsSingleSelection())
		return;
	
	TableIndexT row = GetFirstSelectedRow();
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	
	// If the node is an ordinary calendar then refresh is done on the parent
	if (!node->IsProtocol() && !node->IsDirectory())
		node = node->GetParent();
	
	// Refrech based on type
	if (node->IsProtocol())
		node->GetProtocol()->RefreshList();
	else if (node->IsDirectory())
		node->GetProtocol()->RefreshSubList(node);
	
	// Reset all views
	CCalendarView::ResetAll();
}

// Add selected mboxes to list
bool CCalendarStoreTable::AddSelectionToList(TableIndexT row, calstore::CCalendarStoreNodeList* list)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	list->push_back(node);

	return true;
}

// Remove rows and adjust parts
#if __dest_os == __mac_os_x
void CCalendarStoreTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, Boolean inRefresh)
#else
void CCalendarStoreTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh)
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
calstore::CCalendarStoreNode* CCalendarStoreTable::GetCellNode(TableIndexT row, bool worow) const
{
	TableIndexT	woRow = (worow ? row : mCollapsableTree->GetWideOpenIndex(row));
	return mData[woRow - 1];
}

// Get the requested calendar protocol
calstore::CCalendarProtocol* CCalendarStoreTable::GetCellCalendarProtocol(TableIndexT row) const
{
	calstore::CCalendarStoreNode* node = GetCellNode(row);
	return node->GetProtocol();
}

// Add a node to the list
void CCalendarStoreTable::AddNode(calstore::CCalendarStoreNode* node, TableIndexT& row, bool child, bool refresh)
{
	TableIndexT parent_row;

	if (child)
	{
		parent_row = AddLastChildRow(row, &node, sizeof(calstore::CCalendarStoreNode*), node->IsDirectory(), refresh);
	}
	else
	{
		parent_row = row = InsertSiblingRows(1, row, &node, sizeof(calstore::CCalendarStoreNode*), node->IsDirectory(), refresh);
	}
	mData.insert(mData.begin() + (parent_row - 1), node);

	// Insert children
	AddChildren(node, parent_row, refresh);

	// Expand if previously expanded (do this AFTER adding all the children)
	ExpandRestore(parent_row);
}

// Add child nodes to the list
void CCalendarStoreTable::AddChildren(calstore::CCalendarStoreNode* node, TableIndexT& parent_row, bool refresh)
{
	// Insert children
	if (node->HasInferiors())
	{
		for(calstore::CCalendarStoreNodeList::const_iterator iter = node->GetChildren()->begin(); iter != node->GetChildren()->end(); iter++)
			AddNode(*iter, parent_row, true, refresh);
	}
}

// Remove child nodes from the list
void CCalendarStoreTable::RemoveChildren(TableIndexT& parent_row, bool refresh)
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
void CCalendarStoreTable::AddProtocol(calstore::CCalendarProtocol* proto)
{
	// Get list from manager
	const calstore::CCalendarStoreNodeList& nodes = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNodes();

	// Get last top-level sibling row
	TableIndexT row = 0;
	AddNode(nodes.back(), row, true, true);
	
	proto->Add_Listener(this);
}

// Insert protocol somewhere in the middle
void CCalendarStoreTable::InsertProtocol(calstore::CCalendarProtocol* proto)
{
	// For now do complete reset
	ResetTable();
}

// Remove protocol from list
void CCalendarStoreTable::RemoveProtocol(calstore::CCalendarProtocol* proto)
{
	unsigned long pos;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->FindProtocol(proto, pos))
	{
		// Remove this and all children
		TableIndexT temp = pos;
		RemoveRows(1, temp, true);
	}
}

// Clear protocol list
void CCalendarStoreTable::ClearProtocol(calstore::CCalendarProtocol* proto)
{
	unsigned long pos;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->FindProtocol(proto, pos))
	{
		// Remove all children
		TableIndexT temp = pos;
		RemoveChildren(temp, true);
	}
}

// Add protocol children from list
void CCalendarStoreTable::RefreshProtocol(calstore::CCalendarProtocol* proto)
{
	unsigned long pos;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->FindProtocol(proto, pos))
	{
		// Remove this and all children
		TableIndexT woRow = pos;
		if (!HasChildren(woRow))
		{
			TableIndexT row = GetExposedIndex(woRow);
			if (row)
			{
				calstore::CCalendarStoreNode* node = GetCellNode(row);
				AddChildren(node, woRow, true);
				RefreshRow(row);
			}
		}
	}
}

// Remove protocol children from list
void CCalendarStoreTable::SwitchProtocol(calstore::CCalendarProtocol* proto)
{
	// Calendar objects remain after logoff
	unsigned long pos;
	if (calstore::CCalendarStoreManager::sCalendarStoreManager->FindProtocol(proto, pos))
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
void CCalendarStoreTable::InsertNode(calstore::CCalendarStoreNode* node)
{
	// Find its wide-open row location
	
	// Iterate back over parents accumulating offsets into parents
	uint32_t parent_row = 0;
	uint32_t sibling_row = 0;
	node->GetInsertRows(parent_row, sibling_row);
	
	// Now insert using child/sibling switch
	if (sibling_row == 0)
	{
		TableIndexT worow = InsertChildRows(1, parent_row, &node, sizeof(calstore::CCalendarStoreNode*), node->IsDirectory(), true);
		mData.insert(mData.begin() + (worow - 1), node);
	}
	else
	{
		TableIndexT worow = InsertSiblingRows(1, sibling_row, &node, sizeof(calstore::CCalendarStoreNode*), node->IsDirectory(), true);
		mData.insert(mData.begin() + (worow - 1), node);
	}
}

// Delete a node from the list
void CCalendarStoreTable::DeleteNode(calstore::CCalendarStoreNode* node)
{
	// Find its wide-open row location
	uint32_t woRow = node->GetRow();
	RemoveRows(1, woRow, true);
}

// Refresh a node from the list
void CCalendarStoreTable::RefreshNode(calstore::CCalendarStoreNode* node)
{
	// Find its wide-open row location
	uint32_t woRow = node->GetRow();
	TableIndexT exp_row = GetExposedIndex(woRow);
	if (exp_row)
		RefreshRow(exp_row);
}

void CCalendarStoreTable::ClearSubList(calstore::CCalendarStoreNode* node)
{
	// Find its wide-open row location
	TableIndexT woRow = node->GetRow();
	RemoveChildren(woRow, true);
}

void CCalendarStoreTable::RefreshSubList(calstore::CCalendarStoreNode* node)
{
	// Find its wide-open row location
	TableIndexT woRow = node->GetRow();
	AddChildren(node, woRow, true);
}

#pragma mark ____________________________Hierarchy Related

// Create new hierarchy
void CCalendarStoreTable::OnNewHierarchy(void)
{
	// New hierarchy dialog
	cdstring new_name;
	
	if (CGetStringDialog::PoseDialog("Alerts::Server::NewHierarchy", new_name))
	{
		TableIndexT row = GetFirstSelectedRow();
		calstore::CCalendarStoreNode* selected = GetCellNode(row);
		selected->GetProtocol()->AddWD(new_name);
	}
}

// Rename hierarchy
void CCalendarStoreTable::OnRenameHierarchy(void)
{
	// Add selection to list
	calstore::CCalendarStoreNodeList selected;
	selected.set_delete_data(false);
	if (DoToSelection1((DoToSelection1PP) &CCalendarStoreTable::AddSelectionToList, &selected))
	{
		// Rename (do in reverse)
		for(calstore::CCalendarStoreNodeList::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
		{
			calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(*iter);

			// Rename it
			cdstring new_name = cal->GetName();
			
			if (CGetStringDialog::PoseDialog("Alerts::Server::NewHierarchy", new_name))
			{
				cal->GetProtocol()->RenameWD(*cal, new_name);
			}
			else
				// Break out of entire loop if user cancels
				break;
		}
	}
	
	RefreshSelection();
}

// Delete hierarchy
void CCalendarStoreTable::OnDeleteHierarchy(void)
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteHierarchy") == CErrorHandler::Ok)
	{
		// Prevent flashes during multiple selection changes
		StDeferSelectionChanged noSelChange(this);
		
		// Add selection to list
		calstore::CCalendarStoreNodeList selected;
		selected.set_delete_data(false);
		if (DoToSelection1((DoToSelection1PP) &CCalendarStoreTable::AddSelectionToList, &selected))
		{
			// Remove (do in reverse)
			for(calstore::CCalendarStoreNodeList::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
			{
				calstore::CCalendarStoreNode* cal = static_cast<calstore::CCalendarStoreNode*>(*iter);
				cal->GetProtocol()->RemoveWD(*cal);
			}
		}
	}
}

#pragma mark ____________________________Tests

// Test for selected servers only
bool CCalendarStoreTable::TestSelectionServer(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return node->IsProtocol();
}

// Test for selected calendars only
bool CCalendarStoreTable::TestSelectionCalendarStoreNode(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return !node->IsProtocol();
}

// Test for selected calendars only
bool CCalendarStoreTable::TestSelectionCanChangeCalendar(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return !node->IsProtocol() && !node->IsInbox() && !node->IsOutbox();
}

// Test for selected calendars only
bool CCalendarStoreTable::TestSelectionRealCalendar(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return node->IsViewableCalendar();
}

// Test for selected web calendars only
bool CCalendarStoreTable::TestSelectionWebCalendar(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return !node->IsProtocol() && !node->IsDirectory() && node->GetProtocol()->IsWebCalendar();
}

// Test for selected uploadable web calendars only
bool CCalendarStoreTable::TestSelectionUploadWebCalendar(TableIndexT row)
{
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return !node->IsProtocol() && !node->IsDirectory() && (node->GetWebcal() != NULL) && !node->GetWebcal()->GetReadOnly();
}

// Test for selected hierarchies only
bool CCalendarStoreTable::TestSelectionHierarchy(TableIndexT row)
{
	// This is deleted
	calstore::CCalendarStoreNode* node = GetCellNode(row, false);
	return node->IsDisplayHierarchy();
}

#pragma mark ____________________________Expand/collapse

void CCalendarStoreTable::CollapseRow(UInt32 inWideOpenRow)
{
	CHierarchyTableDrag::CollapseRow(inWideOpenRow);

	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, false);
}

void CCalendarStoreTable::DeepCollapseRow(UInt32 inWideOpenRow)
{
	CHierarchyTableDrag::DeepCollapseRow(inWideOpenRow);

	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, false);
}

void CCalendarStoreTable::ExpandRow(UInt32 inWideOpenRow)
{
	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, true);

	// Force hierarchy descovery
	ExpandAction(inWideOpenRow, false);

	// Do visual expand
	CHierarchyTableDrag::ExpandRow(inWideOpenRow);
}

void CCalendarStoreTable::DeepExpandRow(UInt32 inWideOpenRow)
{
	// Sync expansion state with prefs
	ProcessExpansion(inWideOpenRow, true);

	// Force hierarchy descovery
	ExpandAction(inWideOpenRow, true);

	// Do visual expand
	CHierarchyTableDrag::DeepExpandRow(inWideOpenRow);
}

void CCalendarStoreTable::ProcessExpansion(UInt32 inWideOpenRow, bool expand)
{
	calstore::CCalendarStoreNode* node = GetCellNode(inWideOpenRow, true);
	
	// Add or remove from expansion preference
	if (expand)
	{
		CPreferences::sPrefs->mExpandedCalendars.Value().insert(node->IsProtocol() ? node->GetName() : node->GetAccountName());
	}
	else
	{
		CPreferences::sPrefs->mExpandedCalendars.Value().erase(node->IsProtocol() ? node->GetName() : node->GetAccountName());
	}
	CPreferences::sPrefs->mExpandedCalendars.SetDirty();
}

void CCalendarStoreTable::ExpandRestore(TableIndexT worow)
{
	// Do expansion if expand state is in prefs - bypass the prefs change
	calstore::CCalendarStoreNode* node = GetCellNode(worow, true);
	bool do_expand = CPreferences::sPrefs->mExpandedCalendars.GetValue().count(node->IsProtocol() ? node->GetName() : node->GetAccountName()) != 0;
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

void CCalendarStoreTable::ExpandAction(TableIndexT worow, bool deep)
{
	// Force hierarchy descovery
	calstore::CCalendarStoreNode* node = GetCellNode(worow, true);
	if (!node->HasExpanded())
	{
		node->GetProtocol()->LoadSubList(node, deep);
	}
}
