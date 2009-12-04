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
#include "CMenuBar.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CServerView.h"
#include "CStaticText.h"
#include "CSplitterView.h"
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

#include "HResourceMap.h"

#include <JXColormap.h>
#include <JXMenuBar.h>
#include <JXStaticText.h>
#include <JXWindow.h>

#include <cassert>

// Static

// Static members

C3PaneWindow* C3PaneWindow::s3PaneWindow = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneWindow::C3PaneWindow(JXDirector* owner)
	: CWindow(owner)
{
	Init3PaneWindow();
}

// Default destructor
C3PaneWindow::~C3PaneWindow()
{
	// Unhook previews from their owners as order of destruction can lead to crashes if this is not done
	mAccounts->GetServerView()->SetPreview(NULL);
	mAccounts->GetContactsView()->SetPreview(NULL);
	if (!CAdminLock::sAdminLock.mPreventCalendars)
		mAccounts->GetCalendarStoreView()->SetPreview(NULL);
	
	mItems->GetMailboxView()->SetPreview(NULL);
	mItems->GetAddressBookView()->SetPreview(NULL);
	if (!CAdminLock::sAdminLock.mPreventCalendars)
		mItems->GetCalendarView()->SetPreview(NULL);

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

	mDoQuitOnClose = true;

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
		C3PaneWindow* wnd = new C3PaneWindow(CMulberryApp::sApp);
		wnd->OnCreate();
		wnd->ResetState();
		wnd->Activate();
		
		// Can now dispose of any existing status window
		CStatusWindow::DestroyWindow();
	}
}

void C3PaneWindow::Destroy3PaneWindow()
{
	if (s3PaneWindow)
	{
		s3PaneWindow->AttemptCloseSelf();
		s3PaneWindow->mDoQuitOnClose = false;
		FRAMEWORK_DELETE_WINDOW(s3PaneWindow)
	}
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cWindowWidth = 500;
const int cWindowHeight = 400;
const int cToolbarTop = CMenuBar::cMenuBarHeight - 2;
const int cToolbarHeight = 56;
const int cStatusHeight = 15;
const int cSplitterTop = cToolbarTop + cToolbarHeight;
const int cSplitterHeight = cWindowHeight - cSplitterTop - cStatusHeight;

// Do various bits
void C3PaneWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,400, "Mulberry");
    assert( window != NULL );
    SetWindow(window);

    JXStaticText* obj1 =
        new JXStaticText("Status:", window,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 0,385, 55,15);
    assert( obj1 != NULL );
    obj1->SetFontSize(10);
    const JFontStyle obj1_style(kTrue, kFalse, 0, kFalse, (GetColormap())->GetBlackColor());
    obj1->SetFontStyle(obj1_style);

    mStatus =
        new CStaticText("Idle", window,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 60,385, 240,15);
    assert( mStatus != NULL );

    mProgress =
        new CStaticText("", window,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 300,385, 60,15);
    assert( mProgress != NULL );

// end JXLayout

	// Create toolbar pane
	mToolbarView = new CToolbarView(window, JXWidget::kHElastic, JXWidget::kFixedTop, 0, cToolbarTop, cWindowWidth, cToolbarHeight);
	mToolbarView->OnCreate();

	// Deal with splitter
	JArray<JCoordinate> heights;
	JArray<JCoordinate> minHeights;
	heights.InsertElementAtIndex(1, cSplitterHeight/2);
	heights.InsertElementAtIndex(2, cSplitterHeight/2);
	minHeights.InsertElementAtIndex(1, 64);
	minHeights.InsertElementAtIndex(2, 64);

	mSplitter1 = new CSplitterView(heights, 1, minHeights, window,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);
	mSplitter2 = new CSplitterView(heights, 1, minHeights, window,
										JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);

	mListView = new C3PaneMainPanel(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);
	mListView->OnCreate();
	mItemsView = new C3PaneMainPanel(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);
	mItemsView->OnCreate();
	mPreviewView = new C3PaneMainPanel(window, JXWidget::kHElastic, JXWidget::kVElastic, 0, cSplitterTop, cWindowWidth, cSplitterHeight);
	mPreviewView->OnCreate();

	// Setup toolbar
	mToolbarView->SetSibling(mSplitter1);
	InstallToolbars();

	// Make the child views - must do after creating toolbars as the views need them
	InstallViews();

	// Now set the initial geometry
	SetGeometry(N3Pane::eListVert);

	// Starts off as mailbox view
	SetViewType(N3Pane::eView_Mailbox);

	// Do inherited
	CWindow::OnCreate();

	CreateMainMenu(window, CMainMenu::fFile | 
								 CMainMenu::fEdit | 
								 CMainMenu::fMailboxes | 
								 CMainMenu::fMessages | 
								 CMainMenu::fDraft | 
								 CMainMenu::fAddresses | 
								 CMainMenu::fCalendar | 
								 CMainMenu::fWindows | 
								 CMainMenu::fHelp);

	// Set current width and height as minimum
	GetWindow()->SetMinSize(GetWindow()->GetBoundsGlobal().width(),
							GetWindow()->GetBoundsGlobal().height());
	GetWindow()->SetWMClass(c3PaneWMClass, cMulberryWMClass);
	GetWindow()->SetIcon(iconFromResource(IDR_MAINFRAME, GetWindow(), 32, 0x00CCCCCC));

	// Set status
	SetOpen();

}

// Handle key presses
bool C3PaneWindow::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	// Get state of command modifiers (not shift)
	bool mods = modifiers.shift() || modifiers.control() || modifiers.alt() || modifiers.meta();

	unsigned long cmd = 0;
	switch (key)
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
			if (key == '9')
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
		CCmdUI cmdui;
		cmdui.mCmd = cmd;
		
		// Only if commander available
		UpdateCommand(cmd, &cmdui);
		
		// Execute if enabled
		if (cmdui.GetEnabled())
			ObeyCommand(cmd, NULL);

		return true;
	}

	// Do default key press processing
	return CCommander::HandleChar(key, modifiers);
}

//	Respond to commands
bool C3PaneWindow::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eLayoutListVert:
		SetGeometry(N3Pane::eListVert);
		return true;
	case CCommand::eLayoutListHoriz:
		SetGeometry(N3Pane::eListHoriz);
		return true;
	case CCommand::eLayoutItemsVert:
		SetGeometry(N3Pane::eItemsVert);
		return true;
	case CCommand::eLayoutItemsHoriz:
		SetGeometry(N3Pane::eItemsHoriz);
		return true;
	case CCommand::eLayoutPreviewVert:
		SetGeometry(N3Pane::ePreviewVert);
		return true;
	case CCommand::eLayoutPreviewHoriz:
		SetGeometry(N3Pane::ePreviewHoriz);
		return true;
	case CCommand::eLayoutAllVert:
		SetGeometry(N3Pane::eAllVert);
		return true;
	case CCommand::eLayoutAllHoriz:
		SetGeometry(N3Pane::eAllHoriz);
		return true;

	case CCommand::eToolbarZoomList:
		ZoomList(!mListZoom);
		return true;

	case CCommand::eToolbarZoomItems:
		ZoomItems(!mItemsZoom);
		return true;

	case CCommand::eToolbarZoomPreview:
		ZoomPreview(!mPreviewZoom);
		return true;

	case CCommand::eWindowsShowList:
	case CCommand::eToolbarShowList:
		ShowList(!mListVisible);
		return true;

	case CCommand::eWindowsShowItems:
	case CCommand::eToolbarShowItems:
		ShowItems(!mItemsVisible);
		return true;

	case CCommand::eWindowsShowPreview:
	case CCommand::eToolbarShowPreview:
		ShowPreview(!mPreviewVisible);
		return true;

	case CCommand::eWindowsSaveDefault:
		SaveDefaultPaneState();
		return true;

	case CCommand::eWindowsResetToDefault:
		ResetPaneState();
		return true;

	default:;
	}

	return CWindow::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void C3PaneWindow::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eLayoutListVert:
	case CCommand::eLayoutListHoriz:
	case CCommand::eLayoutItemsVert:
	case CCommand::eLayoutItemsHoriz:
	case CCommand::eLayoutPreviewVert:
	case CCommand::eLayoutPreviewHoriz:
	case CCommand::eLayoutAllVert:
	case CCommand::eLayoutAllHoriz:
		OnUpdateWindowLayout(cmdui);
		return;

	case CCommand::eWindowsShowList:
		OnUpdateMenuShowList(cmdui);
		return;

	case CCommand::eWindowsShowItems:
		OnUpdateMenuShowItems(cmdui);
		return;

	case CCommand::eWindowsShowPreview:
		OnUpdateMenuShowPreview(cmdui);
		return;

	case CCommand::eToolbarShowList:
		OnUpdateShowList(cmdui);
		return;

	case CCommand::eToolbarShowItems:
		OnUpdateShowItems(cmdui);
		return;

	case CCommand::eToolbarShowPreview:
		OnUpdateShowPreview(cmdui);
		return;

	case CCommand::eToolbarZoomList:
		OnUpdateZoomList(cmdui);
		return;

	case CCommand::eToolbarZoomItems:
		OnUpdateZoomItems(cmdui);
		return;

	case CCommand::eToolbarZoomPreview:
		OnUpdateZoomPreview(cmdui);
		return;

	case CCommand::eWindowsSaveDefault:
	case CCommand::eWindowsResetToDefault:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	default:;
	}

	CWindow::UpdateCommand(cmd, cmdui);
}

void C3PaneWindow::InstallToolbars()
{
	// Create toolbars for each type of view
	mMailboxToolbar = new C3PaneMailboxToolbar(mToolbarView, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
	mMailboxToolbar->OnCreate();
	mToolbarView->AddToolbar(mMailboxToolbar, this, CToolbarView::eStdButtonsGroup);

	mAdbkToolbar = new 	C3PaneAdbkToolbar(mToolbarView, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
	mAdbkToolbar->OnCreate();
	mToolbarView->AddToolbar(mAdbkToolbar, this, CToolbarView::eStdButtonsGroup);

	mCalendarToolbar = new 	C3PaneCalendarToolbar(mToolbarView, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
	mCalendarToolbar->OnCreate();
	mToolbarView->AddToolbar(mCalendarToolbar, this, CToolbarView::eStdButtonsGroup);
}

void C3PaneWindow::InstallViews()
{
	// Read the account view resource
	mAccounts = new C3PaneAccounts(GetListView(), JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 300, 200);
	mAccounts->OnCreate();
	GetListView()->SetSubView(mAccounts);
	mAccounts->FitToEnclosure(kTrue, kTrue);

	// Read the item view resource
	mItems = new C3PaneItems(GetItemsView(), JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 300, 200);
	mItems->OnCreate();
	GetItemsView()->SetSubView(mItems);
	mItems->FitToEnclosure(kTrue, kTrue);

	// Read the preview resource
	mPreview = new C3PanePreview(GetPreviewView(), JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 300, 200);
	mPreview->OnCreate();
	GetPreviewView()->SetSubView(mPreview);
	mPreview->FitToEnclosure(kTrue, kTrue);

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
	StNoRedraw _noredraw(mSplitter1);

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

void C3PaneWindow::Activate()
{
	CWindow::Activate();

	// Force focus to last active panel
	if (C3PaneParentPanel::sCurrentFocus)
		C3PaneParentPanel::sCurrentFocus->Focus();
}

#pragma mark ____________________________Command Updaters

void C3PaneWindow::OnUpdateZoomList(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Check if current zoom
	pCmdUI->SetCheck(mListZoom);
}

void C3PaneWindow::OnUpdateZoomItems(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Check if current zoom
	pCmdUI->SetCheck(mItemsZoom);
}

void C3PaneWindow::OnUpdateZoomPreview(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Check if current zoom
	pCmdUI->SetCheck(mPreviewZoom);
}

void C3PaneWindow::OnUpdateMenuShowList(CCmdUI* pCmdUI)
{
	// Enabled if one other visible
	pCmdUI->Enable(mItemsVisible || mPreviewVisible);

	// Set string based on state
	cdstring s;
	s.FromResource(mListVisible ? IDS_WINDOWSHIDELIST : IDS_WINDOWSSHOWLIST);
	pCmdUI->SetText(s);
}

void C3PaneWindow::OnUpdateShowList(CCmdUI* pCmdUI)
{
	// Enabled if one other visible
	pCmdUI->Enable(mItemsVisible || mPreviewVisible);

	// Check if current zoom
	pCmdUI->SetCheck(mListVisible);
}

void C3PaneWindow::OnUpdateMenuShowItems(CCmdUI* pCmdUI)
{
	// Enabled if one other visible
	pCmdUI->Enable(mListVisible || mPreviewVisible);

	// Set string based on state
	cdstring s;
	s.FromResource(mItemsVisible ? IDS_WINDOWSHIDEITEMS : IDS_WINDOWSSHOWITEMS);
	pCmdUI->SetText(s);
}

void C3PaneWindow::OnUpdateShowItems(CCmdUI* pCmdUI)
{
	// Enabled if one other visible
	pCmdUI->Enable(mListVisible || mPreviewVisible);

	// Check if current zoom
	pCmdUI->SetCheck(mItemsVisible);
}

void C3PaneWindow::OnUpdateMenuShowPreview(CCmdUI* pCmdUI)
{
	// Enabled if one other visible
	pCmdUI->Enable(mItemsVisible || mListVisible);

	// Set string based on state
	cdstring s;
	s.FromResource(mPreviewVisible ? IDS_WINDOWSHIDEPREVIEW : IDS_WINDOWSSHOWPREVIEW);
	pCmdUI->SetText(s);
}

void C3PaneWindow::OnUpdateShowPreview(CCmdUI* pCmdUI)
{
	// Enabled if one other visible
	pCmdUI->Enable(mItemsVisible || mListVisible);

	// Check if current zoom
	pCmdUI->SetCheck(mPreviewVisible);
}

void C3PaneWindow::OnUpdateWindowLayout(CCmdUI* pCmdUI)
{
	// Always enabled
	pCmdUI->Enable(true);

	// Check if current layout
	bool check = false;
	switch(pCmdUI->mCmd)
	{
	case CCommand::eLayoutListVert:
	default:
		check = (mGeometry == N3Pane::eListVert);
		break;
	case CCommand::eLayoutListHoriz:
		check = (mGeometry == N3Pane::eListHoriz);
		break;
	case CCommand::eLayoutItemsVert:
		check = (mGeometry == N3Pane::eItemsVert);
		break;
	case CCommand::eLayoutItemsHoriz:
		check = (mGeometry == N3Pane::eItemsHoriz);
		break;
	case CCommand::eLayoutPreviewVert:
		check = (mGeometry == N3Pane::ePreviewVert);
		break;
	case CCommand::eLayoutPreviewHoriz:
		check = (mGeometry == N3Pane::ePreviewHoriz);
		break;
	case CCommand::eLayoutAllVert:
		check = (mGeometry == N3Pane::eAllVert);
		break;
	case CCommand::eLayoutAllHoriz:
		check = (mGeometry == N3Pane::eAllHoriz);
		break;
	}
	pCmdUI->SetCheck(check);
}

#pragma mark ____________________________Commands

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
	mStatus->SetText(str);
	mStatus->Redraw();
}

void C3PaneWindow::SetProgress(const cdstring& str)
{
	mProgress->SetText(str);
	mProgress->Redraw();
}

// Force quit after user closes it
bool C3PaneWindow::AttemptClose()
{
	bool result = AttemptCloseSelf();
	if (result && mDoQuitOnClose && CMulberryApp::sApp && !CMulberryApp::sApp->mQuitting)
		// Force Mulberry to quit
		CMulberryApp::sApp->Quit();
	
	return result;	
}

// Hide instead of close
bool C3PaneWindow::AttemptCloseSelf()
{
	if (!IsOpen())
		return true;

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
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->m3PaneWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset bounds
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// Status state is global
	ShowStatus(state->GetShowStatus());
	
	// Set toolbar visible states
	mToolbarView->ShowToolbar(CToolbarView::eStdButtonsGroup, CPreferences::sPrefs->mToolbarShow.GetValue());

	// Set zoom state
	//if (state->GetState() == eWindowStateMax)
	//	DoSetZoom(true);

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
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Add info to prefs
	C3PaneWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								mGeometry, mListVisible, mItemsVisible, mPreviewVisible, mStatusVisible,
								mSplitter1->GetRelativeSplitPos(), mSplitter2->GetRelativeSplitPos(),
								mToolbarView->IsVisible(CToolbarView::eStdButtonsGroup));
	if (CPreferences::sPrefs->m3PaneWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->m3PaneWindowDefault.SetDirty();
}
