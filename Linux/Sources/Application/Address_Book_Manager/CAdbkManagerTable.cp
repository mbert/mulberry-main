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


// Source for CAdbkManagerTable class

#include "CAdbkManagerTable.h"

#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkProtocol.h"
#include "CAddressBook.h"
#include "CAddressBookManager.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CCommands.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerBrowse.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"
#include "CTitleTableView.h"
#include "CWaitCursor.h"

#include "StPenState.h"

#include <JPainter.h>
#include <JXImage.h>
#include <JXColormap.h>

#include <stdio.h>
#include <string.h>

const short cTextIndent = 26;

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerTable::CAdbkManagerTable(JXScrollbarSet* scrollbarSet, 
										JXContainer* enclosure,
										const HSizingOption hSizing, 
										const VSizingOption vSizing,
										const JCoordinate x, const JCoordinate y,
										const JCoordinate w, const JCoordinate h)
	: CHierarchyTableDrag(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	SetBorderWidth(0);

	mManager = NULL;
	mListChanging = false;
	mHierarchyCol = 1;

	// Create storage
	mTableGeometry = new CTableRowGeometry(this, 128, 18);
	mTableSelector = new CTableRowSelector(this);
	//mTableStorage = new LTableArrayStorage(this, sizeof(CAdbkList::node_type*));

	SetRowSelect(true);
}


// Default destructor
CAdbkManagerTable::~CAdbkManagerTable()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________


// Get details of sub-panes
void CAdbkManagerTable::OnCreate()
{
	const JXContainer* parent = GetEnclosure();
	while(parent && !dynamic_cast<const CAdbkManagerView*>(parent))
		parent = parent->GetEnclosure();
	mTableView = const_cast<CAdbkManagerView*>(dynamic_cast<const CAdbkManagerView*>(parent));

	CHierarchyTableDrag::OnCreate();

	CreateContextMenu(CMainMenu::eContextAdbkManager);
}

// Handle key presses
bool CAdbkManagerTable::HandleChar(const int key, const JXKeyModifiers& modifiers)
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
	
	return CHierarchyTableDrag::HandleChar(key, modifiers);
}

//	Respond to commands
bool CAdbkManagerTable::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
		OnAddressBookProperties();
		return true;
		
	case CCommand::eAddressesNew:
	case CCommand::eToolbarAdbkMgrNewBtn:
		OnNewAddressBook();
		return true;

	case CCommand::eAddressesOpen:
	case CCommand::eToolbarAdbkMgrOpenBtn:
		OnOpenAddressBook();
		return true;
		
	case CCommand::eAddressesRename:
		OnRenameAddressBook();
		return true;
		
	case CCommand::eAddressesDelete:
	case CCommand::eToolbarAdbkMgrDeleteBtn:
		OnDeleteAddressBook();
		return true;
		
	case CCommand::eAddressesLogin:
		OnLogin();
		return true;
		
	case CCommand::eDispHNew:
		OnNewHierarchy();
		return true;

	case CCommand::eDispHEdit:
		OnRenameHierarchy();
		return true;

	case CCommand::eDispHRemove:
		OnDeleteHierarchy();
		return true;

	case CCommand::eAddressesRefresh:
		OnRefreshAddressBook();
		return true;
		
	case CCommand::eAddressesSynchronise:
		OnSynchroniseAddressBook();
		return true;
		
	case CCommand::eAddressesClearDisconnected:
		OnClearDisconnectAddressBook();
		return true;

	case CCommand::eToolbarAdbkMgrSearchBtn:
		OnSearchAddressBook();
		return true;
		
	default:;
	}
	
	return CHierarchyTableDrag::ObeyCommand(cmd, menu);
}

void CAdbkManagerTable::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{

	// These ones must have a selection
	case CCommand::eEditProperties:
	case CCommand::eToolbarDetailsBtn:
	case CCommand::eAddressesRename:
	case CCommand::eAddressesDelete:
	case CCommand::eToolbarAdbkMgrDeleteBtn:
		OnUpdateAllAdbk(cmdui);
		return;

	case CCommand::eAddressesSynchronise:
		OnUpdateDisconnectedSelection(cmdui);
		return;

	case CCommand::eAddressesClearDisconnected:
		OnUpdateClearDisconnectedSelection(cmdui);
		return;
		
		// These can only have logged on protocol selection
	case CCommand::eAddressesNew:
	case CCommand::eToolbarAdbkMgrNewBtn:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eAddressesOpen:
	case CCommand::eToolbarAdbkMgrOpenBtn:
		OnUpdateAdbkSelection(cmdui);
		return;

		// These can only have logged on protocol selection
	case CCommand::eAddressesLogin:
		OnUpdateLoggedOutSelection(cmdui);
		return;

	case CCommand::eMailboxDisplayHierarchy:
	case CCommand::eDispHNew:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eDispHEdit:
	case CCommand::eDispHRemove:
		OnUpdateHierarchy(cmdui);
		return;

		// These can only have logged off protocol selection
	case CCommand::eAddressesRefresh:
	case CCommand::eAddressesLogout:
		OnUpdateLoggedInSelection(cmdui);
		return;
		 
	// Always available
	case CCommand::eToolbarAdbkMgrSearchBtn:
		OnUpdateAlways(cmdui);
		return;

	default:;
	}

	CHierarchyTableDrag::UpdateCommand(cmd, cmdui);
}

// Keep titles in sync
void CAdbkManagerTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Click in the cell
void CAdbkManagerTable::LClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eAdbkColumnOpen:
	case eAdbkColumnResolve:
	case eAdbkColumnSearch:
	{
		CAddressBook* adbk = GetCellNode(inCell.row);

		// Do status flag
		if (adbk->IsAdbk())
		{
			bool set;
			switch(col_info.column_type)
			{
			case eAdbkColumnOpen:
				set = !adbk->IsOpenOnStart();
				adbk->SetFlags(CAddressBook::eOpenOnStart, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, set);
		
				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookOpenOnStart(adbk, set);
				break;

			case eAdbkColumnResolve:
				set = !adbk->IsLookup();
				adbk->SetFlags(CAddressBook::eLookup, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, set);
		
				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookLookup(adbk, set);
				break;

			case eAdbkColumnSearch:
				set = !adbk->IsSearch();
				adbk->SetFlags(CAddressBook::eSearch, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(adbk, set);
		
				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookSearch(adbk, set);
				break;
			}
			
			RefreshRow(inCell.row);
		}
		break;
	}
	default:
		CHierarchyTableDrag::LClickCell(inCell, modifiers);
		DoSingleClick(inCell.row, CKeyModifiers(modifiers));
		break;
	}
}

void CAdbkManagerTable::LDblClickCell(const STableCell& inCell, const JXKeyModifiers& modifiers)
{
	DoDoubleClick(inCell.row, CKeyModifiers(modifiers));
}

// Draw the titles
void CAdbkManagerTable::DrawCell(JPainter* pDC, const STableCell& inCell,
									const JRect& inLocalRect)
{
	// Not if changing
	if (mListChanging)
		return;

	// Get cell data item
	int	woRow = GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);
	CAddressBook* adbk = GetCellNode(inCell.row);
	cdstring theTxt;

	// Erase to ensure drag hightlight is overwritten
	unsigned long bkgnd = 0x00FFFFFF;
	if (UsesBackgroundColor(inCell))
	{
		StPenState save(pDC);
		JRect bgrect(inLocalRect);
		//If we don't make the rectangle smaller, there will be no border between
		//color stripes.  If we just use the table row's border, then the tree
		//lines get broken up.
		bgrect.bottom -=1;
		
		JColorIndex bkgnd_index = GetBackgroundColor(inCell);
		pDC->SetPenColor(bkgnd_index);
		pDC->SetFilling(kTrue);
		pDC->Rect(bgrect);
		
		JSize r, g, b;
		GetColormap()->GetRGB(bkgnd_index, &r, &g, &b);
		bkgnd = ((r & 0xFF00) << 8) | (g & 0xFF00) | ((b & 0xFF00) >> 8);
	}

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];
	switch(col_info.column_type)
	{
	case eAdbkColumnName:
		{
		// Get suitable icon
		ResIDT iconID = GetPlotIcon(adbk, GetCellAdbkProtocol(inCell.row));
		JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);

		// Determine name to use
		theTxt = adbk->GetDisplayShortName();

		// Add protocol state descriptor
		if (adbk->IsProtocol())
		{
			if (adbk->GetProtocol()->IsDisconnected() && !CConnectionManager::sConnectionManager.IsConnected())
			{
				theTxt.AppendResource("UI::Server::TitleDisconnected");
			}
			else if (adbk->GetProtocol()->IsDisconnected() || (!adbk->GetProtocol()->IsOffline() && !adbk->GetProtocol()->IsLoggedOn()))
			{
				theTxt.AppendResource("UI::Server::TitleOffline");
			}
		}

		// Draw the string
		bool strike = false;
		SetTextStyle(pDC, adbk, GetCellAdbkProtocol(inCell.row), strike);
		int text_start = DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, icon);

		// Always cache column number
		mHierarchyCol = inCell.col;
		break;
		}
	case eAdbkColumnOpen:
		if (adbk->IsAdbk())
		{
			ResIDT iconID;
			if (adbk->IsOpenOnStart())
				iconID = IDI_DIAMONDTICKED;
			else
				iconID = IDI_DIAMOND;
			JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
		break;
		
	case eAdbkColumnResolve:
		if (adbk->IsAdbk())
		{
			ResIDT iconID;
			if (adbk->IsLookup())
				iconID = IDI_DIAMONDTICKED;
			else
				iconID = IDI_DIAMOND;
			JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
		break;
		
	case eAdbkColumnSearch:
		if (adbk->IsAdbk())
		{
			ResIDT iconID;
			if (adbk->IsSearch())
				iconID = IDI_DIAMONDTICKED;
			else
				iconID = IDI_DIAMOND;
			JXImage* icon = CIconLoader::GetIcon(iconID, this, 16, bkgnd);
			pDC->Image(*icon, icon->GetBounds(), inLocalRect.left, inLocalRect.top + mIconOrigin);
		}
		break;
		
	}
}


// Get appropriate icon id
JIndex CAdbkManagerTable::GetPlotIcon(const CAddressBook* adbk, CAdbkProtocol* proto)
{
	if (adbk->IsProtocol())
	{
		if (!proto)
			return IDI_SERVERLOCAL;
		else if (proto->CanDisconnect())
			return proto->IsDisconnected() ? IDI_SERVERDISCONNECTED : IDI_SERVERREMOTE;
		else if (proto->GetAccountType() == CINETAccount::eLocalAdbk)
			return IDI_SERVERLOCAL;
		else
			return IDI_SERVERREMOTE;
	}
	else if (adbk->IsDirectory() && !adbk->IsAdbk())
	{
		return IDI_SERVERDIR;
	}
	else
	{
		return adbk->IsCachedAdbk() ? IDI_ADDRESSFILECACHED : IDI_ADDRESSFILEUNCACHED;
	}
}

// Get text style
void CAdbkManagerTable::SetTextStyle(JPainter* pDC, const CAddressBook* adbk, CAdbkProtocol* proto, bool& strike)
{
	strike = false;
	JFontStyle text_style;
	short style = normal;

	if (adbk->IsProtocol())
	{
		if (!proto || proto->IsLoggedOn())
		{
			GetColormap()-> JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerOpenStyle.GetValue().color, &text_style.color);
			style = style | CPreferences::sPrefs->mServerOpenStyle.GetValue().style;
		}
		else
		{
			GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerClosedStyle.GetValue().color, &text_style.color);
			style = style | CPreferences::sPrefs-> mServerClosedStyle.GetValue().style;
		}
	}
	else
	{
		GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mMboxClosedStyle.GetValue().color, &text_style.color);
		style = style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style;
	}
	text_style.bold = JBoolean(style & bold);
	text_style.italic = JBoolean(style & italic);
	text_style.underlineCount = JBoolean((style & underline) ? 1 : 0);
	text_style.strike = JBoolean(style & strike);

	pDC->SetPenColor(text_style.color);

	pDC->SetFontStyle(text_style);
}

bool CAdbkManagerTable::UsesBackgroundColor(const STableCell& inCell) const
{
	TableIndexT	woRow = mCollapsableTree->GetWideOpenIndex(inCell.row);
	return mCollapsableTree->GetNestingLevel(woRow) == 0;
}

JColorIndex CAdbkManagerTable::GetBackgroundColor(const STableCell& inCell) const
{
	JColorIndex ret;
	GetColormap()->JColormap::AllocateStaticColor(CPreferences::sPrefs->mServerBkgndStyle.GetValue().color, &ret);
	return ret;
}

SColumnInfo CAdbkManagerTable::GetColumnInfo(TableIndexT col)
{
	return mTableView->GetColumnInfo()[col - 1];
}

#if NOTYET
// Adjust cursor over drop area
bool CAdbkManagerTable::IsCopyCursor(DragReference inDragRef)
{
	return true;
}

// Test drag insert cursor
bool CAdbkManagerTable::IsDropCell(DragReference inDragRef, STableCell theCell)
{
	return IsValidCell(theCell) && IsCellAdbk(theCell.row);
}

// Handle multiple messages
void CAdbkManagerTable::DoDragReceive(DragReference inDragRef)
{
	// Init drop params
	mDropFirst = true;
	mDropSort = false;
	mDropAdbk = NULL;
	mDropAdbkWnd = NULL;
	mAddressAdded = false;
	mGroupAdded = false;

	try
	{
		// Do default action
		CHierarchyTableDrag::DoDragReceive(inDragRef);
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Do not allow failure
	}

	// Always force window update
	if (mDropAdbkWnd)
		mDropAdbkWnd->ResetTable(mAddressAdded, mGroupAdded);
}

// Drop data into cell
void CAdbkManagerTable::DropDataIntoCell(FlavorType theFlavor, char* drag_data,
										Size data_size, const STableCell& theCell)
{
	if (mDropFirst)
	{
		// Determine adbk to use for copy
		mDropAdbk = GetCellAdbk(theCell.row);

		// Does window already exist?
		mDropAdbkWnd = CAddressBookWindow::FindWindow(mDropAdbk);
		mDropSort = (mDropAdbkWnd != NULL);

		mDropFirst = false;
	}

	switch (theFlavor)
	{

		case cDragMsgType:
			CMessage* theMsg = *((CMessage**) drag_data);
			CEnvelope* theEnv = theMsg->GetEnvelope();
			ThrowIfNil_(theEnv);

			// Add From
			if (theEnv->GetFrom()->size())
			{
				// Add to list
				mDropAdbk->AddAddress(new CAdbkAddress(*theEnv->GetFrom()->front()), mDropSort);
				mAddressAdded = true;
			}

			// Add Reply-To
			if (theEnv->GetReplyTo()->size())
			{
				// Add to list
				mDropAdbk->AddAddress(new CAdbkAddress(*theEnv->GetReplyTo()->front()), mDropSort);
				mAddressAdded = true;
			}

			// Add all To: except me
			for(CAddressList::iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				// Add to list
				mDropAdbk->AddAddress(new CAdbkAddress(**iter), mDropSort);
				mAddressAdded = true;
			}

			// Add all Ccs except me
			for(CAddressList::iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				// Add to list
				mDropAdbk->AddAddress(new CAdbkAddress(**iter), mDropSort);
				mAddressAdded = true;
			}

			break;

		case cDragAddrType:
			CAddress* theAddr = *((CAddress**) drag_data);
			{
				// Add to list
				mDropAdbk->AddAddress(new CAdbkAddress(*theAddr), mDropSort);
				mAddressAdded = true;
			}
			break;

		case cDragAddrGrpType:
			CGroup* theGrp = *((CGroup**) drag_data);

			// Add group to new group
			mDropAdbk->AddGroup(new CGroup(*theGrp), mDropSort);
			mGroupAdded = true;
			break;

		case 'TEXT':

			// Parse text into list
			CAddressList list(drag_data, data_size);

			// Try to add these to table
			for(CAddressList::iterator iter = list.begin(); iter != list.end(); iter++)
			{
				// Now add to address book
				mDropAdbk->AddAddress(new CAdbkAddress(**iter), mDropSort);
				mAddressAdded = true;
			}
			break;
	}
}
#endif

void CAdbkManagerTable::OnUpdateAllAdbk(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbk));
}

void CAdbkManagerTable::OnUpdateAdbkSelection(CCmdUI* pCmdUI)
{
	if (IsSingleSelection())
	{
		STableCell aCell(0,0);
		GetNextSelectedCell(aCell);
		CAdbkProtocol* proto = GetCellAdbkProtocol(aCell.row);
		pCmdUI->Enable((proto == NULL) || ((pCmdUI->mCmd == CCommand::eAddressesNew) ? proto->IsLoggedOn() : false));
	}
	else if (!IsSelectionValid())
	{
		pCmdUI->Enable((pCmdUI->mCmd == CCommand::eAddressesNew) ? false :
						!CAdminLock::sAdminLock.mNoLocalAdbks);
	}
	else
		pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbk));
}

void CAdbkManagerTable::OnUpdateLoggedInSelection(CCmdUI* pCmdUI)
{
	if (IsSingleSelection())
	{
		STableCell aCell(0,0);
		GetNextSelectedCell(aCell);
		CAdbkProtocol* proto = GetCellAdbkProtocol(aCell.row);
		pCmdUI->Enable((proto != NULL) && proto->IsLoggedOn());
	}
	else
		pCmdUI->Enable(false);
}

void CAdbkManagerTable::OnUpdateLoggedOutSelection(CCmdUI* pCmdUI)
{
	if (IsSingleSelection())
	{
		STableCell aCell(0,0);
		GetNextSelectedCell(aCell);
		CAdbkProtocol* proto = GetCellAdbkProtocol(aCell.row);
		pCmdUI->Enable((proto != NULL) && !proto->IsLoggedOn());
	}
	else
		pCmdUI->Enable(false);
}

void CAdbkManagerTable::OnUpdateDisconnectedSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbkDisconnected));
}

void CAdbkManagerTable::OnUpdateClearDisconnectedSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionAdbkClearDisconnected));
}

void CAdbkManagerTable::OnUpdateHierarchy(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TestSelectionAnd((TestSelectionPP) &CAdbkManagerTable::TestSelectionHierarchy));
}
