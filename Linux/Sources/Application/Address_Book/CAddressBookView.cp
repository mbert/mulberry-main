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


// Source for CAddressBookView class

#include "CAddressBookView.h"

#include "CAdbkIOPlugin.h"
#include "CAdbkProtocol.h"
#include "CAdbkManagerTable.h"
#include "CAddressBook.h"
#include "CAddressBookToolbar.h"
#include "CAddressBookTitleTable.h"
#include "CAddressBookWindow.h"
#include "CAddressTable.h"
#include "CCommands.h"
#include "CErrorHandler.h"
#include "CFocusBorder.h"
#include "CGroupTable.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CTableScrollbarSet.h"
#include "CSplitterView.h"
#include "CTableScrollbarSet.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "CTwister.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneWindow.h"

#include "cdfstream.h"

#include <JXChooseSaveFile.h>
#include <JXColormap.h>
#include <JXFlatRect.h>
#include "JXIconTextPushButton.h"
#include <JXStaticText.h>
#include <JXTextMenu.h>
#include <JXWindow.h>
#include <jFileUtil.h>
#include <jXGlobals.h>

#include <algorithm>
#include <cassert>

// Static members

cdmutexprotect<CAddressBookView::CAddressBookViewList> CAddressBookView::sAddressBookViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAddressBookView::CAddressBookView(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CTableView(enclosure, hSizing, vSizing, x, y, w, h)
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

const int cWindowWidth = 300;
const int cWindowHeight = 320;
const int cTableHeight = 150;
const int cTwistCaptionHeight = 25;
const int cMinTableHeight = 100;

const int cTitleHeight = 16;

// Setup help balloons
void CAddressBookView::OnCreate()
{
	// Do inherited
	SetWindowInfo(eAddrColumnName, 80, false);
	CTableView::OnCreate();

	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cTableHeight);
	heights.InsertElementAtIndex(2, cTableHeight);
	minHeights.InsertElementAtIndex(1, cMinTableHeight);
	minHeights.InsertElementAtIndex(2, cMinTableHeight);
	
// begin JXLayout1

    mPartition =
        new CSplitterView(heights, 1, minHeights, this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,180);
    assert( mPartition != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Addresses:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 0,185, 60,15);
    assert( obj1 != NULL );
    obj1->SetFontSize(10);

    mAddrNumber =
        new JXStaticText("text", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,185, 60,15);
    assert( mAddrNumber != NULL );
    mAddrNumber->SetFontSize(10);

    JXStaticText* obj2 =
        new JXStaticText("Groups:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 130,185, 50,15);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);

    mGroupNumber =
        new JXStaticText("text", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 180,185, 60,15);
    assert( mGroupNumber != NULL );
    mGroupNumber->SetFontSize(10);

// end JXLayout1

// begin JXLayout2

    mAddressPane =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 350,150);
    assert( mAddressPane != NULL );

    mAddrTwister =
        new CTwister(mAddressPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,0, 16,16);
    assert( mAddrTwister != NULL );

    mShowAddrTitle =
        new JXStaticText("Show Addresses", mAddressPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 26,1, 105,16);
    assert( mShowAddrTitle != NULL );
    const JFontStyle mShowAddrTitle_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mShowAddrTitle->SetFontStyle(mShowAddrTitle_style);

    mHideAddrTitle =
        new JXStaticText("Hide Addresses", mAddressPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,1, 105,16);
    assert( mHideAddrTitle != NULL );
    const JFontStyle mHideAddrTitle_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mHideAddrTitle->SetFontStyle(mHideAddrTitle_style);

    mAddressNew =
        new JXIconTextPushButton("Add", mAddressPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 134,0, 80,25);
    assert( mAddressNew != NULL );

    mAddressDraft =
        new JXIconTextPushButton("New Draft to", mAddressPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 214,0, 110,25);
    assert( mAddressDraft != NULL );

    mAddresses =
        new CTableScrollbarSet( mAddressPane,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,25, 344,120);
    assert( mAddresses != NULL );

// end JXLayout2
// begin JXLayout3

    mGroupPane =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 350,150);
    assert( mGroupPane != NULL );

    mGroupTwister =
        new CTwister(mGroupPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,0, 16,16);
    assert( mGroupTwister != NULL );

    mShowGroupTitle =
        new JXStaticText("Show Groups", mGroupPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 26,1, 105,16);
    assert( mShowGroupTitle != NULL );
    const JFontStyle mShowGroupTitle_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mShowGroupTitle->SetFontStyle(mShowGroupTitle_style);

    mHideGroupTitle =
        new JXStaticText("Hide Groups", mGroupPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 25,1, 105,16);
    assert( mHideGroupTitle != NULL );
    const JFontStyle mHideGroupTitle_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    mHideGroupTitle->SetFontStyle(mHideGroupTitle_style);

    mGroupNew =
        new JXIconTextPushButton("Add", mGroupPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 134,0, 80,25);
    assert( mGroupNew != NULL );

    mGroupDraft =
        new JXIconTextPushButton("New Draft to", mGroupPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 214,0, 110,25);
    assert( mGroupDraft != NULL );

    mGroups =
        new CTableScrollbarSet( mGroupPane,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,26, 344,120);
    assert( mGroups != NULL );

// end JXLayout3

	mAddrTwister->SetState(kTrue);
	mShowAddrTitle->Hide();
	mGroupTwister->SetState(kTrue);
	mShowGroupTitle->Hide();

	mAddressNew->SetImage(IDI_ADDRESS_NEW_SINGLE);
	mAddressDraft->SetImage(IDI_MAILBOXNEWMSG);
	mGroupNew->SetImage(IDI_ADDRESS_NEW_GROUP);
	mGroupDraft->SetImage(IDI_MAILBOXNEWMSG);

	// Create address table and it's titles inside scrollbar set
	mAddressTable = new CAddressTable(mAddresses,
										mAddresses->GetScrollEnclosure(),
														JXWidget::kHElastic,
														JXWidget::kVElastic,
														0,cTitleHeight, 105, 100);
	CAddressBookTitleTable* titles = new CAddressBookTitleTable(mAddresses, mAddresses->GetScrollEnclosure(),
																	JXWidget::kHElastic,
																	JXWidget::kFixedTop,
																	0, 0, 105, cTitleHeight);

	mAddressTable->OnCreate();
	titles->OnCreate();
	mAddressTable->SetSuperCommander(this);

	// Create group table
	mGroupTable = new CGroupTable(mGroups, mGroups->GetScrollEnclosure(),
									JXWidget::kHElastic, JXWidget::kVElastic,
									0,0, 105, 100);

	mGroupTable->OnCreate();
	mGroupTable->Add_Listener(this);
	mGroupTable->SetSuperCommander(this);

	// Install the splitter items
	mPartition->InstallViews(mAddressPane, mGroupPane, true);
	mPartition->SetMinima(64, 64);
	mPartition->SetVisibleLock(true);

	// Set status
	PostCreate(mAddressTable, titles);
	SetOpen();
	UpdateCaptions();

	ListenTo(mAddrTwister);
	ListenTo(mAddressNew);
	ListenTo(mAddressDraft);
	ListenTo(mGroupTwister);
	ListenTo(mGroupNew);
	ListenTo(mGroupDraft);
}

//	Respond to commands
bool CAddressBookView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	bool	cmdHandled = true;

	switch (cmd)
	{
	case CCommand::eAddressNew:
	case CCommand::eToolbarAddressBookAddressBtn:
		OnNewAddress();
		return true;

	case CCommand::eToolbarAddressBookGroupBtn:
		OnNewGroup();
		return true;

	case CCommand::eAddressesSynchronise:
		OnSynchroniseAddressBook();
		return true;

	case CCommand::eAddressesClearDisconnected:
		OnClearDisconnectAddressBook();
		return true;

	default:;
	}

	// Special check for import/export
	if (menu)
	{
		CWindow* wnd = dynamic_cast<CWindow*>(GetOwningWindow());
		CFileDocument* adbk_wnd = dynamic_cast<CFileDocument*>(GetOwningWindow());
		JXTextMenu* import_menu = (wnd ? wnd->GetMenus()[CMainMenu::eImportAddresses] :
									(adbk_wnd ? adbk_wnd->GetMenus()[CMainMenu::eImportAddresses] : NULL));
		JXTextMenu* export_menu = (wnd ? wnd->GetMenus()[CMainMenu::eExportAddresses] :
									(adbk_wnd ? adbk_wnd->GetMenus()[CMainMenu::eExportAddresses] : NULL));
		if (menu->mMenu == import_menu)
		{
			OnAddrImport(menu->mMenu, menu->mIndex);
			return true;
		}
		else if (menu->mMenu == export_menu)
		{
			OnAddrExport(menu->mMenu, menu->mIndex);
			return true;
		}
	}

	return CCommander::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CAddressBookView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eAddressNew:
	case CCommand::eToolbarAddressBookAddressBtn:
	case CCommand::eToolbarAddressBookGroupBtn:
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eAddressesSynchronise:
		OnUpdateDisconnectedSelection(cmdui);
		return;

	case CCommand::eAddressesClearDisconnected:
		OnUpdateClearDisconnectedSelection(cmdui);
		return;

	case CCommand::eEditUndo:
		OnUpdateEditUndo(cmdui);
		return;

	default:;
	}

	CCommander::UpdateCommand(cmd, cmdui);
}

// Respond to clicks in the icon buttons
void CAddressBookView::Receive(JBroadcaster* sender, const Message& message)
{
	// Button Pushed
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mAddrTwister)
		{
			DoAddressTwist();
		}
		else if (sender == mGroupTwister)
		{
			DoGroupTwist();
		}
	}	
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mAddressNew)
		{
			OnNewAddress();
		}
		else if (sender == mAddressDraft)
		{
			OnAddressDraft();
		}
		else if (sender == mGroupNew)
		{
			OnNewGroup();
		}
		else if (sender == mGroupDraft)
		{
			OnGroupDraft();
		}
	}	

	CTableView::Receive(sender, message);
}

void CAddressBookView::OnUpdateEditUndo(CCmdUI* pCmdUI)
{
	mUndoer.FindUndoStatus(pCmdUI->mMenu, pCmdUI->mMenuIndex);
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

// New address
void CAddressBookView::OnNewAddress()
{
	// Expand to make addresses visible
	mAddrTwister->SetState(kTrue);

	// Create a new address
	GetAddressTable()->CreateNewEntry();
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
	mGroupTwister->SetState(kTrue);

	// Create a new address
	GetGroupTable()->CreateNewEntry();
}

// Group draft
void CAddressBookView::OnGroupDraft()
{
	// Create a new draft
	GetGroupTable()->CreateNewLetter(false);
}

void CAddressBookView::DoAddressTwist()
{
	bool expanding = mAddrTwister->IsChecked();

	int moveby = mAddresses->GetFrameHeight();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of table before collapsing
		mAddresses->SetSizing(JXWidget::kHElastic, JXWidget::kFixedTop);

		// Switch over captions
		mHideAddrTitle->Hide();
		mShowAddrTitle->Show();
		
		// Get current split pos
		mSplitPos = mPartition->GetRelativeSplitPos();

		// Expand view and unlock
		mPartition->SetMinima(cTwistCaptionHeight, 64);
		mPartition->SetPixelSplitPos(mPartition->GetPixelSplitPos() - moveby);
		mPartition->SetLocks(true, false);
		
		// Always focus
		mGroupTable->Focus();

		// Disable group twist
		mGroupTwister->Deactivate();
	}
	else
	{
		// Switch over captions
		mShowAddrTitle->Hide();
		mHideAddrTitle->Show();
		
		// Unlock then expand view
		mPartition->SetLocks(false, false);
		mPartition->SetRelativeSplitPos(mSplitPos);

		// Get size and location of parent panel after splitter resize
		int parent_height = mAddressPane->GetApertureHeight();

		// Resize address panel to current parent size less top offset
		mAddresses->AdjustSize(0, parent_height - cTwistCaptionHeight - moveby);

		// Turn on resize of parts after expanding
		mAddresses->SetSizing(JXWidget::kHElastic, JXWidget::kVElastic);

		mPartition->SetMinima(64, 64);

		// Always focus
		mAddressTable->Focus();

		// Enable group twist
		mGroupTwister->Activate();
	}
	
	mAddressDraft->SetActive(expanding ? kTrue : kFalse);
}

void CAddressBookView::DoGroupTwist()
{
	bool expanding = mGroupTwister->IsChecked();

	int moveby = mGroups->GetFrameHeight();

	// Determine motion size and hide/show
	if (!expanding)
	{
		// Turn off resize of table before collapsing
		mGroups->SetSizing(JXWidget::kHElastic, JXWidget::kFixedTop);

		// Switch over captions
		mHideGroupTitle->Hide();
		mShowGroupTitle->Show();
		
		// Get current split pos
		mSplitPos = mPartition->GetRelativeSplitPos();

		// Expand view and unlock
		mPartition->SetMinima(64, cTwistCaptionHeight);
		mPartition->SetPixelSplitPos(mPartition->GetPixelSplitPos() + moveby);
		mPartition->SetLocks(false, true);

		// Always focus
		mAddressTable->Focus();

		// Disable address twist
		mAddrTwister->Deactivate();
	}
	else
	{
		// Switch over captions
		mShowGroupTitle->Hide();
		mHideGroupTitle->Show();
		
		// Unlock then expand view
		mPartition->SetLocks(false, false);
		mPartition->SetRelativeSplitPos(mSplitPos);
		
		// Get size and location of parent panel after splitter resize
		int parent_height = mGroupPane->GetApertureHeight();

		// Resize address panel to current parent size less top offset
		mGroups->AdjustSize(0, parent_height - cTwistCaptionHeight - moveby);

		// Turn on resize of parts after expanding
		mGroups->SetSizing(JXWidget::kHElastic, JXWidget::kVElastic);

		mPartition->SetMinima(64, 64);

		// Always focus
		mGroupTable->Focus();

		// Enable group twist
		mAddrTwister->Activate();
	}
	
	mGroupDraft->SetActive(expanding ? kTrue : kFalse);
}

// Import addresses via plugin
void CAddressBookView::OnAddrImport(JXTextMenu* menu, JIndex item)
{
	// First get plugin name from menu
	cdstring name = menu->GetItemText(item).GetCString();
	
	// Now get plugin from manager
	CAdbkIOPlugin* plugin = CPluginManager::sPluginManager.FindAdbkIOPlugin(name);
	if (!plugin)
		return;
		
	// Do standard open dialog
	JString fname;
	if (JXGetChooseSaveFile()->ChooseFile("Address Book to Import:", NULL, &fname))
	{
		CAddressList addrs;
		CGroupList grps;

		// Do import
		if (plugin->DoImport(fname, addrs, grps))
		{
			// Add results to adbk
			AddAddressList(&addrs);
			AddGroupList(&grps);
		}
	}
}

// Export addresses via plugin
void CAddressBookView::OnAddrExport(JXTextMenu* menu, JIndex item)
{
	// First get plugin name from menu
	cdstring name = menu->GetItemText(item).GetCString();
	
	// Now get plugin from manager
	CAdbkIOPlugin* plugin = CPluginManager::sPluginManager.FindAdbkIOPlugin(name);
	if (!plugin)
		return;
	
	JString fname;
	if (JXGetChooseSaveFile()->SaveFile("Export Address Book to:", NULL, fname, &fname))
	{
		try
		{
			// Create file object
			{
				cdofstream touchit(fname);
			}

			// Check it exists
			if (!JFileWritable(fname))
			{
				CLOG_LOGTHROW(CGeneralException, -1L);
				throw CGeneralException(-1L);
			}
			
			if (plugin->DoExport(fname, *mAdbk->GetAddressList(), *mAdbk->GetGroupList()) == 0)
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
		CAddressBookToolbar* tb = new CAddressBookToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
		tb->OnCreate();
		mToolbar = tb;

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(mToolbar);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(mToolbar, NULL, CToolbarView::eStdButtonsGroup);

		// Force toolbar to be active
		Broadcast_Message(eBroadcast_ViewActivate, this);
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
		txt = (long) GetAddressBook()->GetAddressList()->size();
	else
		txt = "--";
	mAddrNumber->SetText(txt);

	if (GetAddressBook())
		txt = (long) GetAddressBook()->GetGroupList()->size();
	else
		txt = "--";
	mGroupNumber->SetText(txt);

	bool enable = (GetAddressBook() != NULL);
	mAddressNew->SetActive(enable ? kTrue : kFalse);
	mAddressDraft->SetActive(enable ? kTrue : kFalse);
	mGroupNew->SetActive(enable ? kTrue : kFalse);
	mGroupDraft->SetActive(enable ? kTrue : kFalse);
}

// Reset state from prefs
void CAddressBookView::ResetState(bool force)
{
	if (!GetAddressBook())
		return;

	// Get name as cstr (strip trailing suffix)
	cdstring name = GetAddressBook()->GetURL();

	// Check for available state
	CNewAddressBookWindowState* state = CPreferences::sPrefs->GetAddressBookWindowInfo(name);

	// If no prefs try default
	if (!state || force)
		state = &CPreferences::sPrefs->mNewAddressBookWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mNewAddressBookWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect);

			// Reset bounds
			GetAddressBookWindow()->GetWindow()->Place(set_rect.left, set_rect.top);
			GetAddressBookWindow()->GetWindow()->SetSize(set_rect.width(), set_rect.height());
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mNewAddressBookWindowDefault.GetValue()));

	// Set sort trait
	mSortColumnType = MapSortToColumn(state->GetSort());
	SetSortBy(GetSortBy());
	
	// Expand both
	if (state->GetAddressShow() && state->GetGroupShow())
	{
		// Make sure both are expanded
		mAddrTwister->SetState(kTrue);
		mGroupTwister->SetState(kTrue);
		
		// Now set splitter
		mPartition->SetRelativeSplitPos(state->GetAddressSize());
	}
	
	// Expand address only
	else if (state->GetAddressShow())
	{
		// Make sure both are expanded (expand address first)
		mAddrTwister->SetState(kTrue);
		mGroupTwister->SetState(kFalse);
		
		// Now set splitter pos for collapsed state
		mSplitPos = state->GetAddressSize();
	}

	// Expand group only
	else
	{
		// Make sure both are expanded (expand group first)
		mGroupTwister->SetState(kTrue);
		mAddrTwister->SetState(kFalse);
		
		// Now set splitter pos for collapsed state
		mSplitPos = state->GetAddressSize();
	}

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetAddressBookWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	// If forced reset, save it
	if (force)
		SaveState();

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetWindow()->ShowWindow(SW_SHOWMAXIMIZED);

	// Always activate
	if (!Is3Pane() && !GetWindow()->IsVisible())
		GetAddressBookWindow()->Activate();
}

// Save current state in prefs
void CAddressBookView::SaveState(void)
{
	if (!GetAddressBook())
		return;

	// Get name as cstr (strip trailing suffix)
	cdstring name = GetAddressBook()->GetURL();

	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	JRect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Sync size
	long address_size = 0;
	if (mAddrTwister->IsChecked() && mGroupTwister->IsChecked())
		address_size = mPartition->GetRelativeSplitPos();
	else
		address_size = mSplitPos;

	// Sync column state
	SyncColumns();

	// Add info to prefs
	CNewAddressBookWindowState* info = new CNewAddressBookWindowState(name,
																		&user_bounds,
																		zoomed ? eWindowStateMax : eWindowStateNormal,
																		&mColumnInfo,
																		MapColumnToSort(mSortColumnType),
																		address_size,
																		0, 
																		mAddrTwister->IsChecked(),
																		mGroupTwister->IsChecked(),
																		Is3Pane() ? 0 : GetAddressBookWindow()->GetSplitter()->GetRelativeSplitPos());
	if (info)
		CPreferences::sPrefs->AddAddressBookWindowInfo(info);
}

// Save current state in prefs
void CAddressBookView::SaveDefaultState(void)
{
	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	JRect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Sync size
	long address_size = 0;
	if (mAddrTwister->IsChecked() && mGroupTwister->IsChecked())
		address_size = mPartition->GetRelativeSplitPos();
	else
		address_size = mSplitPos;

	// Sync column state
	SyncColumns();

	// Add info to prefs
	CNewAddressBookWindowState state(NULL,
										&user_bounds,
										zoomed ? eWindowStateMax : eWindowStateNormal,
										&mColumnInfo,
										MapColumnToSort(mSortColumnType),
										address_size,
										0,
										mAddrTwister->IsChecked(),
										mGroupTwister->IsChecked(),
										Is3Pane() ? 0 : GetAddressBookWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mNewAddressBookWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mNewAddressBookWindowDefault.SetDirty();
}

// Reset text traits from prefs
void CAddressBookView::ResetFont(const SFontInfo& finfo)
{
	// Change fonts
	GetAddressTable()->ResetFont(finfo);
	GetBaseTitles()->ResetFont(finfo);
	GetBaseTitles()->SyncTable(GetAddressTable(), true);
	GetGroupTable()->ResetFont(finfo);
}
