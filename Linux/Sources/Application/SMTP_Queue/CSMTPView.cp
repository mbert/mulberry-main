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


// Source for CSMTPView class

#include "CSMTPView.h"

#include "CMailboxWindow.h"
#include "CMbox.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTableScrollbarSet.h"
#include "CSMTPToolbar.h"
#include "CSMTPTable.h"
#include "CSplitterView.h"
#include "CToolbarView.h"

#include "HResourceMap.h"

#include "JXMultiImageCheckbox.h"
#include <JXStaticText.h>
#include <JXUpRect.h>

#include <cassert>

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPView::CSMTPView(JXContainer* enclosure,
							const HSizingOption hSizing, const VSizingOption vSizing,
							const JCoordinate x, const JCoordinate y,
							const JCoordinate w, const JCoordinate h)
	: CMailboxView(enclosure, hSizing, vSizing, x, y, w, h)
{
	mSender = NULL;
}

// Default destructor
CSMTPView::~CSMTPView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

const int cTitleHeight = 16;

// Setup help balloons
void CSMTPView::OnCreate()
{
	// Do inherited
	CMailboxView::OnCreate();

// begin JXLayout1

    CTableScrollbarSet* sbs =
        new CTableScrollbarSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,180);
    assert( sbs != NULL );

    mSortDirectionBtn =
        new JXMultiImageCheckbox(sbs,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 484,0, 16,16);
    assert( mSortDirectionBtn != NULL );

    JXUpRect* obj1 =
        new JXUpRect(this,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 0,180, 500,20);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Total:", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,2, 40,16);
    assert( obj2 != NULL );
    obj2->SetFontSize(10);

    mTotal =
        new JXStaticText("000000", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 45,2, 50,16);
    assert( mTotal != NULL );
    mTotal->SetFontSize(10);

// end JXLayout1

	// Create sort button positioned inside scrollbars
	mSortBtn = mSortDirectionBtn;
	mSortDirectionBtn->SetImages(IDI_SORTASCENDING, 0, IDI_SORTDESCENDING, 0);
	mSortDirectionBtn->SetHint(stringFromResource(IDC_MAILBOXSORTBTN));
	ListenTo(mSortDirectionBtn);

	// Zoom scrollbar pane to fit the view
	sbs->FitToEnclosure(kTrue, kTrue);

	mTitles = new CMailboxTitleTable(sbs, sbs->GetScrollEnclosure(),
										JXWidget::kHElastic, JXWidget::kFixedTop,
											 0, 0, 500, cTitleHeight);

	mTable = new CSMTPTable(sbs, sbs->GetScrollEnclosure(),
								 JXWidget::kHElastic, JXWidget::kVElastic,
								 0, cTitleHeight, 500, sbs->GetApertureHeight());
	
	mTable->OnCreate();
	mTitles->OnCreate();

	PostCreate(mTable, mTitles);
	
	// Always start disabled until mailbox is set
	Deactivate();
	
	// Set status
	SetOpen();	
}

// Make a toolbar appropriate for this view
void CSMTPView::MakeToolbars(CToolbarView* parent)
{
	// Create a suitable toolbar
	CSMTPToolbar* tb = new CSMTPToolbar(parent, JXWidget::kHElastic, JXWidget::kFixedTop, 0, 0, 128, 45);
	tb->OnCreate();
	mToolbar = tb;

	// Toolbar must listen to view to get activate/deactive broadcast
	Add_Listener(tb);
	
	// Now give toolbar to its view as standard buttons
	parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);

	// Force toolbar to be active
	Broadcast_Message(eBroadcast_ViewActivate, this);
}

// Set window state
void CSMTPView::ResetState(bool force)
{
	// Only do this if a mailbox has been set
	if (!GetMbox())
		return;

	// Get default state
	CMailboxWindowState* state = &CPreferences::sPrefs->mSMTPWindowDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mSMTPWindowDefault.GetValue());
	if (!set_rect.IsEmpty())
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset bounds
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mSMTPWindowDefault.GetValue()));

	// Adjust sorting
	SetSortBy(state->GetSortBy());

	// Sorting button
	if (state->GetShowBy() == cShowMessageDescending)
		mSortDirectionBtn->SetState(kTrue);

	// Init the preview state once if we're in a window
	if (!Is3Pane() && !mPreviewInit)
	{
		mMessageView->ResetState();
		mPreviewInit = true;
	}

	// Init splitter pos
	if (!Is3Pane() && (state->GetSplitterSize() != 0))
		GetMailboxWindow()->GetSplitter()->SetRelativeSplitPos(state->GetSplitterSize());

	// If forced reset, save it
	if (force)
		SaveDefaultState();

	// Do zoom
	//if (state->GetState() == eWindowStateMax)
	//	GetParentFrame()->ShowWindow(SW_SHOWMAXIMIZED);

	// Always activate
	GetMailboxWindow()->Activate();

}

// Save current state as default
void CSMTPView::SaveDefaultState(void)
{
	// Only do this if a mailbox has been set
	if (!GetMbox())
		return;

	// Get bounds
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);
	bool zoomed = false;

	// Get unzoomed bounds
	Rect user_bounds(0, 0, 0, 0);
	user_bounds = bounds;

	// Get current match item
	CMatchItem match;

	// Check whether quitting
	bool is_quitting = CMulberryApp::sApp->mQuitting;

	// Sync column state
	SyncColumns();

	// Add info to prefs
	CMailboxWindowState state(nil,
								&bounds,
								zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo,
								(ESortMessageBy) GetMbox()->GetSortBy(),
								(EShowMessageBy) GetMbox()->GetShowBy(),
								is_quitting ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All,
								&match,
								Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mSMTPWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mSMTPWindowDefault.SetDirty();
}
