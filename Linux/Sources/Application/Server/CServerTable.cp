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
#include "CCommands.h"
#include "CMailAccountManager.h"
#include "CMailboxInfoTable.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMboxRef.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulSelectionData.h"
#include "CPreferences.h"
#include "CServerTitleTable.h"
#include "CServerView.h"
#include "CServerViewPopup.h"
#include "CServerWindow.h"

#include "HButtonText.h"

#include "JXMultiImageCheckbox.h"
#include <JXDNDManager.h>

// __________________________________________________________________________________________________
// C L A S S __ C S E R V E R T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerTable::CServerTable(JXScrollbarSet* scrollbarSet, 
							JXContainer* enclosure,	
							const HSizingOption hSizing, 
							const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
  : CServerBrowse(scrollbarSet, enclosure, hSizing, vSizing, x,y,w,h)
{
	SetBorderWidth(0);
}

// Default destructor
CServerTable::~CServerTable()
{
}

void CServerTable::OnCreate()
{
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CServerView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CServerView*>(dynamic_cast<const CServerView*>(parent));

	CServerBrowse::OnCreate();

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

	// Create the context menu
	CreateContextMenu(CMainMenu::eContextServerTable);

	//make keypresses come to us
	WantInput(kTrue, kTrue, kTrue, kTrue);
}

// Keep titles in sync
void CServerTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CServerBrowse::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

void CServerTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eToolbarServerLoginBtn:
		OnUpdateLoginBtn(cmdui);
		return;

	case CCommand::eFileOpenMailbox:
		OnUpdateOpenMailbox(cmdui);
		return;

	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
		OnUpdateProperties(cmdui);
		return;

	case CCommand::eMailboxCreate:
	case CCommand::eToolbarCreateMailboxBtn:
		OnUpdateMailboxCreate(cmdui);
		return;

	case CCommand::eMailboxRename:
	case CCommand::eMailboxMarkContentsSeen:
	case CCommand::eMailboxSubscribe:
	case CCommand::eMailboxUnsubscribe:
	case CCommand::eMailboxCheckMail:
	case CCommand::eMailboxSearchAgain:
		// Must be mailboxes on a logged in server
		OnUpdateLoggedInSelection(cmdui);
		return;

	case CCommand::eToolbarCheckMailboxBtn:
		OnUpdateAlways(cmdui);
		break;

	case CCommand::eMailboxSynchronise:
		OnUpdateMailboxSynchronise(cmdui);
		return;

	case CCommand::eMailboxClearDisconnected:
		OnUpdateMailboxClearDisconnect(cmdui);
		return;

	case CCommand::eMailboxDelete:
		OnUpdateLoggedInDSelection(cmdui);
		return;

	case CCommand::eMailboxSearch:
	case CCommand::eToolbarSearchMailboxBtn:
		OnUpdateLoggedIn(cmdui);
		return;

	case CCommand::eMailboxDisplayHierarchy:
	case CCommand::eDispHNew:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eDispHEdit:
	case CCommand::eDispHRemove:
		OnUpdateHierarchy(cmdui);
		return;

	case CCommand::eDispHReset:
		OnUpdateResetHierarchy(cmdui);
		return;

	case CCommand::eDispHRefresh:
		OnUpdateHierarchic(cmdui);
		return;

	case CCommand::eMailboxCabinets:
	case CCommand::eToolbarCabinetPopup:
	case CCommand::eCabNew:
		OnUpdateManager(cmdui);
		return;

	case CCommand::eCabEdit:
	case CCommand::eCabRemove:
		OnUpdateFavouriteRemove(cmdui);
		return;

	case CCommand::eCabAddWild:
		OnUpdateFavouriteWildcard(cmdui);
		return;

	case CCommand::eCabEditWild:
		OnUpdateFavouriteWildcardItems(cmdui);
		return;

	case CCommand::eCabRemoveItem:
		OnUpdateFavouriteRemoveItems(cmdui);
		return;
	}

	CServerBrowse::UpdateCommand(cmd, cmdui);
}

bool CServerTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eToolbarServerLoginBtn:
		OnServerLogon();
		return true;

	case CCommand::eFileNewMessage:
	case CCommand::eToolbarNewLetter:
		OnFileNewDraft();
		return true;

	case CCommand::eFileOpenMailbox:
		DoChooseMailbox();
		return true;

	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
		OnEditProperties();
		return true;

	case CCommand::eMailboxCreate:
	case CCommand::eToolbarCreateMailboxBtn:
		OnCreateMailbox();
		return true;

	case CCommand::eMailboxRename:
		DoRenameMailbox();
		return true;

	case CCommand::eMailboxMarkContentsSeen:
		DoPuntUnseenMailbox();
		return true;

	case CCommand::eMailboxDelete:
		DoDeleteMailbox();
		return true;

	case CCommand::eMailboxCheckMail:
		DoCheckMail();
		return true;

	case CCommand::eToolbarCheckMailboxBtn:
		CMulberryApp::sApp->ObeyCommand(CCommand::eMailboxCheckFavourites);
		return true;

	case CCommand::eDispHNew:
		DoNewHierarchy();
		return true;

	case CCommand::eDispHEdit:
		DoRenameHierarchy();
		return true;

	case CCommand::eDispHRemove:
		DoDeleteHierarchy();
		return true;

	case CCommand::eDispHReset:
		DoResetHierarchy();
		return true;

	case CCommand::eDispHRefresh:
		DoRefreshHierarchy();
		return true;

	case CCommand::eCabNew:
		DoNewFavourite();
		return true;

	case CCommand::eCabEdit:
		DoRenameFavourite();
		return true;

	case CCommand::eCabRemove:
		DoRemoveFavourite();
		return true;

	case CCommand::eToolbarCabinetPopup:
		OnServerView(menu->mIndex);
		return true;

	case CCommand::eCabAddWild:
		DoNewWildcardItem();
		return true;

	case CCommand::eCabEditWild:
		DoRenameWildcardItem();
		return true;

	case CCommand::eCabRemoveItem:
		DoRemoveFavouriteItems();
		return true;

	case CCommand::eMailboxSynchronise:
		DoSynchroniseMailbox();
		return true;

	case CCommand::eMailboxClearDisconnected:
		DoClearDisconnectMailbox();
		return true;

	case CCommand::eMailboxSubscribe:
		DoSubscribeMailbox();
		return true;

	case CCommand::eMailboxUnsubscribe:
		DoUnsubscribeMailbox();
		return true;

	case CCommand::eMailboxSearch:
	case CCommand::eToolbarSearchMailboxBtn:
		OnSearchMailbox();
		return true;

	case CCommand::eMailboxSearchAgain:
		OnSearchAgainMailbox();
		return true;
	}

	//we don't handle it, so pass it along to parent 
	return CServerBrowse::ObeyCommand(cmd, menu);
}

SColumnInfo	CServerTable::GetColumnInfo(TableIndexT col)
{
	return mTableView->GetColumnInfo()[col - 1];
}

//#pragma mark ____________________________Commands/Actions

// Handle key down
bool CServerTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
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
	case '\t':
		SelectNextRecent(modifiers.shift());
		return true;
	case '\b':
	case 127: //delete
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
		return CServerBrowse::HandleChar(key, modifiers);
	}
}

// Clicked item
void CServerTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	CServerBrowse::LClickCell(inCell, modifiers);
	DoSingleClick(inCell.row, CKeyModifiers(modifiers));
}

// Double-clicked item
void CServerTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoDoubleClick(inCell.row, CKeyModifiers(modifiers));
}



void CServerTable::DoSelectionChanged(void)
{
	// Flat/hierarchic button if only one hierarchy selected
	// NB This button is always present whether we are in a 1-pane or 3-pane window
	if (IsSingleSelection())
	{
		SetListening(kFalse);
		TableIndexT row = GetFirstSelectedRow();
		TableIndexT woRow = mCollapsableTree->GetWideOpenIndex(row + TABLE_ROW_ADJUST);

		if ((GetCellDataType(woRow) == eServerBrowseWD) ||
			(GetCellDataType(woRow) == eServerBrowseSubs))
		{
			mTableView->GetHierarchyBtn()->Activate();
			mTableView->GetHierarchyBtn()->SetState(JBoolean(GetCellMboxList(woRow)->IsHierarchic()));
		}
		else if (GetCellDataType(woRow) == eServerBrowseMboxRefList)
		{
			mTableView->GetHierarchyBtn()->Activate();
			mTableView->GetHierarchyBtn()->SetState(JBoolean(GetCellMboxRefList(woRow)->IsHierarchic()));
		}
		else
			mTableView->GetHierarchyBtn()->Deactivate();
		SetListening(kTrue);
	}
	else if (mSingle && GetSingleServer()->FlatWD())
	{
		SetListening(kFalse);
		mTableView->GetHierarchyBtn()->Activate();
		mTableView->GetHierarchyBtn()->SetState(JBoolean(GetSingleServer()->GetHierarchies().at(1)->IsHierarchic()));
		SetListening(kTrue);
	}
	else
		mTableView->GetHierarchyBtn()->Deactivate();

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

void CServerTable::OnUpdateLoginBtn(CCmdUI* pCmdUI)
{
	// Get button state
	bool enabled = false;
	bool pushed = false;
	GetLogonState(enabled, pushed);
	
	// Update command
	pCmdUI->Enable(enabled);
	pCmdUI->SetCheck(pushed);
	cdstring title;
	title.FromResource(pushed ? IDS_LOGOUT : IDS_LOGIN);
	pCmdUI->SetText(title);
}

void CServerTable::OnFileNewDraft(void)
{
	DoNewLetter(GetDisplay()->GetLatestKeyModifiers().meta());
}

// Edit:Properties command
void CServerTable::OnEditProperties(void)
{
	if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionServer) ||
				mSingle && !IsSelectionValid())
		DoServerProperties();
	else if (TestSelectionAnd((TestSelectionPP) &CServerTable::TestSelectionMboxAll))
		DoMailboxProperties();

	Focus();
}
// Server logon button
void CServerTable::OnServerLogon(void)
{
	// Set focus back to table after button push
	// Do this before anthing else as mailbox windows may be created during login
	Focus();

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
void CServerTable::OnServerView(JIndex index)
{
	if (index == CServerViewPopup::eServerView_New)
		DoNewFavourite();
	else
		SetView(index - CServerViewPopup::eServerView_First); 
}

// Create Mailbox
void CServerTable::OnCreateMailbox(void)
{
	DoCreateMailbox(true);

	// Set focus back to table after button push
	Focus();
}

//#pragma mark ____________________________Server Related


//#pragma mark ____________________________Hierarchy Related

// Change to flat view of hierarchy
void CServerTable::DoFlatHierarchy(void)
{
	SetFlatHierarchy(mTableView->GetHierarchyBtn()->IsChecked());

	// Set focus back to table after button push
	Focus();
}

//#pragma mark ____________________________Drag & Drop

// Prevent drag if improper selection
bool CServerTable::DoDrag(const JPoint& pt,
							const JXButtonStates& buttonStates,
							const JXKeyModifiers& modifiers)
{
	return ValidDragSelection() && CServerBrowse::DoDrag(pt, buttonStates, modifiers);
}

//return the atom for the given selection
bool CServerTable::GetSelectionAtom(CFlavorsList& atoms)
{
	TableIndexT row = GetFirstSelectedRow();
	unsigned long woRow = GetWideOpenIndex(row + TABLE_ROW_ADJUST);

	// Assume drag only starts when selection is all the same type
	// Thus type of this cell is type of all cells
	EServerBrowseDataType type = GetCellDataType(woRow);

	switch(type)
	{
	case eServerBrowseServer:
		// Dragging server to another location
		atoms.push_back(CMulberryApp::sFlavorServerList);
		break;
	case eServerBrowseWD:
		// Dragging WD to new location
		atoms.push_back(CMulberryApp::sFlavorWDList);
		break;
	case eServerBrowseMbox:
		// Dragging mailbox
		atoms.push_back(CMulberryApp::sFlavorMboxList);
		break;
	case eServerBrowseMboxRef:
		// Dragging mailbox ref
		atoms.push_back(CMulberryApp::sFlavorMboxRefList);
		break;
	default:
		// Anything else is illegal!
		return false;
		
	}
	//if we get out of the switch then we're ok
	mDragFlavor = atoms.front();
	return true;
}

bool CServerTable::RenderSelectionData(CMulSelectionData* seldata, Atom type)
{
	bool rendered = false;

	// gcc is stupid and does not know which ItemIsAcceptable to use
	// here even though the argument types are clearly different
	if (CTableDragAndDrop::ItemIsAcceptable(type))
	{
		ulvector rows;
		GetSelectedRows(rows);
		int count = rows.size();

		// Allocate global memory for the text
		unsigned long dataLength = count * sizeof(void*) + sizeof(int);
		unsigned char* data = new unsigned char[dataLength];
		if (data)
		{
			int* i = reinterpret_cast<int*>(data);
			*i = count;
			i++;
			void** vdata = reinterpret_cast<void**>(i);
			for(ulvector::const_iterator iter = rows.begin(); iter != rows.end(); iter++)
			{
				TableIndexT woRow = GetWideOpenIndex(*iter + TABLE_ROW_ADJUST);
				if (type == CMulberryApp::sFlavorServerList)
				{
					*vdata = GetCellServer(woRow);
					vdata++;
				}
				else if (type == CMulberryApp::sFlavorWDList)
				{
					*vdata = GetCellMboxList(woRow);
					vdata++;
				}
				else if (type == CMulberryApp::sFlavorMboxList)
				{
					*vdata = GetCellMbox(woRow);
					vdata++;
				}
				else if (type == CMulberryApp::sFlavorMboxRefList)
				{
					*vdata = GetCellMboxRef(woRow);
					vdata++;
				}
			}
			seldata->SetData(type, data, dataLength);
			rendered = true;
		}
	}

	return rendered;
}

// Check for valid drag selection
bool CServerTable::ValidDragSelection() const
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
		default:;
		}
		
		// Can only have one type
		if (got_favourite + got_wd + got_server + got_mbox + got_mboxref > 1)
			return false;
	}
	
	return true;
}

// Check whether drag item is acceptable
bool CServerTable::ItemIsAcceptable(const JArray<Atom>& typeList)
{
	// Do inherited first
	if (CServerBrowse::ItemIsAcceptable(typeList))
	{
		// Look for a format
		JOrderedSetIterator<Atom>* ti = typeList.NewIterator();
		Atom i;
		while (ti->Next(&i))
		{
			// Some formats cannot be dragged outside the source window
			if (((i == CMulberryApp::sFlavorWDList) ||
				(i == CMulberryApp::sFlavorMboxRefList)) &&
				(sTableDragSource != sTableDropTarget))
			{
				// Only allow if source and target are the same
				delete ti;
				return false;
			}
		}
		delete ti;
		return true;
	}
	
	return false;

} // CTableDragAndDrop::ItemIsAcceptable

// Draw drag insert cursor
bool CServerTable::IsDropCell(JArray<Atom>& typeList, const STableCell& cell)
{
	if (IsValidCell(cell))
	{
		// Get flavor for this item
		Atom theFlavor;
		FlavorsMatch(typeList, mDropFlavors,&theFlavor);

		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(cell.row);
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
				Atom theFlavor;
				GetDropData(CurrentTime);
				if (mDropData)
				{
					// Allow drop if same server
					void* data = mDropData;
					data = ((char*)data) + sizeof(int);
					CMbox* src = ((CMbox**) data)[0]; 
					can_do = (src->GetProtocol() == GetCellMboxList(woRow)->GetProtocol());
				}
				return can_do;
			}
			case eServerBrowseMboxRefList:
				{
					// Allow drop into editable lists
					CMailAccountManager::EFavourite fav = 
						CMailAccountManager::sMailAccountManager->
						GetFavouriteType(GetCellMboxRefList(woRow));
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
bool CServerTable::IsDropAtCell(JArray<Atom>& typeList, STableCell& cell)
{
	// Get flavor for this item
	Atom theFlavor;
	FlavorsMatch(typeList, mDropFlavors,&theFlavor);

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
			
			//Are we dropping before server?
			if (type == eServerBrowseServer)
				return true;
			
			// always allow after last row
			if (adjust)
				return true;

			// Are we dropping right after a server?
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
				Atom theFlavor;
				GetDropData(CurrentTime);
				if (mDropData)
				{
					void* data = mDropData;
					data = ((char*)data) + sizeof(int);
					CMboxList* src = ((CMboxList**) data)[0];

					// Check for same server of previous row
					bool fail = (woRow > 1) ? (src->GetProtocol() != ResolveCellServer(woRow - 1 + adjust)) : true;
				
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
bool CServerTable::CanDropExpand(const JArray<Atom>& typeList, TableIndexT woRow)
{
	// Get flavor for this item
	//unsigned int theFlavor = GetBestFlavor(pDataObject);
	Atom theFlavor;
	FlavorsMatch(typeList, mDropFlavors,&theFlavor);

	if ((theFlavor == CMulberryApp::sFlavorMsgList) ||
		(theFlavor == CMulberryApp::sFlavorMboxList) ||
		(theFlavor == CMulberryApp::sFlavorMboxRefList))
		return CServerBrowse::CanDropExpand(typeList, woRow);
	else
		// Anything else cannot
		return false;
}

// Determine effect
Atom CServerTable::GetDNDAction(const JXContainer* target,
									 const JXButtonStates& buttonStates,
									 const JXKeyModifiers& modifiers)
{
	// Get flavor for this item
	unsigned int theFlavor = mDragFlavor;
	JXDNDManager* dndMgr = GetDNDManager();
	Atom copy = dndMgr->GetDNDActionCopyXAtom();
	Atom move = dndMgr->GetDNDActionMoveXAtom();

	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		bool option_key = modifiers.control();

		if (CPreferences::sPrefs->deleteAfterCopy.GetValue() ^ option_key)
			return move;
		else
			return copy;
	}
	else if ((theFlavor == CMulberryApp::sFlavorServerList) ||
			 (theFlavor == CMulberryApp::sFlavorWDList))
		// Server & WDs always moved
		return move;

	else if ((theFlavor == CMulberryApp::sFlavorMboxList) ||
			 (theFlavor == CMulberryApp::sFlavorMboxRefList))
	{
		// Mailboxes & references moved if drop at or no drop cell
		if (mLastDropCursor.row || !mLastDropCell.row)
			return move;
		
		// Must check cell dropped into to see if directory
		// Get drop cell type
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(mLastDropCell.row + TABLE_ROW_ADJUST);
		EServerBrowseDataType type = GetCellDataType(woRow);
		
		switch(type)
		{
		case eServerBrowseMbox:
			return ((GetCellMbox(woRow) != NULL) && !GetCellMbox(woRow)->IsDirectory()) ? copy : move;
		default:
			return copy;
		}
	}

	else
		// Anything else is an error!
		return move;
}

// Drop data into cell
bool CServerTable::DropData(Atom theFlavor, unsigned char* drag_data, 
														unsigned long data_size)
{
	if (theFlavor == CMulberryApp::sFlavorServerList)
	{
		CMboxProtocol* theServer = NULL;
		int count = *(int*)(drag_data);
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
bool CServerTable::DropDataIntoCell(Atom theFlavor, unsigned char* drag_data,
											unsigned long data_size, const STableCell& theCell)
{
	if (theFlavor == CMulberryApp::sFlavorMsgList)
	{
		// Determine mbox to use for copy
		CMbox* mbox_to = NULL;
		TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(theCell.row + TABLE_ROW_ADJUST);
		if (GetCellDataType(woRow) == eServerBrowseMbox)
			mbox_to = GetCellMbox(woRow);
		else if (GetCellDataType(woRow) == eServerBrowseMboxRef)
			mbox_to = GetCellMboxRef(woRow)->ResolveMbox(true);
		if (mbox_to == NULL)
			return false;

		// Get list of chosen message nums
		ulvector nums;
		CMessageList* msgs = reinterpret_cast<CMessageList*>(drag_data);
		CMbox* mbox_from = NULL;
		for(CMessageList::const_iterator iter = msgs->begin(); iter != msgs->end(); iter++)
		{
			nums.push_back((*iter)->GetMessageNumber());
			mbox_from = (*iter)->GetMbox();
		}

		// Do mail message copy from mbox in drag to this mbox
		if (mbox_from && (nums.size() > 0))
		{
			try
			{
				// Do copy action - delete if drop was a move
				CActionManager::CopyMessage(mbox_from, mbox_to, &nums, mDropActionMove);

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
bool CServerTable::DropDataAtCell(Atom theFlavor, unsigned char* drag_data,
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

		//int count = *((int*) drag_data)++;
		int count = *((int*)drag_data);
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
		//int count = *((int*) drag_data)++;
		int count = *(int*)(drag_data);
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
		//int count = *((int*) drag_data)++;
		int count = *(int*)drag_data;
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
