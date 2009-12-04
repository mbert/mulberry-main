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
#include "CCommands.h"
#include "CDayWeekView.h"
#include "CEventPreview.h"
#include "CFreeBusyView.h"
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

#include <URegions.h>

cdmutexprotect<CCalendarView::CCalendarViewList> CCalendarView::sCalendarViews;

// ---------------------------------------------------------------------------
//	CCalendarView														  [public]
/**
	Default constructor */

CCalendarView::CCalendarView(LStream* inStream) :
	CBaseView(inStream)
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
	// Save window state
	SaveState();

	// Remove from list
	cdmutexprotect<CCalendarViewList>::lock _lock(sCalendarViews);
	sCalendarViews->erase(this);
}

#pragma mark -

void CCalendarView::FinishCreateSelf()
{
	CBaseView::FinishCreateSelf();

	// Get the UI objects
	mViewContainer = dynamic_cast<LView*>(FindPaneByID(eCalendars_ID));
	mCurrentView = NULL;

	mToDoViewContainer = dynamic_cast<LView*>(FindPaneByID(eToDo_ID));
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
		mToolbar = static_cast<CCalendarToolbar*>(UReanimator::CreateView(paneid_CalendarToolbar1, parent, NULL));
		
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

	// Always focus on the table
	if (IsVisible())
		// Switch target to the table (will focus)
		LCommander::SwitchTarget(mCurrentView->GetTable());
	else
		// Make the table the target to use when this is activated
		GetOwningWindow()->SetLatentSub(mCurrentView->GetTable());
}

//	Respond to commands
Boolean CCalendarView::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Save:
	case cmd_ToolbarFileSaveBtn:
		OnFileSave();
		break;

	case cmd_NewToDo:
	case cmd_ToolbarNewToDoBtn:
		OnNewToDoBtn();
		break;

	case cmd_ToolbarCalendarDayBtn:
		OnDayBtn();
		break;

	case cmd_ToolbarCalendarWorkWeekBtn:
		OnWorkWeekBtn();
		break;

	case cmd_ToolbarCalendarWeekBtn:
		OnWeekBtn();
		break;

	case cmd_ToolbarCalendarMonthBtn:
		OnMonthBtn();
		break;

	case cmd_ToolbarCalendarYearBtn:
		OnYearBtn();
		break;

	case cmd_ToolbarCalendarSummaryBtn:
		OnSummaryBtn();
		break;

	case cmd_ToolbarCalendarToDoBtn:
		OnToDoBtn();
		break;

	// Pass these off to the calendar table
	case cmd_NewEvent:
	case cmd_ToolbarNewEventBtn:
	case cmd_GotoToday:
	case cmd_ToolbarShowTodayBtn:
	case cmd_GotoDate:
	case cmd_ToolbarGoToDateBtn:
		cmdHandled = mCurrentView->GetTable()->ObeyCommand(inCommand, ioParam);
		break;

	case cmd_CheckCalendar:
	case cmd_ToolbarCheckMailboxBtn:
		OnCheckCalendar();
		break;

	default:
		cmdHandled = CBaseView::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CCalendarView::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
	case cmd_Save:
	case cmd_ToolbarFileSaveBtn:
		// Always enabled
		outEnabled = true;
		break;

	case cmd_NewToDo:
	case cmd_ToolbarNewToDoBtn:
	{
		// Only if subscribed nodes
		if (calstore::CCalendarStoreManager::sCalendarStoreManager != NULL)
		{
			const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetSubscribedCalendars();
			outEnabled = (mCalendar != NULL) || (cals.size() > 0);
		}
		else
			outEnabled = false;
		break;
	}

	case cmd_ToolbarCalendarDayBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mViewType == NCalendarView::eViewDay) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarCalendarWorkWeekBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mViewType == NCalendarView::eViewWorkWeek) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarCalendarWeekBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mViewType == NCalendarView::eViewWeek) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarCalendarMonthBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mViewType == NCalendarView::eViewMonth) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarCalendarYearBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mViewType == NCalendarView::eViewYear) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarCalendarSummaryBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = (mViewType == NCalendarView::eViewSummary) ? (UInt16)checkMark : (UInt16)noMark;
		break;

	case cmd_ToolbarCalendarToDoBtn:
		// Always enabled
		outEnabled = true;
		outUsesMark = true;
		outMark = mShowToDo ? (UInt16)checkMark : (UInt16)noMark;
		break;

	// Pass these off to the calendar table
	case cmd_NewEvent:
	case cmd_ToolbarNewEventBtn:
	case cmd_GotoToday:
	case cmd_ToolbarShowTodayBtn:
		mCurrentView->GetTable()->FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;

	case cmd_CheckCalendar:
	case cmd_ToolbarCheckMailboxBtn:
		// Always enabled
		outEnabled = true;
		break;

	default:
		CBaseView::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
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

	// Now change the tyope
	mViewType = type;

	// Get current views date
	iCal::CICalendarDateTime date((dt != NULL) ? *dt : (mCurrentView != NULL ? mCurrentView->GetSelectedDate() : iCal::CICalendarDateTime::GetToday()));
	if (mCurrentView == NULL)
		date.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());

	// Delete any existing view
	delete mCurrentView;
	mCurrentView = NULL;
	

	// Read the mailbox view resource

	// Create new view
	switch(mViewType)
	{
	case NCalendarView::eViewDay:
	case NCalendarView::eViewWorkWeek:
	case NCalendarView::eViewWeek:
		mCurrentView = static_cast<CDayWeekView*>(UReanimator::CreateView(CDayWeekView::pane_ID, mViewContainer, this));
		mViewContainer->ExpandSubPane(mCurrentView, true, true);
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
		mCurrentView = static_cast<CMonthView*>(UReanimator::CreateView(CMonthView::pane_ID, mViewContainer, this));
		mViewContainer->ExpandSubPane(mCurrentView, true, true);
		break;
	case NCalendarView::eViewYear:
		mCurrentView = static_cast<CYearView*>(UReanimator::CreateView(CYearView::pane_ID, mViewContainer, this));
		mViewContainer->ExpandSubPane(mCurrentView, true, true);
		static_cast<CYearView*>(mCurrentView)->SetLayout(mYearLayout);
		break;
	case NCalendarView::eViewSummary:
		mCurrentView = static_cast<CSummaryView*>(UReanimator::CreateView(CSummaryView::pane_ID, mViewContainer, this));
		mViewContainer->ExpandSubPane(mCurrentView, true, true);
		static_cast<CSummaryView*>(mCurrentView)->SetType(mSummaryType);
		static_cast<CSummaryView*>(mCurrentView)->SetRange(mSummaryRange);
		break;
	case NCalendarView::eViewFreeBusy:
		mCurrentView = static_cast<CFreeBusyView*>(UReanimator::CreateView(CFreeBusyView::pane_ID, mViewContainer, this));
		mViewContainer->ExpandSubPane(mCurrentView, true, true);
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
		mCurrentView->SetVisible(true);
		
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

		SDimension16 tframe;
		mToDoViewContainer->GetFrameSize(tframe);
		
		if (mShowToDo)
		{
			mViewContainer->ResizeFrameBy(-tframe.width, 0, true);

			// Create the actuial view if its not present
			if (mToDoView == NULL)
			{
				mToDoView = static_cast<CToDoView*>(UReanimator::CreateView(CToDoView::pane_ID, mToDoViewContainer, this));
				mToDoViewContainer->ExpandSubPane(mToDoView, true, true);
				mToDoView->SetCalendarView(this);
				mToDoView->Add_Listener(this);

				// Make sure toolbar commander chain is set
				mToolbar->AddCommander(mToDoView->GetTable());
			}

			mToDoView->SetDate(iCal::CICalendarDateTime::GetToday());
			mToDoViewContainer->SetVisible(true);
			
		}
		else
		{
			mToDoViewContainer->SetVisible(false);
			
			mViewContainer->ResizeFrameBy(tframe.width, 0, true);
		}
		
		if (mToolbar != NULL)
			mToolbar->UpdateToolbarState();
	}
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
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mCalendarWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetCalendarWindow());

			// Reset bounds
			GetCalendarWindow()->DoSetBounds(set_rect);
		}
	}

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	{
		//GetCalendarWindow()->ResetStandardSize();
		GetCalendarWindow()->DoSetZoom(true);
	}

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
	default:
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
}

void CCalendarView::SaveState()
{
	// Use "Calendars" for the 1-pane/3-pane calendar manager
	// Use calendar name for other 1-pane windows
	cdstring name = cdstring("Calendars");

	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetCalendarWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetCalendarWindow()->GetUserBounds(user_bounds);

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
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetCalendarWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetCalendarWindow()->GetUserBounds(user_bounds);

	// Add info to prefs
	CCalendarWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, mViewType,
								mShowToDo, mYearLayout, mDayWeekRange, mDayWeekScale, mSummaryType, mSummaryRange,
								Is3Pane() ? 0 : GetCalendarWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mCalendarWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mCalendarWindowDefault.SetDirty();
}
