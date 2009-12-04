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


// Source for CMailboxInfoView class

#include "CMailboxInfoView.h"

#include "CCommands.h"
#include "CErrorHandler.h"
#include "CLetterWindow.h"
#include "CMailAccountManager.h"
#include "CMailboxTitleTable.h"
#include "CMailboxInfoTable.h"
#include "CMailboxInfoToolbar.h"
#include "CMailboxInfoWindow.h"
#include "CMboxProtocol.h"
#include "CMbox.h"
#include "CMessageWindow.h"
#include "CMessage.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CQuickSearchSavedPopup.h"
#include "CSearchWindow.h"
#include "CSplitterView.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CToolbarView.h"
#include "CWindowsMenu.h"
#include "C3PaneMailboxToolbar.h"
#include "C3PaneWindow.h"

#include <LBevelButton.h>
#include <LPopupButton.h>

#include <string.h>

#include <numeric>

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X W I N D O W
// __________________________________________________________________________________________________


// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CMailboxInfoView::CMailboxInfoView()
{
	mNoSearchReset = false;
	mShowQuickSearch = true;
}

// Constructor from stream
CMailboxInfoView::CMailboxInfoView(LStream *inStream)
		: CMailboxView(inStream)
{
	mNoSearchReset = false;
	mShowQuickSearch = true;
}

// Default destructor
CMailboxInfoView::~CMailboxInfoView()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup help balloons
void CMailboxInfoView::FinishCreateSelf(void)
{
	// Do inherited
	CMailboxView::FinishCreateSelf();

	// Get panes
	mQuickSearchArea = (LView*) FindPaneByID(paneid_MailboxQuickSearchArea);
	mQuickSearchIndicator = FindPaneByID(paneid_MailboxQuickSearchIndicator);
	mQuickSearchIndicator->Hide();
	mQuickSearchCriteria = (LPopupButton*) FindPaneByID(paneid_MailboxQuickSearchCriteria);
	long adjusted_value = CPreferences::sPrefs->mQuickSearch.GetValue() + 1;
	if (adjusted_value > eQuickSearchBody)
		adjusted_value++;
	mQuickSearchCriteria->SetValue(adjusted_value);
	mQuickSearchCriteria->AddListener(this);
	mQuickSearchText = (CTextFieldX*) FindPaneByID(paneid_MailboxQuickSearchText);
	mQuickSearchText->SetBroadcastReturn(true);
	mQuickSearchText->AddListener(this);
	mQuickSearchSaved = (LPopupButton*) FindPaneByID(paneid_MailboxQuickSearchSaved);
	mQuickSearchSaved->SetValue(1);
	mQuickSearchSaved->AddListener(this);
	mQuickSearchProgress = (LView*) FindPaneByID(paneid_MailboxQuickSearchProgress);
	mQuickSearchProgress->Hide();
	mQuickSearchCancel = (LBevelButton*) FindPaneByID(paneid_MailboxQuickSearchCancel);
	mQuickSearchCancel->AddListener(this);
	mQuickSearchCancel->SetEnabled(false);
	mTableArea = (LView*) FindPaneByID(paneid_MailboxTableArea);

	// Get caption panes
	mTotal = (CStaticText*) FindPaneByID(paneid_MailboxTotalNum);
	mUnseen = (CStaticText*) FindPaneByID(paneid_MailboxNewNum);
	mDeleted = (CStaticText*) FindPaneByID(paneid_MailboxDeletedNum);
	mMatched = (CStaticText*) FindPaneByID(paneid_MailboxMatchedNum);
	
	// Init quick search area
	DoQuickSearchPopup(adjusted_value);

	// Hide quick search if user does not want it
	if (!CPreferences::sPrefs->mQuickSearchVisible.GetValue())
		ShowQuickSearch(false);
}

// Respond to commands
Boolean CMailboxInfoView::ObeyCommand(CommandT inCommand,void *ioParam)
{
	Boolean cmdHandled = true;

	switch (inCommand)
	{
	case cmd_ShowQuickSearch:
		// Change and update command
		ShowQuickSearch(!mShowQuickSearch);
		CPreferences::sPrefs->mQuickSearchVisible.SetValue(mShowQuickSearch);
		break;

	default:
		cmdHandled = CMailboxView::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

//	Pass back status of a (menu) command
void CMailboxInfoView::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	switch (inCommand)
	{
	case cmd_ShowQuickSearch:
		{
			outEnabled = true;
			LStr255 txt(STRx_Standards, mShowQuickSearch ? str_HideQuickSearch : str_ShowQuickSearch);
			::PLstrcpy(outName, txt);
		}
		break;

	default:
		CMailboxView::FindCommandStatus(inCommand, outEnabled, outUsesMark,
							outMark, outName);
		break;
	}
}

// Respond to clicks in the icon buttons
void CMailboxInfoView::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_MailboxQuickSearchCriteria:
		DoQuickSearchPopup(mQuickSearchCriteria->GetValue());
	
		// Always force focus to the quick search text field
		if (mQuickSearchCriteria->GetValue() != eQuickSearchSaved)
			LCommander::SetTarget(mQuickSearchText);
		break;
	case msg_MailboxQuickSearchText:
		StartQuickSearchTimer();
		break;
	case msg_MailboxQuickSearchSaved:
		DoQuickSearchSavedPopup(mQuickSearchSaved->GetValue());
		break;
	case msg_MailboxQuickSearchText + 1:
		StopRepeating();
		DoQuickSearch(true);
	case msg_MailboxQuickSearchText + 2:
		if (IsRepeating())
		{
			StopRepeating();
			DoQuickSearch(true);
		}
		else
			SwitchTarget(GetInfoTable());
		break;
	case msg_MailboxQuickSearchCancel:
		DoQuickSearchCancel();
		break;
	default:;
	}
}

CMailboxToolbarPopup* CMailboxInfoView::GetCopyBtn() const
{
	if (Is3Pane())
	{
		// Copy btn from toolbar
		return C3PaneWindow::s3PaneWindow->GetMailboxToolbar()->GetCopyBtn();
	}
	else
	{
		// Match popup from toolbar
		return static_cast<CMailboxInfoToolbar*>(mToolbar)->GetCopyBtn();
	}
}

// Make a toolbar appropriate for this view
void CMailboxInfoView::MakeToolbars(CToolbarView* parent)
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
		CMailboxInfoToolbar* tb = static_cast<CMailboxInfoToolbar*>(UReanimator::CreateView(paneid_MailboxInfoToolbar1, parent, NULL));
		mToolbar = tb;

		// Toolbar must listen to view to get activate/deactive broadcast
		Add_Listener(tb);
		
		// Now give toolbar to its view as standard buttons
		parent->AddToolbar(tb, GetTable(), CToolbarView::eStdButtonsGroup);
	}
}

// Enable quick search items
void CMailboxInfoView::EnableQuickSearch(bool enable)
{
	mQuickSearchCriteria->SetEnabled(enable);
	mQuickSearchText->SetEnabled(enable);
	mQuickSearchProgress->SetEnabled(enable);
	mQuickSearchCancel->SetEnabled(false);
	mQuickSearchIndicator->SetVisible(false);
}

// Show quick search area
void CMailboxInfoView::ShowQuickSearch(bool show)
{
	if (!(mShowQuickSearch ^ show))
		return;

	SDimension16 move_size;
	mQuickSearchArea->GetFrameSize(move_size);
	SInt16 moveby = move_size.height;

	if (show)
	{
		// Shrink/move table pane
		mTableArea->ResizeFrameBy(0, -moveby, false);
		mTableArea->MoveBy(0, moveby, false);

		// Show parts after all other changes
		mQuickSearchArea->Show();
	}
	else
	{
		// Hide parts before other changes
		mQuickSearchArea->Hide();

		// Expand/move splitter
		mTableArea->ResizeFrameBy(0, moveby, false);
		mTableArea->MoveBy(0, -moveby, false);
	}

	mShowQuickSearch = show;

	Refresh();
}

// Do quick search
void CMailboxInfoView::DoQuickSearchPopup(long value)
{
	// Store current value in prefs
	long adjusted_value = value - ((value == eQuickSearchSaved) ? 1 : 0);
	CPreferences::sPrefs->mQuickSearch.SetValue(adjusted_value - 1);
	
	// Hide/show saved popup
	if (value == eQuickSearchSaved)
	{
		mQuickSearchText->Hide();
		mQuickSearchSaved->Show();
		mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eChoose);
	}
	else
	{
		mQuickSearchSaved->Hide();
		mQuickSearchText->Show();
	}
}

// Do quick search
void CMailboxInfoView::DoQuickSearch(bool change_focus)
{
	mQuickSearchText->Hide();
	mQuickSearchProgress->Show();

	cdstring txt = mQuickSearchText->GetText();

	std::auto_ptr<CSearchItem> spec;
	switch(mQuickSearchCriteria->GetValue())
	{
	case eQuickSearchFrom:
		spec.reset(new CSearchItem(CSearchItem::eFrom, txt));
		break;
	case eQuickSearchTo:
		spec.reset(new CSearchItem(CSearchItem::eTo, txt));
		break;
	case eQuickSearchRecipient:
		spec.reset(new CSearchItem(CSearchItem::eRecipient, txt));
		break;
	case eQuickSearchCorrespondent:
		spec.reset(new CSearchItem(CSearchItem::eCorrespondent, txt));
		break;
	case eQuickSearchSubject:
		spec.reset(new CSearchItem(CSearchItem::eSubject, txt));
		break;
	case eQuickSearchBody:
		spec.reset(new CSearchItem(CSearchItem::eBody, txt));
		break;
	}
	
	// Make sure search failure does not prevent clean-up
	try
	{
		if (txt.length() != 0)
			GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec.get(), false);
		else
			GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
	}
	catch(...)
	{
		CLOG_LOGCATCH(...);
	}

	mQuickSearchCancel->SetEnabled(txt.length() != 0);
	mQuickSearchIndicator->SetVisible(txt.length() != 0);
	if (change_focus)
		SwitchTarget(GetInfoTable());

	mQuickSearchProgress->Hide();
	mQuickSearchText->Show();
	if (!change_focus)
		SwitchTarget(mQuickSearchText);
}

// Do quick search saved
void CMailboxInfoView::DoQuickSearchSavedPopup(long value)
{
	if (value == CQuickSearchSavedPopup::eChoose)
	{
		GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
		mQuickSearchCancel->SetEnabled(false);
		mQuickSearchIndicator->SetVisible(false);
	}
	else
	{
		mQuickSearchSaved->Hide();
		mQuickSearchProgress->Show();

		// Get search set
		std::auto_ptr<CSearchItem> spec(new CSearchItem(CSearchItem::eNamedStyle, CPreferences::sPrefs->mSearchStyles.GetValue().at(value - CQuickSearchSavedPopup::eFirst)->GetName()));

		// Make sure search failure does not prevent clean-up
		try
		{
			GetInfoTable()->SetMatch(NMbox::eViewMode_ShowMatch, spec.get(), false);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}

		mQuickSearchCancel->SetEnabled(true);
		mQuickSearchIndicator->SetVisible(true);
		SwitchTarget(GetInfoTable());

		mQuickSearchProgress->Hide();
		mQuickSearchSaved->Show();
	}
}

// Cancel quick search
void CMailboxInfoView::DoQuickSearchCancel()
{
	GetInfoTable()->SetMatch(NMbox::eViewMode_All, NULL, false);
	mQuickSearchCancel->SetEnabled(false);
	mQuickSearchIndicator->SetVisible(false);
	
	if (mQuickSearchSaved->IsVisible())
	{
		StopListening();
		mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eChoose);
		StartListening();
	}
}

// Sync quick search with current mailbox Match state
void CMailboxInfoView::SyncQuickSearch()
{
	// Ignore this if a search is already in progress as we are already sync'd
	if (mQuickSearchProgress->IsVisible())
		return;

	if (GetMbox() && (GetMbox()->GetViewMode() == NMbox::eViewMode_ShowMatch) && (GetMbox()->GetViewSearch() != NULL))
	{
		EnableQuickSearch(true);
		const CSearchItem* spec = GetMbox()->GetViewSearch();
		
		// Some types we can handle
		long menu_item = 0;
		cdstring txt;
		switch(spec->GetType())
		{
		case CSearchItem::eFrom:
		case CSearchItem::eTo:
		case CSearchItem::eRecipient:
		case CSearchItem::eCorrespondent:
		case CSearchItem::eSubject:
		case CSearchItem::eBody:
			switch(spec->GetType())
			{
			case CSearchItem::eFrom:
				menu_item = eQuickSearchFrom;
				break;
			case CSearchItem::eTo:
				menu_item = eQuickSearchTo;
				break;
			case CSearchItem::eRecipient:
				menu_item = eQuickSearchRecipient;
				break;
			case CSearchItem::eCorrespondent:
				menu_item = eQuickSearchCorrespondent;
				break;
			case CSearchItem::eSubject:
				menu_item = eQuickSearchSubject;
				break;
			case CSearchItem::eBody:
				menu_item = eQuickSearchBody;
				break;
			default:;
			}
			if (spec->GetData() != NULL)
				txt = *reinterpret_cast<const cdstring*>(spec->GetData());
			
			// Now set controls
			StopListening();
			mQuickSearchCriteria->SetValue(menu_item);
			mQuickSearchText->SetText(txt);
			DoQuickSearchPopup(menu_item);
			mQuickSearchCancel->SetEnabled(true);
			mQuickSearchIndicator->SetVisible(true);
			StartListening();
			break;
		case CSearchItem::eNamedStyle:
			if (spec->GetData() != NULL)
				txt = *reinterpret_cast<const cdstring*>(spec->GetData());
			menu_item = CPreferences::sPrefs->mSearchStyles.GetValue().FindIndexOf(txt);
			
			// Now set controls
			StopListening();
			mQuickSearchCriteria->SetValue(eQuickSearchSaved);
			DoQuickSearchPopup(eQuickSearchSaved);
			if (menu_item != -1)
				mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eFirst + menu_item);
			else
				mQuickSearchSaved->SetValue(CQuickSearchSavedPopup::eChoose);
			mQuickSearchCancel->SetEnabled(true);
			mQuickSearchIndicator->SetVisible(true);
			StartListening();
			break;
		default:;
		}
	}
	else
		// Enable/disable but not in quick search mode
		EnableQuickSearch(GetMbox() != NULL);
}

// Called during idle
void CMailboxInfoView::SpendTime(const EventRecord &inMacEvent)
{
	// See if we are equal or greater than trigger
	if (TickCount() >= mQuickSearchLastTime)
	{
		StopRepeating();
		DoQuickSearch(false);
	}
}

// Start timer to trigger quick search
void CMailboxInfoView::StartQuickSearchTimer()
{
	// Cache the time at which we trigger
	mQuickSearchLastTime = TickCount() + GetDblTime();

	// Start idle time processing
	StartRepeating();
}

// Set window state
void CMailboxInfoView::ResetState(bool force)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Get new name of this window
	cdstring name = mbox->GetAccountName();

	// Get window state from prefs
	CMailboxWindowState* state = CPreferences::sPrefs->GetMailboxWindowInfo(name);

	// If no prefs try default
	if (!state || force)
		state = &CPreferences::sPrefs->mMailboxWindowDefault.Value();

	// Do not set if empty
	Rect set_rect = state->GetBestRect(CPreferences::sPrefs->mMailboxWindowDefault.GetValue());
	if (!::EmptyRect(&set_rect))
	{
		// Only resize if 1-pane
		if (!Is3Pane())
		{
			// Clip to screen
			::RectOnScreen(set_rect, GetMailboxWindow());

			// Reset bounds
			GetMailboxWindow()->DoSetBounds(set_rect);
		}
	}

	// Adjust size of tables
	ResetColumns(state->GetBestColumnInfo(CPreferences::sPrefs->mMailboxWindowDefault.GetValue()));

	// Do match BEFORE sorting so that sort is only done on the matched set rather than being done twice

	// Only do search state if required
	if (!mNoSearchReset)
	{
		// Match bits and force mbox update
		GetInfoTable()->SetCurrentMatch(state->GetMatchItem());
		GetInfoTable()->SetMatch(state->GetViewMode(), GetInfoTable()->GetCurrentMatch().ConstructSearch(NULL), false);
	}
	
	// Adjust sorting
	SetSortBy(state->GetSortBy());

	// Set sorting button
	mSortDirectionBtn->StopBroadcasting();
	mSortDirectionBtn->SetValue((state->GetShowBy() == cShowMessageDescending) ? 1 : 0);
	mSortDirectionBtn->StartBroadcasting();
	SetShowBy(state->GetShowBy());

	// Set zoom state if 1-pane
	if (!Is3Pane() && (state->GetState() == eWindowStateMax))
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
		SaveState();

} // CMailboxInfoView::SetState

// Save current state in prefs
void CMailboxInfoView::SaveState(void)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Can only do if mbox still open
	if (!mbox->IsOpen())
		return;

	// Get name as cstr
	cdstring name = mbox->GetAccountName();

	// Get bounds
	Rect bounds;
	bool zoomed = (!Is3Pane() ? GetMailboxWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetMailboxWindow()->GetUserBounds(user_bounds);

	// Get current match item
	CMatchItem match;
	if (!mbox->GetAutoViewMode())
		match = GetInfoTable()->GetCurrentMatch();
	NMbox::EViewMode mode = mbox->GetAutoViewMode() ? NMbox::eViewMode_All : mbox->GetViewMode();
	if (!std::accumulate(match.GetBitsSet().begin(), match.GetBitsSet().end(), false, std::logical_or<bool>()) && !match.GetSearchSet().size())
		mode = NMbox::eViewMode_All;

	// Add info to prefs
	CMailboxWindowState* info = new CMailboxWindowState(name,
														&user_bounds,
														zoomed ? eWindowStateMax : eWindowStateNormal,
														&mColumnInfo,
														(ESortMessageBy) mbox->GetSortBy(),
														(EShowMessageBy) mbox->GetShowBy(),
														mode,
														&match,
														Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());

	if (info)
		CPreferences::sPrefs->AddMailboxWindowInfo(info);
}

// Save current state as default
void CMailboxInfoView::SaveDefaultState(void)
{
	CMbox* mbox = GetMbox();
	if (!mbox)
		return;

	// Get bounds
	Rect bounds;
	bool zoomed = (!Is3Pane() ? GetMailboxWindow()->CalcStandardBounds(bounds) : false);

	// Get unzoomed bounds
	Rect user_bounds = {0, 0, 0, 0};
	if (!Is3Pane())
		GetMailboxWindow()->GetUserBounds(user_bounds);

	// Get current match item
	CMatchItem match;
	if (!mbox->GetAutoViewMode())
		match = GetInfoTable()->GetCurrentMatch();

	// Add info to prefs
	CMailboxWindowState state(NULL,
								&user_bounds,
								zoomed ? eWindowStateMax : eWindowStateNormal,
								&mColumnInfo,
								(ESortMessageBy) mbox->GetSortBy(),
								(EShowMessageBy) mbox->GetShowBy(),
								mbox->GetAutoViewMode() ? NMbox::eViewMode_All : mbox->GetViewMode(),
								&match,
								Is3Pane() ? 0 : GetMailboxWindow()->GetSplitter()->GetRelativeSplitPos());
	if (CPreferences::sPrefs->mMailboxWindowDefault.Value().Merge(state))
		CPreferences::sPrefs->mMailboxWindowDefault.SetDirty();

}
