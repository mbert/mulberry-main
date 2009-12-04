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


// Source for CFindReplaceWindow class

#include "CFindReplaceWindow.h"

#include "CCommands.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>
#include <LPushButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

CFindReplaceWindow*	CFindReplaceWindow::sFindReplaceWindow = nil;
cdstring CFindReplaceWindow::sFindText;
cdstring CFindReplaceWindow::sReplaceText;
EFindMode CFindReplaceWindow::sFindMode = eFind;
cdstrvect CFindReplaceWindow::sFindHistory;
cdstrvect CFindReplaceWindow::sReplaceHistory;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFindReplaceWindow::CFindReplaceWindow()
{
	sFindReplaceWindow = this;
	mTargetDisplay = NULL;
}

// Constructor from stream
CFindReplaceWindow::CFindReplaceWindow(LStream *inStream)
		: LWindow(inStream)
{
	sFindReplaceWindow = this;
	mTargetDisplay = NULL;
}

// Default destructor
CFindReplaceWindow::~CFindReplaceWindow()
{
	sFindReplaceWindow = nil;

	SaveDefaultState();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create it or bring it to the front
void CFindReplaceWindow::CreateFindReplaceWindow(CTextDisplay* display)
{
	// Create find & replace window or bring to front
	if (sFindReplaceWindow)
		FRAMEWORK_WINDOW_TO_TOP(sFindReplaceWindow)
	else
	{
		CFindReplaceWindow* find_replace = (CFindReplaceWindow*) LWindow::CreateWindow(paneid_FindReplaceWindow, CMulberryApp::sApp);
		find_replace->ResetState();
		find_replace->Show();
	}

	// Set new display and update buttons
	sFindReplaceWindow->mTargetDisplay = display;
	sFindReplaceWindow->UpdateButtons();

	// Make find text active and select
	sFindReplaceWindow->mFindText->SelectAll();
	LCommander::SwitchTarget(sFindReplaceWindow->mFindText);
}

// Destroy it
void CFindReplaceWindow::DestroyFindReplaceWindow()
{
	// Update from window
	delete sFindReplaceWindow;
	sFindReplaceWindow = nil;
}

// Update details in any open window
void CFindReplaceWindow::UpdateFindReplace()
{
	// Update from window
	if (sFindReplaceWindow)
		sFindReplaceWindow->CaptureState();
}

// Set find text and push into history
void CFindReplaceWindow::SetFindText(const char* text)
{
	sFindText = text;
	cdstrvect::const_iterator found = std::find(sFindHistory.begin(), sFindHistory.end(), sFindText);
	if (found == sFindHistory.end())
	{
		sFindHistory.insert(sFindHistory.begin(), sFindText);

		// Update window pop
		if (sFindReplaceWindow)
			sFindReplaceWindow->InitHistory(sFindReplaceWindow->mFindPopup, sFindHistory);
	}
}

// Do various bits
void CFindReplaceWindow::FinishCreateSelf()
{
	// Do inherited
	LWindow::FinishCreateSelf();

	// Get all controls
	mFindText = (CTextFieldX*) FindPaneByID(paneid_FindReplaceFind);
	mFindText->AddListener(this);
	mFindPopup = (LPopupButton*) FindPaneByID(paneid_FindReplaceFindPopup);
	mReplaceText = (CTextFieldX*) FindPaneByID(paneid_FindReplaceReplace);
	mReplaceText->AddListener(this);
	mReplacePopup = (LPopupButton*) FindPaneByID(paneid_FindReplaceReplacePopup);
	mCaseSensitive = (LCheckBox*) FindPaneByID(paneid_FindReplaceCase);
	mBackwards = (LCheckBox*) FindPaneByID(paneid_FindReplaceBackwards);
	mWrap = (LCheckBox*) FindPaneByID(paneid_FindReplaceWrap);
	mEntireWord = (LCheckBox*) FindPaneByID(paneid_FindReplaceWord);
	mFindBtn = (LPushButton*) FindPaneByID(paneid_FindReplaceFindBtn);
	mFindBtn->SetDefaultButton(true);
	mReplaceBtn = (LPushButton*) FindPaneByID(paneid_FindReplaceReplaceBtn);
	mReplaceFindBtn = (LPushButton*) FindPaneByID(paneid_FindReplaceBothBtn);
	mReplaceAllBtn = (LPushButton*) FindPaneByID(paneid_FindReplaceAllBtn);

	// Update popup menus
	InitItems();

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CFindReplaceBtns);

} // CFindReplaceWindow::FinishCreateSelf

// Handle key presses
Boolean CFindReplaceWindow::HandleKeyPress(const EventRecord &inKeyEvent)
{
	switch (inKeyEvent.message & charCodeMask)
	{
		// Edit the address
		case char_Return:
		case char_Enter:
			if (mFindBtn->IsEnabled())
				mFindBtn->SimulateHotSpotClick(kControlButtonPart);
			break;

		default:
			return LWindow::HandleKeyPress(inKeyEvent);
			break;
	}

	return true;
}

//	Respond to commands
Boolean CFindReplaceWindow::ObeyCommand(CommandT inCommand,void *ioParam)
{
	bool	cmdHandled = true;

	switch (inCommand)
	{
		case cmd_FindText:
			OnFindText();
			break;

		case cmd_ReplaceText:
			OnReplaceText();
			break;

		case cmd_ReplaceFindText:
		case cmd_ReplaceFindBackText:
			OnReplaceFindText();
			break;

		case cmd_ReplaceAllText:
			OnReplaceAllText();
			break;

		default:
			cmdHandled = LWindow::ObeyCommand(inCommand, ioParam);
			break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CFindReplaceWindow::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;

	switch (inCommand)
	{
		case cmd_FindText:
			outEnabled = mFindBtn->IsEnabled();
			break;

		case cmd_ReplaceText:
			outEnabled = mReplaceBtn->IsEnabled();
			break;

		case cmd_ReplaceFindText:
			outEnabled = mReplaceFindBtn->IsEnabled();
			break;

		case cmd_ReplaceAllText:
			outEnabled = mReplaceAllBtn->IsEnabled();
			break;

		default:
			LWindow::FindCommandStatus(inCommand, outEnabled, outUsesMark, outMark, outName);
			break;
	}
}

// Respond to clicks in the icon buttons
void CFindReplaceWindow::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
		case msg_FindReplaceFindText:
			// Typing in find field - update buttons
			UpdateButtons();
			break;

		case msg_FindReplaceFindPopup:
			// Selected history item
			SelectHistory(sFindHistory, *(long*) ioParam, mFindText);
			break;

		case msg_FindReplaceReplacePopup:
			// Selected history item
			SelectHistory(sReplaceHistory, *(long*) ioParam, mReplaceText);
			break;

		case msg_FindReplaceFindBtn:
			OnFindText();
			break;

		case msg_FindReplaceReplaceBtn:
			OnReplaceText();
			break;

		case msg_FindReplaceBothBtn:
			OnReplaceFindText();
			break;

		case msg_FindReplaceAllBtn:
			OnReplaceAllText();
			break;
	}
}

// Initialise history popups and text
void CFindReplaceWindow::InitItems()
{
	mFindText->SetText(sFindText);

	mReplaceText->SetText(sReplaceText);

	// Do popup menus
	InitHistory(mFindPopup, sFindHistory);
	InitHistory(mReplacePopup, sReplaceHistory);

	// Do check boxes
	mCaseSensitive->SetValue((sFindMode & eCaseSensitive) != 0);
	mBackwards->SetValue((sFindMode & eBackwards) != 0);
	mWrap->SetValue((sFindMode & eWrap) != 0);
	mEntireWord->SetValue((sFindMode & eEntireWord) != 0);
}

// Init menu items
void CFindReplaceWindow::InitHistory(LPopupButton* popup, const cdstrvect& history)
{
	// Delete existing items in menu
	MenuHandle menuH = popup->GetMacMenuH();
	for(short i = ::CountMenuItems(menuH); i >= 1; i--)
		::DeleteMenuItem(menuH, i);

	// Add all items in history
	short menu_pos = 1;
	for(cdstrvect::const_iterator iter = history.begin(); iter != history.end(); iter++, menu_pos++)
		::AppendItemToMenu(menuH, menu_pos, *iter);

	// Force max/min update
	popup->SetMenuMinMax();
}

// Update buttons
void CFindReplaceWindow::UpdateButtons()
{
	// Check for find text
	bool has_find = !mFindText->GetText().empty();

	// Check for selection in editable target
	SInt32 sel_start = 0;
	SInt32 sel_end = 0;
	if (mTargetDisplay != NULL)
		mTargetDisplay->GetSelectionRange(sel_start, sel_end);
	bool has_selection = (sel_start != sel_end);
	bool is_editable = (mTargetDisplay != NULL) ? !mTargetDisplay->IsReadOnly() : true;

	// Now update buttons
	if (has_find)
		mFindBtn->Enable();
	else
		mFindBtn->Disable();

	if (has_selection && is_editable)
		mReplaceBtn->Enable();
	else
		mReplaceBtn->Disable();

	if (has_find && has_selection && is_editable)
		mReplaceFindBtn->Enable();
	else
		mReplaceFindBtn->Disable();

	if (has_find && is_editable)
		mReplaceAllBtn->Enable();
	else
		mReplaceAllBtn->Disable();
}

// History item chosen
void CFindReplaceWindow::SelectHistory(const cdstrvect& history, long menu_pos, CTextFieldX* field)
{
	field->SetText(history.at(menu_pos - 1));
	field->SelectAll();
	field->Activate();
}

// Initialise history popups and text
void CFindReplaceWindow::CaptureState(void)
{
	sFindText = mFindText->GetText();

	sReplaceText = mReplaceText->GetText();

	// Do history
	cdstrvect::const_iterator found = std::find(sFindHistory.begin(), sFindHistory.end(), sFindText);
	if (found == sFindHistory.end())
	{
		sFindHistory.insert(sFindHistory.begin(), sFindText);
		InitHistory(mFindPopup, sFindHistory);
	}
	found = std::find(sReplaceHistory.begin(), sReplaceHistory.end(), sReplaceText);
	if (found == sReplaceHistory.end())
	{
		sReplaceHistory.insert(sReplaceHistory.begin(), sReplaceText);
		InitHistory(mReplacePopup, sReplaceHistory);
	}

	// Find mode
	sFindMode = eFind;
	if (mCaseSensitive->GetValue())
		sFindMode = (EFindMode) (sFindMode | eCaseSensitive);
	if (mBackwards->GetValue())
		sFindMode = (EFindMode) (sFindMode | eBackwards);
	if (mWrap->GetValue())
		sFindMode = (EFindMode) (sFindMode | eWrap);
	if (mEntireWord->GetValue())
		sFindMode = (EFindMode) (sFindMode | eEntireWord);
}

#pragma mark ____________________________Execute

void CFindReplaceWindow::OnFindText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	// Do 'Find Next' command as 'Find' command only display this window!
	mTargetDisplay->DoFindNextText(sFindMode & eBackwards);

	// Close this window (must do after last access to instance variables)
	delete this;
}

void CFindReplaceWindow::OnReplaceText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceText();

	// Close this window (must do after last access to instance variables)
	delete this;
}

void CFindReplaceWindow::OnReplaceFindText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceFindText(sFindMode & eBackwards);

	// Close this window (must do after last access to instance variables)
	delete this;
}

void CFindReplaceWindow::OnReplaceAllText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceAllText();

	// Close this window (must do after last access to instance variables)
	delete this;
}

// Prepare target before execution
void CFindReplaceWindow::PrepareTarget()
{
	// Capture state
	CaptureState();

	// Switch to window of target
	// Find window in super view chain
	LPane* aWindow = mTargetDisplay;
	while(!dynamic_cast<LWindow*>(aWindow))
		aWindow = (LPane*) aWindow->GetSuperView();

	// Bring window to top
	FRAMEWORK_WINDOW_TO_TOP(dynamic_cast<LWindow*>(aWindow))
}

#pragma mark ____________________________Window State

// Reset state from prefs
void CFindReplaceWindow::ResetState(bool force)
{
	// Get name as cstr
	char name = 0;

	// Get default state
	CFindReplaceWindowState* state = &CPreferences::sPrefs->mFindReplaceWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mFindReplaceWindowDefault.GetValue());
	if (set_rect.left && set_rect.top)
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset position
		Point pt;
		pt.h = set_rect.left;
		pt.v = set_rect.top;
		DoSetPosition(pt);
	}

	// Set button states
	sFindMode = eFind;
	if (state->GetCaseSensitive())
		sFindMode = (EFindMode) (sFindMode | eCaseSensitive);
	if (state->GetBackwards())
		sFindMode = (EFindMode) (sFindMode | eBackwards);
	if (state->GetWrap())
		sFindMode = (EFindMode) (sFindMode | eWrap);
	if (state->GetEntireWord())
		sFindMode = (EFindMode) (sFindMode | eEntireWord);

	mCaseSensitive->SetValue((sFindMode & eCaseSensitive) != 0);
	mBackwards->SetValue((sFindMode & eBackwards) != 0);
	mWrap->SetValue((sFindMode & eWrap) != 0);
	mEntireWord->SetValue((sFindMode & eEntireWord) != 0);

	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CFindReplaceWindow::SaveDefaultState(void)
{
	// Get bounds - convert to position only
	Rect bounds = mUserBounds;
	bounds.right = 0;
	bounds.bottom = 0;

	// Add info to prefs
	CFindReplaceWindowState state(nil, &bounds, eWindowStateNormal,
		mCaseSensitive->GetValue(), mBackwards->GetValue(), mWrap->GetValue(), mEntireWord->GetValue());
	if (CPreferences::sPrefs->mFindReplaceWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mFindReplaceWindowDefault.SetDirty();
}
