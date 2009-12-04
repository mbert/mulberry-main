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


// CFindReplaceWindow.cpp : implementation file
//

#include "CFindReplaceWindow.h"

#include "CCmdEditView.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CPopupButton.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CWindowStates.h"

/////////////////////////////////////////////////////////////////////////////
// CFindReplaceWindow dialog

// Static members

CFindReplaceWindow*	CFindReplaceWindow::sFindReplaceWindow = nil;
cdstring CFindReplaceWindow::sFindText;
cdstring CFindReplaceWindow::sReplaceText;
EFindMode CFindReplaceWindow::sFindMode = eFind;
cdstrvect CFindReplaceWindow::sFindHistory;
cdstrvect CFindReplaceWindow::sReplaceHistory;

CFindReplaceWindow::CFindReplaceWindow()
	: CHelpDialog()
{
	sFindReplaceWindow = this;
}


void CFindReplaceWindow::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFindReplaceWindow)
	DDX_Control(pDX, IDC_FINDREPLACE_FINDTEXT, mFindText);
	//DDX_Control(pDX, IDC_FINDREPLACE_FINDPOPUP, mFindPopup);
	DDX_Control(pDX, IDC_FINDREPLACE_REPLACETEXT, mReplaceText);
	//DDX_Control(pDX, IDC_FINDREPLACE_REPLACEPOPUP, mReplacePopup);
	DDX_Control(pDX, IDC_FINDREPLACE_CASE, mCaseSensitive);
	DDX_Control(pDX, IDC_FINDREPLACE_BACK, mBackwards);
	DDX_Control(pDX, IDC_FINDREPLACE_WRAP, mWrap);
	DDX_Control(pDX, IDC_FINDREPLACE_WORD, mEntireWord);
	DDX_Control(pDX, IDC_FINDREPLACE_FINDBTN, mFindBtn);
	DDX_Control(pDX, IDC_FINDREPLACE_REPLACEBTN, mReplaceBtn);
	DDX_Control(pDX, IDC_FINDREPLACE_REPLACEFINDBTN, mReplaceFindBtn);
	DDX_Control(pDX, IDC_FINDREPLACE_REPLACEALLBTN, mReplaceAllBtn);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFindReplaceWindow, CHelpDialog)
	//{{AFX_MSG_MAP(CFindReplaceWindow)
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	
	ON_EN_CHANGE(IDC_FINDREPLACE_FINDTEXT, OnChangeFindText)
	ON_COMMAND_RANGE(IDM_FIND_HISTORY_Start, IDM_FIND_HISTORY_End, OnFindPopup)
	ON_COMMAND_RANGE(IDM_REPLACE_HISTORY_Start, IDM_REPLACE_HISTORY_End, OnReplacePopup)
	ON_BN_CLICKED(IDC_FINDREPLACE_FINDBTN, OnFindText)
	ON_BN_CLICKED(IDC_FINDREPLACE_REPLACEBTN, OnReplaceText)
	ON_BN_CLICKED(IDC_FINDREPLACE_REPLACEFINDBTN, OnReplaceFindText)
	ON_BN_CLICKED(IDC_FINDREPLACE_REPLACEALLBTN, OnReplaceAllText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFindReplaceWindow message handlers

// Hide instead of close
void CFindReplaceWindow::OnClose()
{
	// Destroy it
	DestroyWindow();
}

// Hide instead of close
void CFindReplaceWindow::OnDestroy()
{
	// Save window state
	OnSaveDefaultState();
	
	sFindReplaceWindow = nil;
}

// Create it or bring it to the front
void CFindReplaceWindow::CreateFindReplaceWindow(CCmdEditView* display)
{
	// Create find & replace window or bring to front
	if (sFindReplaceWindow)
	{
		FRAMEWORK_WINDOW_TO_TOP(sFindReplaceWindow)
	}
	else
	{
		CFindReplaceWindow* find_replace = new CFindReplaceWindow;
		find_replace->Create(CFindReplaceWindow::IDD, CSDIFrame::GetAppTopWindow());
		find_replace->InitItems();
		find_replace->ResetState();
		find_replace->ShowWindow(SW_SHOW);
	}
	
	// Set new display and update buttons
	sFindReplaceWindow->mTargetDisplay = display;
	sFindReplaceWindow->UpdateButtons();
	
	// Make find text active and select
	sFindReplaceWindow->mFindText.SetSel(0, -1);
	sFindReplaceWindow->GotoDlgCtrl(&sFindReplaceWindow->mFindText);
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
	cdstrvect::const_iterator found = ::find(sFindHistory.begin(), sFindHistory.end(), sFindText);
	if (found == sFindHistory.end())
	{
		sFindHistory.insert(sFindHistory.begin(), sFindText);
		
		// Update window pop
		if (sFindReplaceWindow)
			sFindReplaceWindow->InitHistory(&sFindReplaceWindow->mFindPopup, IDM_FIND_HISTORY_Start, sFindHistory);
	}
}

// Initialise history popups and text
void CFindReplaceWindow::InitItems()
{
	// Do data exchange to get controls
	UpdateData(false);

	CUnicodeUtils::SetWindowTextUTF8(&mFindText, sFindText);

	CUnicodeUtils::SetWindowTextUTF8(&mReplaceText, sReplaceText);
	
	// Do popup menus
	mFindPopup.SubclassDlgItem(IDC_FINDREPLACE_FINDPOPUP, this, IDI_POPUPBTN);
	mFindPopup.SetMenu(IDR_POPUP_FIND_HISTORY);
	InitHistory(&mFindPopup, IDM_FIND_HISTORY_Start, sFindHistory);
	mReplacePopup.SubclassDlgItem(IDC_FINDREPLACE_REPLACEPOPUP, this, IDI_POPUPBTN);
	mReplacePopup.SetMenu(IDR_POPUP_REPLACE_HISTORY);
	InitHistory(&mReplacePopup, IDM_REPLACE_HISTORY_Start, sReplaceHistory);
	
	// Do check boxes
	mCaseSensitive.SetCheck((sFindMode & eCaseSensitive) != 0);
	mBackwards.SetCheck((sFindMode & eBackwards) != 0);
	mWrap.SetCheck((sFindMode & eWrap) != 0);
	mEntireWord.SetCheck((sFindMode & eEntireWord) != 0);
}

// Init menu items
void CFindReplaceWindow::InitHistory(CPopupButton* popup, UINT cmd_start, const cdstrvect& history)
{
	CMenu* pPopup = popup->GetPopupMenu();

	// Remove any existing items
	short num_menu = pPopup->GetMenuItemCount();
	for(short i = 0; i < num_menu; i++)
		pPopup->RemoveMenu(0, MF_BYPOSITION);
	
	// Add all items in history
	int menu_pos = 0;
	for(cdstrvect::const_iterator iter = history.begin(); iter != history.end(); iter++, menu_pos++)
		CUnicodeUtils::AppendMenuUTF8(pPopup, MF_STRING, cmd_start + menu_pos, *iter);
}

// Update buttons
void CFindReplaceWindow::UpdateButtons()
{
	// Check for find text
	bool has_find = mFindText.GetWindowTextLength();
	
	// Check for selection in editable target
	long sel_start;
	long sel_end;
	mTargetDisplay->GetSelectionRange(sel_start, sel_end);
	bool has_selection = (sel_start != sel_end);
	bool is_editable = !(mTargetDisplay->GetRichEditCtrl().GetStyle() & ES_READONLY);
	
	// Now update buttons
	mFindBtn.EnableWindow(has_find);
	mReplaceBtn.EnableWindow(has_selection && is_editable);
	mReplaceFindBtn.EnableWindow(has_find && has_selection && is_editable);
	mReplaceAllBtn.EnableWindow(has_find && is_editable);
}

// History item chosen
void CFindReplaceWindow::SelectHistory(const cdstrvect& history, long menu_pos, CEdit* field)
{
	CUnicodeUtils::SetWindowTextUTF8(field, history.at(menu_pos));
	field->SetSel(0, -1);
	GotoDlgCtrl(field);
}

// Initialise history popups and text
void CFindReplaceWindow::CaptureState(void)
{
	sFindText = CUnicodeUtils::GetWindowTextUTF8(&mFindText);

	sReplaceText = CUnicodeUtils::GetWindowTextUTF8(mReplaceText);
	
	// Do history
	cdstrvect::const_iterator found = ::find(sFindHistory.begin(), sFindHistory.end(), sFindText);
	if (found == sFindHistory.end())
	{
		sFindHistory.insert(sFindHistory.begin(), sFindText);
		InitHistory(&mFindPopup, IDM_FIND_HISTORY_Start, sFindHistory);
	}
	found = ::find(sReplaceHistory.begin(), sReplaceHistory.end(), sReplaceText);
	if (found == sReplaceHistory.end())
	{
		sReplaceHistory.insert(sReplaceHistory.begin(), sReplaceText);
		InitHistory(&mReplacePopup, IDM_REPLACE_HISTORY_Start, sReplaceHistory);
	}

	// Find mode
	sFindMode = eFind;
	if (mCaseSensitive.GetCheck())
		sFindMode = (EFindMode) (sFindMode | eCaseSensitive);
	if (mBackwards.GetCheck())
		sFindMode = (EFindMode) (sFindMode | eBackwards);
	if (mWrap.GetCheck())
		sFindMode = (EFindMode) (sFindMode | eWrap);
	if (mEntireWord.GetCheck())
		sFindMode = (EFindMode) (sFindMode | eEntireWord);
}

void CFindReplaceWindow::PostNcDestroy()
{
	delete this;
}

#pragma mark ____________________________Commands

void CFindReplaceWindow::OnChangeFindText()
{
	// Typing in find field - update buttons
	UpdateButtons();
}

void CFindReplaceWindow::OnFindPopup(UINT nID)
{
	SelectHistory(sFindHistory, nID - IDM_FIND_HISTORY_Start, &mFindText);
}

void CFindReplaceWindow::OnReplacePopup(UINT nID)
{
	SelectHistory(sReplaceHistory, nID - IDM_REPLACE_HISTORY_Start, &mReplaceText);
}

#pragma mark ____________________________Execute

void CFindReplaceWindow::OnFindText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	// Do 'Find Next' command as 'Find' command only display this window!
	mTargetDisplay->DoFindNextText(sFindMode & eBackwards);
	
	// Must close the window (do after last access to instance variables as delete this called)
	DestroyWindow();
}

void CFindReplaceWindow::OnReplaceText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceText();
	
	// Must close the window (do after last access to instance variables as delete this called)
	DestroyWindow();
}

void CFindReplaceWindow::OnReplaceFindText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceFindText(sFindMode & eBackwards);
	
	// Must close the window (do after last access to instance variables as delete this called)
	DestroyWindow();
}

void CFindReplaceWindow::OnReplaceAllText()
{
	// Prepare for command
	PrepareTarget();

	// Execute command in target
	mTargetDisplay->DoReplaceAllText();
	
	// Must close the window (do after last access to instance variables as delete this called)
	DestroyWindow();
}

// Prepare target before execution
void CFindReplaceWindow::PrepareTarget()
{
	// Capture state
	CaptureState();

	// Switch to window of target
	FRAMEWORK_WINDOW_TO_TOP(mTargetDisplay)
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
	CRect set_rect = state->GetBestRect(CPreferences::sPrefs->mFindReplaceWindowDefault.GetValue());
	if (set_rect.left && set_rect.top)
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset position - keep width the same as dlog units may result in scaling
		SetWindowPos(nil, set_rect.left, set_rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
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

	mCaseSensitive.SetCheck((sFindMode & eCaseSensitive) != 0);
	mBackwards.SetCheck((sFindMode & eBackwards) != 0);
	mWrap.SetCheck((sFindMode & eWrap) != 0);
	mEntireWord.SetCheck((sFindMode & eEntireWord) != 0);
	
	// If forced reset, save it
	if (force)
		OnSaveDefaultState();
}

// Save current state in prefs
void CFindReplaceWindow::OnSaveDefaultState(void)
{
	// Get bounds
	CRect bounds;
	GetWindowRect(bounds);
	bounds.right = 0;
	bounds.bottom = 0;

	// Add info to prefs
	CFindReplaceWindowState state(nil, &bounds, eWindowStateNormal,
				mCaseSensitive.GetCheck(), mBackwards.GetCheck(), mWrap.GetCheck(), mEntireWord.GetCheck());
	if (CPreferences::sPrefs->mFindReplaceWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mFindReplaceWindowDefault.SetDirty();
}
