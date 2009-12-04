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
#include "CAddressBookWindow.h"
#include "CAddressTable.h"
#include "CCommands.h"
#include "CErrorHandler.h"
#include "CGroupTable.h"
#include "CLog.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CReplyChooseDialog.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneWindow.h"

#include <LBevelButton.h>
#include <LDisclosureTriangle.h>
#include <UStandardDialogs.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

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

// Constructor from stream
CAddressBookView::CAddressBookView(LStream *inStream)
		: CTableView(inStream)
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

// Setup help balloons
void CAddressBookView::FinishCreateSelf(void)
{
	// Do inherited
	SetViewInfo(eAddrColumnName, 80, false);

	// Need to get group table BEFORE doing inherited as inherited function will
	// call ResetTextTraits which needs the group table to be present
	mGroups = (CGroupTable*) FindPaneByID(paneid_AddressBookViewGroupTable);
	mGroups->Add_Listener(this);

	// Do inherited
	CTableView::FinishCreateSelf();

	// Get items
	mSplitter = (CSplitterView*) FindPaneByID(paneid_AddressBookViewSplitter);
	mAddressTwist = (LDisclosureTriangle*) FindPaneByID(paneid_AddressBookViewAddressTwist);
	mAddressTwist->AddListener(this);
	LBevelButton* btn = (LBevelButton*) FindPaneByID(paneid_AddressBookViewAddressAdd);
	btn->AddListener(this);
	btn = (LBevelButton*) FindPaneByID(paneid_AddressBookViewAddressDraft);
	btn->AddListener(this);
	mShowAddresses = (CStaticText*) FindPaneByID(paneid_AddressBookViewAddressShow);
	mShowAddresses->Hide();
	mHideAddresses = (CStaticText*) FindPaneByID(paneid_AddressBookViewAddressHide);
	mAddressPanel = (LView*) FindPaneByID(paneid_AddressBookViewAddressPanel);
	mGroupTwist = (LDisclosureTriangle*) FindPaneByID(paneid_AddressBookViewGroupTwist);
	mGroupTwist->AddListener(this);
	btn = (LBevelButton*) FindPaneByID(paneid_AddressBookViewGroupAdd);
	btn->AddListener(this);
	btn = (LBevelButton*) FindPaneByID(paneid_AddressBookViewGroupDraft);
	btn->AddListener(this);
	mShowGroups = (CStaticText*) FindPaneByID(paneid_AddressBookViewGroupShow);
	mShowGroups->Hide();
	mHideGroups = (CStaticText*) FindPaneByID(paneid_AddressBookViewGroupHide);
	mGroupPanel = (LView*) FindPaneByID(paneid_AddressBookViewGroupPanel);
	mAddressTotal = (CStaticText*) FindPaneByID(paneid_AddressBookViewAddressTotal);
	mGroupTotal = (CStaticText*) FindPaneByID(paneid_AddressBookViewGroupTotal);
	
	// Install splitter items
	LView* addr_view = (LView*)  FindPaneByID(paneid_AddressBookViewAddresses);
	LView* grp_view = (LView*)  FindPaneByID(paneid_AddressBookViewGroups);
	mSplitter->InstallViews(addr_view, grp_view, true);
	mSplitter->SetMinima(64, 64);
	mSplitter->SetVisibleLock(true);

	// Always start disabled until mailbox is set
	Disable();
}

//	Respond to commands
Boolean CAddressBookView::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	ResIDT	menuID;
	SInt16	menuItem;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Show then select the requested window
		switch (menuID)
		{
		case MENU_ImportAddresses:
			DoImport(menuItem);
			cmdHandled = true;
			break;

		case MENU_ExportAddresses:
			DoExport(menuItem);
			cmdHandled = true;
			break;

		default:
			cmdHandled = CTableView::ObeyCommand(inCommand, ioParam);
		}
	}
	else
	{
		switch (inCommand)
		{
		case cmd_NewAddress:
		case cmd_ToolbarAddressBookAddressBtn:
			OnNewAddress();
			break;

		case cmd_NewGroup:
		case cmd_ToolbarAddressBookGroupBtn:
			OnNewGroup();
			break;

		case cmd_SynchroniseAddressBook:
			OnSynchroniseAddressBook();
			break;

		case cmd_ClearDisconnectAddressBook:
			OnClearDisconnectAddressBook();
			break;

		default:
			cmdHandled = CTableView::ObeyCommand(inCommand, ioParam);
			break;
		}
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CAddressBookView::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	ResIDT	menuID;
	SInt16	menuItem;

	if (IsSyntheticCommand(inCommand, menuID, menuItem))
	{
		// Always enable windows menu
		switch (menuID)
		{
		case MENU_ImportAddresses:
			outEnabled = true;
			break;
		case MENU_ExportAddresses:
			outEnabled = true;
			break;
		default:
			CTableView::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		}
	}
	else
	{
		switch (inCommand)
		{
		case cmd_NewAddress:
		case cmd_ToolbarAddressBookAddressBtn:
		case cmd_NewGroup:
		case cmd_ToolbarAddressBookGroupBtn:
			outEnabled = true;
			break;

		case cmd_SynchroniseAddressBook:
			outEnabled = (mAdbk != NULL) && mAdbk->GetProtocol()->CanDisconnect() &&
							mAdbk->GetProtocol()->IsDisconnected();
			break;

		case cmd_ClearDisconnectAddressBook:
			outEnabled = (mAdbk != NULL) && mAdbk->GetProtocol()->CanDisconnect() &&
							!mAdbk->GetProtocol()->IsDisconnected();
			break;

		default:
			CTableView::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		}
	}
}

// Respond to clicks in the icon buttons
void CAddressBookView::ListenToMessage(MessageT inMessage, void *ioParam)
{
	switch(inMessage)
	{
	case msg_AddressBookViewAddressTwist:
		DoAddressTwist();
		break;
	case msg_AddressBookViewAddressNew:
		OnNewAddress();
		break;
	case msg_AddressBookViewAddressDraft:
		GetAddressTable()->CreateNewLetter(false);
		break;
	case msg_AddressBookViewGroupTwist:
		DoGroupTwist();
		break;
	case msg_AddressBookViewGroupNew:
		OnNewGroup();
		break;
	case msg_AddressBookViewGroupDraft:
		GetGroupTable()->CreateNewLetter(false);
		break;
	}
}

// New address
void CAddressBookView::OnNewAddress()
{
	// Expand to make addresses visible
	mAddressTwist->SetValue(1);

	// Create a new address
	GetAddressTable()->CreateNewEntry();
}

// New group
void CAddressBookView::OnNewGroup()
{
	// Expand to make addresses visible
	mGroupTwist->SetValue(1);

	// Create a new address
	GetGroupTable()->CreateNewEntry();
}

void CAddressBookView::DoAddressTwist()
{
	bool expanding = (mAddressTwist->GetValue() == 1);

	// Get size and location of address panel
	SDimension16 frame;
	mAddressPanel->GetFrameSize(frame);

	// Determine motion size and hide/show
	if (expanding)
	{
		// Switch over captions
		mShowAddresses->Hide();
		mHideAddresses->Show();
		
		// Expand view and unlock
		mSplitter->SetRelativeSplitPos(mSplitPos);
		mSplitter->SetLocks(false, false);

		// Get size and location of parent panel after splitter resize
		SDimension16 parent_frame;
		mAddressPanel->GetSuperView()->GetFrameSize(parent_frame);
		SPoint32 parent_frameLoc;
		mAddressPanel->GetSuperView()->GetFrameLocation(parent_frameLoc);
		SPoint32 frameLoc;
		mAddressPanel->GetFrameLocation(frameLoc);

		// Resize address panel to current parent size less top offset
		mAddressPanel->ResizeFrameBy(0, parent_frame.height - frame.height - (frameLoc.v - parent_frameLoc.v), false);

		// Turn on resize of frame and show it
		SBooleanRect binding;
		mAddressPanel->GetFrameBinding(binding);
		binding.bottom = true;
		mAddressPanel->SetFrameBinding(binding);
		GetAddressTable()->Enable();
		mAddressPanel->Show();

		mSplitter->SetMinima(64, 64);

		// Enable group twist
		mGroupTwist->Enable();

		SwitchTarget(GetAddressTable());
	}
	else
	{
		// Switch over captions
		mHideAddresses->Hide();
		mShowAddresses->Show();

		// Turn off resize of frame and hide it
		SBooleanRect binding;
		mAddressPanel->GetFrameBinding(binding);
		binding.bottom = false;
		mAddressPanel->SetFrameBinding(binding);
		GetAddressTable()->Disable();
		mAddressPanel->Hide();
		
		// Get current split pos
		mSplitPos = mSplitter->GetRelativeSplitPos();

		// Collapse view and lock
		mSplitter->SetMinima(16, 64);
		mSplitter->SetPixelSplitPos(mSplitter->GetPixelSplitPos() - frame.height);
		mSplitter->SetLocks(true, false);
		
		// Disable group twist
		mGroupTwist->Disable();

		SwitchTarget(GetGroupTable());
	}

	FindPaneByID(paneid_AddressBookViewAddressDraft)->SetEnabled(expanding);
}

void CAddressBookView::DoGroupTwist()
{
	bool expanding = (mGroupTwist->GetValue() == 1);

	// Get size and location of address panel and parent panel
	SDimension16 frame;
	mGroupPanel->GetFrameSize(frame);

	// Determine motion size and hide/show
	if (expanding)
	{
		// Switch over captions
		mShowGroups->Hide();
		mHideGroups->Show();
		
		// Expand view and unlock
		mSplitter->SetRelativeSplitPos(mSplitPos);
		mSplitter->SetLocks(false, false);

		// Get size and location of parent panel after splitter resize
		SDimension16 parent_frame;
		mGroupPanel->GetSuperView()->GetFrameSize(parent_frame);
		SPoint32 parent_frameLoc;
		mGroupPanel->GetSuperView()->GetFrameLocation(parent_frameLoc);
		SPoint32 frameLoc;
		mGroupPanel->GetFrameLocation(frameLoc);

		// Resize group panel to current parent size less top offset
		mGroupPanel->ResizeFrameBy(0, parent_frame.height - frame.height - (frameLoc.v - parent_frameLoc.v), false);

		// Turn on resize of frame and show it
		SBooleanRect binding;
		mGroupPanel->GetFrameBinding(binding);
		binding.bottom = true;
		mGroupPanel->SetFrameBinding(binding);
		GetGroupTable()->Enable();
		mGroupPanel->Show();

		mSplitter->SetMinima(64, 64);
		
		// Enable address twist
		mAddressTwist->Enable();

		SwitchTarget(GetGroupTable());
	}
	else
	{
		// Switch over captions
		mHideGroups->Hide();
		mShowGroups->Show();

		// Turn off resize of frame and hide it
		SBooleanRect binding;
		mGroupPanel->GetFrameBinding(binding);
		binding.bottom = false;
		mGroupPanel->SetFrameBinding(binding);
		GetGroupTable()->Disable();
		mGroupPanel->Hide();
		
		// Get current split pos
		mSplitPos = mSplitter->GetRelativeSplitPos();

		// Collapse view and lock
		mSplitter->SetMinima(64, 16);
		mSplitter->SetPixelSplitPos(mSplitter->GetPixelSplitPos() + frame.height);
		mSplitter->SetLocks(false, true);
		
		// Disable address twist
		mAddressTwist->Disable();

		SwitchTarget(GetAddressTable());
	}

	FindPaneByID(paneid_AddressBookViewGroupDraft)->SetEnabled(expanding);
}

// Import addresses via plugin
void CAddressBookView::DoImport(long item)
{
	// First get menu name
	LMenu* import_menu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_ImportAddresses);
	cdstring name = ::GetMenuItemTextUTF8(import_menu->GetMacMenuH(), item);

	// Now get plugin from manager
	CAdbkIOPlugin* plugin = CPluginManager::sPluginManager.FindAdbkIOPlugin(name);
	if (!plugin)
		return;

	// Post action
	PPx::FSObject	fspec;
	if (PP_StandardDialogs::AskOpenOneFile(plugin->GetFileType()[0], fspec,
												kNavDefaultNavDlogOptions | kNavSelectAllReadableItem | kNavAllowPreviews | kNavAllFilesInPopup))
	{
		CAddressList addrs;
		CGroupList grps;

		// Do import
		if (plugin->DoImport(&fspec, addrs, grps))
		{
			// Add results to adbk
			AddAddressList(&addrs);
			AddGroupList(&grps);
		}
	}

}

// Export addresses via plugin
void CAddressBookView::DoExport(long item)
{
	// First get menu name
	LMenu* export_menu = LMenuBar::GetCurrentMenuBar()->FetchMenu(MENU_ExportAddresses);
	cdstring name = ::GetMenuItemTextUTF8(export_menu->GetMacMenuH(), item);

	// Now get plugin from manager
	CAdbkIOPlugin* plugin = CPluginManager::sPluginManager.FindAdbkIOPlugin(name);
	if (!plugin)
		return;

	// Now get file
	PPx::CFString defaultName("");
	PPx::FSObject fspec;
	bool replacing;
	if (!PP_StandardDialogs::AskSaveFile(defaultName, 'TEXT', fspec, replacing, kNavDefaultNavDlogOptions | kNavNoTypePopup))
		return;

	// Fix for window manager bug after a replace operation
	Activate();

	// See whether file currently exists
	if (replacing)
		// Delete existing
		fspec.Delete();

	try
	{
		// Create file
		LFileStream file(fspec);
		file.CreateNewDataFile(plugin->GetCreator(), 'TEXT', 0);
		fspec.Update();

		if (plugin->DoExport(&fspec, *mAdbk->GetAddressList(), *mAdbk->GetGroupList()) == 0)
		{
			CLOG_LOGTHROW(CGeneralException, -1);
			throw CGeneralException(-1);
		}
	}
	catch (...)
	{
		CLOG_LOGCATCH(...);

		// Must delete any file created
		fspec.Delete();

		// Throw up
		CLOG_LOGRETHROW;
		throw;
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
		mToolbar = static_cast<CToolbar*>(UReanimator::CreateView(paneid_AddressBookToolbar1, parent, NULL));

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
	cdstring aStr;
	if (GetAddressBook())
		aStr = GetAddressBook()->GetAddressList()->size();
	else
		aStr = "--";
	mAddressTotal->SetText(aStr);

	if (GetAddressBook())
		aStr = GetAddressBook()->GetGroupList()->size();
	else
		aStr = "--";
	mGroupTotal->SetText(aStr);
	
	bool enable = (GetAddressBook() != NULL);
	FindPaneByID(paneid_AddressBookViewAddressAdd)->SetEnabled(enable);
	FindPaneByID(paneid_AddressBookViewAddressDraft)->SetEnabled(enable);
	FindPaneByID(paneid_AddressBookViewGroupAdd)->SetEnabled(enable);
	FindPaneByID(paneid_AddressBookViewGroupDraft)->SetEnabled(enable);
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
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mNewAddressBookWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetAddressBookWindow());

			// Reset bounds
			GetAddressBookWindow()->DoSetBounds(set_rect);
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
		mAddressTwist->SetValue(true);
		mGroupTwist->SetValue(true);
		
		// Now set splitter
		mSplitter->SetRelativeSplitPos(state->GetAddressSize());
	}
	
	// Expand address only
	else if (state->GetAddressShow())
	{
		// Make sure both are expanded (expand address first)
		mAddressTwist->SetValue(true);
		mGroupTwist->SetValue(false);
		
		// Now set splitter pos for collapsed state
		mSplitPos = state->GetAddressSize();
	}

	// Expand group only
	else
	{
		// Make sure both are expanded (expand group first)
		mGroupTwist->SetValue(true);
		mAddressTwist->SetValue(false);
		
		// Now set splitter pos for collapsed state
		mSplitPos = state->GetAddressSize();
	}

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetAddressBookWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	// If forced reset, save it
	if (force)
		SaveState();
}

// Save current state in prefs
void CAddressBookView::SaveState(void)
{
	if (!GetAddressBook())
		return;

	// Get name as cstr (strip trailing suffix)
	cdstring name = GetAddressBook()->GetURL();

	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetAddressBookWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetAddressBookWindow()->GetUserBounds(user_bounds);

	// Sync size
	long address_size = 0;
	if (mAddressTwist->GetValue() && mGroupTwist->GetValue())
		address_size = mSplitter->GetRelativeSplitPos();
	else
		address_size = mSplitPos;

	// Add info to prefs
	CNewAddressBookWindowState* info = new CNewAddressBookWindowState(name,
																		&user_bounds,
																		zoomed ? eWindowStateMax : eWindowStateNormal,
																		&mColumnInfo,
																		MapColumnToSort(mSortColumnType),
																		address_size,
																		0,
																		mAddressTwist->GetValue(),
																		mGroupTwist->GetValue(),
																		Is3Pane() ? 0 : GetAddressBookWindow()->GetSplitter()->GetRelativeSplitPos());
	if (info)
		CPreferences::sPrefs->AddAddressBookWindowInfo(info);
}

// Save current state in prefs
void CAddressBookView::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetAddressBookWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetAddressBookWindow()->GetUserBounds(user_bounds);

	// Sync size
	long address_size = 0;
	if (mAddressTwist->GetValue() && mGroupTwist->GetValue())
		address_size = mSplitter->GetRelativeSplitPos();
	else
		address_size = mSplitPos;

	// Add info to prefs
	CNewAddressBookWindowState state(NULL,
										&user_bounds,
										zoomed ? eWindowStateMax : eWindowStateNormal,
										&mColumnInfo,
										MapColumnToSort(mSortColumnType),
										address_size,
										0,
										mAddressTwist->GetValue(),
										mGroupTwist->GetValue(),
										Is3Pane() ? 0 : GetAddressBookWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mNewAddressBookWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mNewAddressBookWindowDefault.SetDirty();
}

// Reset text traits from prefs
void CAddressBookView::ResetTextTraits(const TextTraitsRecord& list_traits)
{
	// Get old header row height
	short old_height = mTitles->GetRowHeight(1);

	// Change fonts
	mTitles->SetTextTraits(list_traits);
	GetAddressTable()->SetTextTraits(list_traits);
	GetGroupTable()->SetTextTraits(list_traits);

	// Get new row height
	short diff_height = mTitles->GetRowHeight(1) - old_height;

	// Change panes if row height changed
	if (diff_height)
	{
		// Offset and resize table
		GetAddressTable()->GetSuperView()->MoveBy(0, diff_height, false);
		GetAddressTable()->GetSuperView()->ResizeFrameBy(0, -diff_height, true);

		// Do header and button
		mTitles->ResizeFrameBy(0, diff_height, true);
	}
}
