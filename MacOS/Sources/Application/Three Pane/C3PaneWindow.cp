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


// Source for C3PaneWindow class

#include "C3PaneWindow.h"

#include "CAdbkManagerView.h"
#include "CAddressBookView.h"
#include "CAddressView.h"
#include "CAdminLock.h"
#include "CCalendarStoreView.h"
#include "CCalendarView.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CMailboxInfoView.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerView.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CStatusWindow.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"
#include "CWindowOptionsDialog.h"
#include "C3PaneAccounts.h"
#include "C3PaneItems.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneMainPanel.h"
#include "C3PanePreview.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

C3PaneWindow* C3PaneWindow::s3PaneWindow = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneWindow::C3PaneWindow() :
	CHelpTagWindow(this)
{
	Init3PaneWindow();
}

// Constructor from stream
C3PaneWindow::C3PaneWindow(LStream *inStream) :
	LWindow(inStream),
	CHelpTagWindow(this)
{
	Init3PaneWindow();
}

// Default destructor
C3PaneWindow::~C3PaneWindow()
{
	// Remove from list
	CWindowsMenu::RemoveWindow(this);

	s3PaneWindow = NULL;
}

// Default constructor
void C3PaneWindow::Init3PaneWindow()
{
	s3PaneWindow = this;

	mGeometry = N3Pane::eNone;
	mViewType = N3Pane::eView_Empty;

	mListVisible = true;
	mItemsVisible = true;
	mPreviewVisible = true;
	mStatusVisible = true;
	mListZoom = false;
	mItemsZoom = false;
	mPreviewZoom = false;

	// Add to list
	CWindowsMenu::AddWindow(this, true);
}

// Create it or bring it to the front
void C3PaneWindow::Create3PaneWindow()
{
	// Create find & replace window or bring to front
	if (s3PaneWindow)
	{
		// Just bring existing window to front
		FRAMEWORK_WINDOW_TO_TOP(s3PaneWindow)
	}
	else
	{
		// Create the window since it does not currently exist
		C3PaneWindow* wnd = (C3PaneWindow*) LWindow::CreateWindow(paneid_3PaneWindow, CMulberryApp::sApp);
		wnd->ResetState();
		wnd->Show();
		
		// Can now dispose of any existing status window
		if (CStatusWindow::sStatusWindow)
			delete CStatusWindow::sStatusWindow;
	}
}

void C3PaneWindow::Destroy3PaneWindow()
{
	if (s3PaneWindow)
		s3PaneWindow->AttemptClose();
	FRAMEWORK_DELETE_WINDOW(s3PaneWindow)
	s3PaneWindow = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneWindow::FinishCreateSelf()
{
	// Do inherited (by pass the MLTE event handler setting until we have loaded all panes)
	LWindow::FinishCreateSelf();
	SetupHelpTags();

	// Deal with splitter
	mSplitter1 = (CSplitterView*) FindPaneByID(paneid_3PaneSplitter1);
	mSplitter2 = (CSplitterView*) FindPaneByID(paneid_3PaneSplitter2);

	mListView = (C3PaneMainPanel*) FindPaneByID(paneid_3PaneSplitA);
	mItemsView = (C3PaneMainPanel*) FindPaneByID(paneid_3PaneSplitB);
	mPreviewView = (C3PaneMainPanel*) FindPaneByID(paneid_3PaneSplitC);

	mStatus = (CStaticText*) FindPaneByID(paneid_3PaneStatus);
	mProgress = (CStaticText*) FindPaneByID(paneid_3PaneProgress);

	// Get toolbar
	mToolbarView = (CToolbarView*) FindPaneByID(paneid_3PaneToolbarView);
	mToolbarView->SetSibling(mSplitter1);
	InstallToolbars();

	// Make the child views - must do after creating toolbars as the views need them
	InstallViews();

	// Now set the initial geometry
	SetGeometry(N3Pane::eListVert);

	// Starts off as mailbox view
	SetViewType(N3Pane::eView_Mailbox);

	// Set status
	SetOpen();

#ifdef NOTYET
	CContextMenuAttachment::AddUniqueContext(this, 9003, GetTable());
#endif
}

// Handle key presses
Boolean C3PaneWindow::HandleKeyPress(const EventRecord	&inKeyEvent)
{
	// Get state of command modifiers (not shift)
	bool mods = ((inKeyEvent.modifiers & (optionKey | cmdKey | controlKey)) != 0);

	CommandT cmd = 0;
	switch (inKeyEvent.message & charCodeMask)
	{
	// Key shortcuts
	case '1':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			if (mListVisible)
				mAccounts->Focus();
			return true;
		}
		break;
	case '2':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			if (mItemsVisible && mItems->IsSpecified())
				mItems->Focus();
			return true;
		}
		break;
	case '3':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			if (mPreviewVisible && mPreview->IsSpecified())
				mPreview->Focus();
			return true;
		}
		break;
	case '4':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			SetViewType(N3Pane::eView_Mailbox);
			return true;
		}
		break;
	case '5':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			SetViewType(N3Pane::eView_Contacts);
			return true;
		}
		break;
	case '6':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			SetViewType(N3Pane::eView_Calendar);
			return true;
		}
		break;
	case '9':
	case '0':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			// Create ordered arrays of panes and visible/specified states
			C3PaneParentPanel* focus[3] = {mAccounts, mItems, mPreview};
			bool visible[3] = {mListVisible, mItemsVisible && mItems->IsSpecified(), mPreviewVisible && mPreview->IsSpecified()};
			
			// Get currently focussed pane index
			unsigned long index = 0;
			if (mAccounts->HasFocus())
				index = 3;
			else if (mItems->HasFocus())
				index = 4;
			else if (mPreview->HasFocus())
				index = 5;
			
			// Adjust index to prev/next visible item and focus it
			if ((inKeyEvent.message & charCodeMask) == '9')
			{
				if (visible[--index % 3] || visible[--index % 3])
					focus[index % 3]->Focus();
			}
			else
			{
				if (visible[++index % 3] || visible[++index % 3])
					focus[index % 3]->Focus();
			}
		}
		break;
	default:;
	}

	// Check for command execute
	if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods && (cmd != 0))
	{
		// Check whether command is valid right now
		Boolean outEnabled;
		Boolean outUsesMark;
		UInt16 outMark;
		Str255 outName;
		FindCommandStatus(cmd, outEnabled, outUsesMark, outMark, outName);
		
		// Execute if enabled
		if (outEnabled)
			ObeyCommand(cmd, NULL);

		return true;
	}

	// Do default key press processing
	return LWindow::HandleKeyPress(inKeyEvent);
}

//	Respond to commands
Boolean C3PaneWindow::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_GeometryS__M_P:
		SetGeometry(N3Pane::eListVert);
		break;
	case cmd_GeometryS_P__M:
		SetGeometry(N3Pane::eListHoriz);
		break;
	case cmd_GeometryS__M__P:
		SetGeometry(N3Pane::eItemsVert);
		break;
	case cmd_GeometryM_P__S:
		SetGeometry(N3Pane::eItemsHoriz);
		break;
	case cmd_GeometryS__P__M:
		SetGeometry(N3Pane::ePreviewVert);
		break;
	case cmd_GeometryS_M__P:
		SetGeometry(N3Pane::ePreviewHoriz);
		break;
	case cmd_GeometryS_M_P:
		SetGeometry(N3Pane::eAllVert);
		break;
	case cmd_GeometryS_M_P_:
		SetGeometry(N3Pane::eAllHoriz);
		break;

	case cmd_ToolbarZoomList:
		ZoomList(!mListZoom);
		break;

	case cmd_ToolbarZoomItems:
		ZoomItems(!mItemsZoom);
		break;

	case cmd_ToolbarZoomPreview:
		ZoomPreview(!mPreviewZoom);
		break;

	case cmd_ShowList:
	case cmd_ToolbarShowList:
		ShowList(!mListVisible);
		break;

	case cmd_ShowItems:
	case cmd_ToolbarShowItems:
		ShowItems(!mItemsVisible);
		break;

	case cmd_ShowPreview:
	case cmd_ToolbarShowPreview:
		ShowPreview(!mPreviewVisible);
		break;

	case cmd_SetDefaultSize:
		SaveDefaultPaneState();
		break;

	case cmd_ResetDefaultWindow:
		ResetPaneState();
		break;

	default:
		cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void C3PaneWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_Geometry:
		// Always enabled
		outEnabled = true;
		break;

	case cmd_GeometryS__M_P:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::eListVert) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryS_P__M:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::eListHoriz) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryS__M__P:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::eItemsVert) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryM_P__S:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::eItemsHoriz) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryS__P__M:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::ePreviewVert) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryS_M__P:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::ePreviewHoriz) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryS_M_P:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::eAllVert) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_GeometryS_M_P_:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mGeometry == N3Pane::eAllHoriz) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ShowList:
	{
		outEnabled = mItemsVisible || mPreviewVisible;
		LStr255 txt(STRx_Standards, !mListVisible ? str_ShowList : str_HideList);
		::PLstrcpy(outName, txt);
		break;
	}

	case cmd_ShowItems:
	{
		// Always enabled but text may change
		outEnabled = mListVisible || mPreviewVisible;
		LStr255 txt(STRx_Standards, !mItemsVisible ? str_ShowItems : str_HideItems);
		::PLstrcpy(outName, txt);
		break;
	}

	case cmd_ShowPreview:
	{
		// Always enabled but text may change
		outEnabled = mItemsVisible || mListVisible;
		LStr255 txt(STRx_Standards, !mPreviewVisible ? str_ShowPreview : str_HidePreview);
		::PLstrcpy(outName, txt);
		break;
	}

	case cmd_ToolbarShowList:
		outEnabled = mItemsVisible || mPreviewVisible;
		outUsesMark = true;
		outMark = mListVisible ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarShowItems:
		outEnabled = mListVisible || mPreviewVisible;
		outUsesMark = true;
		outMark = mItemsVisible ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarShowPreview:
		outEnabled = mItemsVisible || mListVisible;
		outUsesMark = true;
		outMark = mPreviewVisible ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarZoomList:
		outEnabled = true;
		outUsesMark = true;
		outMark = mListZoom ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarZoomItems:
		outEnabled = true;
		outUsesMark = true;
		outMark = mItemsZoom ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarZoomPreview:
		outEnabled = true;
		outUsesMark = true;
		outMark = mPreviewZoom ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_SetDefaultSize:
	case cmd_ResetDefaultWindow:
	case cmd_Toolbar:
		// Always enabled
		outEnabled = true;
		break;

	default:
		LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

void C3PaneWindow::InstallToolbars()
{
	// Create toolbars for each type of view
	mMailboxToolbar = static_cast<C3PaneMailboxToolbar*>(UReanimator::CreateView(paneid_3PaneMailboxToolbar, mToolbarView, this));
	mToolbarView->AddToolbar(mMailboxToolbar, this, CToolbarView::eStdButtonsGroup);

	mAdbkToolbar = static_cast<C3PaneAdbkToolbar*>(UReanimator::CreateView(paneid_3PaneAdbkToolbar, mToolbarView, this));
	mToolbarView->AddToolbar(mAdbkToolbar, this, CToolbarView::eStdButtonsGroup);

	mCalendarToolbar = static_cast<C3PaneCalendarToolbar*>(UReanimator::CreateView(paneid_3PaneCalendarToolbar, mToolbarView, this));
	mToolbarView->AddToolbar(mCalendarToolbar, this, CToolbarView::eStdButtonsGroup);
}

void C3PaneWindow::InstallViews()
{
	// Read the account view resource
	mAccounts = static_cast<C3PaneAccounts*>(UReanimator::CreateView(paneid_3PaneAccounts, GetListView(), this));
	GetListView()->SetSubView(mAccounts);
	GetListView()->ExpandSubPane(mAccounts, true, true);

	// Read the item view resource
	mItems = static_cast<C3PaneItems*>(UReanimator::CreateView(paneid_3PaneItems, GetItemsView(), this));
	GetItemsView()->SetSubView(mItems);
	GetItemsView()->ExpandSubPane(mItems, true, true);

	// Read the preview resource
	mPreview = static_cast<C3PanePreview*>(UReanimator::CreateView(paneid_3PanePreview, GetPreviewView(), this));
	GetPreviewView()->SetSubView(mPreview);
	GetPreviewView()->ExpandSubPane(mPreview, true, true);

	// Hook up listeners

	// Give mailbox preview to server view
	mAccounts->GetServerView()->SetPreview(mItems->GetMailboxView());
	
	// Give server view to mailbox view
	mItems->GetMailboxView()->SetOwnerView(mAccounts->GetServerView());

	// Give message preview to mailbox view
	mItems->GetMailboxView()->SetPreview(mPreview->GetMessageView());
	
	// Give mailbox preview to message view
	mPreview->GetMessageView()->SetOwnerView(mItems->GetMailboxView());

	// Give address book preview to contacts view
	mAccounts->GetContactsView()->SetPreview(mItems->GetAddressBookView());
	
	// Give message preview to mailbox view
	mItems->GetAddressBookView()->SetPreview(mPreview->GetAddressView());

	// Don't do if admin locks it out
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		// Give calendar preview to calendar store view
		mAccounts->GetCalendarStoreView()->SetPreview(mItems->GetCalendarView());
		C3PaneWindow::s3PaneWindow->DoneInitCalendarAccounts();
		
		// Give calendar item preview to calendar view
		mItems->GetCalendarView()->SetPreview(mPreview->GetEventView());
	}
}

void C3PaneWindow::SetGeometry(N3Pane::EGeometry geometry)
{
	// Only bother if different
	if (mGeometry == geometry)
		return;

	// Hide main splitter first
	mSplitter1->Hide();

	// Move views around as appropriate
	mGeometry = geometry;
	switch(mGeometry)
	{
	case N3Pane::eListVert:
	case N3Pane::eNone:
		mSplitter1->InstallViews(GetListView(), mSplitter2, false);
		mSplitter2->InstallViews(GetItemsView(), GetPreviewView(), true);
		break;

	case N3Pane::eListHoriz:
		mSplitter1->InstallViews(GetListView(), mSplitter2, true);
		mSplitter2->InstallViews(GetItemsView(), GetPreviewView(), false);
		break;

	case N3Pane::eItemsVert:
		mSplitter1->InstallViews(mSplitter2, GetItemsView(), false);
		mSplitter2->InstallViews(GetListView(), GetPreviewView(), true);
		break;

	case N3Pane::eItemsHoriz:
		mSplitter1->InstallViews(GetItemsView(), mSplitter2, true);
		mSplitter2->InstallViews(GetListView(), GetPreviewView(), false);
		break;

	case N3Pane::ePreviewVert:
		mSplitter1->InstallViews(mSplitter2, GetPreviewView(), false);
		mSplitter2->InstallViews(GetListView(), GetItemsView(), true);
		break;

	case N3Pane::ePreviewHoriz:
		mSplitter1->InstallViews(mSplitter2, GetPreviewView(), true);
		mSplitter2->InstallViews(GetListView(), GetItemsView(), false);
		break;

	case N3Pane::eAllVert:
		mSplitter1->InstallViews(GetListView(), mSplitter2, false);
		mSplitter2->InstallViews(GetItemsView(), GetPreviewView(), false);
		break;

	case N3Pane::eAllHoriz:
		mSplitter1->InstallViews(GetListView(), mSplitter2, true);
		mSplitter2->InstallViews(GetItemsView(), GetPreviewView(), true);
		break;
	};

	// Show main splitter first
	mSplitter1->Show();
	
	// Update toolbars
	mToolbarView->UpdateToolbarState();
}

void C3PaneWindow::SetViewType(N3Pane::EViewType view)
{
	// Only bother if different
	if (mViewType == view)
		return;

	// Prevent ugly screen redraws while doing bug UI switch over
	StNoRedraw _noredraw(this);

	// Save state of current view before changing to new one
	// Only if view is valid type
	if (mViewType != N3Pane::eView_Empty)
		OptionsSaveView();

	// Set the pane contents
	mViewType = view;

	// Tell accounts view we have changed
	mAccounts->SetViewType(mViewType);
	mItems->SetViewType(mViewType);
	mPreview->SetViewType(mViewType);
	
	// Update toolbars
	mToolbarView->UpdateToolbarState();
	
	// Initialise the view state
	OptionsSetView();
}

void C3PaneWindow::SetUseSubstitute(bool subs)
{
	mItems->SetUseSubstitute(subs);
}

void C3PaneWindow::ZoomList(bool zoom)
{
	// Only bother if different
	if (mListZoom == zoom)
		return;
	mListZoom = zoom;
	if (mListZoom)
	{
		// Turn off zoom for other items
		mItemsZoom = false;
		mPreviewZoom = false;
		
		// Force focus to the list pane on zoom
		mAccounts->Focus();
	}

	// Change its state based on geometry
	UpdateView();
}

void C3PaneWindow::ZoomItems(bool zoom)
{
	// Only bother if different
	if (mItemsZoom == zoom)
		return;
	mItemsZoom = zoom;
	if (mItemsZoom)
	{
		// Turn off zoom for other items
		mListZoom = false;
		mPreviewZoom = false;
		
		// Force focus to the list pane on zoom
		mItems->Focus();
	}
	else
	{
		// May need to force focus to a valid pane
		if (!mItems->IsSpecified())
			mAccounts->Focus();
	}

	// Change its state based on geometry
	UpdateView();
}

void C3PaneWindow::ZoomPreview(bool zoom)
{
	// Only bother if different
	if (mPreviewZoom == zoom)
		return;
	mPreviewZoom = zoom;
	if (mPreviewZoom)
	{
		// Turn off zoom for other items
		mItemsZoom = false;
		mListZoom = false;
		
		// Force focus to the list pane on zoom
		mPreview->Focus();
	}
	else
	{
		// May need to force focus to a valid pane
		if (!mPreview->IsSpecified())
		{
			if (mItems->IsSpecified())
				mItems->Focus();
			else
				mAccounts->Focus();
		}
	}

	// Change its state based on geometry
	UpdateView();
}

void C3PaneWindow::ShowList(bool show)
{
	// Only bother if different
	if (mListVisible == show)
		return;
	mListVisible = show;

	// Change its state based on geometry
	UpdateView();
}

void C3PaneWindow::ShowItems(bool show)
{
	// Only bother if different
	if (mItemsVisible == show)
		return;
	mItemsVisible = show;

	// Change its state based on geometry
	UpdateView();
}

void C3PaneWindow::ShowPreview(bool show)
{
	// Only bother if different
	if (mPreviewVisible == show)
		return;
	mPreviewVisible = show;

	// Change its state based on geometry
	UpdateView();
	
	// Let the items pane know its preview is visible or not
	mItems->UsePreview(mPreviewVisible);
}

void C3PaneWindow::ShowStatus(bool show)
{
	// Only bother if different
	if (mStatusVisible == show)
		return;
	mStatusVisible = show;
}

void C3PaneWindow::UpdateView()
{
	// Determine which items are visible based on zoom and show state
	bool list_visible = (mListZoom | mListVisible) && !mItemsZoom && !mPreviewZoom;
	bool items_visible = (mItemsZoom | mItemsVisible) && !mListZoom && !mPreviewZoom;
	bool preview_visible = (mPreviewZoom | mPreviewVisible) && !mItemsZoom && !mListZoom;

	// Change its state based on geometry
	bool split11;
	bool split12;
	bool split21;
	bool split22;
	switch(mGeometry)
	{
	case N3Pane::eNone:
	case N3Pane::eListVert:
	case N3Pane::eListHoriz:
	case N3Pane::eAllVert:
	case N3Pane::eAllHoriz:
		split11 = list_visible;
		split12 = items_visible || preview_visible;
		split21 = items_visible;
		split22 = preview_visible;
		break;

	case N3Pane::eItemsVert:
		split11 = list_visible || preview_visible;
		split12 = items_visible;
		split21 = list_visible;
		split22 = preview_visible;
		break;

	case N3Pane::eItemsHoriz:
		split11 = items_visible;
		split12 = list_visible || preview_visible;
		split21 = list_visible;
		split22 = preview_visible;
		break;

	case N3Pane::ePreviewVert:
	case N3Pane::ePreviewHoriz:
		split11 = list_visible || items_visible;
		split12 = preview_visible;
		split21 = list_visible;
		split22 = items_visible;
		break;
	}

	mSplitter1->ShowView(split11, split12);
	mSplitter2->ShowView(split21, split22);
	
	// Update toolbars
	mToolbarView->UpdateToolbarState();
}

void C3PaneWindow::DoneInitMailAccounts()
{
	mItems->DoneInitMailAccounts();
}

void C3PaneWindow::DoneInitAdbkAccounts()
{
	mItems->DoneInitAdbkAccounts();
}

void C3PaneWindow::DoneInitCalendarAccounts()
{
	mItems->DoneInitCalendarAccounts();
}

void C3PaneWindow::GetOpenItems(cdstrvect& items) const
{
	mItems->GetOpenItems(items);
}

void C3PaneWindow::CloseOpenItems()
{
	mItems->CloseOpenItems();
}

void C3PaneWindow::SetOpenItems(const cdstrvect& items)
{
	mItems->SetOpenItems(items);
}

// Get the server view
CServerView* C3PaneWindow::GetServerView() const
{
	return mAccounts->GetServerView();
}

// Get the server view
CAdbkManagerView* C3PaneWindow::GetContactsView() const
{
	return mAccounts->GetContactsView();
}

// Get the server view
CCalendarStoreView* C3PaneWindow::GetCalendarStoreView() const
{
	return mAccounts->GetCalendarStoreView();
}

void C3PaneWindow::SetStatus(const cdstring& str)
{
	StGrafPortSaver graf_port_save;
	mStatus->SetText(str);
	mStatus->DontRefresh();
	mStatus->Draw(NULL);
	OutOfFocus(NULL);
}

void C3PaneWindow::SetProgress(const cdstring& str)
{
	StGrafPortSaver graf_port_save;
	mProgress->SetText(str);
	mProgress->DontRefresh();
	mProgress->Draw(NULL);
	OutOfFocus(NULL);
}

// Hide instead of close
void C3PaneWindow::AttemptClose()
{
	// Get each of its panes to acknowledge the close
	if (!GetPreviewView()->TestClose() ||
		!GetItemsView()->TestClose() ||
		!GetListView()->TestClose())
		return;
	
	// Set status
	SetClosing();

	// Save state
	SaveDefaultState();

	// Now close each pane
	GetPreviewView()->DoClose();
	GetItemsView()->DoClose();
	GetListView()->DoClose();

	// Set status
	SetClosed();
	
	// Now force the close
	delete this;
}

// Save state on quit
Boolean C3PaneWindow::AttemptQuitSelf(SInt32 inSaveOption)
{
	// Get each of its panes to acknowledge the close
	if (!GetPreviewView()->TestClose() ||
		!GetItemsView()->TestClose() ||
		!GetListView()->TestClose())
		return false;
	
	// Set status
	SetClosing();

	// Save state
	SaveDefaultState();

	// Now close each pane
	GetPreviewView()->DoClose();
	GetItemsView()->DoClose();
	GetListView()->DoClose();

	// Set status
	SetClosed();
	
	// Now force the close
	delete this;
	
	return true;
}

// Initial view from options
void C3PaneWindow::OptionsInit()
{
	// Set initial view
	SetViewType(CPreferences::sPrefs->m3PaneOptions.GetValue().GetInitialView());
}

// Set current view state from options
void C3PaneWindow::OptionsSetView()
{
	// Get options for current view type
	const C3PaneOptions::C3PaneViewOptions& options = CPreferences::sPrefs->m3PaneOptions.GetValue().GetViewOptions(mViewType);
	
	// Set the geometry
	SetGeometry(options.GetGeometry());
	
	// Set the splits
	mSplitter1->SetRelativeSplitPos(options.GetSplit1Pos());
	mSplitter2->SetRelativeSplitPos(options.GetSplit2Pos());
	
	// Set visibility (force at least list to be on - sanity check)
	ShowList(options.GetListVisible() || !options.GetItemsVisible() && !options.GetPreviewVisible());
	ShowItems(options.GetItemsVisible());
	ShowPreview(options.GetPreviewVisible());
	
	// Try to refocus on the last view with focus
	bool done_focus = false;
	switch(options.GetFocusedPanel())
	{
	case C3PaneOptions::C3PaneViewOptions::eListView:
		if (mListVisible && !mItemsZoom && !mPreviewZoom)
		{
			mAccounts->Focus();
			done_focus = true;
		}
		break;
	case C3PaneOptions::C3PaneViewOptions::eItemsView:
		if (mItemsVisible && !mListZoom && !mPreviewZoom)
		{
			mItems->Focus();
			done_focus = true;
		}
		break;
	case C3PaneOptions::C3PaneViewOptions::ePreviewView:
		if (mPreviewVisible && !mListZoom && !mItemsZoom)
		{
			mPreview->Focus();
			done_focus = true;
		}
		break;
	default:;
	}

	// Always focus on one
	if (!done_focus)
	{
		if (mListZoom)
			mAccounts->Focus();
		else if (mItemsZoom)
			mItems->Focus();
		else if (mPreviewZoom)
			mPreview->Focus();
		else if (mListVisible)
			mAccounts->Focus();
		else if (mItemsVisible)
			mItems->Focus();
		else if (mPreviewVisible)
			mPreview->Focus();
	}
}

void C3PaneWindow::OptionsSaveView()
{
	bool dirty = false;

	// Get options for current view type
	C3PaneOptions::C3PaneViewOptions& options = CPreferences::sPrefs->m3PaneOptions.Value().GetViewOptions(mViewType);
	
	// Set the geometry
	if (options.GetGeometry() != mGeometry)
	{
		options.SetGeometry(mGeometry);
		dirty = true;
	}
	
	// Set the splits
	if (options.GetSplit1Pos() != mSplitter1->GetRelativeSplitPos())
	{
		options.SetSplit1Pos(mSplitter1->GetRelativeSplitPos());
		dirty = true;
	}
	if (options.GetSplit2Pos() != mSplitter2->GetRelativeSplitPos())
	{
		options.SetSplit2Pos(mSplitter2->GetRelativeSplitPos());
		dirty = true;
	}
	
	// Set visibility
	if (options.GetListVisible() != mListVisible)
	{
		options.SetListVisible(mListVisible);
		dirty = true;
	}
	if (options.GetItemsVisible() != mItemsVisible)
	{
		options.SetItemsVisible(mItemsVisible);
		dirty = true;
	}
	if (options.GetPreviewVisible() != mPreviewVisible)
	{
		options.SetPreviewVisible(mPreviewVisible);
		dirty = true;
	}
	
	// Save current focus view
	if (mAccounts->HasFocus())
		options.SetFocusedPanel(C3PaneOptions::C3PaneViewOptions::eListView);
	else if (mItems->HasFocus())
		options.SetFocusedPanel(C3PaneOptions::C3PaneViewOptions::eItemsView);
	else if (mPreview->HasFocus())
		options.SetFocusedPanel(C3PaneOptions::C3PaneViewOptions::ePreviewView);

	// See if different and set in prefs
	if (dirty)
		CPreferences::sPrefs->m3PaneOptions.SetDirty();
}

// Reset state from prefs
void C3PaneWindow::ResetPaneState()
{
	if (mAccounts->HasFocus())
		mAccounts->ResetState();
	else if (mItems->HasFocus())
		mItems->ResetState();
	else if (mPreview->HasFocus())
		mPreview->ResetState();
}

// Save current state in prefs
void C3PaneWindow::SaveDefaultPaneState()
{
	if (mAccounts->HasFocus())
		mAccounts->SaveDefaultState();
	else if (mItems->HasFocus())
		mItems->SaveDefaultState();
	else if (mPreview->HasFocus())
		mPreview->SaveDefaultState();
}

// Reset state from prefs
void C3PaneWindow::ResetState(bool force)
{
	// Get default state
	C3PaneWindowState* state = &CPreferences::sPrefs->m3PaneWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->m3PaneWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}

	// Status state is global
	ShowStatus(state->GetShowStatus());
	
	// Set toolbar visible states
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set zoom state
	if (state->GetState() == eWindowStateMax)
		DoSetZoom(true);

	// Reset state for standard view - do after setting zoom state to avoid
	// splitter position changes if zoom is triggered
	OptionsSetView();

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void C3PaneWindow::SaveDefaultState()
{
	// Always save the current options state
	OptionsSaveView();

	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	C3PaneWindowState state(NULL, &mUserBounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								mGeometry, mListVisible, mItemsVisible, mPreviewVisible, mStatusVisible,
								mSplitter1->GetRelativeSplitPos(), mSplitter2->GetRelativeSplitPos(),
								mToolbarView->IsVisible(CToolbarView::eStdButtonsGroup));
	if (CPreferences::sPrefs->m3PaneWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->m3PaneWindowDefault.SetDirty();
}
