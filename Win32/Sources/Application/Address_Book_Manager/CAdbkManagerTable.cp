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

#include "CActionManager.h"
#include "CAdbkManagerView.h"
#include "CAdbkManagerWindow.h"
#include "CAdbkProtocol.h"
#include "CAddressBookDoc.h"
#include "CAddressBookManager.h"
#include "CAddressBookWindow.h"
#include "CAdminLock.h"
#include "CDrawUtils.h"
#include "CEnvelope.h"
#include "CFontCache.h"
#include "CIconLoader.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CRemoteAddressBook.h"
#include "CTableRowSelector.h"
#include "CTableRowGeometry.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X T I T L E T A B L E
// __________________________________________________________________________________________________


IMPLEMENT_DYNCREATE(CAdbkManagerTable, CHierarchyTableDrag)

BEGIN_MESSAGE_MAP(CAdbkManagerTable, CHierarchyTableDrag)

	ON_COMMAND(IDC_ADDRESS_NEWMSG, OnNewDraft)

	ON_UPDATE_COMMAND_UI(ID_EDIT_CLEAR, OnUpdateNever)				// Do not allow delete: this is read-only

	ON_UPDATE_COMMAND_UI(IDM_EDIT_PROPERTIES, OnUpdateAllAdbk)
	ON_COMMAND(IDM_EDIT_PROPERTIES, OnAddressBookProperties)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_NEW, OnUpdateAlways)
	ON_COMMAND(IDM_ADDR_NEW, OnNewAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_OPEN, OnUpdateAdbkSelection)
	ON_COMMAND(IDM_ADDR_OPEN, OnOpenAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_RENAME, OnUpdateAllAdbk)
	ON_COMMAND(IDM_ADDR_RENAME, OnRenameAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_DELETE, OnUpdateAllAdbk)
	ON_COMMAND(IDM_ADDR_DELETE, OnDeleteAddressBook)

	ON_COMMAND(IDM_ADDR_SEARCH, OnSearchAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_LOGIN, OnUpdateLoggedOutSelection)
	ON_COMMAND(IDM_ADDR_LOGIN, OnLoginAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_LOGOUT, OnUpdateLoggedInSelection)
	ON_COMMAND(IDM_ADDR_LOGOUT, OnLogoutAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_REFRESH, OnUpdateLoggedInSelection)
	ON_COMMAND(IDM_ADDR_REFRESH, OnRefreshAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_SYNC, OnUpdateDisconnectedSelection)
	ON_COMMAND(IDM_ADDR_SYNC, OnSynchroniseAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_CLEAR_DISCONNECT, OnUpdateClearDisconnectedSelection)
	ON_COMMAND(IDM_ADDR_CLEAR_DISCONNECT, OnClearDisconnectAddressBook)

	ON_WM_CREATE()

	// Toolbar
	ON_UPDATE_COMMAND_UI(IDC_TOOLBARDETAILSBTN, OnUpdateAllAdbk)
	ON_COMMAND(IDC_TOOLBARDETAILSBTN, OnAddressBookProperties)

	ON_COMMAND(IDC_TOOLBARADBKMGRNEWBTN, OnNewAddressBook)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARADBKMGROPENBTN, OnUpdateAdbkSelection)
	ON_COMMAND(IDC_TOOLBARADBKMGROPENBTN, OnOpenAddressBook)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARADBKMGRDELETEBTN, OnUpdateAllAdbk)
	ON_COMMAND(IDC_TOOLBARADBKMGRDELETEBTN, OnDeleteAddressBook)

	ON_COMMAND(IDC_TOOLBARADBKMGRSEARCHBTN, OnSearchAddressBook)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerTable::CAdbkManagerTable()
{
	mManager = NULL;
	mListChanging = false;
	mHierarchyCol = 0;
	mHasOthers = false;

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

int CAdbkManagerTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CHierarchyTableDrag::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set Drag & Drop info

	AddDropFlavor(CMulberryApp::sFlavorMsgList);
	AddDropFlavor(CMulberryApp::sFlavorAddrList);
	AddDropFlavor(CMulberryApp::sFlavorGrpList);
	AddDropFlavor(CF_UNICODETEXT);

	SetReadOnly(false);
	SetDropCell(true);
	SetAllowDrag(false);
	SetSelfDrag(false);
	SetAllowMove(false);

	CWnd* parent = GetParent();
	while(parent && !dynamic_cast<CAdbkManagerView*>(parent))
		parent = parent->GetParent();
	mTableView = dynamic_cast<CAdbkManagerView*>(parent);
	SetSuperCommander(mTableView);

	return 0;
}

// Command updaters
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
		pCmdUI->Enable((proto == nil) || ((pCmdUI->m_nID == IDM_ADDR_NEW) ? proto->IsLoggedOn() : false));
	}
	else if (!IsSelectionValid())
	{
		pCmdUI->Enable((pCmdUI->m_nID == IDM_ADDR_NEW) ? false :
						!mHasOthers && !CAdminLock::sAdminLock.mNoLocalAdbks);
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
		pCmdUI->Enable((proto != nil) && proto->IsLoggedOn());
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
		pCmdUI->Enable((proto != nil) && !proto->IsLoggedOn());
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

// Keep titles in sync
void CAdbkManagerTable::ScrollImageBy(SInt32 inLeftDelta, SInt32 inTopDelta, bool inRefresh)
{
	// Find titles in owner chain
	mTableView->GetBaseTitles()->ScrollImageBy(inLeftDelta, 0, inRefresh);

	CHierarchyTableDrag::ScrollImageBy(inLeftDelta, inTopDelta, inRefresh);
}

// Clicked item
void CAdbkManagerTable::LClickCell(const STableCell& inCell, UINT nFlags)
{
	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eAdbkColumnOpen:
	case eAdbkColumnResolve:
	case eAdbkColumnSearch:
	{
		CAdbkList::node_type* node = GetCellNode(inCell.row);

		// Do status flag
		if (node->IsSelectable())
		{
			bool set;
			switch(col_info.column_type)
			{
			case eAdbkColumnOpen:
				set = !node->GetSelectData()->IsOpenOnStart();
				node->GetSelectData()->SetFlags(CAddressBook::eOpenOnStart, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(node->GetSelectData(), set);
		
				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookOpenOnStart(node->GetSelectData(), set);
				break;

			case eAdbkColumnResolve:
				set = !node->GetSelectData()->IsLookup();
				node->GetSelectData()->SetFlags(CAddressBook::eLookup, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(node->GetSelectData(), set);
		
				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookLookup(node->GetSelectData(), set);
				break;

			case eAdbkColumnSearch:
				set = !node->GetSelectData()->IsSearch();
				node->GetSelectData()->SetFlags(CAddressBook::eSearch, set);
				CAddressBookManager::sAddressBookManager->SyncAddressBook(node->GetSelectData(), set);
		
				// Change prefs list
				CPreferences::sPrefs->ChangeAddressBookSearch(node->GetSelectData(), set);
				break;
			}
			
			RefreshRow(inCell.row);
		}
		break;
	}
	default:
		CHierarchyTableDrag::LClickCell(inCell, nFlags);
		DoSingleClick(inCell.row, CKeyModifiers(0));
		break;
	}
}

// Double-clicked item
void CAdbkManagerTable::LDblClickCell(const STableCell& inCell, UINT nFlags)
{
	DoDoubleClick(inCell.row, CKeyModifiers(0));
}

// Handle key down
bool CAdbkManagerTable::HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

	// Did not handle key
	return CHierarchyTableDrag::HandleKeyDown(nChar, nRepCnt, nFlags);
}

void CAdbkManagerTable::OnNewDraft(void)
{
	// New draft with default identity
	CActionManager::NewDraft();
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
void CAdbkManagerTable::DoSingleClick(TableIndexT row, const CKeyModifiers& mods)
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
void CAdbkManagerTable::DoDoubleClick(TableIndexT row, const CKeyModifiers& mods)
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

// Reset the table from the mboxList
void CAdbkManagerTable::ResetTable(void)
{
	// Prevent window update during changes
	StDeferTableAdjustment changing(this);

	// Start cursor for busy operation
	CWaitCursor wait;

	// Delete all existing rows (do not allow redraw)
	Clear();
	mHierarchyCol = 1;
	mData.clear();

	// Only if we have the manager
	if (!mManager)
		return;

	// Get list from manager
	const CAdbkList& adbks = mManager->GetAddressBooks();

	// Add each node
	TableIndexT row = 0;
	TableIndexT exp_row = 1;
	mHasOthers = true;
	ulvector expand_rows;
	for(CAdbkList::node_list::const_iterator iter = adbks.GetChildren()->begin();
		iter != adbks.GetChildren()->end(); iter++)
	{
		// Don't add empty first row
		if ((iter != adbks.GetChildren()->begin()) || (*iter)->CountChildren())
		{
			AddNode(*iter, row, false);

			// Listen to each protocol
			CAdbkProtocol* proto = GetCellAdbkProtocol(exp_row);
			if (proto)
				proto->Add_Listener(this);

			// Check expansion - always expand if pure local address account
			if (!proto || proto->GetAddressAccount()->GetExpanded())
			{
				// Add wide open row to expansion list which gets processed later
				TableIndexT	woRow = GetWideOpenIndex(exp_row + TABLE_ROW_ADJUST);
				expand_rows.push_back(woRow);
			}
			
			// Bump up exposed row counter
			exp_row++;
		}
		else
			mHasOthers = false;
	}

	// Do expansions
	if (expand_rows.size())
	{
		for(ulvector::const_iterator iter = expand_rows.begin(); iter != expand_rows.end(); iter++)
			ExpandRow(*iter);
	}
	
	// Always expand a single account
	else if (exp_row == 2)
		ExpandRow(1);

	// Make sure it listens to changes
	Start_Listening();

}

// Reset the table from the mboxList
void CAdbkManagerTable::ClearTable()
{
	// Delete all existing rows
	Clear();
	mData.clear();

	// Refresh list
	FRAMEWORK_REFRESH_WINDOW(this);
	
	// Force toolbar update
	mTableView->RefreshToolbar();
	
	// Stop listening to broadcast changes
	Stop_Listening();
}

// Remove rows and adjust parts
void CAdbkManagerTable::RemoveRows(UInt32 inHowMany, TableIndexT inFromRow, bool inRefresh)
{
	// Count number to remove (this + descendents)
	UInt32 remove_count = CountAllDescendents(inFromRow) + 1;

	// Do standard removal
	CHierarchyTableDrag::RemoveRows(inHowMany, inFromRow, inRefresh);
	
	// Remove our data using count
	mData.erase(mData.begin() + (inFromRow - 1), mData.begin() + (inFromRow - 1 + remove_count));
}

// Get the selected adbk
void* CAdbkManagerTable::GetCellData(TableIndexT woRow)
{
	return mData.at(woRow - 1);
}

// Reset button & caption state as well
void CAdbkManagerTable::RefreshSelection(void)
{
	// Update buttons then do inherited
	mTableView->RefreshToolbar();

	CHierarchyTableDrag::RefreshSelection();

} // CMailboxInfoTable::RefreshSelection

// Reset button & caption state as well
void CAdbkManagerTable::RefreshRow(TableIndexT row)
{
	// Update buttons then do inherited
	mTableView->RefreshToolbar();

	CHierarchyTableDrag::RefreshRow(row);
}

// Get appropriate icon id
int CAdbkManagerTable::GetPlotIcon(const CAdbkList::node_type* node, CAdbkProtocol* proto)
{
	if (!node->IsSelectable())
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
	else
	{
		const CRemoteAddressBook* adbk = dynamic_cast<const CRemoteAddressBook*>(node->GetSelectData());
		if (adbk && adbk->GetProtocol()->CanDisconnect() && adbk->IsLocalAdbk())
			return adbk->IsCachedAdbk() ? IDI_ADDRESSFILECACHED : IDI_ADDRESSFILEUNCACHED;
		else
			return IDI_ADDRESSFILE;
	}
}

// Get text style
void CAdbkManagerTable::SetTextStyle(CDC* pDC, const CAdbkList::node_type* node, CAdbkProtocol* proto, bool& strike)
{
	strike = false;

	// Select appropriate color and style of text
	//if (UEnvironment::HasFeature(env_SupportsColor))
	{
		bool color_set = false;
		RGBColor text_color;
		bool style_set = false;
		short text_style = normal;

		if (!node->IsSelectable())
		{
			if (!proto || proto->IsLoggedOn())
			{
				text_color = CPreferences::sPrefs->mServerOpenStyle.GetValue().color;
				text_style = CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x007F;
				strike = ((CPreferences::sPrefs->mServerOpenStyle.GetValue().style & 0x0080) != 0);
			}
			else
			{
				text_color = CPreferences::sPrefs->mServerClosedStyle.GetValue().color;
				text_style = CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x007F;
				strike =  ((CPreferences::sPrefs->mServerClosedStyle.GetValue().style & 0x0080) != 0);
			}
			
			color_set = true;
			style_set = true;
		}
		else
		{
			text_color = CPreferences::sPrefs->mMboxClosedStyle.GetValue().color;
			text_style = text_style | CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x007F;
			strike =  strike || ((CPreferences::sPrefs->mMboxClosedStyle.GetValue().style & 0x0080) != 0);
			
			color_set = true;
			style_set = true;
		}

		if (color_set)
			pDC->SetTextColor(text_color);

		// Set appropriate font
		if (style_set)
		{
			switch(text_style)
			{
			case normal:
			default:
				//pDC->SelectObject(CMulberryApp::sAppListFont);
				break;
			case bold:
				pDC->SelectObject(CFontCache::GetListFontBold());
				break;
			case italic:
				pDC->SelectObject(CFontCache::GetListFontItalic());
				break;
			case bold + italic:
				pDC->SelectObject(CFontCache::GetListFontBoldItalic());
				break;
			case underline:
				pDC->SelectObject(CFontCache::GetListFontUnderline());
				break;
			case bold + underline:
				pDC->SelectObject(CFontCache::GetListFontBoldUnderline());
				break;
			case italic + underline:
				pDC->SelectObject(CFontCache::GetListFontItalicUnderline());
				break;
			case bold + italic + underline:
				pDC->SelectObject(CFontCache::GetListFontBoldItalicUnderline());
				break;
			}
		}
	}
}

bool CAdbkManagerTable::UsesBackgroundColor(const CAdbkList::node_type* node) const
{
	return !node->IsSelectable();
}

COLORREF CAdbkManagerTable::GetBackgroundColor(const CAdbkList::node_type* node) const
{
	return CPreferences::sPrefs->mServerBkgndStyle.GetValue().color;
}

// Draw the titles
void CAdbkManagerTable::DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect)
{
	// Not if changing
	if (mListChanging)
		return;

	StDCState		save(pDC);

	// Get cell data item
	int	woRow = GetWideOpenIndex(inCell.row + TABLE_ROW_ADJUST);
	CAdbkList::node_type* node = GetCellNode(inCell.row);
	cdstring theTxt;

	// Erase to ensure drag hightlight is overwritten
	if (UsesBackgroundColor(node))
	{
		pDC->SetBkColor(GetBackgroundColor(node));

	   	// Erase the entire area. Using ExtTextOut is a neat alternative to FillRect and quicker, too!
	   	CRect cellRect = inLocalRect;
	   	cellRect.bottom--;
		pDC->ExtTextOut(cellRect.left, cellRect.top, ETO_OPAQUE, cellRect, _T(""), 0, nil);
	}

	// Determine which heading it is
	SColumnInfo col_info = mTableView->GetColumnInfo()[inCell.col - 1];

	switch(col_info.column_type)
	{
	case eAdbkColumnName:
	{
		// Get suitable icon
		UINT iconID = GetPlotIcon(node, GetCellAdbkProtocol(inCell.row));

		// Determine name to use
		theTxt = node->IsSelectable() ? node->GetSelectData()->GetName() : *node->GetNoSelectData();

		// Draw the string
		bool strike = false;
		SetTextStyle(pDC, node, GetCellAdbkProtocol(inCell.row), strike);
		int text_start = DrawHierarchyRow(pDC, inCell.row, inLocalRect, theTxt, iconID);
		if (strike)
		{
			CPen temp(PS_SOLID, 1, pDC->GetTextColor());
			CPen* old_pen = (CPen*) pDC->SelectObject(&temp);
			pDC->MoveTo(text_start, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->LineTo(inLocalRect.right, (inLocalRect.top + inLocalRect.bottom)/2);
			pDC->SelectObject(old_pen);
		}

		// Always cache column number
		mHierarchyCol = inCell.col;
		break;
	}

	case eAdbkColumnOpen:
		if (node->IsSelectable())
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, node->GetSelectData()->IsOpenOnStart() ? IDI_DIAMONDTICKED : IDI_DIAMOND, 16);
		break;
		
	case eAdbkColumnResolve:
		if (node->IsSelectable())
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, node->GetSelectData()->IsLookup() ? IDI_DIAMONDTICKED : IDI_DIAMOND, 16);
		break;
		
	case eAdbkColumnSearch:
		if (node->IsSelectable())
			CIconLoader::DrawIcon(pDC, inLocalRect.left, inLocalRect.top + mIconOrigin, node->GetSelectData()->IsSearch() ? IDI_DIAMONDTICKED : IDI_DIAMOND, 16);
		break;
		
	}
}

// Draw drag insert cursor
bool CAdbkManagerTable::IsDropCell(COleDataObject* pDataObject, const STableCell& theCell)
{
	return IsValidCell(theCell) && IsCellAdbk(theCell.row);
}

// Drop data into cell
bool CAdbkManagerTable::DropDataIntoCell(unsigned int theFlavor, char* drag_data,
											unsigned long data_size, const STableCell& theCell)
{
	// Init drop params
	bool addressAdded = false;
	bool groupAdded = false;

	// Determine adbk to use for copy
	CAddressBook* aDropAdbk = GetCellAdbk(theCell.row);

	// Does window already exist?
	CAddressBookWindow* aDropAdbkWnd = CAddressBookWindow::FindWindow(aDropAdbk);
	bool aDropSort = (aDropAdbkWnd != nil);

	// Look for closed remote address book
	CRemoteAddressBook* aDropRemoteAdbk = NULL;
	if (!aDropAdbk->IsOpen())
		aDropRemoteAdbk = dynamic_cast<CRemoteAddressBook*>(aDropAdbk);
	if (aDropRemoteAdbk)
		aDropRemoteAdbk->Read();

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
				CAddress* theAddr = theEnv->GetFrom()->front();
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!aDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					aDropAdbk->AddAddress(new CAdbkAddress(*theEnv->GetFrom()->front()), aDropSort);
					addressAdded = true;
				}
			}
			
			// Add Reply-To
			if (theEnv->GetReplyTo()->size())
			{
				CAddress* theAddr = theEnv->GetReplyTo()->front();
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!aDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					aDropAdbk->AddAddress(new CAdbkAddress(*theEnv->GetReplyTo()->front()), aDropSort);
					addressAdded = true;
				}
			}
			
			// Add all To: except me
			for(CAddressList::iterator iter = theEnv->GetTo()->begin(); iter != theEnv->GetTo()->end(); iter++)
			{
				CAddress* theAddr = *iter;
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!aDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					aDropAdbk->AddAddress(new CAdbkAddress(**iter), aDropSort);
					addressAdded = true;
				}
			}

			// Add all Ccs except me
			for(CAddressList::iterator iter = theEnv->GetCC()->begin(); iter != theEnv->GetCC()->end(); iter++)
			{
				CAddress* theAddr = *iter;
				if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
					!aDropAdbk->GetAddressList()->IsDuplicate(theAddr))
				{
					// Add to list
					aDropAdbk->AddAddress(new CAdbkAddress(**iter), aDropSort);
					addressAdded = true;
				}
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

			if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
				!aDropAdbk->GetAddressList()->IsDuplicate(theAddr))
			{
				// Add to list
				aDropAdbk->AddAddress(new CAdbkAddress(*theAddr), aDropSort);
				addressAdded = true;
			}
		}
	}

	else if (theFlavor == CMulberryApp::sFlavorGrpList)
	{
		int count = *((int*) drag_data);
		drag_data += sizeof(int);
		for(int i = 0; i < count; i++)
		{
			CGroup* theGrp = ((CGroup**) drag_data)[i];

			// Add group to new group
			aDropAdbk->AddGroup(new CGroup(*theGrp), aDropSort);
			groupAdded = true;
		}
	}

	else if (theFlavor == CF_UNICODETEXT)
	{
		// Parse text into list
		cdustring utf16(drag_data, data_size);
		cdstring utf8 = utf16.ToUTF8();
		CAddressList list(utf8, utf8.length());
		
		// Try to add these to table
		for(CAddressList::iterator iter = list.begin(); iter != list.end(); iter++)
		{				
			CAddress* theAddr = *iter;
			if (!CPreferences::sPrefs->TestSmartAddress(*theAddr) &&
				!aDropAdbk->GetAddressList()->IsDuplicate(theAddr))
			{
				// Now add to address book
				aDropAdbk->AddAddress(new CAdbkAddress(**iter), aDropSort);
				addressAdded = true;
			}
		}
	}

	// Always force window update
	if (aDropAdbkWnd)
		aDropAdbkWnd->GetAddressBookView()->ResetTable();

	
	// Always reset if remote was opened
	if (aDropRemoteAdbk)
	{
		aDropRemoteAdbk->GetProtocol()->CloseAdbk(aDropRemoteAdbk);
		aDropRemoteAdbk->Clear();
		aDropRemoteAdbk->SetFlags(CAddressBook::eLoaded, false);
		aDropRemoteAdbk = NULL;
	}
	
	return addressAdded || groupAdded;
}
