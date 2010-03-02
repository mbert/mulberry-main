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


// Source for CAdbkManagerView class

#include "CAdbkManagerView.h"

#include "CAdbkManagerToolbar.h"
#include "CAdbkManagerWindow.h"
#include "CFontCache.h"
#include "CHierarchyTableDrag.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableViewWindow.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneWindow.h"

BEGIN_MESSAGE_MAP(CAdbkManagerView, CTableView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

// Static members

cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList> CAdbkManagerView::sAdbkManagerViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerView::CAdbkManagerView()
{
	mAddressBookView = NULL;

	// Add to list
	{
		cdmutexprotect<CAdbkManagerViewList>::lock _lock(sAdbkManagerViews);
		sAdbkManagerViews->push_back(this);
	}
}

// Default destructor
CAdbkManagerView::~CAdbkManagerView()
{
	// Remove from list
	cdmutexprotect<CAdbkManagerViewList>::lock _lock(sAdbkManagerViews);
	CAdbkManagerViewList::iterator found = std::find(sAdbkManagerViews->begin(), sAdbkManagerViews->end(), this);
	if (found != sAdbkManagerViews->end())
		sAdbkManagerViews->erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

int CAdbkManagerView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetViewInfo(eAdbkColumnOpen, 16, true);
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
	mAdbkTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent + cTitleHeight, width - focus_indent, height - focus_indent), &mFocusRing, IDC_SERVERTABLE);
	mAdbkTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mAdbkTable.ResetFont(CFontCache::GetListFont());
	mAdbkTable.SetColumnInfo(mColumnInfo);
	mAdbkTable.SetContextMenuID(IDR_POPUP_CONTEXT_ADDRMANAGER);
	//mAdbkTable.SetContextView(this);
	mFocusRing.AddAlignment(new CWndAlignment(&mAdbkTable, CWndAlignment::eAlign_WidthHeight));

	// Get titles
	mAdbkTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent, width - focus_indent, focus_indent + cTitleHeight), &mFocusRing, IDC_STATIC);
	mFocusRing.AddAlignment(new CWndAlignment(&mAdbkTitles, CWndAlignment::eAlign_TopWidth));
	mAdbkTitles.SetFont(CFontCache::GetListFont());

	PostCreate(&mAdbkTable, &mAdbkTitles);

	// Set commander
	if (Is3Pane())
		SetSuperCommander(C3PaneWindow::s3PaneWindow);

	// Set status
	SetOpen();

	return 0;
}

// Make a toolbar appropriate for this view
void CAdbkManagerView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetAdbkToolbar());
		C3PaneWindow::s3PaneWindow->GetAdbkToolbar()->AddCommander(GetTable());
	}
	else
	{
		// Create a suitable toolbar
		CAdbkManagerToolbar* tb = new CAdbkManagerToolbar;
		tb->InitToolbar(false, parent);

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
	}
}

// Init columns and text
void CAdbkManagerView::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();

	// Add default columns
	
	// Resolve, width = 16
	AppendColumn(eAdbkColumnResolve, 16);

	// Search, width = 16
	AppendColumn(eAdbkColumnSearch, 16);

	// Flags, width = 240
	AppendColumn(eAddrColumnName, 240);
}

// Reset state from prefs
void CAdbkManagerView::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get default state
	CAdbkManagerWindowState* state = &CPreferences::sPrefs->mAdbkManagerWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mAdbkManagerWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, NULL);

			// Reset bounds
			GetParentFrame()->SetWindowPos(nil, set_rect.left, set_rect.top,
											set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
		}
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mAdbkManagerWindowDefault.GetValue()));

	// Show/hide window based on state
	//if (state->GetHide())
		//GetParentFrame()->SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	//	GetParentFrame()->ShowWindow(SW_HIDE);
	//else
		//GetParentFrame()->SendMessage(WM_SYSCOMMAND, SC_RESTORE);
	//	GetParentFrame()->ShowWindow(SW_SHOW);

	if (force)
		SaveDefaultState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Do zoom
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);
	else if (!Is3Pane() && (state->GetState() == eWindowStateMin))
		GetParentFrame()->ShowWindow(SW_SHOWMINIMIZED);

	if (!force && !Is3Pane())
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		GetParentFrame()->RedrawWindow();
	}
	else
		GetParentFrame()->RedrawWindow();
}

// Save current state in prefs
void CAdbkManagerView::SaveState(void)
{
	SaveDefaultState();
}

// Save current state in prefs
void CAdbkManagerView::SaveDefaultState(void)
{
	// Get bounds
	CRect bounds(0, 0, 0, 0);
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	EWindowState wstate = eWindowStateNormal;
	if (!Is3Pane())
	{
		if (wp.showCmd == SW_SHOWMAXIMIZED)
			wstate = eWindowStateMax;
		else if (wp.showCmd == SW_SHOWMINIMIZED)
			wstate = eWindowStateMin;
		bounds = wp.rcNormalPosition;
	}

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Add info to prefs
	cdstrvect empty;
	CAdbkManagerWindowState state(nil, &bounds, wstate, &mColumnInfo, (wp.showCmd == SW_HIDE), &empty);
	if (CPreferences::sPrefs->mAdbkManagerWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mAdbkManagerWindowDefault.SetDirty();
}
