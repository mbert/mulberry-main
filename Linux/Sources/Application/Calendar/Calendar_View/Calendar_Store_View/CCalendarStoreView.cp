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
#include "CCalendarStoreTitleTable.h"
#include "CCalendarStoreToolbar.h"
#include "CCalendarStoreWindow.h"
#include "CCommands.h"
#include "CCalendarView.h"
#include "CFocusBorder.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CToolbarView.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneWindow.h"

#include "CCalendarStoreManager.h"

#include <algorithm>
#include <cassert>

// Static members

cdmutexprotect<CCalendarStoreView::CCalendarStoreViewList> CCalendarStoreView::sCalendarStoreViews;

// ---------------------------------------------------------------------------
//	CCalendarStoreView														  [public]
/**
	Default constructor */

CCalendarStoreView::CCalendarStoreView(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CTableView(enclosure, hSizing, vSizing, x, y, w, h)
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

// Setup help balloons
void CCalendarStoreView::OnCreate()
{
	// Do inherited
	SetWindowInfo(eCalendarStoreColumnSubscribe, 16, true);
	CTableView::OnCreate();
	
// begin JXLayout1

    CFocusBorder* focus_border =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,200);
    assert( focus_border != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(focus_border,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 494,194);
    assert( sbs != NULL );

// end JXLayout1

	// Only use focus if 3pane
	if (!Is3Pane())
	{
		focus_border->HasFocus(false);
		focus_border->SetBorderWidth(0);
	}

	// Zoom scrollbar pane to fit the view
	sbs->FitToEnclosure(kTrue, kTrue);

	// Create table and it's titles inside scrollbar set
	mTitles = new CCalendarStoreTitleTable(sbs, sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 105, cTitleHeight);
	mTable = new CCalendarStoreTable(sbs,sbs->GetScrollEnclosure(),
								JXWidget::kHElastic, JXWidget::kVElastic, 0,cTitleHeight, 105, 100);
	mTable->OnCreate();
	mTitles->OnCreate();

	PostCreate(mTable, mTitles);

	// Set status
	SetOpen();
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
		CCalendarStoreToolbar* tb = new CCalendarStoreToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
		tb->OnCreate();

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);

		// Force toolbar to be active
		Broadcast_Message(eBroadcast_ViewActivate, this);
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

//	Respond to commands
bool CCalendarStoreView::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch (cmd)
	{
	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
		OnFileSave();
		return true;

	// Pass these off to the calendar view
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
	case CCommand::eCalendarNewToDo:
	case CCommand::eToolbarCalendarNewToDoBtn:
	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
	case CCommand::eToolbarCalendarDayBtn:
	case CCommand::eToolbarCalendarWorkBtn:
	case CCommand::eToolbarCalendarWeekBtn:
	case CCommand::eToolbarCalendarMonthBtn:
	case CCommand::eToolbarCalendarYearBtn:
	case CCommand::eToolbarCalendarSummaryBtn:
	case CCommand::eToolbarCalendarTasksBtn:
		return (mCalendarView != NULL) ? mCalendarView->ObeyCommand(cmd, menu) : false;

	default:;
	};

	return CTableView::ObeyCommand(cmd, menu);
}

//	Pass back status of a (menu) command
void CCalendarStoreView::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch (cmd)
	{
	case CCommand::eFileSave:
	case CCommand::eToolbarFileSaveBtn:
		// Always enabled
		OnUpdateAlways(cmdui);
		return;

	// Pass these off to the calendar view
	case CCommand::eCalendarNewEvent:
	case CCommand::eToolbarCalendarNewEventBtn:
	case CCommand::eCalendarNewToDo:
	case CCommand::eToolbarCalendarNewToDoBtn:
	case CCommand::eCalendarGotoToday:
	case CCommand::eToolbarCalendarTodayBtn:
	case CCommand::eCalendarGotoDate:
	case CCommand::eToolbarCalendarGotoBtn:
	case CCommand::eToolbarCalendarDayBtn:
	case CCommand::eToolbarCalendarWorkBtn:
	case CCommand::eToolbarCalendarWeekBtn:
	case CCommand::eToolbarCalendarMonthBtn:
	case CCommand::eToolbarCalendarYearBtn:
	case CCommand::eToolbarCalendarSummaryBtn:
	case CCommand::eToolbarCalendarTasksBtn:
		if (mCalendarView != NULL)
		{
			mCalendarView->UpdateCommand(cmd, cmdui);
			return;
		}
		break;

	default:;
	}

	CTableView::UpdateCommand(cmd, cmdui);
}

// Reset state from prefs
void CCalendarStoreView::ResetState(bool force)
{
	// Get default state
	CCalendarStoreWindowState* state = &CPreferences::sPrefs->mCalendarStoreWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mCalendarStoreWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect);

			// Reset bounds
			GetCalendarStoreWindow()->GetWindow()->Place(set_rect.left, set_rect.top);
			GetCalendarStoreWindow()->GetWindow()->SetSize(set_rect.width(), set_rect.height());
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mCalendarStoreWindowDefault.GetValue()));

	// Set zoom state if 1-pane
	//if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	//{
	//	GetCalendarStoreWindow()->ResetStandardSize();
	//	GetCalendarStoreWindow()->DoSetZoom(true);
	//}

	// If forced reset, save it
	if (force)
		SaveDefaultState();

	// Always activate
	if (!Is3Pane())
	{
		// Always activate
		GetCalendarStoreWindow()->Activate();
	}
}

// Save current state in prefs
void CCalendarStoreView::SaveDefaultState(void)
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

	// Sync column state
	SyncColumns();

	// Add info to prefs
	cdstrvect empty;
	CCalendarStoreWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo, &empty);
	if (CPreferences::sPrefs->mCalendarStoreWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mCalendarStoreWindowDefault.SetDirty();
}
