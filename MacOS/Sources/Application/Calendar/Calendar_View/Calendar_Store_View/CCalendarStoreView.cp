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
#include "CCommands.h"
#include "CContextMenu.h"
#include "CCalendarView.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CToolbarView.h"
#include "C3PaneCalendarToolbar.h"
#include "C3PaneWindow.h"

#include "CCalendarStoreManager.h"

#include <algorithm>

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

// Constructor from stream
CCalendarStoreView::CCalendarStoreView(LStream *inStream)
		: CTableView(inStream)
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

// Setup help balloons
void CCalendarStoreView::FinishCreateSelf(void)
{
	// Do inherited
	SetViewInfo(eCalendarStoreColumnSubscribe, 16, true);
	CTableView::FinishCreateSelf();
	
	CContextMenuAttachment::AddUniqueContext(this, 1834, GetTable());

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
		CCalendarStoreToolbar* tb = static_cast<CCalendarStoreToolbar*>(UReanimator::CreateView(paneid_CalendarStoreToolbar1, parent, NULL));
		
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

//	Respond to commands
Boolean CCalendarStoreView::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool cmdHandled = true;

	switch (inCommand)
	{
	case cmd_Save:
	case cmd_ToolbarFileSaveBtn:
		OnFileSave();
		break;

	// Pass these off to the calendar view
	case cmd_NewEvent:
	case cmd_ToolbarNewEventBtn:
	case cmd_NewToDo:
	case cmd_ToolbarNewToDoBtn:
	case cmd_GotoToday:
	case cmd_ToolbarShowTodayBtn:
	case cmd_GotoDate:
	case cmd_ToolbarGoToDateBtn:
	case cmd_ToolbarCalendarDayBtn:
	case cmd_ToolbarCalendarWorkWeekBtn:
	case cmd_ToolbarCalendarWeekBtn:
	case cmd_ToolbarCalendarMonthBtn:
	case cmd_ToolbarCalendarYearBtn:
	case cmd_ToolbarCalendarSummaryBtn:
	case cmd_ToolbarCalendarToDoBtn:
		cmdHandled = (mCalendarView != NULL) ? mCalendarView->ObeyCommand(inCommand, ioParam) : false;
		break;

	default:
		cmdHandled = CTableView::ObeyCommand(inCommand, ioParam);
		break;
	};

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CCalendarStoreView::FindCommandStatus(
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

	// Pass these off to the calendar view
	case cmd_NewEvent:
	case cmd_ToolbarNewEventBtn:
	case cmd_NewToDo:
	case cmd_ToolbarNewToDoBtn:
	case cmd_GotoToday:
	case cmd_ToolbarShowTodayBtn:
	case cmd_GotoDate:
	case cmd_ToolbarGoToDateBtn:
	case cmd_ToolbarCalendarDayBtn:
	case cmd_ToolbarCalendarWorkWeekBtn:
	case cmd_ToolbarCalendarWeekBtn:
	case cmd_ToolbarCalendarMonthBtn:
	case cmd_ToolbarCalendarYearBtn:
	case cmd_ToolbarCalendarSummaryBtn:
	case cmd_ToolbarCalendarToDoBtn:
		if (mCalendarView != NULL)
		{
			mCalendarView->FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
		}
		// else fall through

	default:
		CTableView::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
		break;
	}
}

// Reset state from prefs
void CCalendarStoreView::ResetState(bool force)
{
	// Get default state
	CCalendarStoreWindowState* state = &CPreferences::sPrefs->mCalendarStoreWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mCalendarStoreWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetCalendarStoreWindow());

			// Reset bounds
			GetCalendarStoreWindow()->DoSetBounds(set_rect);
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mCalendarStoreWindowDefault.GetValue()));

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	{
		GetCalendarStoreWindow()->ResetStandardSize();
		GetCalendarStoreWindow()->DoSetZoom(true);
	}

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CCalendarStoreView::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetCalendarStoreWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetCalendarStoreWindow()->GetUserBounds(user_bounds);

	// Add info to prefs
	cdstrvect empty;
	CCalendarStoreWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo, &empty);
	if (CPreferences::sPrefs->mCalendarStoreWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mCalendarStoreWindowDefault.SetDirty();
}
