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


// Source for CAddressBookView class

#include "CAddressBookView.h"

#include "CAdbkIOPlugin.h"
#include "CAdbkProtocol.h"
#include "CAdbkManagerTable.h"
#include "CAddressBook.h"
#include "CAddressBookToolbar.h"
#include "CAddressBookWindow.h"
#include "CAddressTable.h"
#include "CErrorHandler.h"
#include "CFontCache.h"
#include "CGroupTable.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CSDIFrame.h"
#include "CTableViewWindow.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "CUnicodeUtils.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneWindow.h"

const int cWindowWidth = 128;
const int cWindowHeight = 128;
const int cPaneHeight = 24;
const int cTwisterHOffset = 8;
const int cTwisterVOffset = 4;
const int cCaptionHOffset = cTwisterHOffset + 20;
const int cCaptionVOffset = 6;
const int cCaptionWidth = 96;
const int cCaptionHeight = 16;
const int cNewHOffset = cCaptionHOffset + cCaptionWidth + 4;
const int cNewVOffset = 1;
const int cNewWidth = 60;
const int cNewHeight = 22;
const int cDraftHOffset = cNewHOffset + cNewWidth;
const int cDraftVOffset = 1;
const int cDraftWidth = 116;
const int cDraftHeight = 22;

const int cTitleHeight = 16;

BEGIN_MESSAGE_MAP(CAddressBookView, CTableView)
	ON_UPDATE_COMMAND_UI(ID_EDIT_UNDO, OnUpdateEditUndo)
	ON_COMMAND(ID_EDIT_UNDO, OnEditUndo)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_SYNC, OnUpdateDisconnectedSelection)
	ON_COMMAND(IDM_ADDR_SYNC, OnSynchroniseAddressBook)

	ON_UPDATE_COMMAND_UI(IDM_ADDR_CLEAR_DISCONNECT, OnUpdateClearDisconnectedSelection)
	ON_COMMAND(IDM_ADDR_CLEAR_DISCONNECT, OnClearDisconnectAddressBook)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_AddressImportStart, IDM_AddressImportEnd, OnUpdateAddrImport)
	ON_COMMAND_RANGE(IDM_AddressImportStart, IDM_AddressImportEnd, OnAddrImport)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_AddressExportStart, IDM_AddressExportEnd, OnUpdateAddrExport)
	ON_COMMAND_RANGE(IDM_AddressExportStart, IDM_AddressExportEnd, OnAddrExport)

	ON_COMMAND(IDC_ADDRESSTWIST, OnAddressTwist)
	ON_COMMAND(IDC_ADDRESSNEW, OnNewAddress)
	ON_COMMAND(IDC_ADDRESSDRAFT, OnAddressDraft)
	ON_COMMAND(IDC_GROUPTWIST, OnGroupTwist)
	ON_COMMAND(IDC_GROUPNEW, OnNewGroup)
	ON_COMMAND(IDC_GROUPDRAFT, OnGroupDraft)

	ON_COMMAND(IDC_ADDRESS_NEW_SINGLE, OnNewAddress)
	ON_COMMAND(IDC_ADDRESS_NEW_GROUP, OnNewGroup)

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	
	// Toolbar
	ON_COMMAND(IDC_TOOLBARADDRESSBOOKADDRESSBTN, OnNewAddress)

	ON_COMMAND(IDC_TOOLBARADDRESSBOOKGROUPBTN, OnNewGroup)

END_MESSAGE_MAP()

// Static members

cdmutexprotect<CAddressBookView::CAddressBookViewList> CAddressBookView::sAddressBookViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressBookView::CAddressBookView()
{
	mAdbk = NULL;
	mAddressView = NULL;
	mUsePreview = true;

	mSplitPos = 0x10000000 / 2; // Splitter ratio of 0.5;
	mSortColumnType = eAddrColumnName;
	mAscending = true;

	// Add to list
	{
		cdmutexprotect<CAddressBookViewList>::lock _lock(sAddressBookViews);
		sAddressBookViews->push_back(this);
	}
}

// Default destructor
CAddressBookView::~CAddressBookView()
{
	// Remove from list
	cdmutexprotect<CAddressBookViewList>::lock _lock(sAddressBookViews);
	CAddressBookViewList::iterator found = std::find(sAddressBookViews->begin(), sAddressBookViews->end(), this);
	if (found != sAddressBookViews->end())
		sAddressBookViews->erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int CAddressBookView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetViewInfo(eAddrColumnName, 80, false);
	if (CTableView::OnCreate(lpCreateStruct) == -1)
		return -1;

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create footer first so we get its scaled height
	mFooter.CreateDialogItems(IDD_ADDRESSBOOKFOOTER, this);
	mFooter.ModifyStyle(0, WS_CLIPSIBLINGS);
	CRect ftrrect;
	mFooter.GetWindowRect(ftrrect);
	mFooter.ExecuteDlgInit(MAKEINTRESOURCE(IDD_ADDRESSBOOKFOOTER));
	mFooter.SetFont(CMulberryApp::sAppSmallFont);
	mFooter.MoveWindow(CRect(0, height - ftrrect.Height(), width, height));
	mFooter.ShowWindow(SW_SHOW);

	// Subclass footer controls for our use
	mFooter.GetDlgItem(IDC_ADDRESSESNUMTITLE)->SetFont(CMulberryApp::sAppSmallFont);
	mAddressTotal.SubclassDlgItem(IDC_ADDRESSESNUM, &mFooter);
	mAddressTotal.SetFont(CMulberryApp::sAppSmallFont);
	mFooter.GetDlgItem(IDC_GROUPSNUMTITLE)->SetFont(CMulberryApp::sAppSmallFont);
	mGroupTotal.SubclassDlgItem(IDC_GROUPSNUM, &mFooter);
	mGroupTotal.SetFont(CMulberryApp::sAppSmallFont);

	// Create splitter view
	CRect rect(0, 0, width, height - ftrrect.Height());
	mSplitter.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Address pane
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mAddressPane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	rect = CRect(cTwisterHOffset, cTwisterVOffset, cTwisterHOffset + 16, cTwisterVOffset + 16);
	mAddressTwist.Create(rect, &mAddressPane, IDC_ADDRESSTWIST);
	mAddressTwist.SetPushed(true);

	CString s;
	s.LoadString(IDS_SHOWADDRESSES);
	rect = CRect(cCaptionHOffset, cCaptionVOffset, cCaptionHOffset + cCaptionWidth, cCaptionVOffset + cCaptionHeight);
	mShowAddresses.Create(s, WS_CHILD, rect, &mAddressPane, IDC_STATIC);
	mShowAddresses.SetFont(CMulberryApp::sAppFontBold);

	s.LoadString(IDS_HIDEADDRESSES);
	rect = CRect(cCaptionHOffset, cCaptionVOffset, cCaptionHOffset + cCaptionWidth, cCaptionVOffset + cCaptionHeight);
	mHideAddresses.Create(s, WS_CHILD | WS_VISIBLE, rect, &mAddressPane, IDC_STATIC);
	mHideAddresses.SetFont(CMulberryApp::sAppFontBold);

	//s.LoadString(IDS_HIDEADDRESSES);
	rect = CRect(cNewHOffset, cNewVOffset, cNewHOffset + cNewWidth, cNewVOffset + cNewHeight);
	s.LoadString(IDS_ADDADDRESS);
	mAddressNew.Create(s, rect, &mAddressPane, IDC_ADDRESSNEW, IDI_ADDRESS_NEW_SINGLE);
	mAddressNew.SetFont(CMulberryApp::sAppFont);

	rect = CRect(cDraftHOffset, cDraftVOffset, cDraftHOffset + cDraftWidth + 2 * small_offset, cDraftVOffset + cDraftHeight);
	s.LoadString(IDS_NEWMESSAGEADDRESS);
	mAddressDraft.Create(s, rect, &mAddressPane, IDC_ADDRESSDRAFT, IDI_MAILBOXNEWMSG);
	mAddressDraft.SetFont(CMulberryApp::sAppFont);

	// Addresses focus ring
	mAddressesFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, cPaneHeight, cWindowWidth, cWindowHeight), &mAddressPane, IDC_STATIC);
	mAddressPane.AddAlignment(mAddressesAlign = new CWndAlignment(&mAddressesFocusRing, CWndAlignment::eAlign_WidthHeight));
	mAddressesFocusRing.SetFocusBorder();
	UINT focus_indent = 3;

	// Address table
	mAddresses.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent + cTitleHeight, cWindowWidth - focus_indent, cWindowHeight - cPaneHeight - focus_indent), &mAddressesFocusRing, IDC_STATIC);
	mAddressesFocusRing.AddAlignment(new CWndAlignment(&mAddresses, CWndAlignment::eAlign_WidthHeight));
	mAddresses.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mAddresses.ResetFont(CFontCache::GetListFont());
	mAddresses.SetContextMenuID(IDR_POPUP_CONTEXT_ADDR_TABLE);
	mAddresses.SetContextView(static_cast<CView*>(GetOwningWindow()));
	mAddresses.SetSuperCommander(this);

	// Get titles
	mAddressTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent, cWindowWidth - focus_indent, focus_indent + cTitleHeight), &mAddressesFocusRing, IDC_STATIC);
	mAddressesFocusRing.AddAlignment(new CWndAlignment(&mAddressTitles, CWndAlignment::eAlign_TopWidth));
	mAddressTitles.ResetFont(CFontCache::GetListFont());

	// Group  pane
	rect = CRect(0, 0, cWindowWidth, cWindowHeight);
	mGroupPane.Create(NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	rect = CRect(cTwisterHOffset, cTwisterVOffset, cTwisterHOffset + 16, cTwisterVOffset + 16);
	mGroupTwist.Create(rect, &mGroupPane, IDC_GROUPTWIST);
	mGroupTwist.SetPushed(true);

	s.LoadString(IDS_SHOWGROUPS);
	rect = CRect(cCaptionHOffset, cCaptionVOffset, cCaptionHOffset + cCaptionWidth, cCaptionVOffset + cCaptionHeight);
	mShowGroups.Create(s, WS_CHILD, rect, &mGroupPane, IDC_STATIC);
	mShowGroups.SetFont(CMulberryApp::sAppFontBold);

	s.LoadString(IDS_HIDEGROUPS);
	rect = CRect(cCaptionHOffset, cCaptionVOffset, cCaptionHOffset + cCaptionWidth, cCaptionVOffset + cCaptionHeight);
	mHideGroups.Create(s, WS_CHILD | WS_VISIBLE, rect, &mGroupPane, IDC_STATIC);
	mHideGroups.SetFont(CMulberryApp::sAppFontBold);

	rect = CRect(cNewHOffset, cNewVOffset, cNewHOffset + cNewWidth, cNewVOffset + cNewHeight);
	s.LoadString(IDS_ADDADDRESS);
	mGroupNew.Create(s, rect, &mGroupPane, IDC_GROUPNEW, IDI_ADDRESS_NEW_GROUP);
	mGroupNew.SetFont(CMulberryApp::sAppFont);

	rect = CRect(cDraftHOffset, cDraftVOffset, cDraftHOffset + cDraftWidth + 2 * small_offset, cDraftVOffset + cDraftHeight);
	s.LoadString(IDS_NEWMESSAGEADDRESS);
	mGroupDraft.Create(s, rect, &mGroupPane, IDC_GROUPDRAFT, IDI_MAILBOXNEWMSG);
	mGroupDraft.SetFont(CMulberryApp::sAppFont);

	// Group focus ring
	mGroupFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, cPaneHeight, cWindowWidth, cWindowHeight), &mGroupPane, IDC_STATIC);
	mGroupPane.AddAlignment(mGroupsAlign = new CWndAlignment(&mGroupFocusRing, CWndAlignment::eAlign_WidthHeight));
	mGroupFocusRing.SetFocusBorder();

	// Group table
	mGroups.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
							CRect(focus_indent, focus_indent, cWindowWidth - focus_indent, cWindowHeight - cPaneHeight - focus_indent), &mGroupFocusRing, IDC_STATIC);
	mGroupFocusRing.AddAlignment(new CWndAlignment(&mGroups, CWndAlignment::eAlign_WidthHeight));
	mGroups.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mGroups.ResetFont(CFontCache::GetListFont());
	mGroups.SetContextMenuID(IDR_POPUP_CONTEXT_GRP_TABLE);
	mGroups.SetContextView(static_cast<CView*>(GetOwningWindow()));
	mGroups.Add_Listener(this);
	mGroups.SetSuperCommander(this);

	PostCreate(&mAddresses, &mAddressTitles);

	// Install the splitter items
	mSplitter.InstallViews(&mAddressPane, &mGroupPane, true);
	mSplitter.SetMinima(64, 64);
	mSplitter.SetVisibleLock(true);

	// Create alignment details
	// Must have minimum height to prevent status pane dissappearing when size if zeroed
	MinimumResize(32, 64);
	AddAlignment(new CWndAlignment(&mFooter, CWndAlignment::eAlign_BottomWidth));
	AddAlignment(new CWndAlignment(&mSplitter, CWndAlignment::eAlign_WidthHeight));

	// Set status
	SetOpen();
	UpdateCaptions();

	return 0;
}

void CAddressBookView::OnDestroy(void)
{
	// Do standard close behaviour
	DoClose();

	// Do default action now
	CTableView::OnDestroy();
}

void CAddressBookView::OnSetFocus(CWnd* pOldWnd)
{
	// Set focus based on table state
	if (mAddressTwist.IsPushed())
		GetAddressTable()->SetFocus();
	else
		GetGroupTable()->SetFocus();
}

void CAddressBookView::OnUpdateAlways(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);	// Always
}

void CAddressBookView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	mUndoer.FindUndoStatus(pCmdUI);
}

void CAddressBookView::OnUpdateDisconnectedSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((mAdbk != NULL) &&
					mAdbk->GetProtocol()->CanDisconnect() &&
					!mAdbk->GetProtocol()->IsDisconnected());
}

void CAddressBookView::OnUpdateClearDisconnectedSelection(CCmdUI* pCmdUI)
{
	pCmdUI->Enable((mAdbk != NULL) &&
					mAdbk->GetProtocol()->CanDisconnect() &&
					mAdbk->GetProtocol()->IsDisconnected());
}

void CAddressBookView::OnUpdateAddrImport(CCmdUI* pCmdUI)
{
	// Eventually this should check read-only
	pCmdUI->Enable(true);
}

void CAddressBookView::OnUpdateAddrExport(CCmdUI* pCmdUI)
{
	// Always allow export
	pCmdUI->Enable(true);
}

void CAddressBookView::OnEditUndo(void)
{
	mUndoer.ToggleAction();
}

// New address
void CAddressBookView::OnNewAddress()
{
	// Expand to make addresses visible
	if (!mAddressTwist.IsPushed())
		DoAddressTwist();

	// Create a new address
	GetAddressTable()->OnNewAddress();
}

// Address draft
void CAddressBookView::OnAddressDraft()
{
	// Create a new draft
	GetAddressTable()->CreateNewLetter(false);
}

// New group
void CAddressBookView::OnNewGroup()
{
	// Expand to make addresses visible
	if (!mGroupTwist.IsPushed())
		DoGroupTwist();

	// Create a new address
	GetGroupTable()->OnNewGroup();
}

// Group draft
void CAddressBookView::OnGroupDraft()
{
	// Create a new draft
	GetGroupTable()->CreateNewLetter(false);
}

void CAddressBookView::OnAddrImport(UINT nID)
{
	// First get menu name
	CMenu* menu = AfxGetApp()->m_pMainWnd->GetMenu();
	CMenu* addr_menu = menu->GetSubMenu(4);
	CMenu* import_menu = NULL;
	for(int i = 0; i < addr_menu->GetMenuItemCount(); i++)
	{
		if (addr_menu->GetMenuItemID(i) == IDM_ADDR_RENAME)
		{
			import_menu = addr_menu->GetSubMenu(i + 2);
			break;
		}
	}
	if (!import_menu)
		return;
	cdstring name = CUnicodeUtils::GetMenuStringUTF8(import_menu, nID, MF_BYCOMMAND);
	
	// Now get plugin from manager
	CAdbkIOPlugin* plugin = CPluginManager::sPluginManager.FindAdbkIOPlugin(name);
	if (!plugin)
		return;
		
	// Do standard open dialog
	// prompt the user
	CString filter = cdustring(plugin->GetFileType());
	CFileDialog dlgFile(true, NULL, NULL, OFN_FILEMUSTEXIST, filter, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		CAddressList* addrs = new CAddressList;
		CGroupList* grps = new CGroupList;

		// Do import
		cdstring fpath(dlgFile.GetPathName());
		if (plugin->DoImport(fpath, *addrs, *grps))
		{
			// Add results to adbk
			AddAddressList(addrs);
			AddGroupList(grps);
		}
	}
}

void CAddressBookView::OnAddrExport(UINT nID)
{
	// First get menu name
	CMenu* menu = AfxGetApp()->m_pMainWnd->GetMenu();
	CMenu* addr_menu = menu->GetSubMenu(4);
	CMenu* export_menu = NULL;
	for(int i = 0; i < addr_menu->GetMenuItemCount(); i++)
	{
		if (addr_menu->GetMenuItemID(i) == IDM_ADDR_RENAME)
		{
			export_menu = addr_menu->GetSubMenu(i + 3);
			break;
		}
	}
	if (!export_menu)
		return;
	cdstring name = CUnicodeUtils::GetMenuStringUTF8(export_menu, nID, MF_BYCOMMAND);
	
	// Now get plugin from manager
	CAdbkIOPlugin* plugin = CPluginManager::sPluginManager.FindAdbkIOPlugin(name);
	if (!plugin)
		return;
	
	// Do standard open dialog
	// prompt the user
	CFileDialog dlgFile(false, _T("txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, NULL, CSDIFrame::GetAppTopWindow());

	// Check for good reply
	if (dlgFile.DoModal() == IDOK)
	{
		try
		{
			// Create file object
			CFile export_file(dlgFile.GetPathName(), CFile::modeCreate | CFile::modeReadWrite | CFile::shareExclusive);
			export_file.Close();
			
			cdstring fpath(dlgFile.GetPathName());
			if (plugin->DoExport(fpath, *mAdbk->GetAddressList(), *mAdbk->GetGroupList()) == 0)
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
		}
		catch (...)
		{
			CLOG_LOGCATCH(...);

			// Inform user
			CErrorHandler::PutStopAlertRsrc("Alerts::Adbk::ExportError");

			// Throw up
			CLOG_LOGRETHROW;
			throw;
		}
	}
}

void CAddressBookView::OnAddressTwist()
{
	DoAddressTwist();
}

void CAddressBookView::OnGroupTwist()
{
	DoGroupTwist();
}

void CAddressBookView::DoAddressTwist()
{
	bool expanding = !mAddressTwist.IsPushed();

	CRect table_size;
	mAddressesFocusRing.GetWindowRect(table_size);
	int moveby = table_size.Height();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts before collapsing
		mAddressesAlign->SetAlignment(true, true, true, false);

		// Switch over captions
		mHideAddresses.ShowWindow(SW_HIDE);
		mShowAddresses.ShowWindow(SW_SHOW);
		
		// Get current split pos
		mSplitPos = mSplitter.GetRelativeSplitPos();

		// Collapse view and lock
		mSplitter.SetMinima(cPaneHeight, 64);
		mSplitter.SetPixelSplitPos(mSplitter.GetPixelSplitPos() - moveby);
		mSplitter.SetLocks(true, false);
		
		// Show/hide items
		mAddressesFocusRing.ShowWindow(SW_HIDE);
		mAddressTwist.SetPushed(false);

		// Only do this when in 1-pane mode
		mGroups.SetFocus();

		// Disable group twist
		mGroupTwist.EnableWindow(false);
	}
	else
	{
		// Switch over captions
		mShowAddresses.ShowWindow(SW_HIDE);
		mHideAddresses.ShowWindow(SW_SHOW);
		
		// Expand view and unlock
		mSplitter.SetRelativeSplitPos(mSplitPos);
		mSplitter.SetLocks(false, false);
		
		// Show/hide items
		mAddressesFocusRing.ShowWindow(SW_SHOW);
		mAddressTwist.SetPushed(true);

		// Get size and location of parent panel after splitter resize
		CRect parent_size;
		mAddressesFocusRing.GetParent()->GetWindowRect(parent_size);

		// Resize address panel to current parent size less top offset
		::ResizeWindowBy(&mAddressesFocusRing, 0, parent_size.Height() - cPaneHeight - table_size.Height(), false);

		// Turn on resize of parts after expanding
		mAddressesAlign->SetAlignment(true, true, true, true);

		mSplitter.SetMinima(64, 64);

		// Only do this when in 1-pane mode
		mAddresses.SetFocus();

		// Enable group twist
		mGroupTwist.EnableWindow(true);
	}

	mAddressDraft.EnableWindow(expanding);
}

void CAddressBookView::DoGroupTwist()
{
	bool expanding = !mGroupTwist.IsPushed();

	CRect table_size;
	mGroupFocusRing.GetWindowRect(table_size);
	int moveby = table_size.Height();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of parts before collapsing
		mGroupsAlign->SetAlignment(true, true, true, false);

		// Switch over captions
		mHideGroups.ShowWindow(SW_HIDE);
		mShowGroups.ShowWindow(SW_SHOW);
		
		// Get current split pos
		mSplitPos = mSplitter.GetRelativeSplitPos();

		// Collapse view and lock
		mSplitter.SetMinima(64, cPaneHeight);
		mSplitter.SetPixelSplitPos(mSplitter.GetPixelSplitPos() + moveby);
		mSplitter.SetLocks(false, true);
		
		// Show/hide items
		mGroupFocusRing.ShowWindow(SW_HIDE);
		mGroupTwist.SetPushed(false);

		// Only do this when in 1-pane mode
		mAddresses.SetFocus();

		// Disable address twist
		mAddressTwist.EnableWindow(false);
	}
	else
	{
		// Switch over captions
		mShowGroups.ShowWindow(SW_HIDE);
		mHideGroups.ShowWindow(SW_SHOW);
		
		// Expand view and unlock
		mSplitter.SetRelativeSplitPos(mSplitPos);
		mSplitter.SetLocks(false, false);
		
		// Show/hide items
		mGroupFocusRing.ShowWindow(SW_SHOW);
		mGroupTwist.SetPushed(true);

		// Get size and location of parent panel after splitter resize
		CRect parent_size;
		mGroupFocusRing.GetParent()->GetWindowRect(parent_size);

		// Resize address panel to current parent size less top offset
		::ResizeWindowBy(&mGroupFocusRing, 0, parent_size.Height() - cPaneHeight - table_size.Height(), false);

		// Turn on resize of parts after expanding
		mGroupsAlign->SetAlignment(true, true, true, true);

		mSplitter.SetMinima(64, 64);

		// Only do this when in 1-pane mode
		mGroups.SetFocus();

		// Enable group twist
		mAddressTwist.EnableWindow(true);
	}

	mGroupDraft.EnableWindow(expanding);
}

// Add Address list
void CAddressBookView::AddAddressList(CAddressList* list)
{
	// Add unique addresses only - remove them from the list
	mAdbk->AddUniqueAddresses(*list);
	mAdbk->GetAddressList()->Sort();

	ResetTable();
}

// Add Group list
void CAddressBookView::AddGroupList(CGroupList* list)
{
	// Add unique groups only - remove them from the list
	mAdbk->AddUniqueGroups(*list);
	mAdbk->GetGroupList()->Sort();

	ResetTable();
}

// Make a toolbar appropriate for this view
void CAddressBookView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		mToolbar = C3PaneWindow::s3PaneWindow->GetAdbkToolbar();
		Add_Listener(mToolbar);
	}
	else
	{
		// Create a suitable toolbar
		mToolbar = new CAddressBookToolbar;
		mToolbar->InitToolbar(Is3Pane(), parent);

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(mToolbar);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(mToolbar, NULL, CToolbarView::eStdButtonsGroup);
	}
}

// Init columns and text
void CAddressBookView::InitColumns(void)
{
	SColumnInfo newInfo;

	// Remove any previous columns
	mColumnInfo.clear();

	// Add default columns
	mTitles->InsertCols(1, 1);
	mTable->InsertCols(1, 1);
	GetGroupTable()->InsertCols(1, 1);

	// Name, width = 240
	newInfo.column_type = eAddrColumnName;
	newInfo.column_width = 240;
	mColumnInfo.push_back(newInfo);
	mTitles->SetColWidth(newInfo.column_width, 1, 1);
	mTable->SetColWidth(newInfo.column_width, 1, 1);
}

// Update captions
void CAddressBookView::UpdateCaptions(void)
{
	cdstring txt;
	if (GetAddressBook())
		txt = (long)GetAddressBook()->GetAddressList()->size();
	else
		txt = "--";
	CUnicodeUtils::SetWindowTextUTF8(mAddressTotal, txt);

	if (GetAddressBook())
		txt = (long)GetAddressBook()->GetGroupList()->size();
	else
		txt = "--";
	CUnicodeUtils::SetWindowTextUTF8(mGroupTotal, txt);

	bool enable = (GetAddressBook() != NULL);
	mAddressNew.EnableWindow(enable);
	mAddressDraft.EnableWindow(enable);
	mGroupNew.EnableWindow(enable);
	mGroupDraft.EnableWindow(enable);
}

// Reset state from prefs
void CAddressBookView::ResetState(bool force)
{
	if (!GetAddressBook())
		return;

	// Get name as cstr (strip trailing suffix)
	cdstring name = GetAddressBook()->GetURL();

	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Check for available state
	CNewAddressBookWindowState* state = CPreferences::sPrefs->GetAddressBookWindowInfo(name);

	// If no prefs try default
	if (!state || force)
		state = &CPreferences::sPrefs->mNewAddressBookWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mNewAddressBookWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, NULL);

			// Reset bounds
			GetParentFrame()->SetWindowPos(NULL, set_rect.left, set_rect.top,
											set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
		}
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mNewAddressBookWindowDefault.GetValue()));
	
	// Set sort trait
	mSortColumnType = MapSortToColumn(state->GetSort());
	SetSortBy(GetSortBy());

	// Expand both
	if (state->GetAddressShow() && state->GetGroupShow())
	{
		// Make sure both are expanded
		if (!mAddressTwist.IsPushed())
			OnAddressTwist();
		if (!mGroupTwist.IsPushed())
			OnGroupTwist();
		
		// Now set splitter
		mSplitter.SetRelativeSplitPos(state->GetAddressSize());
	}
	
	// Expand address only
	else if (state->GetAddressShow())
	{
		// Make sure both are expanded (expand address first)
		if (!mAddressTwist.IsPushed())
			OnAddressTwist();
		if (mGroupTwist.IsPushed())
			OnGroupTwist();
		
		// Now set splitter pos for collapsed state
		mSplitPos = state->GetAddressSize();
	}

	// Expand group only
	else
	{
		// Make sure both are expanded (expand group first)
		if (!mGroupTwist.IsPushed())
			OnGroupTwist();
		if (mAddressTwist.IsPushed())
			OnAddressTwist();
		
		// Now set splitter pos for collapsed state
		mSplitPos = state->GetAddressSize();
	}

	if (force)
		SaveState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Do zoom
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetAddressBookWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	if (!force && !Is3Pane())
	{
		if (!GetParentFrame()->IsWindowVisible())
		{
			GetParentFrame()->ActivateFrame();
			SetFocus();
		}
		GetParentFrame()->RedrawWindow();
	}
	else
		GetParentFrame()->RedrawWindow();
}

// Save current state in prefs
void CAddressBookView::SaveState(void)
{
	if (!GetAddressBook())
		return;

	// Get name as cstr (strip trailing suffix)
	cdstring name = GetAddressBook()->GetURL();

	// Get bounds
	CRect bounds(0, 0, 0, 0);
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = !Is3Pane() && (wp.showCmd == SW_SHOWMAXIMIZED);
	if (!Is3Pane())
		bounds = wp.rcNormalPosition;

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetAddressTable()->GetColWidth(i + 1);

	// Sync size
	long address_size = 0;
	if (mAddressTwist.IsPushed() && mGroupTwist.IsPushed())
		address_size = mSplitter.GetRelativeSplitPos();
	else
		address_size = mSplitPos;

	// Add info to prefs
	CNewAddressBookWindowState* info = new CNewAddressBookWindowState(name,
																		&bounds,
																		zoomed ? eWindowStateMax : eWindowStateNormal,
																		&mColumnInfo,
																		MapColumnToSort(mSortColumnType),
																		address_size,
																		0,
																		mAddressTwist.IsPushed(),
																		mGroupTwist.IsPushed(),
																		Is3Pane() ? 0 : GetAddressBookWindow()->GetSplitter()->GetRelativeSplitPos());

	if (info)
		CPreferences::sPrefs->AddAddressBookWindowInfo(info);
}

// Save current state in prefs
void CAddressBookView::SaveDefaultState(void)
{
	if (!GetAddressBook())
		return;

	// Get bounds
	CRect bounds(0, 0, 0, 0);
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	if (!Is3Pane())
		bounds = wp.rcNormalPosition;

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetAddressTable()->GetColWidth(i + 1);

	// Sync size
	long address_size = 0;
	if (mAddressTwist.IsPushed() && mGroupTwist.IsPushed())
		address_size = mSplitter.GetRelativeSplitPos();
	else
		address_size = mSplitPos;

	// Add info to prefs
	CNewAddressBookWindowState state(NULL,
										&bounds,
										zoomed ? eWindowStateMax : eWindowStateNormal,
										&mColumnInfo,
										MapColumnToSort(mSortColumnType),
										address_size,
										0,
										mAddressTwist.IsPushed(),
										mGroupTwist.IsPushed(),
										Is3Pane() ? 0 : GetAddressBookWindow()->GetSplitter()->GetRelativeSplitPos());

	if (CPreferences::sPrefs->mNewAddressBookWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mNewAddressBookWindowDefault.SetDirty();
}

// Reset text traits from prefs
void CAddressBookView::ResetFont(CFont* font)
{
	GetAddressTable()->ResetFont(font);
	GetGroupTable()->ResetFont(font);
	
	// Force titles to refresh as it does not get SetRowHeight call as table does
	mAddressTitles.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	
	// May need to manually adjust titles and position of table
	UInt16 old_height = mAddressTitles.GetRowHeight(1);
	mAddressTitles.ResetFont(font);
	SInt16 delta = mAddressTitles.GetRowHeight(1) - old_height;

	if (delta)
	{
		// Resize titles
		::ResizeWindowBy(&mAddressTitles, 0, delta, false);
		
		// Resize and move table
		::ResizeWindowBy(GetAddressTable(), 0, -delta, false);
		::MoveWindowBy(GetAddressTable(), 0, delta, false);
		
		GetAddressTable()->RedrawWindow();
		mAddressTitles.RedrawWindow();
	}
}
