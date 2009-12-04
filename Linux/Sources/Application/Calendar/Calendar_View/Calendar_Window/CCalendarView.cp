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

#include "CCalendarView.h"

#include "CCalendarStoreView.h"
#include "CCalendarToolbar.h"
#include "CCalendarWindow.h"
#include "CCommands.h"
#include "CDayWeekView.h"
#include "CEventPreview.h"
#include "CFreeBusyView.h"
#include "CFocusBorder.h"
#include "CMonthView.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSplitterView.h"
#include "CSummaryView.h"
#include "CToDoView.h"
#include "CToolbarView.h"
#include "CYearView.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneWindow.h"

#include "CICalendarManager.h"
#include "CCalendarStoreManager.h"

#include <JXFlatRect.h>

#include <cassert>

cdmutexprotect<CCalendarView::CCalendarViewList> CCalendarView::sCalendarViews;

// ---------------------------------------------------------------------------
//	CCalendarView														  [public]
/**
	Default constructor */

CCalendarView::CCalendarView(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h) :
	CBaseView(enclosure, hSizing, vSizing, x, y, w, h)
{
	mViewType = NCalendarView::eViewMonth;
	mShowToDo = true;
	mYearLayout = NCalendarView::e3x4;
	mDayWeekRange = CDayWeekViewTimeRange::e24Hrs;
	mDayWeekScale = 0;
	mSummaryType = NCalendarView::eList;
	mSummaryRange = NCalendarView::eThisWeek;
	mFreeBusyRange = CDayWeekViewTimeRange::e24Hrs;
	mFreeBusyScale = 0;
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
}

#pragma mark -

void CCalendarView::OnCreate()
{
// begin JXLayout1

    CFocusBorder* focus_border =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,200);
    assert( focus_border != NULL );

    JXFlatRect* view =
        new JXFlatRect(focus_border,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 494,194);
    assert( view != NULL );

    mViewContainer =
        new JXWidgetSet(view,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 247,194);
    assert( mViewContainer != NULL );

    mToDoViewContainer =
        new JXWidgetSet(view,
                    JXWidget::kFixedRight, JXWidget::kVElastic, 247,0, 247,194);
    assert( mToDoViewContainer != NULL );

// end JXLayout1

	CBaseView::OnCreate();

	// Only use focus if 3pane
	if (!Is3Pane())
	{
		focus_border->HasFocus(false);
		focus_border->SetBorderWidth(0);
	}

	// Zoom pane to fit the view
	view->FitToEnclosure(kTrue, kTrue);

	// Get the UI objects
	mCurrentView = NULL;
	mToDoView = NULL;

	ShowToDo(false);

	if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)	
		calstore::CCalendarStoreManager::sCalendarStoreManager->Add_Listener(this);
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
		CCalendarToolbar* tb = new CCalendarToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
		tb->OnCreate();
		mToolbar = tb;
		
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(mToolbar);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(mToolbar, this, CToolbarView::eStdButtonsGroup);

		// Force toolbar to be active
		Broadcast_Message(eBroadcast_ViewActivate, this);
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

	// Always focus on the table
	if (IsVisible() && (mCurrentView != NULL))
		// Switch target to the table (will focus)
		SetTarget(mCurrentView->GetTable());
}

//	Respond to commands
bool CCalendarView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
		OnFileSave();
		return true;

	case CCommand::eCalendarNewToDo:
	case CCommand::eToolbarCalendarNewToDoBtn:
		OnNewToDoBtn();
		return true;

	case CCommand::eToolbarCalendarDayBtn:
		OnDayBtn();
		return true;

	case CCommand::eToolbarCalendarWorkBtn:
		OnWorkWeekBtn();
		return true;

	case CCommand::eToolbarCalendarWeekBtn:
		OnWeekBtn();
		return true;

	case CCommand::eToolbarCalendarMonthBtn:
		OnMonthBtn();
		return true;

	case CCommand::eToolbarCalendarYearBtn:
		OnYearBtn();
		return true;

	case CCommand::eToolbarCalendarSummaryBtn:
		OnSummaryBtn();
		return true;

	case CCommand::eToolbarCalendarTasksBtn:
		OnToDoBtn();
		return true;

	// Pass these off to the calendar table
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
		if (mCurrentView != NULL)
			return mCurrentView->GetTable()->ObeyCommand(cmd, menu);
		break;

	case CCommand::eCalendarCheck:
	case CCommand::eToolbarCalendarCheckBtn:
		OnCheckCalendar();
		return true;

	default:;
	};

	return CBaseView::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CCalendarView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	case CCommand::eCalendarNewToDo:
	case CCommand::eToolbarCalendarNewToDoBtn:
		OnUpdateNewToDo(cmdui);
		return;

	case CCommand::eToolbarCalendarDayBtn:
		OnUpdateDayBtn(cmdui);
		return;

	case CCommand::eToolbarCalendarWorkBtn:
		OnUpdateWorkWeekBtn(cmdui);
		return;

	case CCommand::eToolbarCalendarWeekBtn:
		OnUpdateWeekBtn(cmdui);
		return;

	case CCommand::eToolbarCalendarMonthBtn:
		OnUpdateMonthBtn(cmdui);
		return;

	case CCommand::eToolbarCalendarYearBtn:
		OnUpdateYearBtn(cmdui);
		return;

	case CCommand::eToolbarCalendarSummaryBtn:
		OnUpdateSummaryBtn(cmdui);
		return;

	case CCommand::eToolbarCalendarTasksBtn:
		OnUpdateToDoBtn(cmdui);
		return;

	// Pass these off to the calendar table
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
		if (mCurrentView != NULL)
		{
			mCurrentView->GetTable()->UpdateCommand(cmd, cmdui);
			return;
		}
		break;

	case CCommand::eCalendarCheck:
	case CCommand::eToolbarCalendarCheckBtn:
		OnUpdateAlways(cmdui);
		return;

	default:;
	}

	CBaseView::UpdateCommand(cmd, cmdui);
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
		case NCalendarView::eViewFreeBusy:
			mFreeBusyRange = static_cast<CFreeBusyView*>(mCurrentView)->GetRange();
			mFreeBusyScale = static_cast<CFreeBusyView*>(mCurrentView)->GetScale();
			break;
		default:;
		}
		
		// Remove current view from toolbar commander chain
		mToolbar->RemoveCommander(mCurrentView->GetTable());
	}

	// Now change the type
	mViewType = type;

	// Get current views date
	iCal::CICalendarDateTime date((dt != NULL) ? *dt : (mCurrentView != NULL ? mCurrentView->GetSelectedDate() : iCal::CICalendarDateTime::GetToday()));
	if (mCurrentView == NULL)
		date.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());

	// Delete any existing view
	// NB Need to set mCurrentView to NULL before delete as the delete operation
	// will result in some call backs into this object and we need to not attempt
	// to access mCurrentView at that point
	CCalendarViewBase* temp = mCurrentView;
	mCurrentView = NULL;
	delete temp;
	

	// Read the mailbox view resource

	// Create new view
	switch(mViewType)
	{
	case NCalendarView::eViewDay:
	case NCalendarView::eViewWorkWeek:
	case NCalendarView::eViewWeek:
		mCurrentView = new CDayWeekView(mViewContainer, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
		mCurrentView->OnCreate();
		mCurrentView->FitToEnclosure();
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
		mCurrentView = new CMonthView(mViewContainer, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
		mCurrentView->OnCreate();
		mCurrentView->FitToEnclosure();
		break;
	case NCalendarView::eViewYear:
		mCurrentView = new CYearView(mViewContainer, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
		mCurrentView->OnCreate();
		mCurrentView->FitToEnclosure();
		static_cast<CYearView*>(mCurrentView)->SetLayout(mYearLayout);
		break;
	case NCalendarView::eViewSummary:
		mCurrentView = new CSummaryView(mViewContainer, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
		mCurrentView->OnCreate();
		mCurrentView->FitToEnclosure();
		static_cast<CSummaryView*>(mCurrentView)->SetType(mSummaryType);
		static_cast<CSummaryView*>(mCurrentView)->SetRange(mSummaryRange);
		break;
	case NCalendarView::eViewFreeBusy:
		mCurrentView = new CFreeBusyView(mViewContainer, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
		mCurrentView->OnCreate();
		mCurrentView->FitToEnclosure();
		static_cast<CFreeBusyView*>(mCurrentView)->SetRange(mFreeBusyRange);
		static_cast<CFreeBusyView*>(mCurrentView)->SetScale(mFreeBusyScale);
		break;
	}
	
	// If we have a view initialise it and show it
	if (mCurrentView != NULL)
	{
		mCurrentView->SetCalendarView(this);

		// Listen to double-click messages and table activation
		mCurrentView->Add_Listener(this);
		
		mCurrentView->SetDate(date);
		mCurrentView->SetVisible(kTrue);
		
		// Make sure toolbar commander chain is set
		mToolbar->AddCommander(mCurrentView->GetTable());
	}
	
	// Update toolbars
	//mToolbarView->UpdateToolbarState();
	
	Refresh();
}

void CCalendarView::ShowToDo(bool show)
{
	if (mShowToDo ^ show)
	{
		mShowToDo = show;

		JRect tframe = mToDoViewContainer->GetFrame();
		
		if (mShowToDo)
		{
			mViewContainer->AdjustSize(-tframe.width(), 0);

			// Create the actuial view if its not present
			if (mToDoView == NULL)
			{
				mToDoView = new CToDoView(mToDoViewContainer, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 500, 200);
				mToDoView->OnCreate();
				mToDoView->FitToEnclosure();
				mToDoView->SetCalendarView(this);
				mToDoView->Add_Listener(this);

				// Make sure toolbar commander chain is set
				mToolbar->AddCommander(mToDoView->GetTable());
			}

			mToDoView->SetDate(iCal::CICalendarDateTime::GetToday());
			mToDoViewContainer->SetVisible(kTrue);
			
		}
		else
		{
			mToDoViewContainer->SetVisible(kFalse);
			
			mViewContainer->AdjustSize(tframe.width(), 0);
		}
		
		if (mToolbar != NULL)
			mToolbar->UpdateToolbarState();
	}
}

void CCalendarView::ResetFont(const SFontInfo& finfo)
{
	if (mCurrentView != NULL)
		mCurrentView->ResetFont(finfo);
	if (mToDoView != NULL)
		mToDoView->ResetFont(finfo);
}

void CCalendarView::ResetState(bool force)
{
	// Use "Calendars" for the 1-pane/3-pane calendar manager
	// Use calendar name for other 1-pane windows
	cdstring name = cdstring("Calendars");

	// Check for available state
	CCalendarWindowState* state = CPreferences::sPrefs->GetCalendarWindowInfo(name);
	if (!state || force)
		state = &CPreferences::sPrefs->mCalendarWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mCalendarWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect);

			// Reset bounds
			GetWindow()->Place(set_rect.left, set_rect.top);
			GetWindow()->SetSize(set_rect.width(), set_rect.height());
		}
	}

	// Set zoom state if 1-pane
	//if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	//{
	//	//GetCalendarWindow()->ResetStandardSize();
	//	GetCalendarWindow()->DoSetZoom(true);
	//}

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
		SaveDefaultState();

	// Always activate
	if (!Is3Pane())
	{
		// Always activate
		GetCalendarWindow()->Activate();
	}
}

void CCalendarView::SaveState()
{
	// Use "Calendars" for the 1-pane/3-pane calendar manager
	// Use calendar name for other 1-pane windows
	cdstring name = cdstring("Calendars");

	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	JRect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Add info to prefs
	CCalendarWindowState* info = new CCalendarWindowState(name, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, mViewType,
								mShowToDo, mYearLayout, mDayWeekRange, mDayWeekScale, mSummaryType, mSummaryRange,
								Is3Pane() ? 0 : GetCalendarWindow()->GetSplitter()->GetRelativeSplitPos());
	if (info)
		CPreferences::sPrefs->AddCalendarWindowInfo(info);
}

void CCalendarView::SaveDefaultState()
{
	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	JRect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Add info to prefs
	CCalendarWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, mViewType,
								mShowToDo, mYearLayout, mDayWeekRange, mDayWeekScale, mSummaryType, mSummaryRange,
								Is3Pane() ? 0 : GetCalendarWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mCalendarWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mCalendarWindowDefault.SetDirty();
}
