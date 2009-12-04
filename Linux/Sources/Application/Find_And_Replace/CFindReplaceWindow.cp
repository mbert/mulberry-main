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

#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMulberryWMClass.h"
#include "CPreferences.h"
#include "CTextDisplay.h"

#include "TPopupMenu.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <algorithm>
#include <cassert>

// Static 

// Static members

CFindReplaceWindow*	CFindReplaceWindow::sFindReplaceWindow = NULL;
cdstring CFindReplaceWindow::sFindText;
cdstring CFindReplaceWindow::sReplaceText;
EFindMode CFindReplaceWindow::sFindMode = eFind;
cdstrvect CFindReplaceWindow::sFindHistory;
cdstrvect CFindReplaceWindow::sReplaceHistory;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CFindReplaceWindow::CFindReplaceWindow(JXDirector* owner)
	: JXWindowDirector(owner)
{
	sFindReplaceWindow = this;
}

// Default destructor
CFindReplaceWindow::~CFindReplaceWindow()
{
	sFindReplaceWindow = NULL;
	
	SaveDefaultState();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Create it or bring it to the front
void CFindReplaceWindow::CreateFindReplaceWindow(CTextDisplay* display)
{
	// Create find & replace window or bring to front
	if (sFindReplaceWindow)
		sFindReplaceWindow->Activate();
	else
	{
		CFindReplaceWindow* find_replace = new CFindReplaceWindow(CMulberryApp::sApp);
		find_replace->OnCreate();
		find_replace->ResetState();
		find_replace->Activate();
	}
	
	// Set new display and update buttons
	sFindReplaceWindow->mTargetDisplay = display;
	sFindReplaceWindow->UpdateButtons();
	
	// Make find text active and select
	sFindReplaceWindow->mFindText->SelectAll();
	sFindReplaceWindow->mFindText->Focus();
}

// Destroy it
void CFindReplaceWindow::DestroyFindReplaceWindow()
{
	// Update from window
	if (sFindReplaceWindow)
		FRAMEWORK_DELETE_WINDOW(sFindReplaceWindow);
	sFindReplaceWindow = NULL;
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
void CFindReplaceWindow::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 470,165, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 470,165);
    assert( obj1 != NULL );

    mFindBtn =
        new JXTextButton("Find", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 345,15, 105,25);
    assert( mFindBtn != NULL );
    mFindBtn->SetShortcuts("^M");

    mReplaceBtn =
        new JXTextButton("Replace", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 345,50, 105,25);
    assert( mReplaceBtn != NULL );

    mReplaceFindBtn =
        new JXTextButton("Replace & Find", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 345,85, 105,25);
    assert( mReplaceFindBtn != NULL );

    mReplaceAllBtn =
        new JXTextButton("Replace All", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 345,120, 105,25);
    assert( mReplaceAllBtn != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Find:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,17, 40,20);
    assert( obj2 != NULL );

    mFindText =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,15, 225,35);
    assert( mFindText != NULL );

    mFindPopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,20, 30,15);
    assert( mFindPopup != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Replace:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,62, 60,20);
    assert( obj3 != NULL );

    mReplaceText =
        new CTextInputDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,60, 225,35);
    assert( mReplaceText != NULL );

    mReplacePopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,65, 30,15);
    assert( mReplacePopup != NULL );

    mCaseSensitive =
        new JXTextCheckbox("Case Sensitive", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,110, 115,20);
    assert( mCaseSensitive != NULL );

    mBackwards =
        new JXTextCheckbox("Backwards", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 65,130, 90,20);
    assert( mBackwards != NULL );

    mWrap =
        new JXTextCheckbox("Wrap", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 185,110, 60,20);
    assert( mWrap != NULL );

    mEntireWord =
        new JXTextCheckbox("Entire Word", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 185,130, 95,20);
    assert( mEntireWord != NULL );

// end JXLayout

	GetWindow()->SetWMClass(cFindWMClass, cMulberryWMClass);
	window->SetTitle("Find");
	window->LockCurrentSize();

	// Update popup menus
	InitItems();

	// Need to update buttons for every character typed
	mFindText->ShouldBroadcastAllTextChanged(kTrue);

	// Start listening
	ListenTo(mFindBtn);
	ListenTo(mReplaceBtn);
	ListenTo(mReplaceFindBtn);
	ListenTo(mReplaceAllBtn);
	ListenTo(mFindText);
	ListenTo(mFindPopup);
	ListenTo(mReplacePopup);

} // CFindReplaceWindow::FinishCreateSelf

//	Respond to command
void CFindReplaceWindow::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mFindBtn)
		{
			OnFindText();
			return;
		}
		else if (sender == mReplaceBtn)
		{
			OnReplaceText();
			return;
		}
		else if (sender == mReplaceFindBtn)
		{
			OnReplaceFindText();
			return;
		}
		else if (sender == mReplaceAllBtn)
		{
			OnReplaceAllText();
			return;
		}
	}
	else if (message.Is(CTextInputDisplay::kTextChanged))
	{
		if (sender == mFindText)
		{
			UpdateButtons();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
    	JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
		if (sender == mFindPopup)
		{
			// Selected history item
			SelectHistory(sFindHistory, index, mFindText);
		}
		else if (sender == mReplacePopup)
		{
			// Selected history item
			SelectHistory(sReplaceHistory, index, mReplaceText);
		}
	}
	
	JXWindowDirector::Receive(sender, message);
	return;
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
	mCaseSensitive->SetState(JBoolean((sFindMode & eCaseSensitive) != 0));
	mBackwards->SetState(JBoolean((sFindMode & eBackwards) != 0));
	mWrap->SetState(JBoolean((sFindMode & eWrap) != 0));
	mEntireWord->SetState(JBoolean((sFindMode & eEntireWord) != 0));
}

// Init menu items
void CFindReplaceWindow::InitHistory(HPopupMenu* popup, const cdstrvect& history)
{
	// Delete existing items in menu
	popup->RemoveAllItems();
	
	// Add all items in history
	for(cdstrvect::const_iterator iter = history.begin(); iter != history.end(); iter++)
		popup->AppendItem(*iter, kFalse, kFalse);
}

// Update buttons
void CFindReplaceWindow::UpdateButtons()
{
	// Check for find text
	bool has_find = mFindText->GetTextLength();
	
	// Check for selection in editable target
	bool has_selection = mTargetDisplay->HasSelection();
	bool is_editable = JNegate(mTargetDisplay->IsReadOnly());
	
	// Now update buttons
	if (has_find)
		mFindBtn->Activate();
	else
		mFindBtn->Deactivate();
	
	if (has_selection && is_editable)
		mReplaceBtn->Activate();
	else
		mReplaceBtn->Deactivate();

	if (has_find && has_selection && is_editable)
		mReplaceFindBtn->Activate();
	else
		mReplaceFindBtn->Deactivate();

	if (has_find && is_editable)
		mReplaceAllBtn->Activate();
	else
		mReplaceAllBtn->Deactivate();
}

// History item chosen
void CFindReplaceWindow::SelectHistory(const cdstrvect& history, JIndex menu_pos, CTextInputDisplay* field)
{
	field->SetText(history.at(menu_pos - 1));
	field->SelectAll();
	field->Focus();
}

// Initialise history popups and text
void CFindReplaceWindow::CaptureState(void)
{
	sFindText = cdstring(mFindText->GetText());
	sReplaceText = cdstring(mReplaceText->GetText());
	
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
	if (mCaseSensitive->IsChecked())
		sFindMode = (EFindMode) (sFindMode | eCaseSensitive);
	if (mBackwards->IsChecked())
		sFindMode = (EFindMode) (sFindMode | eBackwards);
	if (mWrap->IsChecked())
		sFindMode = (EFindMode) (sFindMode | eWrap);
	if (mEntireWord->IsChecked())
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
	DestroyFindReplaceWindow();	
}

void CFindReplaceWindow::OnReplaceText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceText();

	// Close this window (must do after last access to instance variables)
	DestroyFindReplaceWindow();	
}

void CFindReplaceWindow::OnReplaceFindText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceFindText(sFindMode & eBackwards);

	// Close this window (must do after last access to instance variables)
	DestroyFindReplaceWindow();	
}

void CFindReplaceWindow::OnReplaceAllText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceAllText();

	// Close this window (must do after last access to instance variables)
	DestroyFindReplaceWindow();	
}

// Prepare target before execution
void CFindReplaceWindow::PrepareTarget()
{
	// Capture state
	CaptureState();
	
	// Switch to window of target
	// Find window in super view chain
	JXContainer* aWindow = mTargetDisplay;
	while(aWindow && !dynamic_cast<JXWindow*>(aWindow))
		aWindow = const_cast<JXContainer*>(aWindow->GetEnclosure());
	
	// Bring window to top
	if (aWindow)
		dynamic_cast<JXWindow*>(aWindow)->Raise();
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
	JRect set_rect = state->GetBestRect(CPreferences::sPrefs->mFindReplaceWindowDefault.GetValue());
	if (set_rect.left && set_rect.top)
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset position
		GetWindow()->Place(set_rect.left, set_rect.top);
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

	mCaseSensitive->SetState(JBoolean((sFindMode & eCaseSensitive) != 0));
	mBackwards->SetState(JBoolean((sFindMode & eBackwards) != 0));
	mWrap->SetState(JBoolean((sFindMode & eWrap) != 0));
	mEntireWord->SetState(JBoolean((sFindMode & eEntireWord) != 0));
	
	// If forced reset, save it
	if (force)
		SaveDefaultState();
}

// Save current state in prefs
void CFindReplaceWindow::SaveDefaultState(void)
{
	// Get bounds - convert to position only
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);

	// Add info to prefs
	CFindReplaceWindowState state(nil, &bounds, eWindowStateNormal,
				mCaseSensitive->IsChecked(), mBackwards->IsChecked(), mWrap->IsChecked(), mEntireWord->IsChecked());
	if (CPreferences::sPrefs->mFindReplaceWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mFindReplaceWindowDefault.SetDirty();
}
