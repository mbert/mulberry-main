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

#include "CMailAccountManager.h"
#include "CMailboxInfoTable.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMulberryApp.h"
#include "CPreferences.h"
#include "CServerView.h"
#include "CServerWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C S E R V E R T A B L E
// __________________________________________________________________________________________________


IMPLEMENT_DYNCREATE(CServerTable, CServerBrowse)

BEGIN_MESSAGE_MAP(CServerTable, CServerBrowse)
	ON_UPDATE_COMMAND_UI(IDM_FILE_NEW_DRAFT, OnUpdateAlways)
	ON_COMMAND(IDM_FILE_NEW_DRAFT, OnFileNewDraft)

	ON_UPDATE_COMMAND_UI(IDM_FILE_OPEN_MAILBOX, OnUpdateOpenMailbox)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateNever)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVE_AS, OnUpdateNever)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNever)				// Do not allow delete: this is read-only

	ON_UPDATE_COMMAND_UI(IDM_EDIT_PROPERTIES, OnUpdateProperties)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CREATE, OnUpdateMailboxCreate)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_RENAME, OnUpdateLoggedInSelection)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_PUNT, OnUpdateLoggedInSelection)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_DELETE, OnUpdateLoggedInDSelection)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SYNCHRONISE, OnUpdateMailboxSynchronise)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CLEARDISCONNECT, OnUpdateMailboxClearDisconnect)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SUBSCRIBE, OnUpdateLoggedInSelection)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_UNSUBSCRIBE, OnUpdateLoggedInSelection)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_CHECK, OnUpdateLoggedInSelection)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SEARCH, OnUpdateLoggedIn)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_SEARCH_AGAIN, OnUpdateLoggedInSelection)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_NEWSEARCH, OnUpdateAlways)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_EDITSEARCH, OnUpdateHierarchy)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_REMOVESEARCH, OnUpdateHierarchy)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_RESETHIERS, OnUpdateResetHierarchy)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_NEWCABINET, OnUpdateManager)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_EDITCABINET, OnUpdateFavouriteRemove)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_REMOVECABINET, OnUpdateFavouriteRemove)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_ADDWILDCARDITEM, OnUpdateFavouriteWildcard)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_EDITWILDCARDITEM, OnUpdateFavouriteWildcardItems)
	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_REMOVEITEM, OnUpdateFavouriteRemoveItems)

	ON_UPDATE_COMMAND_UI(IDM_MAILBOX_REFRESHLIST, OnUpdateHierarchic)

	ON_UPDATE_COMMAND_UI(IDC_LOGINBTN, OnUpdateLoginBtn)

	ON_COMMAND(IDM_FILE_OPEN_MAILBOX, DoChooseMailbox)

	ON_COMMAND(IDC_LOGINBTN, OnServerLogon)
	ON_COMMAND(IDC_OPENMAILBOX, DoOpenMailbox)
	ON_COMMAND(IDC_SERVERFLATBTN, DoFlatHierarchy)

	ON_UPDATE_COMMAND_UI(IDC_SERVERCABINETPOPUP, OnUpdateAlways)
	ON_COMMAND_RANGE(IDM_ServerViewNew, IDM_ServerViewEnd, OnServerView)

	ON_COMMAND(IDM_EDIT_PROPERTIES, OnEditProperties)

	ON_COMMAND(IDM_MAILBOX_CREATE, OnCreateMailbox)
	ON_COMMAND(IDM_MAILBOX_RENAME, DoRenameMailbox)
	ON_COMMAND(IDM_MAILBOX_PUNT, DoPuntUnseenMailbox)
	ON_COMMAND(IDM_MAILBOX_DELETE, DoDeleteMailbox)
	ON_COMMAND(IDM_MAILBOX_SYNCHRONISE, DoSynchroniseMailbox)
	ON_COMMAND(IDM_MAILBOX_CLEARDISCONNECT, DoClearDisconnectMailbox)
	ON_COMMAND(IDM_MAILBOX_CHECK, DoCheckMail)
	ON_COMMAND(IDM_MAILBOX_AUTOCHECK, OnMailboxCheckFavourites)

	ON_COMMAND(IDM_MAILBOX_NEWSEARCH, DoNewHierarchy)
	ON_COMMAND(IDM_MAILBOX_EDITSEARCH, DoRenameHierarchy)
	ON_COMMAND(IDM_MAILBOX_REMOVESEARCH, DoDeleteHierarchy)
	ON_COMMAND(IDM_MAILBOX_RESETHIERS, DoResetHierarchy)

	ON_COMMAND(IDM_MAILBOX_NEWCABINET, DoNewFavourite)
	ON_COMMAND(IDM_MAILBOX_EDITCABINET, DoRenameFavourite)
	ON_COMMAND(IDM_MAILBOX_REMOVECABINET, DoRemoveFavourite)
	ON_COMMAND(IDM_MAILBOX_ADDWILDCARDITEM, DoNewWildcardItem)
	ON_COMMAND(IDM_MAILBOX_EDITWILDCARDITEM, DoRenameWildcardItem)
	ON_COMMAND(IDM_MAILBOX_REMOVEITEM, DoRemoveFavouriteItems)

	ON_COMMAND(IDM_MAILBOX_REFRESHLIST, DoRefreshHierarchy)

	ON_COMMAND(IDM_MAILBOX_SUBSCRIBE, DoSubscribeMailbox)
	ON_COMMAND(IDM_MAILBOX_UNSUBSCRIBE, DoUnsubscribeMailbox)

	ON_COMMAND(IDM_MAILBOX_SEARCH, OnSearchMailbox)
	ON_COMMAND(IDM_MAILBOX_SEARCH_AGAIN, OnSearchAgainMailbox)

	ON_WM_CREATE()
	ON_WM_LBUTTONDOWN()
	
	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSERVERLOGINBTN, OnUpdateLoginBtn)
	ON_COMMAND(IDC_TOOLBARSERVERLOGINBTN, OnServerLogon)

	ON_COMMAND(IDC_TOOLBARNEWLETTER, OnFileNewDraft)
	ON_COMMAND(IDC_TOOLBARNEWLETTEROPTION, OnFileNewDraft)
	ON_COMMAND(IDC_TOOLBARDETAILSBTN, OnEditProperties)
	
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARCREATEMAILBOXBTN, OnUpdateMailboxCreate)
	ON_COMMAND(IDC_TOOLBARCREATEMAILBOXBTN, OnCreateMailbox)

	ON_COMMAND(IDC_TOOLBARCHECKMAILBOXBTN, OnMailboxCheckFavourites)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSEARCHMAILBOXBTN, OnUpdateLoggedIn)
	ON_COMMAND(IDC_TOOLBARSEARCHMAILBOXBTN, OnSearchMailbox)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARCABINETPOPUP, OnUpdateAlways)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerTable::CServerTable()
{
}

// Default destructor
CServerTable::~CServerTable()
{
}

int CServerTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CServerBrowse::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info

	// Can drag servers (manager only), mboxes & mboxrefs (manager only)
	AddDragFlavor(CMulberryApp::sFlavorWDList);
	AddDragFlavor(CMulberryApp::sFlavorMboxList);

	// Can drop servers, mboxes, mboxrefs (manager only) & messages
	AddDropFlavor(CMulberryApp::sFlavorServerList);
	AddDropFlavor(CMulberryApp::sFlavorWDList);
	AddDropFlavor(CMulberryApp::sFlavorMboxList);
	AddDropFlavor(CMulberryApp::sFlavorMsgList);

	SetReadOnly(false);
	SetDropCell(true);
	SetDropCursor(true);
	SetAllowDrag(true);
	SetAllowMove(true);
	SetHandleMove(false);		// The mailbox move/copy operations handles deleting original item
	SetSelfDrag(true);

	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CServerView*>(parent))
		parent = parent->GetParent();
	mTableView = dynamic_cast<CServerView*>(parent);
	SetSuperCommander(mTableView);

	return 0;
}

// Keep titles in sync
void CServerTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CServerBrowse::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

#pragma mark ____________________________Command Updaters

// Update command
void CServerTable::OnUpdateManager(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mManager);
}

// Update command
void CServerTable::OnUpdateLoggedIn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->HasOpenProtocols());
}

// Update command
void CServerTable::OnUpdateLoggedInSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAvailable));
}

// Update command
void CServerTable::OnUpdateLoggedInDSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestDSelectionMbox));
}

// Update command
void CServerTable::OnUpdateProperties(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer) ||
					TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAll) ||
					mSingle && !IsSelectionValid());
}

// Update command
void CServerTable::OnUpdateOpenMailbox(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(!mSingle ? CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->HasOpenProtocols() :
								GetSingleServer() && GetSingleServer()->IsLoggedOn());
}

// Update command
void CServerTable::OnUpdateMailboxCreate(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(CMailAccountManager::sMailAccountManager && CMailAccountManager::sMailAccountManager->HasOpenProtocols());
}

// Update command
void CServerTable::OnUpdateMailboxSynchronise(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxDisconnected));
}

// Update command
void CServerTable::OnUpdateMailboxClearDisconnect(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxClearDisconnected));
}

// Update command
void CServerTable::OnUpdateFavouriteRemove(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemove));
}

// Update command
void CServerTable::OnUpdateFavouriteRemoveItems(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemoveItems));
}

// Update command
void CServerTable::OnUpdateFavouriteWildcard(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteWildcard));
}

// Update command
void CServerTable::OnUpdateFavouriteWildcardItems(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(mManager && TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteWildcardItems));
}

// Update command
void CServerTable::OnUpdateHierarchy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionHierarchy));
}

// Update command
void CServerTable::OnUpdateResetHierarchy(CCmdUI* pCmdUI)
{
	if (IsSelectionValid())
		pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionResetRefresh));
	else
		pCmdUI->Enable(mSingle);
}

// Update command
void CServerTable::OnUpdateHierarchic(CCmdUI* pCmdUI)
{
	if (IsSelectionValid())
		pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionResetRefresh));
	else
		pCmdUI->Enable(mSingle && GetSingleServer()->FlatWD());
}

// Update command
void CServerTable::OnUpdateLoginBtn(CCmdUI* pCmdUI)
{
	// Get button state
	bool enabled = false;
	bool pushed = false;
	GetLogonState(enabled, pushed);
	
	// Update command
	pCmdUI->Enable(enabled);
	pCmdUI->SetCheck(pushed);
	CString title;
	title.LoadString(pushed ? IDS_LOGOUT : IDS_LOGIN);
	pCmdUI->SetText(title);
}

#pragma mark ____________________________Commands/Actions

// Handle key down
bool CServerTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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
	case VK_TAB:
	{
		SelectNextRecent(::GetKeyState(VK_SHIFT) < 0);
		return true;
	}

	case VK_BACK:
	case VK_DELETE:
	{
		if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer))
			DoLogoffServers();
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionHierarchy))
			//DoDeleteHierarchy();
			;	// Do nothing - force use of menu commands
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemove))
			DoRemoveFavourite();
		else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionFavouriteRemoveItems))
			DoRemoveFavouriteItems();
		return true;
	}

	default:
		return CServerBrowse::HandleKeyDown(nChar, nRepCnt, nFlags);
	}
}

// Clicked item
void CServerTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	CServerBrowse::LClickCell(inCell, nFlags);
	DoSingleClick(inCell.row, CKeyModifiers(0));
}

// Double-clicked item
void CServerTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoDoubleClick(inCell.row, CKeyModifiers(0));
}

void CServerTable::DoSelectionChanged(void)
{
	// Update buttons

	// Flat/hierarchic button if only one hierarchy selected
	if (IsSingleSelection())
	{

		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);
		
		if ((GetCellDataType(woRow) == eServerBrowseWD) ||
			(GetCellDataType(woRow) == eServerBrowseSubs))
		{
			mTableView->GetHierarchyBtn()->EnableWindow(true);
			mTableView->GetHierarchyBtn()->SetPushed(GetCellMboxList(woRow)->IsHierarchic());
		}
		else if (GetCellDataType(woRow) == eServerBrowseMboxRefList)
		{
			mTableView->GetHierarchyBtn()->EnableWindow(true);
			mTableView->GetHierarchyBtn()->SetPushed(GetCellMboxRefList(woRow)->IsHierarchic());
		}
		else
			mTableView->GetHierarchyBtn()->EnableWindow(false);
	}			
	else if (mSingle && GetSingleServer()->FlatWD())
	{
		mTableView->GetHierarchyBtn()->EnableWindow(true);
		mTableView->GetHierarchyBtn()->SetPushed(GetSingleServer()->GetHierarchies().at(1)->IsHierarchic());
	}	
	else
		mTableView->GetHierarchyBtn()->EnableWindow(false);
	
	// Do inherited
	CServerBrowse::DoSelectionChanged();
	
	// Determine whether preview is triggered
	const CUserAction& preview = mTableView->GetPreviewAction();
	if (preview.GetSelection())
		DoPreview();

	// Determine whether full view is triggered
	const CUserAction& fullview = mTableView->GetFullViewAction();
	if (fullview.GetSelection())
		DoFullView();
}

// Update status items like toolbar
void CServerTable::UpdateState()
{
	// Force toolbar refresh
	mTableView->RefreshToolbar();
}

SColumnInfo	CServerTable::GetColumnInfo(TableIndexT col)
{
	return mTableView->GetColumnInfo()[col - 1];
}

void CServerTable::OnFileNewDraft(void)
{
	DoNewLetter(::GetKeyState(VK_MENU) < 0);
}

// Edit:Properties command
void CServerTable::OnEditProperties(void)
{
	if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer) ||
				mSingle && !IsSelectionValid())
		DoServerProperties();
	else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAll))
		DoMailboxProperties();

	// Set focus back to table after button push
	SetFocus();
}

void CServerTable::OnMailboxCheckFavourites(void)
{
	CMulberryApp::sApp->OnAppMailCheck();
}

// Create Mailbox
void CServerTable::OnCreateMailbox(void)
{
	DoCreateMailbox(true);

	// Set focus back to table after button push
	SetFocus();
}

#pragma mark ____________________________Server Related

// Server logon button
void CServerTable::OnServerLogon(void)
{
	// Set focus back to table after button push
	// Do this before anthing else as mailbox windows may be created during login
	SetFocus();

	// Get button state
	bool enabled = false;
	bool pushed = false;
	GetLogonState(enabled, pushed);
	
	if (!pushed)
	{
		// Look for selected server cell
		if (mManager && !mSingle)
		{
			// Get selected cell
			TableIndexT row = GetFirstSelectedRow();
			TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

			// Logon to appropriate server
			LogonAction(GetCellServer(woRow));
		}
		
		// Got single server window
		else
			LogonAction(GetSingleServer());
	}
	else
	{
		// Look for selected server cell
		if (mManager && !mSingle)
		{
			// Get selected cell
			TableIndexT row = GetFirstSelectedRow();
			TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

			// Logon to appropriate server
			LogoffAction(GetCellServer(woRow));
		}
		
		// Got single server window
		else
			LogoffAction(GetSingleServer());
	}
	
	// Always reset state
	UpdateState();
}

// Server view popup
void CServerTable::OnServerView(UINT nID)
{
	if (nID == IDM_ServerViewNew)
		DoNewFavourite();

	else
		SetView(nID - IDM_ServerViewStart);
}

#pragma mark ____________________________Hierarchy Related

// Change to flat view of hierarchy
void CServerTable::DoFlatHierarchy(void)
{
	// Change hierarchy state
	SetFlatHierarchy(mTableView->GetHierarchyBtn()->IsPushed());

	// Set focus back to table after button push
	SetFocus();
}

#pragma mark ____________________________Drag & Drop

// Prevent drag if improper selection
BOOL CServerTable::DoDrag(TableIndexT row)
{
	return ValidDragSelection() && CServerBrowse::DoDrag(row);
}

// Only add selected item flavors to drag
void CServerTable::SetDragFlavors(TableIndexT row)
{
	// Get the relevant attachment
	TableIndexT woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	// Assume drag only starts when selection is all the same type
	// Thus type of this cell is type of all cells
	EServerBrowseDataType type = GetCellDataType(woRow);

	switch(type)
	{
	case eServerBrowseServer:
		// Dragging server to another location
		mDrag.DelayRenderData(CMulberryApp::sFlavorServerList);
		break;
	case eServerBrowseWD:
		// Dragging WD to new location
		mDrag.DelayRenderData(CMulberryApp::sFlavorWDList);
		break;
	case eServerBrowseMbox:
		// Dragging mailbox
		mDrag.DelayRenderData(CMulberryApp::sFlavorMboxList);
		break;
	case eServerBrowseMboxRef:
		// Dragging mailbox ref
		mDrag.DelayRenderData(CMulberryApp::sFlavorMboxRefList);
		break;
	default:
		// Anything else is illegal!
		;
	}
}

BOOL CServerTable::OnRenderGlobalData(LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal)
{
	BOOL rendered = false;

	if  ((lpFormatEtc->cfFormat == CMulberryApp::sFlavorServerList) ||
		 (lpFormatEtc->cfFormat == CMulberryApp::sFlavorWDList) ||
		 (lpFormatEtc->cfFormat == CMulberryApp::sFlavorMboxList) ||
		 (lpFormatEtc->cfFormat == CMulberryApp::sFlavorMboxRefList))
	{
		ulvector rows;
		GetSelectedRows(rows);
		int count = rows.size();

		// Allocate global memory for the text if not already
		if (!*phGlobal)
			*phGlobal = ::GlobalAlloc(GMEM_DDESHARE, count * sizeof(void*) + sizeof(int));

		if (*phGlobal)
		{
			// Copy to global after lock
			void** pGrp = (void**) ::GlobalLock(*phGlobal);
			*((int*) pGrp) = count;
			pGrp += sizeof(int);
			for(ulvector::const_iterator iter = rows.begin(); iter != rows.end(); iter++)
			{
				TableIndexT woRow = GetWideOpenIndex(*iter + TABLE_ROW_ADJUST);
				if (lpFormatEtc->cfFormat == CMulberryApp::sFlavorServerList)
					*pGrp++ = GetCellServer(woRow);
				else if (lpFormatEtc->cfFormat == CMulberryApp::sFlavorWDList)
					*pGrp++ = GetCellMboxList(woRow);
				else if (lpFormatEtc->cfFormat == CMulberryApp::sFlavorMboxList)
					*pGrp++ = GetCellMbox(woRow);
				else if (lpFormatEtc->cfFormat == CMulberryApp::sFlavorMboxRefList)
					*pGrp++ = GetCellMboxRef(woRow);
			}
			::GlobalUnlock(*phGlobal);
			
			rendered = true;
		}
	}

	return rendered;
}

// Check for valid drag selection
bool CServerTable::ValidDragSelection(void) const
{
	// Loop over all selected cells
	int got_favourite = 0;
	int got_wd = 0;
	int got_server = 0;
	int got_mbox = 0;
	int got_mboxref = 0;
	CMboxProtocol* proto_wd = NULL;
	STableCell selCell(0, 0);
	while(GetNextSelectedCell(selCell))
	{
		TableIndexT woRow = GetWideOpenIndex(selCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);
		
		switch(type)
		{
		case eServerBrowseMbox:
			got_mbox = 1;
			break;
		case eServerBrowseMboxRef:
			got_mboxref = 1;
			break;
		case eServerBrowseWD:
			// Check same server
			if (!got_wd)
			{
				got_wd = 1;
				proto_wd = ResolveCellServer(woRow);
			}
			else if (ResolveCellServer(woRow) != proto_wd)
				return false;
			break;
		case eServerBrowseSubs:
			// Never drag subscribed
			return false;
#if 0
		case eServerBrowseMboxRefList:
			got_favourite = 1;
			
			// Can only drag non internal
			if (CMailAccountManager::sMailAccountManager->GetFavouriteType(GetCellMboxRefList(woRow)) < CMailAccountManager::eFavouriteOthers)
				return false;
			break;
#endif
		case eServerBrowseServer:
			got_server = 1;
			break;
		default:
			return false;
		}
		
		// Can only have one type
		if (got_favourite + got_wd + got_server + got_mbox + got_mboxref > 1)
			return false;
	}
	
	return true;
}

// Check whether drag item is acceptable
bool CServerTable::ItemIsAcceptable(COleDataObject* pDataObject)
{
	// Do inherited first
	if (CServerBrowse::ItemIsAcceptable(pDataObject))
	{
		// Look for a format
		pDataObject->BeginEnumFormats();
		
		FORMATETC format;
		pDataObject->GetNextFormat(&format);
		
		// Some formats cannot be dragged outside the source window
		if ((format.cfFormat == CMulberryApp::sFlavorWDList) ||
			(format.cfFormat == CMulberryApp::sFlavorMboxRefList))
		{
			// Only allow if source and target are the same
			return (sTableDragSource == sTableDropTarget);
		}
		else
			return true;
	}
	
	return false;

} // CTableDragAndDrop::ItemIsAcceptable

// Draw drag insert cursor
bool CServerTable::IsDropCell(COleDataObject* pDataObject, const STableCell& cell)
{
	if (IsValidCell(cell))
	{
		// Get flavor for this item
		unsigned int theFlavor = GetBestFlavor(pDataObject);

		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(cell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);

		if (theFlavor == CMulberryApp::sFlavorMsgList)
		{
			// Drop into valid mailboxes only
			switch(type)
			{
			case eServerBrowseMbox:
				return (GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->IsDirectory();
			case eServerBrowseMboxRef:
				return (GetCellMboxRef(woRow) != NULL) && !GetCellMboxRef(woRow)->IsDirectory();
			default:
				return false;
			}
		}

		else if ((theFlavor == CMulberryApp::sFlavorServerList) ||
			(theFlavor == CMulberryApp::sFlavorWDList))
			// Serevrs & WDs always moved
			return false;

		else if (theFlavor == CMulberryApp::sFlavorMboxList)
		{
			switch(type)
			{
			case eServerBrowseMbox:
				// Allow drop into any mailbox (directory = move, mbox = copy)
				return true;
			case eServerBrowseSubs:
			{
				bool can_do = false;
				HGLOBAL hglb = pDataObject->GetGlobalData(theFlavor);
				if (hglb)
				{
					// Allow drop if same server
					char* drag_data = (char*) ::GlobalLock(hglb);
					((int*) drag_data);
					drag_data += sizeof(int);
					CMbox* src = ((CMbox**) drag_data)[0]; 
					can_do = (src->GetProtocol() == GetCellMboxList(woRow)->GetProtocol());
					::GlobalUnlock(hglb);
				}
				return can_do;
			}
			case eServerBrowseMboxRefList:
			{
				// Allow drop into editable lists
				CMailAccountManager::EFavourite fav = CMailAccountManager::sMailAccountManager->GetFavouriteType(GetCellMboxRefList(woRow));
				return (fav != CMailAccountManager::eFavouriteNew);
			}
			default:
				return false;
			}
		}
		else if (theFlavor == CMulberryApp::sFlavorMboxRefList)
		{
			// Drop into any other ref list
			switch(type)
			{
			case eServerBrowseMboxRefList:
				// Allow drop into favourite list
				return true;
			default:
				return false;
			}
		}
		else
			// Anything else is an error!
			return false;
	}
	else
		return false;
}

// Test drop at cell
bool CServerTable::IsDropAtCell(COleDataObject* pDataObject, STableCell& cell)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	// Adjust for end of list
	int adjust = 0;
	if (cell.row > mRows)
		adjust = 1;

	if (IsValidCell(cell) || adjust)
	{
		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(cell.row + TABLE_ROW_ADJUST - adjust);
		EServerBrowseDataType type = GetCellDataType(woRow);

		if (theFlavor == CMulberryApp::sFlavorMsgList)
			// Always drop into, never at
			return false;

		else if (theFlavor == CMulberryApp::sFlavorServerList)
		{
			// Must be same window
			if (sTableDragSource != this)
				return false;

			// Server can only be dropped before/after server in the same window
			if (type == eServerBrowseServer)
				return true;
			
			// Beyond last row allowed
			if (adjust)
				return true;

			// Check for row immediately after server
			if (woRow > 1)
			{
				EServerBrowseDataType prev_type = (woRow > 1 ? GetCellDataType(woRow - 1 + adjust) : eServerBrowseNone);
				return (prev_type == eServerBrowseServer);
			}
			return false;
		}

		else if (theFlavor == CMulberryApp::sFlavorWDList)
		{
			// Must be same window
			if (sTableDragSource != this)
				return false;

			{
				HGLOBAL hglb = pDataObject->GetGlobalData(theFlavor);
				if (hglb)
				{
					char* drag_data = (char*) ::GlobalLock(hglb);
					((int*) drag_data);
					drag_data += sizeof(int);
					CMboxList* src = ((CMboxList**) drag_data)[0];

					// Check for same server of previous row
					bool fail = (woRow > 1) ? (src->GetProtocol() != ResolveCellServer(woRow - 1 + adjust)) : true;
				
					::GlobalUnlock(hglb);
					
					if (fail)
						return false;
				}
			}

			// Allow if next cell is WD (or next server)
			if ((type == eServerBrowseWD) || (type == eServerBrowseServer))
				return true;

			// WD can only be dropped at same server in the same window
			return false;
		}

		else if (theFlavor == CMulberryApp::sFlavorMboxList)
		{
			switch(type)
			{
			case eServerBrowseMbox:
				// Allow drop at any mailbox that is not INBOX and not in subscribed hierarchy
				if (IsCellINBOX(woRow))
					return false;
				else
					return (GetCellMboxList(woRow)->GetHierarchyIndex() > 0);
			default:
				return false;
			}
		}

		else if (theFlavor == CMulberryApp::sFlavorMboxRefList)
			// Can only drop into favourites
			return false;

		else
			// Anything else is an error!
			return false;
	}
	else
		return false;
}

// Can cell expand for drop
bool CServerTable::CanDropExpand(COleDataObject* pDataObject, TableIndexT woRow)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	if ((theFlavor == CMulberryApp::sFlavorMsgList) ||
		(theFlavor == CMulberryApp::sFlavorMboxList) ||
		(theFlavor == CMulberryApp::sFlavorMboxRefList))
		return CServerBrowse::CanDropExpand(pDataObject, woRow);
	else
		// Anything else cannot
		return false;
}

// Determine effect
DROPEFFECT CServerTable::GetDropEffect(CWnd* pWnd, COleDataObject* pDataObject, DWORD dwKeyState, CPoint point)
{
	// Get flavor for this item
	unsigned int theFlavor = GetBestFlavor(pDataObject);

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		bool option_key = ((dwKeyState & MK_ALT) == MK_ALT);

		if (CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key)
			return DROPEFFECT_MOVE;
		else
			return DROPEFFECT_COPY;
	}
	else if ((theFlavor == CMulberryApp::sFlavorServerList) ||
			 (theFlavor == CMulberryApp::sFlavorWDList))
		// Server & WDs always moved
		return DROPEFFECT_MOVE;

	else if ((theFlavor == CMulberryApp::sFlavorMboxList) ||
			 (theFlavor == CMulberryApp::sFlavorMboxRefList))
	{
		// Mailboxes & references moved if drop at or no drop cell
		if (mLastDropCursor.row || !mLastDropCell.row)
			return DROPEFFECT_MOVE;
		
		// Must check cell dropped into to see if directory
		// Get drop cell type
		int	woRow = mCollapsableTree->GetWideOpenIndex(mLastDropCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);
		
		switch(type)
		{
		case eServerBrowseMbox:
			return ((GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->IsDirectory()) ? DROPEFFECT_COPY : DROPEFFECT_MOVE;
		default:
			return DROPEFFECT_COPY;
		}
	}

	else
		// Anything else is an error!
		return DROPEFFECT_MOVE;
}

// Drop data into cell
bool CServerTable::DropData(unsigned int theFlavor, char* drag_data, unsigned long data_size)
{
	if (theFlavor == CMulberryApp::sFlavorServerList)
	{
		CMboxProtocol* theServer = NULL;
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		
		// Only do for first one
		if (count)
		{
			// Make this window show this server if not manager
			if (!mManager)
			{
				SetServer(*(CMboxProtocol**) drag_data);
				return true;
			}
		}
	}
	
	return false;
}

// Drop data into cell
bool CServerTable::DropDataIntoCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& theCell)
{
	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		// Determine mbox to use for copy
		CMbox* mbox_to = NULL;
		int	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
		if (GetCellDataType(woRow) == eServerBrowseMbox)
			mbox_to = GetCellMbox(woRow);
		else if (GetCellDataType(woRow) == eServerBrowseMboxRef)
			mbox_to = GetCellMboxRef(woRow)->ResolveMbox(true);
		if (mbox_to == NULL)
			return false;

		// Get list of chosen message nums
		ulvector nums;
		CMessage* theMsg = NULL;
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			theMsg = ((CMessage**) drag_data)[i];

			// Do not allow copy to same mailbox
			if (theMsg->GetMbox() != mbox_to)
				nums.push_back(theMsg->GetMessageNumber());
		}

		// Do mail message copy from mbox in drag to this mbox
		if (theMsg && (nums.size() > 0))
		{
			try
			{
				ulmap temp;
				theMsg->GetMbox()->CopyMessage(nums, false, mbox_to, temp);

				// Set flag to allow delete after copy
				CMailboxInfoTable::sDropOnMailbox = true;
			}
			catch (...)
			{
				CLOG_LOGCATCH(...);

				// Make sure not signalled as mailbox to prevent delete after copy
				CMailboxInfoTable::sDropOnMailbox = false;
				
				CLOG_LOGRETHROW;
				throw;
			}
		}
		return true;
	}
	else if (theFlavor == CMulberryApp::sFlavorWDList)
		return false;

	else if (theFlavor == CMulberryApp::sFlavorMboxList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CMbox* mbox = ((CMbox**) drag_data)[i];

			// Get drop cell type
			TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
			EServerBrowseDataType type = GetCellDataType(woRow);
			
			switch(type)
			{
			case eServerBrowseMbox:
				{
					CMbox* destination = GetCellMbox(woRow);
					if (destination != NULL)
					{
						if (destination->IsDirectory())
						{
							// Move mailbox
							mbox->MoveMbox(destination, false);
						}
						else
						{
							// Do mailbox -> mailbox copy
							CMbox* open_mbox = mbox->GetProtocol()->FindOpenMbox(mbox->GetName());
							if (open_mbox)
								mbox = open_mbox;
							
							// Do copy (will open/close if not already)
							mbox->CopyMbox(destination);
						}
					}
				}
				break;
			case eServerBrowseMboxRefList:
				{
					CMboxRefList* ref_list = GetCellMboxRefList(woRow);

					// Add to list
					CMailAccountManager::sMailAccountManager->AddFavouriteItem(ref_list, mbox);
				}
				break;
			case eServerBrowseSubs:
				{
					// Just subscribe
					mbox->Subscribe();
				}
				break;
			default:;
			}
		}
		return true;
	}

	else if (theFlavor == CMulberryApp::sFlavorMboxRefList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CMboxRef* mboxref = ((CMboxRef**) drag_data)[i];

			// Get drop cell type
			TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
			EServerBrowseDataType type = GetCellDataType(woRow);
			
			switch(type)
			{
			case eServerBrowseMboxRefList:
				{
					CMboxRefList* ref_list = GetCellMboxRefList(woRow);

					// Make sure its not copied back to itself
					if (ref_list->FetchIndexOf(mboxref) == 0)
						// Add to list
						CMailAccountManager::sMailAccountManager->AddFavouriteItem(ref_list, mboxref);
				}
				break;
			default:;
			}
		}
		return true;
	}

	else
		return false;
}

// Drop data at cell
bool CServerTable::DropDataAtCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& beforeCell)
{
	// Adjust for end of list
	int adjust = 0;
	if (beforeCell.row > mRows)
		adjust = 1;

	// Get drop cell type
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(beforeCell.row + TABLE_ROW_ADJUST - adjust);
	EServerBrowseDataType type = GetCellDataType(woRow);
	
	if (theFlavor == CMulberryApp::sFlavorServerList)
	{
		// Get new index
		unsigned long new_index = 0;
		if (adjust)
			new_index = CMailAccountManager::sMailAccountManager->GetProtocolCount();
		else
		{
			CMboxProtocol* above = GetCellServer(woRow);
			if (type == eServerBrowseServer)
				new_index = CMailAccountManager::sMailAccountManager->GetProtocolIndex(above);
			else
				new_index = CMailAccountManager::sMailAccountManager->GetProtocolIndex(above) + 1;
		}

		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CMboxProtocol* moved = ((CMboxProtocol**) drag_data)[i];

			switch(type)
			{
			case eServerBrowseWD:
			case eServerBrowseServer:
			{
				// Get old index
				unsigned long old_index = CMailAccountManager::sMailAccountManager->GetProtocolIndex(moved);

				// Only if different
				if (old_index != new_index)
					CMailAccountManager::sMailAccountManager->MoveProtocol(old_index, new_index);
				
				// Must adjust if dragging above
				if (new_index < old_index)
					new_index++;
				break;
			}
			default:;
			}
		}
		return true;
	}
	else if (theFlavor == CMulberryApp::sFlavorWDList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);

		// Get new index
		unsigned long new_index = 0;
		if (!adjust && (type == eServerBrowseWD))
		{
			CMboxList* drop_list = GetCellMboxList(woRow);
			new_index = drop_list->GetHierarchyIndex();
		}
		else if (count)
			new_index = ((CMboxList**) drag_data)[0]->GetProtocol()->GetHierarchies().size();

		for(int i = 0; i < count; i++)
		{
			CMboxList* list = ((CMboxList**) drag_data)[i];

			switch(type)
			{
			case eServerBrowseWD:
			case eServerBrowseServer:
			{
				// Move old WD to new WD
				unsigned long old_index = list->GetHierarchyIndex();

				// Only if different
				if (old_index != new_index)
					// Change wds
					list->GetProtocol()->MoveWD(old_index, new_index);
				
				// Must adjust if dragging above
				if (new_index < old_index)
					new_index++;
				break;
			}
			default:;
			}
		}
		return true;
	}

	else if (theFlavor == CMulberryApp::sFlavorMboxList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CMbox* mbox = ((CMbox**) drag_data)[i];

			switch(type)
			{
			case eServerBrowseMbox:
				{
					// Get parent mbox
					CMbox* destination = GetCellMbox(woRow);
					if (destination != NULL)
					{
						CMboxList* list = GetCellMboxList(woRow);
						CMbox* parent = (CMbox*) list->GetParentNode(destination);

						// Move mailbox to WD level
						mbox->MoveMbox(parent ? parent : destination, !parent);
					}
				}
				break;
			default:;
			}
		}
		return true;
	}

	else
		return false;
}
