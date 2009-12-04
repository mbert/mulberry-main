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

#include "CActionManager.h"
#include "CAdminLock.h"
#include "CErrorDialog.h"
#include "CErrorHandler.h"
#include "CGetStringDialog.h"
#include "CIMAPCommon.h"
#include "CLog.h"
#include "CMailAccountManager.h"
#include "CMailboxPropDialog.h"
#include "CMailboxInfoView.h"
#include "CMailboxInfoWindow.h"
#include "CMailboxView.h"
#include "CMailControl.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMboxRef.h"
#include "CMessageWindow.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CNewSearchDialog.h"
#include "CNewWildcardDialog.h"
#include "COpenMailboxDialog.h"
#include "CPreferences.h"
#include "CSearchWindow.h"
#include "CServerToolbar.h"
#include "CServerWindow.h"
#include "CStringUtils.h"
#include "CSynchroniseDialog.h"
#include "CTaskClasses.h"
#include "CTCPException.h"
#include "CUserAction.h"
#if __dest_os == __mac_os || __dest_os == __linux_os
#include "CWaitCursor.h"
#include "CWindowsMenu.h"
#endif
#include "C3PaneWindow.h"

#if __dest_os == __mac_os || __dest_os == __mac_os_x
#include "CStringResources.h"

#elif __dest_os == __win32_os
#include "CUnicodeUtils.h"

#include "StValueChanger.h"

#elif __dest_os == __linux_os
#include "StValueChanger.h"
#include <JTableSelection.h>

#endif

#include <stdio.h>
#include <string.h>

#if __dest_os == __win32_os
// BUG in Metrowerks Compiler - need this specialization
inline void allocator<CServerBrowse::SServerBrowseData>::deallocate(CServerBrowse::SServerBrowseData* p, size_t)
{
	delete p;
}
#endif

#pragma mark ____________________________Management

// Set the mboxList
void CServerTable::SetServer(CMboxProtocol* server)
{
	// Do inherited
	CServerBrowse::SetServer(server);

	SetTitle();

	// Hide cabinet items not used in single server window
	Broadcast_Message(CServerToolbar::eBroadcast_HideCabinetItems);

	// Set logon state (after updating table)
	if (mServer && mServer->IsLoggedOn())
		SetLogon(true);
	
	// Now force toolbar update
	UpdateState();
}

// Set the mboxList
void CServerTable::SetManager(void)
{
	// Do inherited
	CServerBrowse::SetManager();

	SetTitle();

	// Manager can drag servers and mbox refs
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	AddDragFlavor(cDragServerType);
	AddDragFlavor(cDragMboxRefType);
#elif __dest_os == __win32_os || __dest_os == __linux_os
	AddDragFlavor(CMulberryApp::sFlavorServerList);
	AddDragFlavor(CMulberryApp::sFlavorMboxRefList);
#else
#error __dest_os
#endif

	// Manager can drop mbox refs
#if __dest_os == __mac_os || __dest_os == __mac_os_x
	AddDropFlavor(cDragMboxRefType);
#elif __dest_os == __win32_os || __dest_os == __linux_os
	AddDropFlavor(CMulberryApp::sFlavorMboxRefList);
#else
#error __dest_os
#endif
	
	// Now force toolbar update
	UpdateState();
}

// Set the title of the window
void CServerTable::SetTitle()
{
	cdstring name;

	// 3-pane uses app title
	if (mTableView->Is3Pane())
		//name.FromResource(IDR_MAINFRAME);
		name = "Mulberry";
	else
	{
		// Single server window has name of its server
		if (mSingle)
		{
			if (GetSingleServer())
				name = GetSingleServer()->GetAccountName();
		}

		// Server manager window has generic name if more than one
		else
			name.FromResource("UI::Server::Title");
	}

	// Add global connected/disconnected state
	cdstring state;
	if (CConnectionManager::sConnectionManager.IsConnected())
		state.FromResource("UI::Server::TitleConnected");
	else
		state.FromResource("UI::Server::TitleDisconnected");
	name += state;
	
	if (mTableView->Is3Pane())
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		MyCFString temp(name, kCFStringEncodingUTF8);
		C3PaneWindow::s3PaneWindow->SetCFDescriptor(temp);
#elif __dest_os == __win32_os
		CUnicodeUtils::SetWindowTextUTF8(AfxGetMainWnd(), name);
#elif __dest_os == __linux_os
		C3PaneWindow::s3PaneWindow->GetWindow()->SetTitle(name);
#endif
	}
	else
	{
#if __dest_os == __mac_os || __dest_os == __mac_os_x
		MyCFString temp(name, kCFStringEncodingUTF8);
		mTableView->GetServerWindow()->SetCFDescriptor(temp);
#elif __dest_os == __win32_os
		mTableView->GetServerWindow()->GetDocument()->SetTitle(name.win_str());
#elif __dest_os == __linux_os
		mTableView->GetServerWindow()->SetTitle(name);
#endif
	}

#if __dest_os == __mac_os || __dest_os == __linux_os
		CWindowsMenu::RenamedWindow();
#endif
}

#pragma mark ____________________________3-pane

// Handle single click
void CServerTable::DoSingleClick(unsigned long row, const CKeyModifiers& mods)
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
void CServerTable::DoDoubleClick(unsigned long row, const CKeyModifiers& mods)
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

void CServerTable::DoPreview(bool clear)
{
	// Check whether 3-pane is available - if not ignore this
	if (!mTableView->Is3Pane() || !mTableView->GetPreview())
		return;
	
	// Use NULL if preview to be cleared
	CMbox* mbox = NULL;
	if (!clear)
	{
		// Add mailbox selection to list
		CMboxList selected;
		DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected);

		// Only do one if preview does not do substitute
		if (mTableView->GetPreview()->GetUseSubstitute() || (selected.size() == 1))
		{
			// Preview each one (only dynamic if there is a single item)
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
				DoPreview(static_cast<CMbox*>(*iter), (selected.size() == 1));
		}
		else
			// Don't change preview if more than one selected
			return;
	}
	else
		// Clear out preview
		DoPreview((CMbox*) NULL);
}

void CServerTable::DoPreview(CMbox* mbox, bool use_dynamic, bool is_search)
{
	// Check whether 3-pane is available - if not ignore this
	if (!mTableView->Is3Pane() || !mTableView->GetPreview())
		return;

	if (mbox)
	{
		// Check whether different - exit if not
		if (IsPreviewed(mbox))
			return;

		// Check that mailbox to be opened is on a logged in server
		bool need_login = !mbox->GetProtocol()->IsLoggedOn();

		// If not already logged on must save/restore selection based on mbox name
		if (need_login)
		{
			// Do logon
			CMboxProtocol* proto = mbox->GetProtocol();
			if (!proto->IsLoggedOn())
			{
				// WARNING mbox list may be corrupt if logon fails
				LogonAction(proto);
				if (!proto->IsLoggedOn())
					return;
			}
		}
	}

	// If null preview immediately other wise do as task
	if (!mbox)
		mTableView->GetPreview()->ViewMbox(mbox);
	else
	{
		// Give it to preview
		CMailboxPreviewTask* task = new CMailboxPreviewTask(mTableView->GetPreview(), mbox, use_dynamic, is_search);
		task->Go();
	}
}

// Is this mailbox being previewed now
bool CServerTable::IsPreviewed(const CMbox* mbox) const
{
	// Check whether preview is available
	if (!mTableView->GetPreview())
		return false;

	// Check whether different
	CMbox* old_mbox = mTableView->GetPreview()->GetMbox();
	cdstring old_url = old_mbox ? old_mbox->GetURL(true) : cdstring::null_str;
	cdstring new_url = mbox && !mbox->IsDirectory() ? mbox->GetURL(true) : cdstring::null_str;
	return new_url == old_url;
}

void CServerTable::DoFullView()
{
	// Check for server
	if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer))
		DoOpenServers();

	// Check for user cabinets
	else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemove))
		DoRenameFavourite();

	// Check for wildcard items
	else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteWildcardItems))
		DoRenameWildcardItem();

	// Check for search hierarchy items
	else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionHierarchy))
		//DoRenameHierarchy();
		;	// Do nothing - force use of menu commands

	else
		DoOpenMailbox();
}

#pragma mark ____________________________general

// Create a new draft
void CServerTable::DoNewLetter(bool option_key)
{
	// Determine context if required
	bool is_context = CPreferences::sPrefs->mContextTied.GetValue() ^ option_key && IsSelectionValid();
	
	if (is_context && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer) && IsSingleSelection())
	{
		// Get selected cell
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

		// New draft with identity tied to server
		CActionManager::NewDraft(GetCellServer(woRow));
	}
	else if (is_context && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAll))
	{
		// Add selection to list
		CMboxList selected;
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddDSelectionToList, &selected))
		{
			// New draft with identity tied to first selected mailbox
			CActionManager::NewDraft(static_cast<CMbox*>(selected.front()));
		}
	}
	else
		// New draft with default identity
		CActionManager::NewDraft();
}

#pragma mark ____________________________Server Related

// Get properties of server
void CServerTable::DoServerProperties(void)
{
	// Get selected cell
	TableIndexT row = GetFirstSelectedRow();
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	// Get appropriate server
	CMboxProtocol* server = nil;
	if (!mSingle)
		server = GetCellServer(woRow);
	else
		server = GetSingleServer();

	// Create the dialog
	CMailboxPropDialog::PoseDialog(server);

} // CServerTable::DoServerProperties

// Open selected server windows
void CServerTable::DoOpenServers(void)
{
	DoToSelection((DoToSelectionPP) &CServerTable::OpenTheServer);
}

// Open specific server window
bool CServerTable::OpenTheServer(TableIndexT row)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	if (GetCellDataType(woRow) == eServerBrowseServer)
	{
		CMailAccountManager::sMailAccountManager->OpenProtocol(GetCellServer(woRow));
		return true;
	}
	else
		return false;
}

// Logon to selected server
void CServerTable::DoLogonServers(void)
{
	// Store list of selected nodes as selection may change
	CServerNodeArray selected;

	// Add selection to list
	if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
	{
		// Logon to all protocols
		for(CServerNodeArray::iterator iter = selected.begin(); iter != selected.end(); iter++)
		{
			if ((*iter).mType == eServerBrowseServer)
				LogonAction((CMboxProtocol*) (*iter).mData);
		}
	}
}

// Logoff to selected server
void CServerTable::DoLogoffServers(void)
{
	// Store list of selected nodes as selection may change
	CServerNodeArray selected;

	// Add selection to list
	if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
	{
		// Logoff to all protocols
		for(CServerNodeArray::iterator iter = selected.begin(); iter != selected.end(); iter++)
		{
			if ((*iter).mType == eServerBrowseServer)
				LogoffAction((CMboxProtocol*) (*iter).mData);
		}
	}
}

// Set logon button
void CServerTable::SetLogon(bool pushed)
{
	// Force update of toolbar
	UpdateState();
}

// Get state of logon/off
void CServerTable::GetLogonState(bool& enabled, bool& pushed)
{
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

			enabled = true;
			pushed = proto && proto->IsLoggedOn();
		}
		else
		{
			enabled = false;
			pushed = false;
		}
	}
	else
	{
		enabled = true;
		pushed = GetSingleServer() && GetSingleServer()->IsLoggedOn();
	}
}

// Logon to the server
bool CServerTable::LogonServer(void)
{
	SetLogon(true);
	return (mSingle ? GetSingleServer()->IsLoggedOn() : false);
}

// Logon to the server
void CServerTable::LogonAction(CMboxProtocol* proto)
{
	// Only do if not already logged on
	if (proto->IsLoggedOn())
		return;

	try
	{
		{
			//StValueChanger<bool> change(mListChanging, true);

			CMailAccountManager::sMailAccountManager->StartProtocol(proto);
			CMailAccountManager::sMailAccountManager->StartedProtocol(proto);

			// Set logon button state
			if (!mManager)
				SetLogon(proto->IsLoggedOn());
		}

		// Reset message/draft window Copy To state
		CMessageWindow::ServerState(true);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Reset buttons
		if (!mManager)
			SetLogon(false);
	}

	// Do button update
	UpdateState();

} // CServerTable::LogonAction

// Logoff from the server
void CServerTable::LogoffAction(CMboxProtocol* proto)
{
	bool ok_logout = true;

	// Only do if not already logged off
	if (!proto->IsLoggedOn())
		return;

	try
	{
		// Delete all mailbox windows - in reverse
		{
			cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
			for(CMailboxView::CMailboxViewList::reverse_iterator riter = CMailboxView::sMailboxViews->rbegin();
					ok_logout && (riter != CMailboxView::sMailboxViews->rend()); riter++)
			{
				CMailboxInfoView* view = dynamic_cast<CMailboxInfoView*>(*riter);
				if (view && view->ContainsProtocol(proto))
				{
					if (view->TestClose(proto))
					{
						// Close the view (will close window at idle time)
						view->DoClose(proto);
					}
					else
						ok_logout = false;
				}
			}
		}

		// Logoff if not cancelled
		if (ok_logout)
			proto->Logoff();

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	// Always force to logged off state
	}

	if (ok_logout)
	{
		// Push logon button
		if (!mManager)
			SetLogon(!ok_logout);

		// Reset message window Copy To state
		CMessageWindow::ServerState(false);
	}

	// Do button update
	UpdateState();
	
	// Selection needs refresh
	RefreshSelection();

	// Now see if user might have intended a quit
	if (CAdminLock::sAdminLock.mPromptLogoutQuit && !CMailAccountManager::sMailAccountManager->HasOpenProtocols())
	{
		// Ask user for quit
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::ServerCloseQuit",
																		"ErrorDialog::Btn::ServerCloseLogout",
																		NULL,
																		NULL,
																		"ErrorDialog::Text::ServerLogoutQuit");
		if (result == CErrorDialog::eBtn1)
#if __dest_os == __mac_os || __dest_os == __mac_os_x
			CMulberryApp::sApp->DoQuit();
#elif __dest_os == __win32_os
			CMulberryApp::sApp->GetMainWnd()->PostMessage(WM_CLOSE);
#elif __dest_os == __linux_os
			CMulberryApp::sApp->Quit();
#else
#error __dest_os
#endif
	}

} // CServerTable::LogoffAction

#pragma mark ____________________________Favourite Related

// Create new hierarchy
void CServerTable::DoNewFavourite(void)
{
	cdstring new_name;

	if (CGetStringDialog::PoseDialog("Alerts::Server::NewFavourite", new_name))
		CMailAccountManager::sMailAccountManager->AddFavourite(new_name);

#if __dest_os == __win32_os
	// Set focus back to table after button push
	SetFocus();
#endif
}

// Rename favourite
void CServerTable::DoRenameFavourite(void)
{
	CServerNodeArray selected;

	// Remove selection from list
	if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
	{
		for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
		{
			CMboxRefList* ref_list = (CMboxRefList*) (*iter).mData;

			// Rename it
			cdstring new_name = ref_list->GetName();

			if (CGetStringDialog::PoseDialog("Alerts::Server::NewFavourite", new_name))
				CMailAccountManager::sMailAccountManager->RenameFavourite(ref_list, new_name);
			else
				// Break out of entire loop if user cancels
				break;
		}
	}

	// Refresh to update names
	RefreshSelection();
}

// Delete favourite
void CServerTable::DoRemoveFavourite(void)
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteFavourite") == CErrorHandler::Ok)
	{
		CServerNodeArray selected;

		// Remove selection from list
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
		{
			for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
			{
				CMboxRefList* ref_list = (CMboxRefList*) (*iter).mData;

				// Remove it
				CMailAccountManager::sMailAccountManager->RemoveFavourite(ref_list);
			}
		}
	}
}

// Delete favourite
void CServerTable::DoRemoveFavouriteItems(void)
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteFavouriteItems") == CErrorHandler::Ok)
	{
		CServerNodeArray selected;

		// Add selection to list
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
		{
			for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
			{
				CMboxRefList* ref_list = (CMboxRefList*) (*iter).mData;
				CMboxRef* mboxref = (CMboxRef*) ref_list->at((*iter).GetIndex());

				// Remove it
				CMailAccountManager::sMailAccountManager->RemoveFavouriteItem(ref_list, mboxref);
			}
		}
	}
}

// Create new hierarchy
void CServerTable::DoNewWildcardItem(void)
{
	// Do new hierarchy dialog
	cdstring new_name;
	if (CNewWildcardDialog::PoseDialog(new_name))
	{
		DoToSelection1((DoToSelection1PP) &CServerTable::NewWildcardItem, &new_name);
	}
}

// Open specific server window
bool CServerTable::NewWildcardItem(TableIndexT row, cdstring* name)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	if (GetCellDataType(woRow) == eServerBrowseMboxRefList)
	{
		CMboxRefList* ref_list = GetCellMboxRefList(woRow);
		char dir_delim = '\r';

		// Get account name (if any)
		const char* p = ::strchr(name->c_str(), cMailAccountSeparator);
		if (p)
		{
			cdstring acct(*name, 0, p - name->c_str());
			CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocol(acct);
			if (proto)
				dir_delim = proto->GetMailAccount()->GetDirDelim();
		}
		else
		{
			CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocolList().front();
			if (proto)
				dir_delim = proto->GetMailAccount()->GetDirDelim();
		}

		CMboxRef* new_ref = new CWildcardMboxRef(name->c_str(), dir_delim);

		// Add to list
		if (CMailAccountManager::sMailAccountManager->AddFavouriteItem(ref_list, new_ref))
		{
			// Adds a copy so must delete
			delete new_ref;
			return true;
		}

		delete new_ref;
	}

	return false;
}

// Rename wildcard item
void CServerTable::DoRenameWildcardItem(void)
{
	DoToSelection((DoToSelectionPP) &CServerTable::RenameWildcardItem);

	RefreshSelection();
}

// Open specific server window
bool CServerTable::RenameWildcardItem(TableIndexT row)
{
	TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
	CMboxRefList* ref_list = GetCellMboxRefList(woRow);
	CMboxRef* mbox_ref = GetCellMboxRef(woRow);

	if (mbox_ref != NULL)
	{
		// Rename it
		cdstring new_name = mbox_ref->GetName();

		if (CGetStringDialog::PoseDialog("Alerts::Server::RenameFavouriteItem", new_name))
		{
			CMailAccountManager::sMailAccountManager->RenameFavouriteItem(ref_list, mbox_ref, new_name);
			return true;
		}
	}

	return false;
}

#pragma mark ____________________________Hierarchy Related

// Create new hierarchy
void CServerTable::DoNewHierarchy(void)
{
	// Do new hierarchy dialog
	// Get index of first selected cell
	long sel_index = 0;
	if (mSingle)
	{
		sel_index = CMailAccountManager::sMailAccountManager->GetProtocolIndex(GetSingleServer());
	}
	else if (IsSelectionValid())
	{
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
		sel_index = CMailAccountManager::sMailAccountManager->GetProtocolIndex(GetCellServer(woRow));
	}

	// Create the dialog
	cdstring wd;
	unsigned long index = sel_index + TABLE_START_INDEX;
	if (CNewSearchDialog::PoseDialog(mSingle, index, wd))
	{
		if (!mSingle)
		{
			CMboxProtocol* proto = CMailAccountManager::sMailAccountManager->GetProtocolList().at(index - 1);
			// If manager use returned server
			// NB dir delim always set in dialog
			proto->AddWD(CDisplayItem(wd), 0);
		}
		else
			// Add this to end of account
			// NB dir delim always set in dialog
			GetSingleServer()->AddWD(CDisplayItem(wd), 0);
	}
}

// Rename hierarchy
void CServerTable::DoRenameHierarchy(void)
{
	CServerNodeArray selected;

	// Add selection to list
	if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
	{
		// Remove wds from account (do in reverse)
		for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
		{
			CMboxList* list = (CMboxList*) (*iter).mData;
			long wd_index = list->GetHierarchyIndex();
			if (wd_index >= 1)
			{
				// Rename it
				cdstring new_name = list->GetName();

				if (CGetStringDialog::PoseDialog("Alerts::Server::NewHierarchy", new_name))
				{
					StValueChanger<bool> change(mListChanging, true);
					list->GetProtocol()->RenameWD(wd_index, new_name);
				}
				else
					// Break out of entire loop if user cancels
					break;
			}
		}
	}

	RefreshSelection();
}

// Delete hierarchy
void CServerTable::DoDeleteHierarchy(void)
{
	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::DeleteHierarchy") == CErrorHandler::Ok)
	{
		// Prevent flashes during multiple selection changes
		StDeferSelectionChanged noSelChange(this);

		CServerNodeArray selected;

		// Add selection to list
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
		{
			// Remove wds from account (do in reverse)
			for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
			{
				CMboxList* list = (CMboxList*) (*iter).mData;
				long wd_index = list->GetHierarchyIndex();
				if (wd_index >= 1)
					list->GetProtocol()->RemoveWD(wd_index);
			}
		}
	}
}

// Reset hierarchy
void CServerTable::DoResetHierarchy(void)
{
	if (IsSelectionValid())
	{
		// Prevent flashes during multiple selection changes
		StDeferSelectionChanged noSelChange(this);

		CServerNodeArray selected;

		// Add selection to list
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
		{
			// Reset hierarchy for each node (avoid doing more than once per server)
			CMboxProtocol* proto = NULL;
			for(CServerNodeArray::iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				CMboxProtocol* server = NULL;
				switch((*iter).GetType())
				{
				case eServerBrowseServer:
				case eServerBrowseINBOX:
				{
					server = reinterpret_cast<CMboxProtocol*>((*iter).mData);
					break;
				}
				case eServerBrowseWD:
				case eServerBrowseSubs:
				case eServerBrowseMbox:
				{
					CMboxList* list = reinterpret_cast<CMboxList*>((*iter).mData);
					server = list->GetProtocol();
					break;
				}
				default:;
				}

				if (server != proto)
				{
					proto = server;
					proto->InitNamespace();
				}
			}
		}
	}
	else
		GetSingleServer()->InitNamespace();
}

// Change to flat view of hierarchy
void CServerTable::SetFlatHierarchy(bool hier)
{
	// Can only be one selected
	if (IsSingleSelection())
	{
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

		switch(GetCellDataType(woRow))
		{
		case eServerBrowseWD:
		case eServerBrowseSubs:
			// Force it to different state
			CMailAccountManager::sMailAccountManager->SetHierarchic(GetCellMboxList(woRow), hier);
			break;
		case eServerBrowseMboxRefList:
			// Force it to different state
			CMailAccountManager::sMailAccountManager->SetFlag(GetCellMboxRefList(woRow), CTreeNodeList::eHierarchic, hier);
			break;
		default:;
		}
	}
	else if (mSingle && GetSingleServer()->FlatWD())
		CMailAccountManager::sMailAccountManager->SetHierarchic(GetSingleServer()->GetHierarchies().at(1), hier);
}

#pragma mark ____________________________Mailbox Related

// Get properties of mailbox
void CServerTable::DoMailboxProperties(void)
{
	CMboxList selected;

	try
	{
		// Add selection to list
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddDSelectionToList, &selected))
		{
			// Check all mboxes first
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				long new_msgs = 0;

				// Get extension info if server is logged in
				CMbox* mbox = static_cast<CMbox*>(*iter);
				if (mbox->GetProtocol()->IsLoggedOn())
				{
					// Must not allow one failure to stop others
					try
					{
						// Do fast check to avoid POP3 check
						if (!mbox->IsDirectory())
							CheckMailbox(mbox, &new_msgs, true);
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);

					}

					try
					{
						if (!mbox->IsDirectory())
							mbox->CheckMyRights();
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);

					}

					try
					{
						if (!mbox->IsDirectory())
							mbox->CheckACLs();
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);

					}

					try
					{
						if (!mbox->IsDirectory())
							mbox->CheckQuotas();
					}
					catch (...)
					{
						CLOG_LOGCATCH(...);

					}
				}
			}

			// Create the dialog
			CMailboxPropDialog::PoseDialog(&selected);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Catch errors to prevent further processing
	}

	// Must refresh for change
	RefreshSelection();

} // CServerTable::DoMailboxProperties

// Select next mailbox with recent messages
void CServerTable::SelectNextRecent(bool reverse)
{
	// Get last selected row
	TableIndexT row = GetLastSelectedRow();

	// Clear selection first
	UnselectAllCells();

	TableIndexT woRow = 0;
	bool found_recent = false;

	if (reverse)
	{
		if (row == 0)
			woRow = mData.size();
		else
			woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST) - 1;
	}
	else
	{
		if (row < TABLE_START_INDEX)
			woRow = 1;
		else
			woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST) + 1;
	}

	// Scan over all data in table to find a mailbox that has recent messages
	for(; reverse ? (woRow > 0) : (woRow <= mData.size()); reverse ? woRow-- : woRow++)
	{
		if ((GetCellDataType(woRow) == eServerBrowseMbox) && (GetCellMbox(woRow) != NULL) && GetCellMbox(woRow)->AnyNew())
		{
			found_recent = true;
			break;
		}
		else if ((GetCellDataType(woRow) == eServerBrowseMboxRef) &&
			  	(CMailAccountManager::sMailAccountManager->GetFavouriteType(GetCellMboxRefList(woRow)) == CMailAccountManager::eFavouriteNew))
		{
			CMbox* mbox = GetCellMboxRef(woRow)->ResolveMbox(true);
			if (mbox && mbox->AnyNew())
			{
				found_recent = true;
				break;
			}
		}
	}

	if (found_recent)
	{
		RevealRow(woRow);
		TableIndexT exposed = GetExposedIndex(woRow) - TABLE_ROW_ADJUST;
		ScrollToRow(exposed, true, true, eScroll_Center);
	}
	else
	{
		ScrollToRow(1, false, true, eScroll_Top);
	}
}

// Check mail on selected mailboxes
void CServerTable::DoCheckMail(void)
{
	long num_new = 0;
	CMboxList selected;

	try
	{
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
		{
			// Check all mboxes
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				long new_msgs = 0;
				if (CheckMailbox(static_cast<CMbox*>(*iter), &new_msgs))
				{
					// Do silent alert (forces update of windows)
					CMailControl::NewMailUpdate((CMbox*) *iter, new_msgs, CPreferences::sPrefs->mMailNotification.GetValue().front());
					num_new += new_msgs;
				}
			}

			if (num_new)
				// Do check alert with no updates
				CMailControl::NewMailAlert(CPreferences::sPrefs->mMailNotification.GetValue().front());
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DoCheckMail

// Check mail on the specified mailbox
bool CServerTable::CheckMailbox(CMbox* mbox, long* num_new, bool fast)
{
	// Only if server is logged in
	if (!mbox->GetProtocol()->IsLoggedOn())
		return false;

	// Do check
	long check = mbox->Check(fast);

	*num_new += check;
	return (check > 0);

} // CServerTable::CheckMailbox

// Pick mailbox name
bool CServerTable::PickMailboxName(CMbox*& mbox)
{
	// Create the dialog
	bool oked = COpenMailboxDialog::PoseDialog(mbox);

	return oked;
} // CServerTable::PickMailboxName

// Choose & open a new mailbox
void CServerTable::DoChooseMailbox(void)
{
	CMbox* mbox = nil;

	// Get mailbox name and open the mailbox
	try {

		bool oked = PickMailboxName(mbox);
		if (oked && mbox)
		{
			// Try to logon
			if (!mManager)
			{
				if (!LogonServer())
					// If failed throw so that mailbox is cleaned up
				{
					CLOG_LOGTHROW(CGeneralException, -1);
					throw CGeneralException(-1);
				}
			}
			else
			{
				// Do logon
				if (!mbox->GetProtocol()->IsLoggedOn())
				{
					// This call will not throw if it fails (why?) so must check again for clean up
					LogonAction(mbox->GetProtocol());
					if (!mbox->GetProtocol()->IsLoggedOn())
					{
						CLOG_LOGTHROW(CGeneralException, -1);
						throw CGeneralException(-1);
					}
				}
			}

			// Open it
			OpenTheMailbox(mbox, false);
		}
		else if (oked)
			CErrorHandler::PutStopAlertRsrc("Alerts::Server::MissingOpenMailbox");
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Remove any singleton mbox from list if it fails
		if (mbox)
			mbox->CloseSilent(true);
	}

} // CServerTable::ChooseMailbox

// Process mailbox after creation
void CServerTable::PostCreateAction(CMbox* mbox)
{
	// Select it
	unsigned long mbox_index = (mManager ?
									CMailAccountManager::sMailAccountManager->FetchIndexOf(mbox, true) :
									GetSingleServer()->FetchIndexOf(mbox, true));
	if (!mbox_index)
		return;

	if (mShowFavourites)
		mbox_index += CMailAccountManager::sMailAccountManager->FetchServerOffset();

	// Must reveal
	RevealRow(mbox_index);
	TableIndexT exposed = GetExposedIndex(mbox_index);

	if (exposed)
		ScrollToRow(exposed - TABLE_ROW_ADJUST, true, true, eScroll_Center);
}

// Open the selected mailboxes
void CServerTable::DoOpenMailbox(void)
{
	// Though this is disabled if not logged on, a double-click will post this message
	// and this could occur if not logged on

	bool need_login = false;

	CMboxList mboxes;

	// Add named selection to list
	DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &mboxes);

	// Check that each mailbox to be opened is on a logged in server
	if (!mSingle)
	{
		// Test each for logon
		for(CMboxList::const_iterator iter = mboxes.begin(); !need_login && (iter != mboxes.end()); iter++)
			need_login = !static_cast<CMbox*>(*iter)->GetProtocol()->IsLoggedOn();
	}
	else
		need_login = !GetSingleServer()->IsLoggedOn();

	// If not already logged on must save/restore selection based on mbox name
	if (need_login)
	{
		cdstrvect selected;

		// Add named selection to list
		DoToSelection1((DoToSelection1PP) &CServerTable::AddNamedSelectionToList, &selected);

		// Try to logon
		if (!mManager)
		{
			if (!LogonServer())
				return;
		}
		else
		{
			// Do each logon
			for(CMboxList::const_iterator iter = mboxes.begin(); iter != mboxes.end(); iter++)
			{
				CMboxProtocol* proto = static_cast<CMbox*>(*iter)->GetProtocol();
				if (!proto->IsLoggedOn())
				{
					// WARNING mbox list may be corrupt if logon fails
					LogonAction(proto);
					if (!proto->IsLoggedOn())
						return;
				}
			}
		}

		// Recover as much of selection as possible
		try
		{
			// Add selection to list
			if (SelectFromList(&selected, &mboxes))
			{
				bool opened = false;

				// Open all selected mboxes - do not subscribe
				for(CMboxList::iterator iter = mboxes.begin(); iter != mboxes.end(); iter++)
					opened = OpenTheMailbox(static_cast<CMbox*>(*iter), false) || opened;

			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
	}
	else
	{
		try
		{
			bool opened = false;

			// Open all selected mboxes - do not subscribe
			for(CMboxList::iterator iter = mboxes.begin(); iter != mboxes.end(); iter++)
				opened = OpenTheMailbox((CMbox*) *iter, false) || opened;
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Catch errors to prevent further processing
			RefreshSelection();
		}
	}
} // CServerTable::DoOpenMailbox

// Open mailbox
// NB Must ensure that the server for the mailbox is logged on before calling this
bool CServerTable::OpenTheMailbox(CMbox* mbox, bool subscribe)
{
	CMailboxWindow* new_window = NULL;

	try
	{
		// Does window already exist?
		bool already_open = (CMailboxView::FindView(mbox) != NULL);

		// Open the new window
		new_window = CMailboxInfoWindow::OpenWindow(mbox);

		if (!already_open)
		{
			// Do not allow subscribe to kill window
			try
			{
				// If requested subscribe to it (do not if INBOX)
				if (subscribe && ::strcmpnocase(mbox->GetName(), cINBOX))
				{
					mbox->Subscribe();
					ResetTable();
				}
			}
			catch (CTCPException& ex)
			{
				CLOG_LOGCATCH(CTCPException&);

				// Allow window kill if aborting
				if (ex.error() == CTCPException::err_TCPAbort)
				{
					CLOG_LOGRETHROW;
					throw;
				}
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

			}
		}

	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Only delete the window if its not already been deleted
		if (new_window && mbox->IsOpen())
			new_window->GetMailboxView()->DoClose();

		// Throw up
		CLOG_LOGRETHROW;
		throw;
	}

	return true;
}

// Add names of selected items to list
bool CServerTable::AddNamedSelectionToList(TableIndexT row, cdstrvect* list)
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	// Ignore directories
	if ((GetCellDataType(woRow) != eServerBrowseMbox) || (GetCellMbox(woRow) == NULL) || GetCellMbox(woRow)->IsDirectory())
		return false;

	// Add mailbox to list (use URL if multiple servers)
	list->push_back(mManager ? GetCellMbox(woRow)->GetAccountName() : GetCellMbox(woRow)->GetName());

	return true;
}

// Select named items in list
bool CServerTable::SelectFromList(cdstrvect* list, CMboxList* mbox_list)
{
	bool selected_one = false;

	// Find new mailbox from previous selection
	for(cdstrvect::iterator iter = list->begin(); iter != list->end(); iter++)
	{
		CMbox* mbox = (mManager ? CMailAccountManager::sMailAccountManager->FindMboxAccount(*iter) : GetSingleServer()->FindMbox(*iter));
		if (mbox)
		{
			mbox_list->push_back(mbox);
			selected_one = true;
		}
	}
	return selected_one;
}

// Punt unseen in selected mailboxes
void CServerTable::DoPuntUnseenMailbox()
{
	// Do warning if required
	if (CPreferences::sPrefs->mWarnPuntUnseen.GetValue())
	{
		bool dontshow = false;
		short answer = CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::PuntUnseen", 0, &dontshow);
		if (dontshow)
			CPreferences::sPrefs->mWarnPuntUnseen.SetValue(false);
		if (answer == CErrorHandler::Cancel)
			return;
	}

	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	CMboxList selected;

	try
	{
		// Get all selected mailboxes
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
		{
			// Punt unseen in all mboxes
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
				PuntUnseenMailbox((CMbox*) *iter);

			// Redo table
			//ResetTable();
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DoDeleteMailbox

// Delete the specified mailbox
void CServerTable::PuntUnseenMailbox(CMbox* mbox)
{
	try
	{
		// Punt unseen the mailbox
		mbox->PuntUnseen();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DeleteMailbox

// Delete the selected mailboxes
void CServerTable::DoDeleteMailbox(void)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	// See if the selection is actually a cabinet item
	bool really_delete = false;
	if (TestSelectionOr((TestSelectionPP)( &CServerTable::TestSelectionFavouriteItems)))
	{
		// Ask user for delete or remove
		CErrorDialog::EDialogResult result = CErrorDialog::PoseDialog(CErrorDialog::eErrDialog_Caution,
																		"ErrorDialog::Btn::Remove",
																		"ErrorDialog::Btn::Delete",
																		"ErrorDialog::Btn::Cancel",
																		NULL,
																		"ErrorDialog::Text::DeleteMboxOrFavourite", 3);

		if (result == CErrorDialog::eBtn3)
			return;
		else if (result == CErrorDialog::eBtn1)
		{
			CServerNodeArray selected;

			// Add selection to list
			if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
			{
				for(CServerNodeArray::reverse_iterator iter = selected.rbegin(); iter != selected.rend(); iter++)
				{
					// Check that it can be removed
					CMboxRefList* ref_list = (CMboxRefList*) (*iter).mData;
					if (!CMailAccountManager::sMailAccountManager->IsRemoveableItemFavouriteType(ref_list))
						continue;

					// Remove it
					CMboxRef* mboxref = (CMboxRef*) ref_list->at((*iter).GetIndex());
					CMailAccountManager::sMailAccountManager->RemoveFavouriteItem(ref_list, mboxref);
				}
			}
			
			return;
		}
		else
			really_delete = true;
	}

	// Check that this is what we want to do
	try
	{
		// Get all selected mailboxes
		CMboxList selected;
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddDSelectionToList, &selected))
		{
			// Generate string for list of items being deleted
			cdstring deletes;
			for(CMboxList::const_iterator iter = selected.begin(); iter != selected.end(); iter++)
			{
				if (!deletes.empty())
					deletes += ", ";
				deletes += static_cast<const CMbox*>(*iter)->GetAccountName();
			}
			
			// Now ask user about delete, providing list of items being deleted
			if (really_delete || CErrorHandler::PutCautionAlertRsrcStr(true, "Alerts::Server::ReallyDelete", deletes) == CErrorHandler::Ok)
			{
				// Delete all mboxes in reverse
				for(CMboxList::reverse_iterator riter = selected.rbegin(); riter != selected.rend(); riter++)
					DeleteMailbox(static_cast<CMbox*>(*riter));
			}
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);
	}
}

// Delete the specified mailbox
void CServerTable::DeleteMailbox(CMbox* mbox)
{
	// Do no allow delete of INBOX in IMAP or POP3
	if (mbox->GetName() == cINBOX)
	{
		switch(mbox->GetProtocol()->GetAccountType())
		{
		case CINETAccount::eIMAP:
		case CINETAccount::ePOP3:
			CErrorHandler::PutStopAlertRsrc("Alerts::Server::NoDeleteINBOX");
			return;
		default:;
		}
	}

	try
	{
		// Resolve to open mailbox
		CMbox* open_mbox = mbox ? mbox->GetProtocol()->FindOpenMbox(mbox->GetName()) : NULL;
		
		// Close the mailbox - this will cleanup message/mailbox views etc
		if (open_mbox)
			mbox->Close();

		// Delete the mailbox using a task as the close operation may queue
		// mailbox window/preview tasks that must complete before the delete
		CDeleteMailboxTask* task = new CDeleteMailboxTask(mbox);
		task->Go();
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DeleteMailbox

// Rename the selected mailboxes
void CServerTable::DoRenameMailbox(void)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	CMboxList selected;

	try
	{
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddDSelectionToList, &selected))
		{
			//StValueChanger<bool> change(mListChanging, true);

			bool renamed = false;

			// Rename all mboxes
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
				renamed = RenameMailbox((CMbox*) *iter) || renamed;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DoRenameMailbox

// Rename the specified mailbox
bool CServerTable::RenameMailbox(CMbox* mbox)
{
	bool duplicate = false;

	do
	{
		// Get a new name for the mailbox (use old name as starter)
		cdstring new_name = mbox->GetName();
		if (CGetStringDialog::PoseDialog("Alerts::Server::RenameAs", new_name))
		{
			// Strip any trailing directory character here
			if (mbox->IsDirectory() && new_name.length() && (new_name[new_name.length() - 1] == mbox->GetDirDelim()))
				new_name[new_name.length() - 1] = 0;

			// Does new name already exist
			if (mbox->GetProtocol()->FindMbox(new_name))
			{
				CErrorHandler::PutStopAlertRsrc("Alerts::Server::DuplicateRename");
				duplicate = true;
			}
			else
			{
				// Rename the mailbox
				mbox->Rename(new_name);

				// Reset window before alert so screen is up to date with alert on screen
				{
					cdmutexprotect<CMailboxView::CMailboxViewList>::lock _lock(CMailboxView::sMailboxViews);
					for(CMailboxView::CMailboxViewList::iterator iter = CMailboxView::sMailboxViews->begin(); iter != CMailboxView::sMailboxViews->end(); iter++)
					{
						if ((*iter)->IsOpen() && (*iter)->ContainsMailbox(mbox))
							(*iter)->RenamedMailbox(mbox);
					}
				}

				return true;
			}
		}
		else
			duplicate = false;
	}
	while (duplicate);

	return false;

} // CServerTable::RenameMailbox

// Do full mailbox synchonrise
void CServerTable::DoSynchroniseMailbox(void)
{
	CMboxList selected;

	try
	{
		// Get all selected mailboxes
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
			CSynchroniseDialog::PoseDialog(&selected);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}
}

// Do clear disconnected cache
void CServerTable::DoClearDisconnectMailbox(void)
{
	CMboxList selected;

	// Check that this is what we want to do
	if (CErrorHandler::PutCautionAlertRsrc(true, "Alerts::Server::ReallyClearDisconnect") == CErrorHandler::Ok)
	{
		try
		{
			// Get all selected mailboxes
			if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
			{
				// Clear all mboxes
				for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
				{
					CMbox* mbox = static_cast<CMbox*>(*iter);

					// Must close any open windows if disconnected
					if (mbox->GetProtocol()->IsDisconnected() && mbox->IsOpen())
					{
						CMailboxView* view = CMailboxView::FindView(mbox);
						if (view)
							view->DoClose();
					}

					// Now clear disconnected cache
					mbox->ClearDisconnect();
				}
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

		}
		
		// State has changed so refresh
		RefreshSelection();
	}
}

// Subscribe to the selected mailboxes
void CServerTable::DoSubscribeMailbox(void)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	CMboxList selected;

	try
	{
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
		{
			//StValueChanger<bool> change(mListChanging, true);

			bool subscribed = false;

			// Subscribe all mboxes
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
				subscribed = SubscribeMailbox(static_cast<CMbox*>(*iter)) || subscribed;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DoSubscribeMailbox

// Subscribe to the specified mailbox
bool CServerTable::SubscribeMailbox(CMbox* mbox)
{
	// Determine subscribe to mailbox
	if (!mbox->IsSubscribed())
	{
		// Subscribe to the mailbox
		mbox->Subscribe();
		return true;
	}
	else
		return false;

} // CServerTable::SubscribeMailbox

// Unsubscribe from the selected mailboxes
void CServerTable::DoUnsubscribeMailbox(void)
{
	// Prevent flashes during multiple selection changes
	StDeferSelectionChanged noSelChange(this);

	CMboxList selected;

	try
	{
		if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
		{
			bool unsubscribed = false;

			// Rename all mboxes
			for(CMboxList::iterator iter = selected.begin(); iter != selected.end(); iter++)
				unsubscribed = UnsubscribeMailbox((CMbox*) *iter) || unsubscribed;
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

	}

} // CServerTable::DoUnsubscribeMailbox

// Subscribe from the specified mailbox
bool CServerTable::UnsubscribeMailbox(CMbox* mbox)
{
	// Determine unsubscribe from mailbox
	if (mbox->IsSubscribed())
	{
		// Unsubscribe from the mailbox
		mbox->Unsubscribe();
		return true;
	}
	else
		return false;

} // CServerTable::UnsubscribeMailbox

// Do mailbox search
void CServerTable::OnSearchMailbox()
{
	CMboxList mboxes;

	// Store list of selected nodes as selection may change
	CServerNodeArray selected;

	// Add selection to list
	if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectedNodesToList, &selected))
	{
		// Logon to all protocols
		for(CServerNodeArray::iterator iter = selected.begin(); iter != selected.end(); iter++)
		{
			switch((*iter).GetType())
			{
			case eServerBrowseServer:
				// Add all mailboxes in server
				{
					// Get the protocol
					CMboxProtocol* proto = reinterpret_cast<CMboxProtocol*>((*iter).mData);
					mboxes.Add(proto);
				}
				break;
			case eServerBrowseWD:
			case eServerBrowseSubs:
				// Add all mailboxes in display hierarchy
				{
					CMboxList* list = reinterpret_cast<CMboxList*>((*iter).mData);
					mboxes.Add(list);
				}
				break;
				// Add all mailboxes in subscribed hierarchy
				break;
			case eServerBrowseINBOX:
				// Add INBOX
				{
					CMboxProtocol* proto = reinterpret_cast<CMboxProtocol*>((*iter).mData);
					mboxes.Add(proto->GetINBOX());
				}
				break;
			case eServerBrowseMbox:
				// Add mailbox
				{
					CMboxList* list = reinterpret_cast<CMboxList*>((*iter).mData);
					unsigned long index = (*iter).GetIndex();
					if (index < list->size())
						mboxes.Add(static_cast<CMbox*>(list->at(index)));
				}
				break;
			case eServerBrowseMboxRefList:
				// Add all cabinet items
				{
					CMboxRefList* list = reinterpret_cast<CMboxRefList*>((*iter).mData);
					mboxes.Add(list);
				}
				break;
			case eServerBrowseMboxRef:
				// Add cabinet item:
				{
					CMboxRefList* list = reinterpret_cast<CMboxRefList*>((*iter).mData);
					unsigned long index = (*iter).GetIndex();
					if (index < list->size())
						mboxes.Add(static_cast<CMboxRef*>(list->at(index)));
				}
				break;
			default:;
			}
		}
	}

	// Just create the search window
	CSearchWindow::CreateSearchWindow();
	if (mboxes.size() != 0)
	{
		// May have duplicates due to multiple selected items so remove those
		mboxes.RemoveDuplicates();
		
		// Give list of unique mailboxes to search window
		CSearchWindow::AddMboxList(&mboxes);
	}
}

// Do mailbox search again
void CServerTable::OnSearchAgainMailbox()
{
	CMboxList selected;

	if (DoToSelection1((DoToSelection1PP) &CServerBrowse::AddSelectionToList, &selected))
	{
		// Do it again
		CSearchWindow::CreateSearchWindow();
		CSearchWindow::AddMboxList(&selected);
	}
}
