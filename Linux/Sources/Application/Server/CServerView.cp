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
#include "CFocusBorder.h"
#include "CLog.h"
#include "CMboxProtocol.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CServerTable.h"
#include "CServerTitleTable.h"
#include "CServerToolbar.h"
#include "CServerWindow.h"
#include "CTableScrollbarSet.h"
#include "CTitleTableView.h"
#include "CToolbarView.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include "HResourceMap.h"

#include "JXMultiImageCheckbox.h"

#include <algorithm>
#include <cassert>

// Static members

cdmutexprotect<CServerView::CServerViewList> CServerView::sServerViews;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CServerView::CServerView(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CTableView(enclosure, hSizing, vSizing, x, y, w, h)
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

const int cTitleHeight = 16;

// Setup help balloons
void CServerView::OnCreate()
{
	// Do inherited
	SetWindowInfo(eServerColumnFlags, 16, true);
	CTableView::OnCreate();

	// Create scrollbars
// begin JXLayout1

    CFocusBorder* focus_border =
        new CFocusBorder(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,200);
    assert( focus_border != NULL );

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(focus_border,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 494,194);
    assert( sbs != NULL );

    mFlatHierarchyBtn =
        new JXMultiImageCheckbox(sbs,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 478,0, 16,16);
    assert( mFlatHierarchyBtn != NULL );

// end JXLayout1

	// Only use focus if 3pane
	if (!Is3Pane())
	{
		focus_border->HasFocus(false);
		focus_border->SetBorderWidth(0);
	}

	mFlatHierarchyBtn->SetImages(IDI_SERVERFLAT, 0, IDI_SERVERHIERARCHIC, 0);
	mFlatHierarchyBtn->SetHint(stringFromResource(IDC_SERVERFLATBTN));
	ListenTo(mFlatHierarchyBtn);

	// Zoom scrollbar pane to fit the view
	sbs->FitToEnclosure(kTrue, kTrue);

	// Create table and it's titles inside scrollbar set
	mTitles = new CServerTitleTable(sbs, sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 105, cTitleHeight);
	mTable = new CServerTable(sbs,sbs->GetScrollEnclosure(),
								JXWidget::kHElastic, JXWidget::kVElastic, 0, cTitleHeight, 105, 100);
	mTable->OnCreate();
	mTitles->OnCreate();

	PostCreate(mTable, mTitles);

	// Set status
	SetOpen();
}

void CServerView::Receive(JBroadcaster* sender, const Message& message) 
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mFlatHierarchyBtn)
		{
			GetTable()->DoFlatHierarchy();
			GetTable()->Refresh();
			return;
		}
	}
	CTableView::Receive(sender, message);
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
		CServerToolbar* tb = new CServerToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
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
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mServerWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect);
			GetServerWindow()->GetWindow()->Place(set_rect.left, set_rect.top);
			GetServerWindow()->GetWindow()->SetSize(set_rect.width(), set_rect.height());
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mServerWindowDefault.GetValue()));

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// If forced reset, save it
	if (force)
		SaveState();

	// Always activate
	if (!Is3Pane())
		GetServerWindow()->Activate();
}

// Save current state in prefs
void CServerView::SaveState(void)
{
	// Use "Mail Accounts" for the 1-pane/3-pane server manager
	// Use server name for other 1-pane windows
	cdstring name = (GetTable()->IsManager() ? cdstring("Mail Accounts") : GetTable()->GetServer()->GetAccountName());

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
	CServerWindowState* info = new CServerWindowState(name, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo);
	if (info)
		CPreferences::sPrefs->AddServerWindowInfo(info);
}

// Save current state in prefs
void CServerView::SaveDefaultState(void)
{
	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	Rect user_bounds(0, 0, 0, 0);
	if (!Is3Pane())
		user_bounds = bounds;

	// Sync column state
	SyncColumns();

	// Add info to prefs
	CServerWindowState state(NULL, &user_bounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo);
	if (CPreferences::sPrefs->mServerWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mServerWindowDefault.SetDirty();
}
