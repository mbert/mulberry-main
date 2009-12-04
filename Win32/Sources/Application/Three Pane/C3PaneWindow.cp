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
#include "CAdminLock.h"
#include "CCalendarStoreView.h"
#include "CCalendarView.h"
#include "CCmdUIChecker.h"
#include "CCommander.h"
#include "CLog.h"
#include "CMailboxInfoView.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CServerView.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneMailboxToolbar.h"
#include "CWindowOptionsDialog.h"

IMPLEMENT_DYNCREATE(C3PaneWindow, CView)

BEGIN_MESSAGE_MAP(C3PaneWindow, CView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()

	ON_UPDATE_COMMAND_UI(ID_FILE_CLOSE, OnUpdateNever)

	ON_UPDATE_COMMAND_UI(IDC_3PANETOOLBAR_ZOOMLIST, OnUpdateZoomList)
	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMLIST, OnZoomList)
	ON_UPDATE_COMMAND_UI(IDC_3PANETOOLBAR_ZOOMITEMS, OnUpdateZoomItems)
	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMITEMS, OnZoomItems)
	ON_UPDATE_COMMAND_UI(IDC_3PANETOOLBAR_ZOOMPREVIEW, OnUpdateZoomPreview)
	ON_COMMAND(IDC_3PANETOOLBAR_ZOOMPREVIEW, OnZoomPreview)

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOWLIST, OnUpdateMenuShowList)
	ON_COMMAND(IDM_WINDOWS_SHOWLIST, OnShowList)
	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOWITEMS, OnUpdateMenuShowItems)
	ON_COMMAND(IDM_WINDOWS_SHOWITEMS, OnShowItems)
	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOWPREVIEW, OnUpdateMenuShowPreview)
	ON_COMMAND(IDM_WINDOWS_SHOWPREVIEW, OnShowPreview)

	ON_COMMAND(IDM_WINDOWS_DEFAULT, OnSaveDefaultPaneState)
	ON_COMMAND(IDM_WINDOWS_RESET, OnResetPaneState)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_LAYOUT_LISTVERT, IDM_LAYOUT_ALLHORIZ, OnUpdateWindowLayout)
	ON_COMMAND_RANGE(IDM_LAYOUT_LISTVERT, IDM_LAYOUT_ALLHORIZ, OnWindowLayout)

	// Toolbar

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSHOWLIST, OnUpdateShowList)
	ON_COMMAND(IDC_TOOLBARSHOWLIST, OnShowList)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSHOWITEMS, OnUpdateShowItems)
	ON_COMMAND(IDC_TOOLBARSHOWITEMS, OnShowItems)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBARSHOWPREVIEW, OnUpdateShowPreview)
	ON_COMMAND(IDC_TOOLBARSHOWPREVIEW, OnShowPreview)

END_MESSAGE_MAP()

// Static

// Static members

C3PaneWindow* C3PaneWindow::s3PaneWindow = NULL;
CMultiDocTemplate* C3PaneWindow::s3PaneDocTemplate = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneWindow::C3PaneWindow()
{
	Init3PaneWindow();
}

// Default destructor
C3PaneWindow::~C3PaneWindow()
{
	// Unhook previews from their owners as order of destruction can lead to crashes if this is not done
	mAccounts.GetServerView()->SetPreview(NULL);
	mAccounts.GetContactsView()->SetPreview(NULL);
	if (!CAdminLock::sAdminLock.mPreventCalendars)
		mAccounts.GetCalendarStoreView()->SetPreview(NULL);
	
	mItems.GetMailboxView()->SetPreview(NULL);
	mItems.GetAddressBookView()->SetPreview(NULL);
	if (!CAdminLock::sAdminLock.mPreventCalendars)
		mItems.GetCalendarView()->SetPreview(NULL);

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
		C3PaneWindow* wnd = C3PaneWindow::ManualCreate();
		wnd->ResetState();
		
		// Force pane focus when first opened
		wnd->OnSetFocus(NULL);
	}
}

void C3PaneWindow::Destroy3PaneWindow()
{
	if (s3PaneWindow)
		s3PaneWindow->DoClose();
	FRAMEWORK_DELETE_WINDOW(s3PaneWindow)
	s3PaneWindow = NULL;
}

// Manually create document
C3PaneWindow* C3PaneWindow::ManualCreate(void)
{
	CDocument* aDoc = s3PaneDocTemplate->OpenDocumentFile(nil, false);
	aDoc->SetTitle(_T(""));
	POSITION pos = aDoc->GetFirstViewPosition();
	CView* pView = aDoc->GetNextView(pos);
	CFrameWnd* aFrame = pView->GetParentFrame();

	// Get view
	CWnd* pWnd = aFrame->GetDescendantWindow(AFX_IDW_PANE_FIRST, TRUE);
	if (pWnd != nil && pWnd->IsKindOf(RUNTIME_CLASS(C3PaneWindow)))
		return (C3PaneWindow*) pWnd;

	CLOG_LOGTHROW(CGeneralException, -1L);
	throw CGeneralException(-1L);
	return nil;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

int C3PaneWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;
	ModifyStyleEx(WS_EX_CLIENTEDGE, 0);

	const int small_offset = CMulberryApp::sLargeFont ? 4 : 0;
	const int large_offset = CMulberryApp::sLargeFont ? 8 : 0;

	int width = 512;
	int height = 384;

	// Create toolbar pane
	const int cToolbarHeight = 56;
	CRect rect = CRect(0, 0, width, cToolbarHeight + large_offset);
	mToolbarView.Create(TOOLBARCLASSNAME, NULL, WS_CHILD | WS_VISIBLE | CCS_NORESIZE, rect, GetParentFrame(), IDC_STATIC);
	mToolbarView.SetBarStyle(CBRS_ALIGN_TOP | CBRS_BORDER_TOP);
	mToolbarView.ShowDivider(true);

	// Create splitter view
	rect = CRect(0, 0, width, height - cToolbarHeight);
	mSplitter1.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	mSplitter2.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	mListView.Create(_T(""), WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	mItemsView.Create(_T(""), WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);
	mPreviewView.Create(_T(""), WS_CHILD | WS_VISIBLE, rect, this, IDC_STATIC);

	// Create toolbars
	mToolbarView.SetSibling(&mSplitter1);
	InstallToolbars();

	// Make the child views - must do after creating toolbars as the views need them
	InstallViews();

	// Now set the initial geometry
	SetGeometry(N3Pane::eListVert);

	// Starts off as mailbox view
	SetViewType(N3Pane::eView_Mailbox);

	// Make server panel the focus - focus will take place when this
	// window is actually displayed
	C3PaneParentPanel::sCurrentFocus = &mAccounts;

	// Set status
	SetOpen();

	return 0;
}

// Destroy
void C3PaneWindow::OnDestroy()
{
	SaveDefaultState();
	CView::OnDestroy();
}

// Resize sub-views
void C3PaneWindow::OnSize(UINT nType, int cx, int cy)
{
	// Resize its splitter view to fit
	if (mSplitter1.m_hWnd != NULL)
	{
		CRect rect(0, 0, cx, cy);
		mSplitter1.MoveWindow(rect);
	}
	
	// Don't do immediate inherit as that changes the size of the sub-view
	// which is not waht we want to change
	CView::OnSize(nType, cx, cy);
}

// Tell window to focus on this one
void C3PaneWindow::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to last active panel
	if (C3PaneParentPanel::sCurrentFocus)
		C3PaneParentPanel::sCurrentFocus->Focus();
}

BOOL C3PaneWindow::OnCmdMsg(UINT nID, int nCode, void* pExtra,
      AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route command to current target
	if (CCommander::ProcessCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return true;

	// If the object(s) in the extended command route don't handle
	// the command, then let the base class OnCmdMsg handle it.
	return CView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

#pragma mark ____________________________Command Updaters

void C3PaneWindow::OnUpdateNever(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(false);	// Never
}

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
	CString s;
	s.LoadString(mListVisible ? IDS_WINDOWSHIDELIST : IDS_WINDOWSSHOWLIST);
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
	CString s;
	s.LoadString(mItemsVisible ? IDS_WINDOWSHIDEITEMS : IDS_WINDOWSSHOWITEMS);
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
	CString s;
	s.LoadString(mPreviewVisible ? IDS_WINDOWSHIDEPREVIEW : IDS_WINDOWSSHOWPREVIEW);
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
	switch(pCmdUI->m_nID)
	{
	case IDM_LAYOUT_LISTVERT:
	default:
		check = (mGeometry == N3Pane::eListVert);
		break;
	case IDM_LAYOUT_LISTHORIZ:
		check = (mGeometry == N3Pane::eListHoriz);
		break;
	case IDM_LAYOUT_ITEMSVERT:
		check = (mGeometry == N3Pane::eItemsVert);
		break;
	case IDM_LAYOUT_ITEMSHORIZ:
		check = (mGeometry == N3Pane::eItemsHoriz);
		break;
	case IDM_LAYOUT_PREVIEWVERT:
		check = (mGeometry == N3Pane::ePreviewVert);
		break;
	case IDM_LAYOUT_PREVIEWHORIZ:
		check = (mGeometry == N3Pane::ePreviewHoriz);
		break;
	case IDM_LAYOUT_ALLVERT:
		check = (mGeometry == N3Pane::eAllVert);
		break;
	case IDM_LAYOUT_ALLHORIZ:
		check = (mGeometry == N3Pane::eAllHoriz);
		break;
	}
	pCmdUI->SetCheck(check);
}

#pragma mark ____________________________Commands

// Handle key down
bool C3PaneWindow::HandleChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	// Get state of command modifiers (not shift)
	bool mods = ((::GetKeyState(VK_CONTROL) < 0) || (::GetKeyState(VK_SHIFT) < 0) || (::GetKeyState(VK_MENU) < 0));

	UINT cmd = 0;
	switch(nChar)
	{
	// Key shortcuts
	case '1':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			if (mListVisible)
				mAccounts.Focus();
			return true;
		}
		break;
	case '2':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			if (mItemsVisible && mItems.IsSpecified())
				mItems.Focus();
			return true;
		}
		break;
	case '3':
		if (CPreferences::sPrefs->mAllowKeyboardShortcuts.GetValue() && !mods)
		{
			if (mPreviewVisible && mPreview.IsSpecified())
				mPreview.Focus();
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
			C3PaneParentPanel* focus[3] = {&mAccounts, &mItems, &mPreview};
			bool visible[3] = {mListVisible, mItemsVisible && mItems.IsSpecified(), mPreviewVisible && mPreview.IsSpecified()};
			
			// Get currently focussed pane index
			unsigned long index = 0;
			if (mAccounts.HasFocus())
				index = 3;
			else if (mItems.HasFocus())
				index = 4;
			else if (mPreview.HasFocus())
				index = 5;
			
			// Adjust index to prev/next visible item and focus it
			if (nChar == '9')
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
		CCmdUIChecker cmdui;
		cmdui.m_nID = cmd;
		
		// Only if commander available
		cmdui.DoUpdate(this, true);
		
		// Execute if enabled
		if (cmdui.GetEnabled())
			SendMessage(WM_COMMAND, cmd);

		return true;
	}

	// Did not handle key
	return CCommander::HandleChar(nChar, nRepCnt, nFlags);
}

void C3PaneWindow::OnZoomList()
{
	ZoomList(!mListZoom);
}

void C3PaneWindow::OnZoomItems()
{
	ZoomItems(!mItemsZoom);
}

void C3PaneWindow::OnZoomPreview()
{
	ZoomPreview(!mPreviewZoom);
}

void C3PaneWindow::OnShowList()
{
	ShowList(!mListVisible);
}

void C3PaneWindow::OnShowItems()
{
	ShowItems(!mItemsVisible);
}

void C3PaneWindow::OnShowPreview()
{
	ShowPreview(!mPreviewVisible);
}

void C3PaneWindow::OnWindowLayout(UINT nID)
{
	switch(nID)
	{
	case IDM_LAYOUT_LISTVERT:
	default:
		SetGeometry(N3Pane::eListVert);
		break;
	case IDM_LAYOUT_LISTHORIZ:
		SetGeometry(N3Pane::eListHoriz);
		break;
	case IDM_LAYOUT_ITEMSVERT:
		SetGeometry(N3Pane::eItemsVert);
		break;
	case IDM_LAYOUT_ITEMSHORIZ:
		SetGeometry(N3Pane::eItemsHoriz);
		break;
	case IDM_LAYOUT_PREVIEWVERT:
		SetGeometry(N3Pane::ePreviewVert);
		break;
	case IDM_LAYOUT_PREVIEWHORIZ:
		SetGeometry(N3Pane::ePreviewHoriz);
		break;
	case IDM_LAYOUT_ALLVERT:
		SetGeometry(N3Pane::eAllVert);
		break;
	case IDM_LAYOUT_ALLHORIZ:
		SetGeometry(N3Pane::eAllHoriz);
		break;
	}
}

#pragma mark ____________________________Behaviours

void C3PaneWindow::InstallToolbars()
{
	// Create toolbars for each type of view
	mMailboxToolbar = new C3PaneMailboxToolbar;
	mMailboxToolbar->InitToolbar(true, &mToolbarView);
	mToolbarView.AddToolbar(mMailboxToolbar, NULL, CToolbarView::eStdButtonsGroup);

	mAdbkToolbar = new C3PaneAdbkToolbar;
	mAdbkToolbar->InitToolbar(true, &mToolbarView);
	mToolbarView.AddToolbar(mAdbkToolbar, NULL, CToolbarView::eStdButtonsGroup);

	mCalendarToolbar = new C3PaneCalendarToolbar;
	mCalendarToolbar->InitToolbar(true, &mToolbarView);
	mToolbarView.AddToolbar(mCalendarToolbar, NULL, CToolbarView::eStdButtonsGroup);
}

void C3PaneWindow::InstallViews()
{
	// Read the account view resource
	mAccounts.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 256, 256), GetListView(), IDC_STATIC);
	GetListView()->SetSubView(&mAccounts);

	// Read the item view resource
	mItems.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 256, 256), GetItemsView(), IDC_STATIC);
	GetItemsView()->SetSubView(&mItems);

	// Read the preview resource
	mPreview.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, 256, 256), GetPreviewView(), IDC_STATIC);
	GetPreviewView()->SetSubView(&mPreview);

	// Hook up listeners

	// Give mailbox preview to server view
	mAccounts.GetServerView()->SetPreview(mItems.GetMailboxView());
	
	// Give server view to mailbox view
	mItems.GetMailboxView()->SetOwnerView(mAccounts.GetServerView());

	// Give message preview to mailbox view
	mItems.GetMailboxView()->SetPreview(mPreview.GetMessageView());
	
	// Give mailbox preview to message view
	mPreview.GetMessageView()->SetOwnerView(mItems.GetMailboxView());

	// Give address book preview to contacts view
	mAccounts.GetContactsView()->SetPreview(mItems.GetAddressBookView());
	
	// Give message preview to mailbox view
	mItems.GetAddressBookView()->SetPreview(mPreview.GetAddressView());

	// Don't do if admin locks it out
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		// Give calendar preview to calendar store view
		mAccounts.GetCalendarStoreView()->SetPreview(mItems.GetCalendarView());
		C3PaneWindow::s3PaneWindow->DoneInitCalendarAccounts();
		
		// Give calendar item preview to calendar view
		mItems.GetCalendarView()->SetPreview(mPreview.GetEventView());
	}
}

void C3PaneWindow::SetGeometry(N3Pane::EGeometry geometry)
{
	// Only bother if different
	if (mGeometry == geometry)
		return;

	// Lock out updates to prevent flashes
	BOOL locked = LockWindowUpdate();
	
	// Hide main splitter first
	mSplitter1.ShowWindow(SW_HIDE);

	// Move views around as appropriate
	mGeometry = geometry;
	switch(mGeometry)
	{
	case N3Pane::eListVert:
	case N3Pane::eNone:
		mSplitter1.InstallViews(GetListView(), &mSplitter2, false);
		mSplitter2.InstallViews(GetItemsView(), GetPreviewView(), true);
		break;

	case N3Pane::eListHoriz:
		mSplitter1.InstallViews(GetListView(), &mSplitter2, true);
		mSplitter2.InstallViews(GetItemsView(), GetPreviewView(), false);
		break;

	case N3Pane::eItemsVert:
		mSplitter1.InstallViews(&mSplitter2, GetItemsView(), false);
		mSplitter2.InstallViews(GetListView(), GetPreviewView(), true);
		break;

	case N3Pane::eItemsHoriz:
		mSplitter1.InstallViews(GetItemsView(), &mSplitter2, true);
		mSplitter2.InstallViews(GetListView(), GetPreviewView(), false);
		break;

	case N3Pane::ePreviewVert:
		mSplitter1.InstallViews(&mSplitter2, GetPreviewView(), false);
		mSplitter2.InstallViews(GetListView(), GetItemsView(), true);
		break;

	case N3Pane::ePreviewHoriz:
		mSplitter1.InstallViews(&mSplitter2, GetPreviewView(), true);
		mSplitter2.InstallViews(GetListView(), GetItemsView(), false);
		break;

	case N3Pane::eAllVert:
		mSplitter1.InstallViews(GetListView(), &mSplitter2, false);
		mSplitter2.InstallViews(GetItemsView(), GetPreviewView(), false);
		break;

	case N3Pane::eAllHoriz:
		mSplitter1.InstallViews(GetListView(), &mSplitter2, true);
		mSplitter2.InstallViews(GetItemsView(), GetPreviewView(), true);
		break;

	};

	// Show main splitter first
	mSplitter1.ShowWindow(SW_SHOW);
	
	// Update toolbars
	mToolbarView.UpdateToolbarState();

	if (locked)
	{
		UnlockWindowUpdate();
		RedrawWindow();
	}
}

void C3PaneWindow::SetViewType(N3Pane::EViewType view)
{
	// Only bother if different
	if (mViewType == view)
		return;

	// Lock out updates to prevent flashes
	BOOL locked = LockWindowUpdate();
	
	// Save state of current view before changing to new one
	// Only if view is valid type
	if (mViewType != N3Pane::eView_Empty)
		OptionsSaveView();

	// Set the pane contents
	mViewType = view;

	// Tell accounts view we have changed
	mAccounts.SetViewType(mViewType);
	mItems.SetViewType(mViewType);
	mPreview.SetViewType(mViewType);
	
	// Update toolbars
	mToolbarView.UpdateToolbarState();
	
	// Initialise the view state
	OptionsSetView();

	if (locked)
	{
		UnlockWindowUpdate();
		RedrawWindow();
	}
}

void C3PaneWindow::SetUseSubstitute(bool subs)
{
	mItems.SetUseSubstitute(subs);
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
		mAccounts.Focus();
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
		mItems.Focus();
	}
	else
	{
		// May need to force focus to a valid pane
		if (!mItems.IsSpecified())
			mAccounts.Focus();
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
		mPreview.Focus();
	}
	else
	{
		// May need to force focus to a valid pane
		if (!mPreview.IsSpecified())
		{
			if (mItems.IsSpecified())
				mItems.Focus();
			else
				mAccounts.Focus();
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
	mItems.UsePreview(mPreviewVisible);
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

	// Lock out updates to prevent flashes
	BOOL locked = LockWindowUpdate();
	
	// Change splitter states
	mSplitter1.ShowView(split11, split12);
	mSplitter2.ShowView(split21, split22);
	
	// Update toolbars
	mToolbarView.UpdateToolbarState();

	if (locked)
	{
		UnlockWindowUpdate();
		RedrawWindow();
	}
}

void C3PaneWindow::DoneInitMailAccounts()
{
	mItems.DoneInitMailAccounts();
}

void C3PaneWindow::DoneInitAdbkAccounts()
{
	mItems.DoneInitAdbkAccounts();
}

void C3PaneWindow::DoneInitCalendarAccounts()
{
	mItems.DoneInitCalendarAccounts();
}

void C3PaneWindow::GetOpenItems(cdstrvect& items) const
{
	mItems.GetOpenItems(items);
}

void C3PaneWindow::CloseOpenItems()
{
	mItems.CloseOpenItems();
}

void C3PaneWindow::SetOpenItems(const cdstrvect& items)
{
	mItems.SetOpenItems(items);
}

// Get the server view
CServerView* C3PaneWindow::GetServerView() const
{
	return mAccounts.GetServerView();
}

// Get the server view
CAdbkManagerView* C3PaneWindow::GetContactsView() const
{
	return mAccounts.GetContactsView();
}

// Get the server view
CCalendarStoreView* C3PaneWindow::GetCalendarStoreView() const
{
	return mAccounts.GetCalendarStoreView();
}

// Check each sub-pane
bool C3PaneWindow::AttemptClose()
{
	// Get each of its panes to acknowledge the close
	if (!GetPreviewView()->TestClose() ||
		!GetItemsView()->TestClose() ||
		!GetListView()->TestClose())
		return false;
	
	return true;
}

// Close each sub-pane
void C3PaneWindow::DoClose()
{
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
	
	// Lock out updates to prevent flashes
	BOOL locked = LockWindowUpdate();
	
	// Set the geometry
	SetGeometry(options.GetGeometry());
	
	// Set the splits
	mSplitter1.SetRelativeSplitPos(options.GetSplit1Pos());
	mSplitter2.SetRelativeSplitPos(options.GetSplit2Pos());
	
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
			mAccounts.Focus();
			done_focus = true;
		}
		break;
	case C3PaneOptions::C3PaneViewOptions::eItemsView:
		if (mItemsVisible && !mListZoom && !mPreviewZoom)
		{
			mItems.Focus();
			done_focus = true;
		}
		break;
	case C3PaneOptions::C3PaneViewOptions::ePreviewView:
		if (mPreviewVisible && !mListZoom && !mItemsZoom)
		{
			mPreview.Focus();
			done_focus = true;
		}
		break;
	default:;
	}

	// Always focus on one
	if (!done_focus)
	{
		if (mListZoom)
			mAccounts.Focus();
		else if (mItemsZoom)
			mItems.Focus();
		else if (mPreviewZoom)
			mPreview.Focus();
		else if (mListVisible)
			mAccounts.Focus();
		else if (mItemsVisible)
			mItems.Focus();
		else if (mPreviewVisible)
			mPreview.Focus();
	}

	if (locked)
	{
		UnlockWindowUpdate();
		RedrawWindow();
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
	if (options.GetSplit1Pos() != mSplitter1.GetRelativeSplitPos())
	{
		options.SetSplit1Pos(mSplitter1.GetRelativeSplitPos());
		dirty = true;
	}
	if (options.GetSplit2Pos() != mSplitter2.GetRelativeSplitPos())
	{
		options.SetSplit2Pos(mSplitter2.GetRelativeSplitPos());
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
	if (mAccounts.HasFocus())
		options.SetFocusedPanel(C3PaneOptions::C3PaneViewOptions::eListView);
	else if (mItems.HasFocus())
		options.SetFocusedPanel(C3PaneOptions::C3PaneViewOptions::eItemsView);
	else if (mPreview.HasFocus())
		options.SetFocusedPanel(C3PaneOptions::C3PaneViewOptions::ePreviewView);

	// See if different and set in prefs
	if (dirty)
		CPreferences::sPrefs->m3PaneOptions.SetDirty();
}

// Reset state from prefs
void C3PaneWindow::OnResetPaneState()
{
	if (mAccounts.HasFocus())
		mAccounts.ResetState();
	else if (mItems.HasFocus())
		mItems.ResetState();
	else if (mPreview.HasFocus())
		mPreview.ResetState();
}

// Save current state in prefs
void C3PaneWindow::OnSaveDefaultPaneState()
{
	if (mAccounts.HasFocus())
		mAccounts.SaveDefaultState();
	else if (mItems.HasFocus())
		mItems.SaveDefaultState();
	else if (mPreview.HasFocus())
		mPreview.SaveDefaultState();
}

// Reset state from prefs
void C3PaneWindow::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get default state
	C3PaneWindowState* state = &CPreferences::sPrefs->m3PaneWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->m3PaneWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// The MDI parent will reset itself so we don't do it again here

		// Clip to screen
		//::RectOnScreen(set_rect, AfxGetMainWnd());

		// Reset bounds
		//AfxGetMainWnd()->SetWindowPos(NULL, set_rect.left, set_rect.top,
		//								set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
	}

	// Status state is global
	ShowStatus(state->GetShowStatus());
	
	// Set toolbar visible states
	mToolbarView.ShowToolbar(CToolbarView::eStdButtonsGroup, state->GetShowButtonToolbar());

	// The MDI parent will reset itself so we don't do it again here

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// Reset state for standard view - do after setting zoom state to avoid
	// splitter position changes if zoom is triggered
	OptionsSetView();

	// If forced reset, save it
	if (force)
		SaveDefaultState();

	if (!force)
	{
		if (!visible)
			GetParentFrame()->ActivateFrame(SW_SHOWMAXIMIZED);
	}

	GetParentFrame()->RedrawWindow();
	
	// Fix for occassional message preview redraw problem
	mSplitter1.SetPixelSplitPos(mSplitter1.GetPixelSplitPos() - 1);
	mSplitter1.SetPixelSplitPos(mSplitter1.GetPixelSplitPos() + 1);
}

// Save current state in prefs
void C3PaneWindow::SaveDefaultState(void)
{
	// Always save the current options state
	OptionsSaveView();

	// Get bounds
	CRect bounds(0, 0, 0, 0);
	WINDOWPLACEMENT wp;
	AfxGetMainWnd()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Add info to prefs
	C3PaneWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal,
								mGeometry, mListVisible, mItemsVisible, mPreviewVisible, mStatusVisible,
								mSplitter1.GetRelativeSplitPos(), mSplitter2.GetRelativeSplitPos(),
								mToolbarView.IsVisible(CToolbarView::eStdButtonsGroup));
	if (CPreferences::sPrefs->m3PaneWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->m3PaneWindowDefault.SetDirty();
}
