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
#include "CSMTPToolbar.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CToolbarView.h"

#include <LBevelButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSMTPView::CSMTPView()
{
	mSender = NULL;
}

// Constructor from stream
CSMTPView::CSMTPView(LStream *inStream)
		: CMailboxView(inStream)
{
	mSender = NULL;
}

// Default destructor
CSMTPView::~CSMTPView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CSMTPView::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxView::FinishCreateSelf();

	// Get caption panes
	mTotal = (CStaticText*) FindPaneByID(paneid_SMTPTotalNum);
}

// Make a toolbar appropriate for this view
void CSMTPView::MakeToolbars(CToolbarView* parent)
{
	// Create a suitable toolbar
	CSMTPToolbar* tb = static_cast<CSMTPToolbar*>(UReanimator::CreateView(paneid_SMTPToolbar1, parent, NULL));
	mToolbar = tb;

	// Toolbar must listen to view to get activate/deactive broadcast
	Add_Listener(tb);
	
	// Now give toolbar to its view as standard buttons
	parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
}

// Set window state
void CSMTPView::ResetState(bool force)
{
	// Get default state
	CMailboxWindowState* state = &CPreferences::sPrefs->mSMTPWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mSMTPWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, GetMailboxWindow());

		// Reset bounds
		GetMailboxWindow()->DoSetBounds(set_rect);
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mSMTPWindowDefault.GetValue()));

	// Adjust sorting
	SetSortBy(state->GetSortBy());

	// Sorting button
	if (state->GetShowBy() == cShowMessageDescending)
		mSortDirectionBtn->SetValue(1);

	// Set zoom state
	if (state->GetState() == eWindowStateMax)
	{
		GetMailboxWindow()->ResetStandardSize();
		GetMailboxWindow()->DoSetZoom(true);
	}

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

}

// Save current state as default
void CSMTPView::SaveDefaultState(void)
{
	// Only do this if a mailbox has been set
	if (!GetMbox())
		return;

	// Get bounds
	Rect bounds;
	bool zoomed = (GetMailboxWindow() ? GetMailboxWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	GetMailboxWindow()->GetUserBounds(user_bounds);

	// Get current match item
	CMatchItem match;

	// Check whether quitting
	bool is_quitting = (CMulberryApp::sApp->GetState() == programState_Quitting);

	// Add info to prefs
	CMailboxWindowState state(nil,
								&user_bounds,
								zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo,
								GetMbox()->GetSortBy(),
								GetMbox()->GetShowBy(),
								is_quitting ? NMbox::eViewMode_ShowMatch : NMbox::eViewMode_All,
								&match,
								Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mSMTPWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mSMTPWindowDefault.SetDirty();
}
