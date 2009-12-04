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


// Source for CCalendarStoreView class

#include "CCalendarStoreView.h"

#include "CCalendarStoreTable.h"
#include "CCalendarStoreToolbar.h"
#include "CCalendarStoreWindow.h"
#include "CCalendarView.h"
#include "CFontCache.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneWindow.h"

#include "CCalendarStoreManager.h"

#include <algorithm.h>

BEGIN_MESSAGE_MAP(CCalendarStoreView, CTableView)
	ON_WM_CREATE()
	
	//ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateAlways)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)

	// Toolbar

END_MESSAGE_MAP()

// Static members

cdmutexprotect<CCalendarStoreView::CCalendarStoreViewList> CCalendarStoreView::sCalendarStoreViews;

// ---------------------------------------------------------------------------
//	CCalendarStoreView														  [public]
/**
	Default constructor */

CCalendarStoreView::CCalendarStoreView()
{
	mCalendarView = NULL;

	// Add to list
	{
		cdmutexprotect<CCalendarStoreViewList>::lock _lock(sCalendarStoreViews);
		sCalendarStoreViews->push_back(this);
	}
}

// ---------------------------------------------------------------------------
//	~CCalendarStoreView														  [public]
/**
	Destructor */

CCalendarStoreView::~CCalendarStoreView()
{
	// Remove from list
	cdmutexprotect<CCalendarStoreViewList>::lock _lock(sCalendarStoreViews);
	sCalendarStoreViews->erase(std::remove(sCalendarStoreViews->begin(), sCalendarStoreViews->end(), this), sCalendarStoreViews->end());
}

#pragma mark -

const int cTitleHeight = 16;

int CCalendarStoreView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	SetViewInfo(eCalendarStoreColumnSubscribe, 16, true);
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
	mCalTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent + cTitleHeight, width - focus_indent, height - focus_indent), &mFocusRing, IDC_CALENDARSTORE_TABLE);
	mCalTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mCalTable.ResetFont(CFontCache::GetListFont());
	mCalTable.SetColumnInfo(mColumnInfo);
	mCalTable.SetContextMenuID(IDR_POPUP_CONTEXT_CALENDARSTORE);
	//mCalTable.SetContextView(this);
	mFocusRing.AddAlignment(new CWndAlignment(&mCalTable, CWndAlignment::eAlign_WidthHeight));

	// Get titles
	mCalTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent, width - focus_indent, focus_indent + cTitleHeight), &mFocusRing, IDC_STATIC);
	mFocusRing.AddAlignment(new CWndAlignment(&mCalTitles, CWndAlignment::eAlign_TopWidth));
	mCalTitles.SetFont(CFontCache::GetListFont());

	PostCreate(&mCalTable, &mCalTitles);

	// Set commander
	if (Is3Pane())
		SetSuperCommander(C3PaneWindow::s3PaneWindow);

	// Set status
	SetOpen();

	return 0;
}

BOOL CCalendarStoreView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route some messages to the preview
	switch(nID)
	{
	case IDM_CALENDAR_NEWEVENT:
	case IDC_TOOLBAR_CALENDAR_NEWEVENT:
	case IDM_CALENDAR_NEWTODO:
	case IDC_TOOLBAR_CALENDAR_NEWTODO:
	case IDM_CALENDAR_GOTO_TODAY:
	case IDC_TOOLBAR_CALENDAR_TODAY:
	case IDM_CALENDAR_GOTO_DATE:
	case IDC_TOOLBAR_CALENDAR_GOTO:
	case IDC_TOOLBAR_CALENDAR_DAY:
	case IDC_TOOLBAR_CALENDAR_WORK:
	case IDC_TOOLBAR_CALENDAR_WEEK:
	case IDC_TOOLBAR_CALENDAR_MONTH:
	case IDC_TOOLBAR_CALENDAR_YEAR:
	case IDC_TOOLBAR_CALENDAR_SUMMARY:
	case IDC_TOOLBAR_CALENDAR_TASKS:
		if (GetCalendarView() != NULL)
			return GetCalendarView()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		break;
	default:;
	}

	// Do inherited
	return CTableView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Make a toolbar appropriate for this view
void CCalendarStoreView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetCalendarToolbar());
		C3PaneWindow::s3PaneWindow->GetCalendarToolbar()->AddCommander(GetTable());
	}
	else
	{
		// Create a suitable toolbar
		CCalendarStoreToolbar* tb = new CCalendarStoreToolbar;
		tb->InitToolbar(false, parent);
		
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
	}
}

// Init columns and text
void CCalendarStoreView::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();

	// Add default columns

	// Subscribe, width = 16
	AppendColumn(eCalendarStoreColumnSubscribe, 16);

	// Name, width = 158
	AppendColumn(eCalendarStoreColumnName, 158);

	// Colour, width = 48
	AppendColumn(eCalendarStoreColumnColour, 48);
}

// Reset state from prefs
void CCalendarStoreView::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get default state
	CCalendarStoreWindowState* state = &CPreferences::sPrefs->mCalendarStoreWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mCalendarStoreWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetCalendarStoreWindow());

			// Reset bounds
			GetParentFrame()->SetWindowPos(NULL, set_rect.left, set_rect.top,
											set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
		}
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mCalendarStoreWindowDefault.GetValue()));

	// If forced reset, save it
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
void CCalendarStoreView::SaveDefaultState(void)
{
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
	CCalendarStoreWindowState state(NULL, &bounds, wstate, &mColumnInfo, &empty);
	if (CPreferences::sPrefs->mCalendarStoreWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mCalendarStoreWindowDefault.SetDirty();
}