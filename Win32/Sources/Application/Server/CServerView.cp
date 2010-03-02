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


// Source for CServerView class

#include "CServerView.h"

#include "CFontCache.h"
#include "CLog.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerTable.h"
#include "CServerToolbar.h"
#include "CServerWindow.h"
#include "CTaskClasses.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

BEGIN_MESSAGE_MAP(CServerView, CTableView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_COMMAND(IDC_SERVERFLATBTN, DoFlatHierarchy)
END_MESSAGE_MAP()

// Static members

cdmutexprotect<CServerView::CServerViewList> CServerView::sServerViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerView::CServerView()
{
	mMailboxView = NULL;

	// Add to list
	{
		cdmutexprotect<CServerViewList>::lock _lock(sServerViews);
		sServerViews->push_back(this);
	}
}

// Default destructor
CServerView::~CServerView()
{
	// Remove from list
	cdmutexprotect<CServerViewList>::lock _lock(sServerViews);
	CServerViewList::iterator found = std::find(sServerViews->begin(), sServerViews->end(), this);
	if (found != sServerViews->end())
		sServerViews->erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

int CServerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetViewInfo(eServerColumnFlags, 16, true);
	if (CTableView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Focus ring
	mFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, width, height), this, IDC_STATIC);
	if (Is3Pane())
		mFocusRing.SetFocusBorder();
	AddAlignment(new CWndAlignment(&mFocusRing, CWndAlignment::eAlign_WidthHeight));
	UINT focus_indent = Is3Pane() ? 3 : 0;

	// Server table
	mServerTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL,
							CRect(focus_indent, focus_indent + cTitleHeight, width - focus_indent, height - focus_indent),
							&mFocusRing, IDC_SERVERTABLE);
	mServerTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mServerTable.ResetFont(CFontCache::GetListFont());
	mServerTable.SetColumnInfo(mColumnInfo);
	mServerTable.SetContextMenuID(IDR_POPUP_CONTEXT_SERVER);
	mServerTable.SetContextView(static_cast<CView*>(GetOwningWindow()));
	mFocusRing.AddAlignment(new CWndAlignment(&mServerTable, CWndAlignment::eAlign_WidthHeight));

	// Get titles
	mServerTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(focus_indent, focus_indent, width - focus_indent - 16, focus_indent + cTitleHeight), &mFocusRing, IDC_STATIC);
	mServerTitles.ResetFont(CFontCache::GetListFont());
	mFocusRing.AddAlignment(new CWndAlignment(&mServerTitles, CWndAlignment::eAlign_TopWidth));

	PostCreate(&mServerTable, &mServerTitles);

	mFlatHierarchyBtn.Create(_T(""), CRect(width - focus_indent - 16, focus_indent, width - focus_indent, focus_indent + 16), &mFocusRing, IDC_SERVERFLATBTN, IDC_STATIC, IDI_SERVERFLAT, IDI_SERVERHIERARCHIC);
	mFocusRing.AddAlignment(new CWndAlignment(&mFlatHierarchyBtn, CWndAlignment::eAlign_TopRight));

	// Set commander
	if (Is3Pane())
		SetSuperCommander(C3PaneWindow::s3PaneWindow);

	// Set status
	SetOpen();

	return 0;
}

void CServerView::OnDestroy(void)
{
	// Do standard close behaviour
	DoClose();

	// Do default action now
	CTableView::OnDestroy();
}

// Make a toolbar appropriate for this view
void CServerView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetMailboxToolbar());
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(GetTable());
	}
	else
	{
		// Create a suitable toolbar
		CServerToolbar* tb = new CServerToolbar;
		tb->InitToolbar(false, parent);
		
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
	}
}

// Init columns and text
void CServerView::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();
	
	// Name, width = 158
	AppendColumn(eServerColumnName, 158);
	
	// Total, width = 48
	AppendColumn(eServerColumnTotal, 48);
	
	// Unseen, width = 32
	AppendColumn(eServerColumnUnseen, 32);
}

// Change to flat view of hierarchy
void CServerView::DoFlatHierarchy()
{
	// Toggle button state
	GetHierarchyBtn()->SetPushed(!GetHierarchyBtn()->IsPushed());
	
	GetTable()->DoFlatHierarchy();
}

// Close it
// Reset state from prefs
void CServerView::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Use "Mail Accounts" for the 1-pane/3-pane server manager
	// Use server name for other 1-pane windows
	cdstring name = (GetTable()->IsManager() ? cdstring("Mail Accounts") : GetTable()->GetServer()->GetAccountName());

	// Check for available state
	CServerWindowState* state = CPreferences::sPrefs->GetServerWindowInfo(name);
	if (!state || force)
		state = &CPreferences::sPrefs->mServerWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mServerWindowDefault.GetValue());
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
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mServerWindowDefault.GetValue()));

	// If forced reset, save it
	if (force)
		SaveState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	if (!force && !Is3Pane())
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		RedrawWindow();
	}
	else
		RedrawWindow();
}

// Save current state in prefs
void CServerView::SaveState(void)
{
	// Use "Mail Accounts" for the 1-pane/3-pane server manager
	// Use server name for other 1-pane windows
	cdstring name = (GetTable()->IsManager() ? cdstring("Mail Accounts") : GetTable()->GetServer()->GetAccountName());

	// Get bounds
	CRect bounds(0, 0, 0, 0);
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = !Is3Pane() && (wp.showCmd == SW_SHOWMAXIMIZED);
	if (!Is3Pane())
		bounds = wp.rcNormalPosition;

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Add info to prefs
	CServerWindowState* info = new CServerWindowState(name, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo);
	if (info)
		CPreferences::sPrefs->AddServerWindowInfo(info);
}

// Save current state in prefs
void CServerView::SaveDefaultState(void)
{
	// Get bounds
	CRect bounds(0, 0, 0, 0);
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = !Is3Pane() && (wp.showCmd == SW_SHOWMAXIMIZED);
	if (!Is3Pane())
		bounds = wp.rcNormalPosition;

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Add info to prefs
	CServerWindowState state(NULL, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo);
	if (CPreferences::sPrefs->mServerWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mServerWindowDefault.SetDirty();

}
