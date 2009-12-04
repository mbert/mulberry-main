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


// Source for CSMTPView class

#include "CSMTPView.h"

#include "CFontCache.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSMTPToolbar.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

// Static members
BEGIN_MESSAGE_MAP(CSMTPView, CMailboxView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPView::CSMTPView()
{
	mSender = NULL;
}

// Default destructor
CSMTPView::~CSMTPView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

int CSMTPView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMailboxView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	UINT focus_indent = Is3Pane() ? 3 : 0;

	// Create footer first so we get its scaled height
	mFooter.CreateDialogItems(IDD_SMTPFOOTER, &mFocusRing);
	mFooter.ModifyStyle(0, WS_CLIPSIBLINGS);
	mFooter.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	CRect rect;
	mFooter.GetWindowRect(rect);
	mFooter.ExecuteDlgInit(MAKEINTRESOURCE(IDD_SMTPFOOTER));
	mFooter.SetFont(CMulberryApp::sAppSmallFont);
	mFooter.MoveWindow(CRect(focus_indent, height - focus_indent - rect.Height(), width - focus_indent, height - focus_indent));
	mFooter.ShowWindow(SW_SHOW);

	// Subclass footer controls for our use
	mTotalText.SubclassDlgItem(IDC_SMTPTOTALTXT, &mFooter);
	mTotalText.SetFont(CMulberryApp::sAppSmallFont);

	// Server table
	mSMTPTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
					CRect(focus_indent, focus_indent + cTitleHeight, width - focus_indent, height - focus_indent - rect.Height()), &mFocusRing, IDC_MAILBOXTABLE);
	mSMTPTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mSMTPTable.ResetFont(CFontCache::GetListFont());
	mSMTPTable.SetColumnInfo(mColumnInfo);
	mSMTPTable.SetContextMenuID(IDR_POPUP_CONTEXT_MAILBOX);
	mSMTPTable.SetContextView(static_cast<CView*>(GetOwningWindow()));

	// Get titles
	mSMTPTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(focus_indent, focus_indent, width - focus_indent - 16, focus_indent + cTitleHeight), &mFocusRing, IDC_STATIC);
	mSMTPTitles.SetFont(CFontCache::GetListFont());
	mFocusRing.AddAlignment(new CWndAlignment(&mSMTPTitles, CWndAlignment::eAlign_TopWidth));

	PostCreate(&mSMTPTable, &mSMTPTitles);

	// Create alignment details
	mFocusRing.AddAlignment(new CWndAlignment(&mFooter, CWndAlignment::eAlign_BottomWidth));
	mFocusRing.AddAlignment(new CWndAlignment(&mSMTPTable, CWndAlignment::eAlign_WidthHeight));

	// Set status
	SetOpen();

	return 0;
}

void CSMTPView::OnDestroy(void)
{
	// Do standard close behaviour
	DoClose();

	// Do default action now
	CMailboxView::OnDestroy();
}

// Make a toolbar appropriate for this view
void CSMTPView::MakeToolbars(CToolbarView* parent)
{
	// Create a suitable toolbar
	CSMTPToolbar* tb = new CSMTPToolbar;
	mToolbar = tb;
	tb->InitToolbar(Is3Pane(), parent);

	// Toolbar must listen to view to get activate/deactive broadcast
	Add_Listener(tb);
	
	// Now give toolbar to its view as standard buttons
	parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
}

// Set window state
void CSMTPView::ResetState(bool force)
{
	if (!GetMbox())
		return;

	// Get name as cstr
	CString name(GetMbox()->GetAccountName());

	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get default state
	CMailboxWindowState* state = &CPreferences::sPrefs->mSMTPWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mSMTPWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset bounds
		GetParentFrame()->SetWindowPos(nil, set_rect.left, set_rect.top, set_rect.Width(), set_rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mServerWindowDefault.GetValue()));

	// Adjust menus
	SetSortBy(state->GetSortBy());
	GetMbox()->ShowBy(state->GetShowBy());

	// Sorting button
	//mSortBtn.SetPushed(state->GetShowBy() == cShowMessageDescending);

	if (force)
		SaveDefaultState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Do zoom
	if (state->GetState() == eWindowStateMax)
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// Init the preview state once if we're in a window
	if (!Is3Pane() && !mPreviewInit)
	{
		mMessageView->ResetState();
		mPreviewInit = true;
	}

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetMailboxWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	if (!force)
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		RedrawWindow();
	}
	else
		RedrawWindow();
}

// Save current state as default
void CSMTPView::SaveDefaultState(void)
{
	// Only do this if a mailbox has been set
	if (!GetMbox())
		return;

	// Get bounds
	CRect bounds;
	WINDOWPLACEMENT wp;
	GetParentFrame()->GetWindowPlacement(&wp);
	bool zoomed = (wp.showCmd == SW_SHOWMAXIMIZED);
	bounds = wp.rcNormalPosition;

	// Sync column widths
	for(int i = 0; i < mColumnInfo.size(); i++)
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Get current match item
	CMatchItem match;

	// Check whether quitting
	bool is_quitting = CMulberryApp::sApp->IsQuitting();

	// Add info to prefs
	CMailboxWindowState state(nil, &bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo,
								(ESortMessageBy) GetMbox()->GetSortBy(),
								(EShowMessageBy) GetMbox()->GetShowBy(),
								is_quitting ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All,
								&match,
								Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mSMTPWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mSMTPWindowDefault.SetDirty();

}
