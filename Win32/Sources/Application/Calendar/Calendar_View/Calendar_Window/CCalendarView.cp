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

#include "CCalendarView.h"

#include "CCalendarStoreView.h"
#include "CCalendarToolbar.h"
#include "CCalendarWindow.h"
#include "CDayWeekView.h"
#include "CEventPreview.h"
#include "CMonthView.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSummaryView.h"
#include "CToDoView.h"
#include "CYearView.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneWindow.h"

#include "CICalendarManager.h"
#include "CCalendarStoreManager.h"

cdmutexprotect<CCalendarView::CCalendarViewList> CCalendarView::sCalendarViews;

BEGIN_MESSAGE_MAP(CCalendarView, CBaseView)
	//ON_UPDATE_COMMAND_UI(ID_FILE_SAVE, OnUpdateAlways)
	ON_COMMAND(ID_FILE_SAVE, OnFileSave)

	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	
	ON_UPDATE_COMMAND_UI(IDM_CALENDAR_NEWTODO, OnUpdateNewToDo)
	ON_COMMAND(IDM_CALENDAR_NEWTODO, OnNewToDoBtn)

	// Toolbar
	ON_COMMAND(IDC_TOOLBARFILESAVEBTN, OnFileSave)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_NEWTODO, OnUpdateNewToDo)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_NEWTODO, OnNewToDoBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_DAY, OnUpdateDayBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_DAY, OnDayBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_WORK, OnUpdateWorkWeekBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_WORK, OnWorkWeekBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_WEEK, OnUpdateWeekBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_WEEK, OnWeekBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_MONTH, OnUpdateMonthBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_MONTH, OnMonthBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_YEAR, OnUpdateYearBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_YEAR, OnYearBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_SUMMARY, OnUpdateSummaryBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_SUMMARY, OnSummaryBtn)

	ON_UPDATE_COMMAND_UI(IDC_TOOLBAR_CALENDAR_TASKS, OnUpdateToDoBtn)
	ON_COMMAND(IDC_TOOLBAR_CALENDAR_TASKS, OnToDoBtn)

END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CCalendarView														  [public]
/**
	Default constructor */

CCalendarView::CCalendarView()
{
	mViewType = NCalendarView::eViewMonth;
	mShowToDo = true;
	mYearLayout = NCalendarView::e3x4;
	mDayWeekRange = CDayWeekViewTimeRange::e24Hrs;
	mDayWeekScale = 0;
	mSummaryType = NCalendarView::eList;
	mSummaryRange = NCalendarView::eThisWeek;
	mSingleCalendar = false;
	mCalendar = NULL;
	mCalendarsView = NULL;
	mCurrentView = NULL;
	mToDoView = NULL;
	mToolbar = NULL;
	mPreview = NULL;
	mUsePreview = true;
	mIgnoreValueFieldChanged = false;
	
	// Add to list
	{
		cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
		sCalendarViews->insert(this);
	}
}


// ---------------------------------------------------------------------------
//	~CCalendarView														  [public]
/**
	Destructor */

CCalendarView::~CCalendarView()
{
	// Remove from list
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	sCalendarViews->erase(this);
	
	// Delete these dynamic views
	delete mCurrentView;
	delete mToDoView;
}

#pragma mark -

const int cToDoWidth = 170;

int CCalendarView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBaseView::OnCreate(lpCreateStruct) == -1)
		return -1;

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Focus ring
	mFocusRing.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(0, 0, width, height), this, IDC_STATIC);
	if (Is3Pane())
		mFocusRing.SetFocusBorder();
	AddAlignment(new CWndAlignment(&mFocusRing, CWndAlignment::eAlign_WidthHeight));
	UINT focus_indent = Is3Pane() ? 3 : 0;

	// Create event container and set alignment
	mViewContainer.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(focus_indent, focus_indent, width - focus_indent, height - focus_indent), &mFocusRing, IDC_STATIC);
	mFocusRing.AddAlignment(new CWndAlignment(&mViewContainer, CWndAlignment::eAlign_WidthHeight));
	mCurrentView = NULL;

	// Create multi-pane container and set alignment
	mToDoViewContainer.Create(_T(""), WS_CHILD | WS_VISIBLE, CRect(width - cToDoWidth, focus_indent, width - focus_indent, height - focus_indent), &mFocusRing, IDC_STATIC);
	mFocusRing.AddAlignment(new CWndAlignment(&mToDoViewContainer, CWndAlignment::eAlign_RightHeight));
	mToDoView = NULL;

	ShowToDo(false);

	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)	
		calstore::CCalendarStoreManager::sCalendarStoreManager->Add_Listener(this);
	
	// Set commander
	if (Is3Pane())
		SetSuperCommander(C3PaneWindow::s3PaneWindow);

	return 0;
}

void CCalendarView::OnDestroy()
{
	// Save window state
	SaveState();

	// Do default action now
	CBaseView::OnDestroy();
}

// Tell window to focus on this one
void CCalendarView::OnSetFocus(CWnd* pOldWnd)
{
	// Force focus to table
	if (mCurrentView != NULL)
		mCurrentView->SetFocus();
}

BOOL CCalendarView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Route some messages to the current view table
	switch(nID)
	{
	case IDM_CALENDAR_NEWEVENT:
	case IDC_TOOLBAR_CALENDAR_NEWEVENT:
	case IDM_CALENDAR_GOTO_TODAY:
	case IDC_TOOLBAR_CALENDAR_TODAY:
	case IDM_CALENDAR_GOTO_DATE:
	case IDC_TOOLBAR_CALENDAR_GOTO:
		if (mCurrentView != NULL)
			return mCurrentView->GetTable()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
		break;
	default:;
	}

	// Do inherited
	return CBaseView::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

// Make a toolbar appropriate for this view
void CCalendarView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		mToolbar = C3PaneWindow::s3PaneWindow->GetCalendarToolbar();
		Add_Listener(mToolbar);
		mToolbar->AddCommander(this);
	}
	else
	{
		// Create a suitable toolbar
		mToolbar = new CCalendarToolbar;
		mToolbar->InitToolbar(false, parent);
		
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(mToolbar);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(mToolbar, this, CToolbarView::eStdButtonsGroup);
	}
}

// Focus on the table
bool CCalendarView::HasFocus() const
{
	return false;
}

// Focus on the table
void CCalendarView::Focus()
{
	if (!Is3Pane())
		// Make window active
		FRAMEWORK_WINDOW_TO_TOP(GetOwningWindow())

	// Always focus on event table
	if (IsWindowVisible() && mCurrentView)
		mCurrentView->GetTable()->SetFocus();
}

void CCalendarView::OnUpdateNewToDo(CCmdUI* pCmdUI)
{
	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
	{
		const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
		pCmdUI->Enable((mCalendar != NULL) || (cals.size() > 0));
	}
	else
		pCmdUI->Enable(false);
}

void CCalendarView::OnUpdateDayBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mViewType == NCalendarView::eViewDay);
}

void CCalendarView::OnUpdateWorkWeekBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mViewType == NCalendarView::eViewWorkWeek);
}

void CCalendarView::OnUpdateWeekBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mViewType == NCalendarView::eViewWeek);
}

void CCalendarView::OnUpdateMonthBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mViewType == NCalendarView::eViewMonth);
}

void CCalendarView::OnUpdateSummaryBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mViewType == NCalendarView::eViewSummary);
}

void CCalendarView::OnUpdateYearBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mViewType == NCalendarView::eViewYear);
}

void CCalendarView::OnUpdateToDoBtn(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(true);
	pCmdUI->SetCheck(mShowToDo);
}

void CCalendarView::ResetView(NCalendarView::EViewType type, iCal::CICalendarDateTime* dt)
{
	// Only if different or non-existant
	if ((mViewType == type) && (mCurrentView != NULL))
	{
		return;
	}
	
	// Sync any state data from the current view
	if (mCurrentView != NULL)
	{
		switch(mViewType)
		{
		case NCalendarView::eViewDay:
		case NCalendarView::eViewWorkWeek:
		case NCalendarView::eViewWeek:
			mDayWeekRange = static_cast<CDayWeekView*>(mCurrentView)->GetRange();
			mDayWeekScale = static_cast<CDayWeekView*>(mCurrentView)->GetScale();
			break;
		case NCalendarView::eViewMonth:
			break;
		case NCalendarView::eViewYear:
			mYearLayout = static_cast<CYearView*>(mCurrentView)->GetLayout();
			break;
		case NCalendarView::eViewSummary:
			mSummaryType = static_cast<CSummaryView*>(mCurrentView)->GetType();
			mSummaryRange = static_cast<CSummaryView*>(mCurrentView)->GetRange();
			break;
		default:;
		}
		
		// Remove current view from toolbar commander chain
		mToolbar->RemoveCommander(mCurrentView->GetTable());
	}

	// Now change the tyope
	mViewType = type;

	// Get current views date
	iCal::CICalendarDateTime date((dt != NULL) ? *dt : (mCurrentView != NULL ? mCurrentView->GetSelectedDate() : iCal::CICalendarDateTime::GetToday()));
	if (mCurrentView == NULL)
		date.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());

	// Delete any existing view
	bool view_had_focus = false;
	if (mCurrentView != NULL)
	{
		view_had_focus = mCurrentView->GetTable()->IsTarget();

		mViewContainer.RemoveChildAlignment(mCurrentView);
		delete mCurrentView;
		mCurrentView = NULL;
	}
	
	CRect tframe;
	mViewContainer.GetWindowRect(tframe);

	// Read the mailbox view resource

	// Create new view
	switch(mViewType)
	{
	case NCalendarView::eViewDay:
	case NCalendarView::eViewWorkWeek:
	case NCalendarView::eViewWeek:
		mCurrentView = new CDayWeekView(this);
		mCurrentView->SetCalendarView(this);
		mCurrentView->CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, tframe, &mViewContainer, IDC_STATIC);
		::ExpandChildToFit(&mViewContainer, mCurrentView, true, true);
		mViewContainer.AddAlignment(new CWndAlignment(mCurrentView, CWndAlignment::eAlign_WidthHeight));
		switch(mViewType)
		{
		case NCalendarView::eViewDay:
		default:
			static_cast<CDayWeekView*>(mCurrentView)->SetType(CDayWeekView::eDay);
			break;
		case NCalendarView::eViewWorkWeek:
			static_cast<CDayWeekView*>(mCurrentView)->SetType(CDayWeekView::eWorkWeek);
			break;
		case NCalendarView::eViewWeek:
			static_cast<CDayWeekView*>(mCurrentView)->SetType(CDayWeekView::eWeek);
			break;
		}
		static_cast<CDayWeekView*>(mCurrentView)->SetRange(mDayWeekRange);
		static_cast<CDayWeekView*>(mCurrentView)->SetScale(mDayWeekScale);
		break;
	case NCalendarView::eViewMonth:
	default:
		mCurrentView = new CMonthView(this);
		mCurrentView->SetCalendarView(this);
		mCurrentView->CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, tframe, &mViewContainer, IDC_STATIC);
		::ExpandChildToFit(&mViewContainer, mCurrentView, true, true);
		mViewContainer.AddAlignment(new CWndAlignment(mCurrentView, CWndAlignment::eAlign_WidthHeight));
		break;
	case NCalendarView::eViewYear:
		mCurrentView = new CYearView(this);
		mCurrentView->SetCalendarView(this);
		mCurrentView->CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, tframe, &mViewContainer, IDC_STATIC);
		::ExpandChildToFit(&mViewContainer, mCurrentView, true, true);
		mViewContainer.AddAlignment(new CWndAlignment(mCurrentView, CWndAlignment::eAlign_WidthHeight));
		static_cast<CYearView*>(mCurrentView)->SetLayout(mYearLayout);
		break;
	case NCalendarView::eViewSummary:
		mCurrentView = new CSummaryView(this);
		mCurrentView->SetCalendarView(this);
		mCurrentView->CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD, tframe, &mViewContainer, IDC_STATIC);
		::ExpandChildToFit(&mViewContainer, mCurrentView, true, true);
		mViewContainer.AddAlignment(new CWndAlignment(mCurrentView, CWndAlignment::eAlign_WidthHeight));
		static_cast<CSummaryView*>(mCurrentView)->SetType(mSummaryType);
		static_cast<CSummaryView*>(mCurrentView)->SetRange(mSummaryRange);
		break;
	}
	
	// If we have a view initialise it and show it
	if (mCurrentView != NULL)
	{
		// Listen to double-click messages and table activation
		mCurrentView->Add_Listener(this);
		
		mCurrentView->SetDate(date);
		mCurrentView->ShowWindow(SW_SHOW);
		
		// Make sure toolbar commander chain is set
		mToolbar->AddCommander(mCurrentView->GetTable());
		
		if (view_had_focus)
			mCurrentView->GetTable()->SetFocus();
	}
	
	// Update toolbars
	//mToolbarView->UpdateToolbarState();
	
	RedrawWindow();
}

void CCalendarView::ShowToDo(bool show)
{
	if (mShowToDo ^ show)
	{
		mShowToDo = show;

		CRect frame;
		mFocusRing.GetClientRect(frame);
		if (mFocusRing.CanFocus())
			frame.DeflateRect(3, 3);
		
		if (mShowToDo)
		{
			::ResizeWindowTo(&mViewContainer, frame.Width() - cToDoWidth, frame.Height(), true);

			// Create the actuial view if its not present
			if (mToDoView == NULL)
			{
				mToDoView = new CToDoView(this);
				mToDoView->SetCalendarView(this);
				mToDoView->CreateEx(WS_EX_CONTROLPARENT, _T("STATIC"), _T(""), WS_CHILD | WS_VISIBLE, frame, &mToDoViewContainer, IDC_STATIC);
				::ExpandChildToFit(&mToDoViewContainer, mToDoView, true, true);
				mToDoViewContainer.AddAlignment(new CWndAlignment(mToDoView, CWndAlignment::eAlign_WidthHeight));
				mToDoView->Add_Listener(this);

				// Make sure toolbar commander chain is set
				mToolbar->AddCommander(mToDoView->GetTable());
			}

			mToDoView->SetDate(iCal::CICalendarDateTime::GetToday());
			mToDoViewContainer.ShowWindow(SW_SHOW);
			
		}
		else
		{
			mToDoViewContainer.ShowWindow(SW_HIDE);
			
			::ResizeWindowTo(&mViewContainer, frame.Width(), frame.Height(), true);
		}
		
		if (mToolbar != NULL)
			mToolbar->UpdateToolbarState();
	}
}

void CCalendarView::ResetState(bool force)
{
	// Get visible state
	bool visible = GetParentFrame()->IsWindowVisible();

	// Use "Calendars" for the 1-pane/3-pane calendar manager
	// Use calendar name for other 1-pane windows
	cdstring name = cdstring("Calendars");

	// Check for available state
	CCalendarWindowState* state = CPreferences::sPrefs->GetCalendarWindowInfo(name);
	if (!state || force)
		state = &CPreferences::sPrefs->mCalendarWindowDefault.Value();

	// Do not set if empty
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mCalendarWindowDefault.GetValue());
	if (!set_rect.IsRectNull())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetCalendarWindow());

			// Reset bounds
			GetParentFrame()->SetWindowPos(NULL, set_rect.left, set_rect.top,
											set_rect.Width(), set_rect.Height(), SWP_NOZORDER | (visible ? 0 : SWP_NOREDRAW));
		}
	}

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
		GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetCalendarWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	// Set side panel views
	if (mShowToDo ^ state->GetShowToDo())
		ShowToDo(state->GetShowToDo());
	mYearLayout = state->GetYearLayout();
	mDayWeekRange = state->GetDayWeekRange();
	mDayWeekScale = state->GetDayWeekScale();
	mSummaryType = state->GetSummaryType();
	mSummaryRange = state->GetSummaryRange();

	// Set type of view
	switch(state->GetType())
	{
	case NCalendarView::eViewDay:
		OnDayBtn();
		break;
	case NCalendarView::eViewWorkWeek:
		OnWorkWeekBtn();
		break;
	case NCalendarView::eViewWeek:
		OnWeekBtn();
		break;
	case NCalendarView::eViewMonth:
		OnMonthBtn();
		break;
	case NCalendarView::eViewYear:
		OnYearBtn();
		break;
	case NCalendarView::eViewSummary:
		OnSummaryBtn();
		break;
	}

	// If forced reset, save it
	if (force)
		SaveState();
	
	if (!force && !Is3Pane())
	{
		if (!GetParentFrame()->IsWindowVisible())
			GetParentFrame()->ActivateFrame();
		RedrawWindow();
	}
	else
		RedrawWindow();
}

void CCalendarView::SaveState()
{
	// Use "Calendars" for the 1-pane/3-pane calendar manager
	// Use calendar name for other 1-pane windows
	cdstring name = cdstring("Calendars");

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

	// Add info to prefs
	CCalendarWindowState* info = new CCalendarWindowState(name, &bounds, wstate, mViewType,
								mShowToDo, mYearLayout, mDayWeekRange, mDayWeekScale, mSummaryType, mSummaryRange,
								Is3Pane() ? 0 : GetCalendarWindow()->GetSplitter()->GetRelativeSplitPos());
	if (info)
		CPreferences::sPrefs->AddCalendarWindowInfo(info);
}

void CCalendarView::SaveDefaultState()
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


	// Add info to prefs
	CCalendarWindowState state(NULL, &bounds, wstate, mViewType,
								mShowToDo, mYearLayout, mDayWeekRange, mDayWeekScale, mSummaryType, mSummaryRange,
								Is3Pane() ? 0 : GetCalendarWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mCalendarWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mCalendarWindowDefault.SetDirty();
}
