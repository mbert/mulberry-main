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

#include "CAdbkManagerToolbar.h"
#include "CAdbkManagerWindow.h"
#include "CContextMenu.h"
#include "CHierarchyTableDrag.h"
#include "CLog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneAdbkToolbar.h"
#include "C3PaneWindow.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

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

// Constructor from stream
CAdbkManagerView::CAdbkManagerView(LStream *inStream)
		: CTableView(inStream)
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

// Setup help balloons
void CAdbkManagerView::FinishCreateSelf(void)
{
	// Do inherited
	SetViewInfo(eAdbkColumnOpen, 16, true);
	CTableView::FinishCreateSelf();
	
	CContextMenuAttachment::AddUniqueContext(this, 9003, GetTable());
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
		CAdbkManagerToolbar* tb = static_cast<CAdbkManagerToolbar*>(UReanimator::CreateView(paneid_AdbkMgrToolbar1, parent, NULL));

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
	// Get default state
	CAdbkManagerWindowState* state = &CPreferences::sPrefs->mAdbkManagerWindowDefault.Value();

	// Show/hide 1-pane window based on state
	if (!Is3Pane())
	{
		if (state->GetHide())
			GetAdbkManagerWindow()->Hide();
		else
			GetAdbkManagerWindow()->Show();
	}

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mAdbkManagerWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetAdbkManagerWindow());

			// Reset bounds
			GetAdbkManagerWindow()->DoSetBounds(set_rect);
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mAdbkManagerWindowDefault.GetValue()));

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	{
		GetAdbkManagerWindow()->ResetStandardSize();
		GetAdbkManagerWindow()->DoSetZoom(true);
	}

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CAdbkManagerView::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetAdbkManagerWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetAdbkManagerWindow()->GetUserBounds(user_bounds);

	// Add info to prefs
	cdstrvect empty;
	CAdbkManagerWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo, !IsVisible(), &empty);
	if (CPreferences::sPrefs->mAdbkManagerWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mAdbkManagerWindowDefault.SetDirty();
}
