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


// Source for CAdbkManagerView class

#include "CAdbkManagerView.h"

#include "CAdbkManagerTitleTable.h"
#include "CAdbkManagerToolbar.h"
#include "CAdbkManagerWindow.h"
#include "CContextMenu.h"
#include "CFocusBorder.h"
#include "CHierarchyTableDrag.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneWindow.h"

#include <algorithm>
#include <cassert>

// Static members

cdmutexprotect<CAdbkManagerView::CAdbkManagerViewList> CAdbkManagerView::sAdbkManagerViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkManagerView::CAdbkManagerView(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CTableView(enclosure, hSizing, vSizing, x, y, w, h)
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

// Setup help balloons
void CAdbkManagerView::OnCreate()
{
	// Do inherited
	SetWindowInfo(eAdbkColumnOpen, 16, true);
	CTableView::OnCreate();

	// Create scrollbars
// begin JXLayout1

    CFocusBorder* focus_border =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 340,270);
    assert( focus_border != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(focus_border,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 334,264);
    assert( sbs != NULL );

// end JXLayout1

	// Only use focus if 3pane
	if (!Is3Pane())
	{
		focus_border->HasFocus(false);
		focus_border->SetBorderWidth(0);
	}

	// Create table and it's titles inside scrollbar set
	mTable = new CAdbkManagerTable(sbs,sbs->GetScrollEnclosure(),
								JXWidget::kHElastic, JXWidget::kVElastic, 0,cTitleHeight, 105, 100);
	mTitles = new CAdbkManagerTitleTable(sbs, sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 105, cTitleHeight);
	mTable->OnCreate();
	mTitles->OnCreate();

	PostCreate(mTable, mTitles);

	// Set status
	SetOpen();
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
		CAdbkManagerToolbar* tb = new CAdbkManagerToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
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
	// Get default state
	CAdbkManagerWindowState* state = &CPreferences::sPrefs->mAdbkManagerWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mAdbkManagerWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect);
			GetAdbkManagerWindow()->GetWindow()->Place(set_rect.left, set_rect.top);
			GetAdbkManagerWindow()->GetWindow()->SetSize(set_rect.width(), set_rect.height());
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mAdbkManagerWindowDefault.GetValue()));

	// Set zoom state if 1-pane
	//if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// If forced reset, save it
	if (force)
		SaveDefaultState();

	// Always activate
	if (!Is3Pane())
	{
		// Always activate
		GetAdbkManagerWindow()->Activate();
	}
}

// Save current state in prefs
void CAdbkManagerView::SaveDefaultState(void)
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
	CAdbkManagerWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo, false, &empty);
	if (CPreferences::sPrefs->mAdbkManagerWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mAdbkManagerWindowDefault.SetDirty();
}
