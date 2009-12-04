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


// Source for CServerView class

#include "CServerView.h"

#include "CContextMenu.h"
#include "CLog.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerTable.h"
#include "CServerToolbar.h"
#include "CServerWindow.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include <LBevelButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// Static members

cdmutexprotect<CServerView::CServerViewList> CServerView::sServerViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerView::CServerView()
{
	mMailboxView = NULL;

	// Add to list
	{
		cdmutexprotect<CServerViewList>::lock _lock(sServerViews);
		sServerViews->push_back(this);
	}
}

// Constructor from stream
CServerView::CServerView(LStream *inStream)
		: CTableView(inStream)
{
	mMailboxView = NULL;

	// Add to list
	{
		cdmutexprotect<CServerViewList>::lock _lock(sServerViews);
		sServerViews->push_back(this);
	}
}

// Default destructor
CServerView::~CServerView()
{
	// Remove from list
	cdmutexprotect<CServerViewList>::lock _lock(sServerViews);
	CServerViewList::iterator found = std::find(sServerViews->begin(), sServerViews->end(), this);
	if (found != sServerViews->end())
		sServerViews->erase(found);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CServerView::FinishCreateSelf(void)
{
	// Do inherited
	SetViewInfo(eServerColumnFlags, 16, true);
	CTableView::FinishCreateSelf();
	
	// Get the hierarchy button
	mFlatHierarchyBtn = (LBevelButton*) FindPaneByID(paneid_ServerFlatHierarchy);

	// Table listens to flat/hierarchy button
	mFlatHierarchyBtn->AddListener(GetTable());

	CContextMenuAttachment::AddUniqueContext(this, 4007, GetTable());

	// Set status
	SetOpen();
}

// Make a toolbar appropriate for this view
void CServerView::MakeToolbars(CToolbarView* parent)
{
	if (Is3Pane())
	{
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(C3PaneWindow::s3PaneWindow->GetMailboxToolbar());
		C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->AddCommander(GetTable());
	}
	else
	{
		// Create a suitable toolbar
		CServerToolbar* tb = static_cast<CServerToolbar*>(UReanimator::CreateView(paneid_ServerToolbar1, parent, NULL));
		
		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
	}
}

// Init columns and text
void CServerView::InitColumns(void)
{
	// Remove any previous columns
	mColumnInfo.clear();

	// Add default columns

	// Name, width = 158
	AppendColumn(eServerColumnName, 158);

	// Total, width = 48
	AppendColumn(eServerColumnTotal, 48);

	// Unseen, width = 32
	AppendColumn(eServerColumnUnseen, 32);
}

// Reset state from prefs
void CServerView::ResetState(bool force)
{
	// Use "Mail Accounts" for the 1-pane/3-pane server manager
	// Use server name for other 1-pane windows
	cdstring name = (GetTable()->IsManager() ? cdstring("Mail Accounts") : GetTable()->GetServer()->GetAccountName());

	// Check for available state
	CServerWindowState* state = CPreferences::sPrefs->GetServerWindowInfo(name);
	if (!state || force)
		state = &CPreferences::sPrefs->mServerWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mServerWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetServerWindow());

			// Reset bounds
			GetServerWindow()->DoSetBounds(set_rect);
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mServerWindowDefault.GetValue()));

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
	{
		GetServerWindow()->ResetStandardSize();
		GetServerWindow()->DoSetZoom(true);
	}

	// If forced reset, save it
	if (force)
		SaveState();
}

// Save current state in prefs
void CServerView::SaveState(void)
{
	// Use "Mail Accounts" for the 1-pane/3-pane server manager
	// Use server name for other 1-pane windows
	cdstring name = (GetTable()->IsManager() ? cdstring("Mail Accounts") : GetTable()->GetServer()->GetAccountName());

	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetServerWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetServerWindow()->GetUserBounds(user_bounds);

	// Add info to prefs
	CServerWindowState* info = new CServerWindowState(name, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo);
	if (info)
		CPreferences::sPrefs->AddServerWindowInfo(info);
}

// Save current state in prefs
void CServerView::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds = {0, 0, 0, 0};
	bool zoomed = (!Is3Pane() ? GetServerWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetServerWindow()->GetUserBounds(user_bounds);

	// Add info to prefs
	CServerWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo);
	if (CPreferences::sPrefs->mServerWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mServerWindowDefault.SetDirty();

}
