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


// Source for CMailboxInfoView class

#include "CMailboxInfoView.h"

#include "CFontCache.h"
#include "CMailAccountManager.h"
#include "CMailboxTitleTable.h"
#include "CMailboxInfoTable.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMboxProtocol.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CToolbarView.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include <string.h>

#include <numeric.h>

const int IDT_TIMER_ID = 12347;

BEGIN_MESSAGE_MAP(CMailboxInfoView, CMailboxView)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SIZE()
	ON_COMMAND(IDC_QUICKSEARCHPOPUP, OnQuickSearchPopup)
	ON_COMMAND(IDC_QUICKSEARCHSAVEDPOPUP, OnQuickSearchSavedPopup)
	ON_COMMAND(IDC_QUICKSEARCHCANCEL, OnQuickSearchCancel)
	ON_WM_TIMER()

	ON_UPDATE_COMMAND_UI(IDM_WINDOWS_SHOW_QUICKSEARCH, OnUpdateWindowsShowQuickSearch)
	ON_COMMAND(IDM_WINDOWS_SHOW_QUICKSEARCH, OnWindowsShowQuickSearch)

END_MESSAGE_MAP()

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoView::CMailboxInfoView() :
	mQuickSearchCriteria(true)
{
	mNoSearchReset = false;
	mShowQuickSearch = true;
	mQuickSearchTimerPending = false;
	mTimerID = 0;
}

// Default destructor
CMailboxInfoView::~CMailboxInfoView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;
const int cQuickSearchAreaHeight = 26;

const int cPopupHeight = 22;

const int cQuickSearchIndicatorHOffset = 4;
const int cQuickSearchIndicatorVOffset = 5;
const int cQuickSearchIndicatorWidth = 16;

const int cQuickSearchPopupHOffset = cQuickSearchIndicatorHOffset + cQuickSearchIndicatorWidth + 2;
const int cQuickSearchPopupVOffset = 2;
const int cQuickSearchPopupWidth = 100;

const int cQuickSearchEditHOffset = cQuickSearchPopupHOffset + cQuickSearchPopupWidth + 4;
const int cQuickSearchEditVOffset = cQuickSearchPopupVOffset;

const int cQuickSearchProgressWidth = 192;
const int cQuickSearchProgressHeight = 16;
const int cQuickSearchProgressVOffset = 5;

const int cQuickSearchCancelHOffset = 20 + 8;
const int cQuickSearchCancelVOffset = cQuickSearchPopupVOffset;

const int cQuickSearchCancelWidth = 24;
const int cQuickSearchCancelHeight = 22;
const int cQuickSearchEditWidthOffset = cQuickSearchCancelHOffset + 16;

int CMailboxInfoView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMailboxView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	UINT focus_indent = Is3Pane() ? 3 : 0;

	// Due to sizing problem, create quick search area with width 100 pixels larger than needed, then reduce its size by 100
	// before adding the wnd aligner
	mQuickSearchArea.Create(NULL, WS_CHILD | WS_VISIBLE, CRect(focus_indent, focus_indent, width + 100 - focus_indent, focus_indent + cQuickSearchAreaHeight), &mFocusRing, IDC_STATIC);

	mQuickSearchIndicator.Create(NULL, WS_CHILD, CRect(cQuickSearchIndicatorHOffset, cQuickSearchIndicatorVOffset, cQuickSearchIndicatorHOffset + 16, cQuickSearchIndicatorVOffset + 16), &mQuickSearchArea, IDC_QUICKSEARCHINDICATOR);
	mQuickSearchIndicator.SetIconID(IDI_MAILBOXSEARCH);

	mQuickSearchCriteria.Create(_T(""), CRect(cQuickSearchPopupHOffset, cQuickSearchPopupVOffset, cQuickSearchPopupHOffset + cQuickSearchPopupWidth, cQuickSearchPopupVOffset + cPopupHeight), &mQuickSearchArea, IDC_QUICKSEARCHPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mQuickSearchCriteria.SetMenu(IDR_POPUP_QUICKSEARCH);
	mQuickSearchCriteria.SetValue(CPreferences::sPrefs->mQuickSearch.GetValue() + IDM_QUICKSEARCH_FROM);
	mQuickSearchCriteria.SetFont(CMulberryApp::sAppFont);
	mQuickSearchArea.AddAlignment(new CWndAlignment(&mQuickSearchCriteria, CWndAlignment::eAlign_TopLeft));

	mQuickSearchText.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_AUTOHSCROLL,
								CRect(cQuickSearchEditHOffset, cQuickSearchEditVOffset, width + 100 - 2 * focus_indent - cQuickSearchEditWidthOffset, cQuickSearchEditVOffset + cPopupHeight), &mQuickSearchArea, IDC_QUICKSEARCHTEXT);
	mQuickSearchText.SetFont(CMulberryApp::sAppFont);
	mQuickSearchArea.AddAlignment(new CWndAlignment(&mQuickSearchText, CWndAlignment::eAlign_TopWidth));
	mQuickSearchText.Add_Listener(this);
	mQuickSearchText.SetContextView(static_cast<CView*>(GetOwningWindow()));

	mQuickSearchSaved.Create(_T(""), CRect(cQuickSearchEditHOffset, cQuickSearchEditVOffset, width + 100 - 2 * focus_indent - cQuickSearchEditWidthOffset, cQuickSearchEditVOffset + cPopupHeight), &mQuickSearchArea, IDC_QUICKSEARCHSAVEDPOPUP, IDC_STATIC, IDI_POPUPBTN);
	mQuickSearchSaved.SetValue(IDM_QUICKSEARCH_CHOOSE);
	mQuickSearchSaved.SetFont(CMulberryApp::sAppFont);
	mQuickSearchArea.AddAlignment(new CWndAlignment(&mQuickSearchSaved, CWndAlignment::eAlign_TopWidth));

	mQuickSearchProgress.Create(PBS_SMOOTH | WS_BORDER | WS_CHILD, CRect(cQuickSearchEditHOffset, cQuickSearchProgressVOffset, cQuickSearchEditHOffset + cQuickSearchProgressWidth, cQuickSearchProgressVOffset + cQuickSearchProgressHeight), &mQuickSearchArea, IDC_QUICKSEARCHPROGRESS);
	mQuickSearchArea.AddAlignment(new CWndAlignment(&mQuickSearchProgress, CWndAlignment::eAlign_TopLeft));

	mQuickSearchCancel.Create(_T(""), CRect(width + 100 - 2 * focus_indent - cQuickSearchCancelHOffset, cQuickSearchCancelVOffset, width + 100 - 2 * focus_indent - cQuickSearchCancelHOffset + cQuickSearchCancelWidth, cQuickSearchCancelVOffset + cQuickSearchCancelHeight), &mQuickSearchArea, IDC_QUICKSEARCHCANCEL, IDC_STATIC, IDI_QUICKSEARCHCANCEL);
	mQuickSearchArea.AddAlignment(new CWndAlignment(&mQuickSearchCancel, CWndAlignment::eAlign_TopRight));
	mQuickSearchCancel.EnableWindow(false);

	::ResizeWindowBy(&mQuickSearchArea, -100, 0, false);
	mFocusRing.AddAlignment(new CWndAlignment(&mQuickSearchArea, CWndAlignment::eAlign_TopWidth));

	// Create footer first so we get its scaled height
	mFooter.CreateDialogItems(IDD_MAILBOXFOOTER, &mFocusRing);
	mFooter.ModifyStyle(0, WS_CLIPSIBLINGS);
	mFooter.ModifyStyleEx(0, WS_EX_DLGMODALFRAME);
	CRect rect;
	mFooter.GetWindowRect(rect);
	mFooter.ExecuteDlgInit(MAKEINTRESOURCE(IDD_MAILBOXFOOTER));
	mFooter.SetFont(CMulberryApp::sAppSmallFont);
	mFooter.MoveWindow(CRect(focus_indent, height - focus_indent - rect.Height(), width - focus_indent, height - focus_indent));
	mFooter.ShowWindow(SW_SHOW);

	// Subclass footer controls for our use
	mTotalText.SubclassDlgItem(IDC_MAILBOXTOTALTXT, &mFooter);
	mTotalText.SetFont(CMulberryApp::sAppSmallFont);
	mUnseenText.SubclassDlgItem(IDC_MAILBOXUNSEENTXT, &mFooter);
	mUnseenText.SetFont(CMulberryApp::sAppSmallFont);
	mDeletedText.SubclassDlgItem(IDC_MAILBOXDELNUMTXT, &mFooter);
	mDeletedText.SetFont(CMulberryApp::sAppSmallFont);
	mMatchedText.SubclassDlgItem(IDC_MAILBOXMATCHNUMTXT, &mFooter);
	mMatchedText.SetFont(CMulberryApp::sAppSmallFont);

	// Mailbox table
	mMailboxInfoTable.Create(NULL, NULL, WS_CHILD | WS_VISIBLE | WS_TABSTOP,
					CRect(focus_indent, focus_indent + cQuickSearchAreaHeight + cTitleHeight, width - focus_indent, height - focus_indent - rect.Height()), &mFocusRing, IDC_MAILBOXTABLE);
	mMailboxInfoTable.ModifyStyleEx(0, WS_EX_NOPARENTNOTIFY, 0);
	mMailboxInfoTable.ResetFont(CFontCache::GetListFont());
	mMailboxInfoTable.SetColumnInfo(mColumnInfo);
	mMailboxInfoTable.SetContextMenuID(IDR_POPUP_CONTEXT_MAILBOX);
	mMailboxInfoTable.SetContextView(static_cast<CView*>(GetOwningWindow()));

	// Must move sort button to account for quick search
	::MoveWindowBy(&mSortBtn, 0, cQuickSearchAreaHeight, true);

	// Get titles
	mMailboxInfoTitles.Create(NULL, NULL, WS_CHILD | WS_VISIBLE, CRect(focus_indent, focus_indent + cQuickSearchAreaHeight, width - focus_indent - 16, focus_indent + cQuickSearchAreaHeight + cTitleHeight), &mFocusRing, IDC_STATIC);
	mMailboxInfoTitles.ResetFont(CFontCache::GetListFont());
	mFocusRing.AddAlignment(new CWndAlignment(&mMailboxInfoTitles, CWndAlignment::eAlign_TopWidth));

	PostCreate(&mMailboxInfoTable, &mMailboxInfoTitles);

	// Set commander
	if (Is3Pane())
		SetSuperCommander(C3PaneWindow::s3PaneWindow);

	// Create alignment details
	// Must have minimum height to prevent status pane dissappearing when size if zeroed
	mFocusRing.AddAlignment(new CWndAlignment(&mFooter, CWndAlignment::eAlign_BottomWidth));
	mFocusRing.AddAlignment(new CWndAlignment(&mMailboxInfoTable, CWndAlignment::eAlign_WidthHeight));

	// Init quick search area
	DoQuickSearchPopup(CPreferences::sPrefs->mQuickSearch.GetValue() + IDM_QUICKSEARCH_FROM);

	// Hide quick search if user does not want it
	if (!CPreferences::sPrefs->mQuickSearchVisible.GetValue())
		ShowQuickSearch(false);
	
	// Set status
	SetOpen();

	return 0;
}

void CMailboxInfoView::OnDestroy(void)
{
	// Do standard close behaviour
	DoCloseMbox();

	// Do default action now
	CMailboxView::OnDestroy();
}

// Resize sub-views
void CMailboxInfoView::OnSize(UINT nType, int cx, int cy)
{
	CMailboxView::OnSize(nType, cx, cy);
	
	// Update cache details
	if (GetMbox())
		GetInfoTable()->UpdateCacheIncrement();
}

void CMailboxInfoView::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CQuickSearchText::eBroadcast_Return:
		DoQuickSearch(true);
		break;
	case CQuickSearchText::eBroadcast_Tab:
		if (mQuickSearchTimerPending)
		{
			DoQuickSearch(true);
		}
		else
			GetInfoTable()->SetFocus();
		break;
		break;
	case CQuickSearchText::eBroadcast_Key:
		StartQuickSearchTimer();
		break;
	default:
		CMailboxView::ListenTo_Message(msg, param);
		break;
	}
}


// Make a toolbar appropriate for this view
void CMailboxInfoView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Copy btn from toolbar
		mCopyBtn = C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->GetCopyBtn();

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetMailboxToolbar());
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(GetTable());
	}
	else
	{
		// Create a suitable toolbar
		CMailboxInfoToolbar* tb = new CMailboxInfoToolbar;
		mToolbar = tb;
		tb->InitToolbar(false, parent);

		// Match popup from toolbar
		mCopyBtn = tb->GetCopyBtn();

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
	}
}

void CMailboxInfoView::OnQuickSearchPopup()
{
	DoQuickSearchPopup(mQuickSearchCriteria.GetValue());
	
	// Always force focus to the quick search text field
	if (mQuickSearchCriteria.GetValue() - IDM_QUICKSEARCH_FROM != eQuickSearchSaved)
		mQuickSearchText.SetFocus();
}

void CMailboxInfoView::OnQuickSearchSavedPopup()
{
	DoQuickSearchSavedPopup(mQuickSearchSaved.GetValue());
}

void CMailboxInfoView::OnQuickSearchCancel()
{
	DoQuickSearchCancel();
}

void CMailboxInfoView::OnUpdateWindowsShowQuickSearch(CCmdUI* pCmdUI)
{
	// Always enabled but text may change
	pCmdUI->Enable(true);
	CString txt;
	txt.LoadString(mShowQuickSearch ? IDS_HIDEQUICKSEARCH : IDS_SHOWQUICKSEARCH);
	
	OnUpdateMenuTitle(pCmdUI, txt);
}

void CMailboxInfoView::OnWindowsShowQuickSearch()
{
	ShowQuickSearch(!mShowQuickSearch);
	CPreferences::sPrefs->mQuickSearchVisible.SetValue(mShowQuickSearch);
}

// Enable quick search items
void CMailboxInfoView::EnableQuickSearch(bool enable)
{
	mQuickSearchCriteria.EnableWindow(enable);
	mQuickSearchText.EnableWindow(enable);
	mQuickSearchProgress.EnableWindow(enable);
	mQuickSearchCancel.EnableWindow(false);
	mQuickSearchIndicator.ShowWindow(false);
}

// Show quick search area
void CMailboxInfoView::ShowQuickSearch(bool show)
{
	if (!(mShowQuickSearch ^ show))
		return;

	CRect move_size;
	mQuickSearchArea.GetWindowRect(move_size);
	int moveby = move_size.Height();

	if (show)
	{
		// Shrink/move table pane
		::ResizeWindowBy(&mMailboxInfoTable, 0, -moveby, true);
		::MoveWindowBy(&mMailboxInfoTable, 0, moveby, false);
		::MoveWindowBy(&mMailboxInfoTitles, 0, moveby, true);
		::MoveWindowBy(&mSortBtn, 0, moveby, true);

		// Show parts after all other changes
		mQuickSearchArea.ShowWindow(SW_SHOW);
	}
	else
	{
		// Hide parts before other changes
		mQuickSearchArea.ShowWindow(SW_HIDE);

		// Expand/move splitter
		::ResizeWindowBy(&mMailboxInfoTable, 0, moveby, true);
		::MoveWindowBy(&mMailboxInfoTable, 0, -moveby, false);
		::MoveWindowBy(&mMailboxInfoTitles, 0, -moveby, true);
		::MoveWindowBy(&mSortBtn, 0, -moveby, true);
	}

	mShowQuickSearch = show;
}

// Do quick search
void CMailboxInfoView::DoQuickSearchPopup(UINT value)
{
	// Store current value in prefs
	CPreferences::sPrefs->mQuickSearch.SetValue(value - IDM_QUICKSEARCH_FROM);
	
	// Hide/show saved popup
	if (value - IDM_QUICKSEARCH_FROM == eQuickSearchSaved)
	{
		mQuickSearchText.ShowWindow(SW_HIDE);
		mQuickSearchSaved.ShowWindow(SW_SHOW);
		mQuickSearchSaved.SetValue(IDM_QUICKSEARCH_CHOOSE);
	}
	else
	{
		mQuickSearchSaved.ShowWindow(SW_HIDE);
		mQuickSearchText.ShowWindow(SW_SHOW);
	}
}

// Do quick search
void CMailboxInfoView::DoQuickSearch(bool change_focus)
{
	if (mTimerID != 0)
		KillTimer(mTimerID);
	mTimerID = 0;
	mQuickSearchTimerPending = false;

	mQuickSearchText.ShowWindow(SW_HIDE);
	mQuickSearchProgress.ShowWindow(SW_SHOW);
	mQuickSearchProgress.SetIndeterminate(true);

	cdstring txt = mQuickSearchText.GetText();

	auto_ptr<CSearchItem> spec;
	switch(mQuickSearchCriteria.GetValue() - IDM_QUICKSEARCH_FROM)
	{
	case eQuickSearchFrom:
		spec.reset(new CSearchItem(CSearchItem::eFrom, txt));
		break;
	case eQuickSearchTo:
		spec.reset(new CSearchItem(CSearchItem::eTo, txt));
		break;
	case eQuickSearchRecipient:
		spec.reset(new CSearchItem(CSearchItem::eRecipient, txt));
		break;
	case eQuickSearchCorrespondent:
		spec.reset(new CSearchItem(CSearchItem::eCorrespondent, txt));
		break;
	case eQuickSearchSubject:
		spec.reset(new CSearchItem(CSearchItem::eSubject, txt));
		break;
	case eQuickSearchBody:
		spec.reset(new CSearchItem(CSearchItem::eBody, txt));
		break;
	}
	
	// Make sure search failure does not prevent clean-up
	try
	{
		if (txt.length() != 0)
			GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec.get(), false);
		else
			GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	mQuickSearchCancel.EnableWindow(txt.length() != 0);
	mQuickSearchIndicator.ShowWindow(txt.length() != 0);
	if (change_focus)
		GetInfoTable()->SetFocus();

	mQuickSearchProgress.ShowWindow(SW_HIDE);
	mQuickSearchProgress.SetIndeterminate(false);
	mQuickSearchText.ShowWindow(SW_SHOW);
	if (!change_focus)
		mQuickSearchText.SetFocus();
}

// Do quick search saved
void CMailboxInfoView::DoQuickSearchSavedPopup(long value)
{
	if (value == IDM_QUICKSEARCH_CHOOSE)
	{
		GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
		mQuickSearchCancel.EnableWindow(false);
		mQuickSearchIndicator.ShowWindow(false);
	}
	else
	{
		mQuickSearchSaved.ShowWindow(SW_HIDE);
		mQuickSearchProgress.ShowWindow(SW_SHOW);

		// Get search set
		auto_ptr<CSearchItem> spec(new CSearchItem(CSearchItem::eNamedStyle, CPreferences::sPrefs->mSearchStyles.GetValue().at(value - IDM_SEARCH_STYLES_Start)->GetName()));

		// Make sure search failure does not prevent clean-up
		try
		{
			GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec.get(), false);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}

		mQuickSearchCancel.EnableWindow(true);
		mQuickSearchIndicator.ShowWindow(true);
		GetInfoTable()->SetFocus();

		mQuickSearchProgress.ShowWindow(SW_HIDE);
		mQuickSearchSaved.ShowWindow(SW_SHOW);
	}
}

// Cancel quick search
void CMailboxInfoView::DoQuickSearchCancel()
{
	GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
	mQuickSearchCancel.EnableWindow(false);
	mQuickSearchIndicator.ShowWindow(false);

	if (mQuickSearchSaved.IsWindowVisible())
	{
		mQuickSearchSaved.SetValue(IDM_QUICKSEARCH_CHOOSE);
	}
}

// Sync quick search with current mailbox Match state
void CMailboxInfoView::SyncQuickSearch()
{
	// Ignore this if a search is already in progress as we are already sync'd
	if (mQuickSearchProgress.IsWindowVisible())
		return;

	if (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch) && (GetMbox()->GetViewSearch() != NULL))
	{
		EnableQuickSearch(true);
		const CSearchItem* spec = GetMbox()->GetViewSearch();
		
		// Some types we can handle
		long menu_item = 0;
		cdstring txt;
		switch(spec->GetType())
		{
		case CSearchItem::eFrom:
		case CSearchItem::eTo:
		case CSearchItem::eRecipient:
		case CSearchItem::eCorrespondent:
		case CSearchItem::eSubject:
		case CSearchItem::eBody:
			switch(spec->GetType())
			{
			case CSearchItem::eFrom:
				menu_item = IDM_QUICKSEARCH_FROM;
				break;
			case CSearchItem::eTo:
				menu_item = IDM_QUICKSEARCH_TO;
				break;
			case CSearchItem::eRecipient:
				menu_item = IDM_QUICKSEARCH_RECIPIENT;
				break;
			case CSearchItem::eCorrespondent:
				menu_item = IDM_QUICKSEARCH_CORRESPONDENT;
				break;
			case CSearchItem::eSubject:
				menu_item = IDM_QUICKSEARCH_SUBJECT;
				break;
			case CSearchItem::eBody:
				menu_item = IDM_QUICKSEARCH_BODY;
				break;
			default:;
			}
			if (spec->GetData() != NULL)
				txt = *reinterpret_cast<const cdstring*>(spec->GetData());
			
			// Now set controls
			mQuickSearchCriteria.SetValue(menu_item);
			mQuickSearchText.SetText(txt);
			DoQuickSearchPopup(menu_item);
			mQuickSearchCancel.EnableWindow(true);
			mQuickSearchIndicator.ShowWindow(true);
			break;
		case CSearchItem::eNamedStyle:
			if (spec->GetData() != NULL)
				txt = *reinterpret_cast<const cdstring*>(spec->GetData());
			menu_item = CPreferences::sPrefs->mSearchStyles.GetValue().FindIndexOf(txt);
			
			// Now set controls
			mQuickSearchCriteria.SetValue(IDM_QUICKSEARCH_SAVEDSEARCH);
			DoQuickSearchPopup(IDM_QUICKSEARCH_SAVEDSEARCH);
			if (menu_item != -1)
				mQuickSearchSaved.SetValue(IDM_SEARCH_STYLES_Start + menu_item);
			else
				mQuickSearchSaved.SetValue(IDM_QUICKSEARCH_CHOOSE);
			mQuickSearchCancel.EnableWindow(true);
			mQuickSearchIndicator.ShowWindow(true);
			break;
		default:;
		}
	}
	else
		// Enable/disable but not in quick search mode
		EnableQuickSearch(GetMbox() != NULL);
}

// Called during idle
void CMailboxInfoView::OnTimer(UINT nIDEvent)
{
	// See if we are equal or greater than trigger
	if (nIDEvent == mTimerID)
	{
		DoQuickSearch(false);
	}
}

// Start timer to trigger quick search
void CMailboxInfoView::StartQuickSearchTimer()
{
	// Start idle time processing
	if (mQuickSearchTimerPending && (mTimerID != 0))
	{
		KillTimer(mTimerID);
		mTimerID = 0;
	}
	mTimerID = SetTimer(IDT_TIMER_ID, 500, NULL);
	mQuickSearchTimerPending = true;
}

// Set window state
void CMailboxInfoView::ResetState(bool force)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Get name as cstr
	cdstring name(mbox->GetAccountName());

	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Get window state from prefs
	CMailboxWindowState* state = CPreferences::sPrefs->GetMailboxWindowInfo(name);

	// If no prefs try default
	if (!state || force)
		state = &CPreferences::sPrefs->mMailboxWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mMailboxWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, NULL);

			// Reset bounds
			GetParentFrame()->SetWindowPos(nil, set_rect.left, set_rect.top, set_rect.Width(), set_rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
		}
	}

	// Prevent window updating while column info is invalid
	bool locked = GetParentFrame()->LockWindowUpdate();

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mMailboxWindowDefault.GetValue()));

	// Do match BEFORE sorting so that sort is only done on the matched set rather than being done twice

	// Only do search state if required
	if (!mNoSearchReset)
	{
		// Match bits and force mbox update
		GetInfoTable()->SetCurrentMatch(state->GetMatchItem());
		GetInfoTable()->SetMatch(state->GetViewMode(), GetInfoTable()->GetCurrentMatch().ConstructSearch(NULL), false);
	}

	// Adjust menus
	SetSortBy(state->GetSortBy());
	mbox->ShowBy(state->GetShowBy());

	// Sorting button
	//mSortBtn.SetPushed(state->GetShowBy() == cShowMessageDescending);

	if (force)
		SaveState();

	if (locked)
		GetParentFrame()->UnlockWindowUpdate();

	// Do zoom
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
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
void CMailboxInfoView::SaveState(void)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Can only do if mbox still open
	if (!mbox->IsOpen())
		return;

	// Get name as cstr
	cdstring name(mbox->GetAccountName());

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

	// Get current match item
	CMatchItem match;
	if (!mbox->GetAutoViewMode())
		match = GetInfoTable()->GetCurrentMatch();
	NMbox::EViewMode mode = mbox->GetAutoViewMode() ? NMbox::eViewMode_All : mbox->GetViewMode();
	if (!::accumulate(match.GetBitsSet().begin(), match.GetBitsSet().end(), false, logical_or<bool>()) && !match.GetSearchSet().size())
		mode = NMbox::eViewMode_All;

	// Add info to prefs
	CMailboxWindowState* info = new CMailboxWindowState(name,
														&bounds,
														zoomed ? eWindowStateMax : eWindowStateNormal,
														&mColumnInfo,
														(ESortMessageBy) mbox->GetSortBy(),
														(EShowMessageBy) mbox->GetShowBy(),
														mode,
														&match,
														Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (info)
		CPreferences::sPrefs->AddMailboxWindowInfo(info);
}

// Save current state as default
void CMailboxInfoView::SaveDefaultState(void)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
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
		mColumnInfo[i].column_width = GetTable()->GetColWidth(i + 1);

	// Get current match item
	CMatchItem match;
	if (!mbox->GetAutoViewMode())
		match = GetInfoTable()->GetCurrentMatch();

	// Add info to prefs
	CMailboxWindowState state(nil,
								&bounds,
								zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo,
								(ESortMessageBy) mbox->GetSortBy(),
								(EShowMessageBy) mbox->GetShowBy(),
								mbox->GetAutoViewMode() ? NMbox::eViewMode_All : mbox->GetViewMode(),
								&match,
								Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mMailboxWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMailboxWindowDefault.SetDirty();

}
