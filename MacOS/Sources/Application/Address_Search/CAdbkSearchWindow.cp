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


// Source for CAdbkSearchWindow class

#include "CAdbkSearchWindow.h"

#include "CContextMenu.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CTextFieldX.h"
#include "CTitleTable.h"
#include "CWindowsMenu.h"

#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

CAdbkSearchWindow*	CAdbkSearchWindow::sAdbkSearch = nil;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CAdbkSearchWindow::CAdbkSearchWindow()
{
	sAdbkSearch = this;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Constructor from stream
CAdbkSearchWindow::CAdbkSearchWindow(LStream *inStream)
		: CTableWindow(inStream)
{
	sAdbkSearch = this;

	// Add to list
	CWindowsMenu::AddWindow(this, false);
}

// Default destructor
CAdbkSearchWindow::~CAdbkSearchWindow()
{
	// Set status
	SetClosing();

	sAdbkSearch = nil;

	// Remove from list
	CWindowsMenu::RemoveWindow(this);

	// Save state
	SaveDefaultState();

	// Set status
	SetClosed();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void CAdbkSearchWindow::FinishCreateSelf()
{
	// Do inherited
	SetWindowInfo(eAddrColumnName, 80, true);
	CTableWindow::FinishCreateSelf();

	SetLatentSub(GetTable());

	// Init columns
	InitColumns();

	// Get text traits resource
	ResetTextTraits(CPreferences::sPrefs->mListTextTraits.GetValue().traits);

	// Set status
	SetOpen();

	CContextMenuAttachment::AddUniqueContext(this, 9003, GetTable());

} // CAdbkSearchWindow::FinishCreateSelf

// Activate search item
void CAdbkSearchWindow::ActivateSelf(void)
{
	CTextFieldX* field = (CTextFieldX*) FindPaneByID(paneid_AdbkSearchField);
	field->GetSuperCommander()->SetLatentSub(field);
	LCommander::SwitchTarget(field);
	field->SelectAll();

	CTableWindow::ActivateSelf();
}

// Init columns and text
void CAdbkSearchWindow::InitColumns(void)
{
	SColumnInfo newInfo;

	// Remove any previous columns
	mColumnInfo.clear();

	// Add default columns

	// Flags, width = 240
	newInfo.column_type = eAddrColumnName;
	newInfo.column_width = 240;
	mColumnInfo.push_back(newInfo);
	mTitles->SetColWidth(newInfo.column_width, 1, 1);
	mTable->SetColWidth(newInfo.column_width, 1, 1);
}

// Reset state from prefs
void CAdbkSearchWindow::ResetState(bool force)
{
	// Get default state
	CAdbkSearchWindowState* state = &CPreferences::sPrefs->mAdbkSearchWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mAdbkSearchWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mAdbkSearchWindowDefault.GetValue()));

	// Always show - the only time reset state is called is when the user asks for the search window

	// Show/hide window based on state
	//if (state->mHide)
	//	Hide();
	//else
	//	Show();

	// Set state of popups
	GetTable()->mMethod->SetValue(state->GetMethod());
	GetTable()->mCriteria->SetValue(state->GetField());
	GetTable()->mMatch->SetValue(state->GetCriteria());

	// Set zoom state
	if (state->GetState() == eWindowStateMax)
	{
		ResetStandardSize();
		DoSetZoom(true);
	}

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CAdbkSearchWindow::SaveDefaultState(void)
{
	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	CAdbkSearchWindowState state(nil, &mUserBounds, zoomed ? eWindowStateMax : eWindowStateNormal, &mColumnInfo,
									(EAdbkSearchMethod) GetTable()->mMethod->GetValue(),
									(EAddrColumn) GetTable()->mCriteria->GetValue(),
									(EAdbkSearchCriteria) GetTable()->mMatch->GetValue(),
									!IsVisible());
	if (CPreferences::sPrefs->mAdbkSearchWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mAdbkSearchWindowDefault.SetDirty();

} // CAdbkSearchWindow::SaveDefaultState
