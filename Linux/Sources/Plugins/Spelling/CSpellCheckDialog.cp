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

// Source for CSpellCheckDialog class

#include "CSpellCheckDialog.h"

#include "CErrorHandler.h"
#include "CInputField.h"
#include "CMulberryCommon.h"
#include "CTableScrollbarSet.h"
//#include "CSpellAddDialog.h"
//#include "CSpellEditDialog.h"
#include "CSpellOptionsDialog.h"
#include "CTextDisplay.h"
#include "CTextField.h"
#include "CTextTable.h"
#include "CUStringUtils.h"
#include "CWindowStates.h"

#include <JXWindow.h>
#include <JXDownRect.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXInputField.h>

#include <jXGlobals.h>

#include <cassert>
#include <string.h>

CPreferenceValue<CWindowState>* CSpellCheckDialog::sWindowState = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellCheckDialog::CSpellCheckDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
	mSpeller = NULL;
	mSuggestions = true;
	mExtendedGuess = true;

	if (!sWindowState)
		sWindowState = new CPreferenceValue<CWindowState>;
}

// Default destructor
CSpellCheckDialog::~CSpellCheckDialog()
{
	// Make sure window position is saved
	SaveState();

	// Reset window being checked if it was moved during auto position
	if (mWindowMoved && mWindow)
		mWindow->GetWindow()->Place(mWindowPos.x, mWindowPos.y);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellCheckDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 490,220, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 490,220);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Replace Word:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,45, 90,20);
    assert( obj2 != NULL );

    mSuggestion =
        new CTextInputField(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 280,45, 200,20);
    assert( mSuggestion != NULL );

    mSendNow =
        new JXTextButton("Send Now", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 300,150, 80,25);
    assert( mSendNow != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 400,150, 80,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    CScrollbarSet* sbs =
        new CScrollbarSet(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 180,200);
    assert( sbs != NULL );

    mSkip =
        new JXTextButton("Skip", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,115, 80,25);
    assert( mSkip != NULL );

    mAdd =
        new JXTextButton("Add...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,80, 80,25);
    assert( mAdd != NULL );

    mOptions =
        new JXTextButton("Options...", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 200,115, 80,25);
    assert( mOptions != NULL );

    mReplace =
        new JXTextButton("Replace", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 300,80, 80,25);
    assert( mReplace != NULL );

    mReplaceAll =
        new JXTextButton("Replace All", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,80, 80,25);
    assert( mReplaceAll != NULL );

    mSkipAll =
        new JXTextButton("Skip All", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 400,115, 80,25);
    assert( mSkipAll != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Wrong Word:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,15, 85,20);
    assert( obj3 != NULL );

    JXDownRect* obj4 =
        new JXDownRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 280,13, 200,24);
    assert( obj4 != NULL );

    mWord =
        new CStaticText("", obj4,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,2, 200,20);
    assert( mWord != NULL );

// end JXLayout

	mList = new CTextTable(sbs,sbs->GetScrollEnclosure(),
															JXWidget::kHElastic,
															JXWidget::kVElastic,
															0,0, 10, 10);
	mList->OnCreate();
	mList->SetSelectionMsg(true);
	mList->SetDoubleClickMsg(true);
	ListenTo(mList);

	SetButtons(mCancelBtn, NULL);
	GetWindow()->SetTitle("Check Spelling");

	// Listen to the buttons
	ListenTo(mSendNow);
	ListenTo(mCancelBtn);
	ListenTo(mSkip);
	ListenTo(mAdd);
	ListenTo(mOptions);
	ListenTo(mReplace);
	ListenTo(mReplaceAll);
	ListenTo(mSkipAll);
}

// Reset state from prefs
void CSpellCheckDialog::ResetState(void)
{
	// Do not set if empty
	JRect set_rect = sWindowState->Value().GetBestRect(sWindowState->GetValue());
	if (set_rect.left && set_rect.top)
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset position
		GetWindow()->Place(set_rect.left, set_rect.top);
	}
}

// Save state in prefs
void CSpellCheckDialog::SaveState(void)
{
	// Get name as cstr
	cdstring name;

	// Get bounds - convert to position only
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);

	// Add info to prefs
	if (sWindowState->Value().Merge(CWindowState(name, &bounds, eWindowStateNormal)))
		sWindowState->SetDirty();
}

// Handle OK button
void CSpellCheckDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (sender == mList)
	{
		if (message.Is(CTextTable::kSelectionChanged))
		{
			// Transfer selected word to suggestion and default to replace
			if (mList->IsSelectionValid())
			{
				cdstrvect sellist;
				mList->GetSelection(sellist);
				SetSuggestion(sellist.front());
			}
			return;
		}
		else if (message.Is(CTextTable::kLDblClickCell))
		{
			// Do replace action with selected item
			if (mList->IsSelectionValid())
			{
				cdstrvect sellist;
				mList->GetSelection(sellist);
				SetSuggestion(sellist.front());
				DoReplace();
			}
			return;
		}
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mAdd)
		{
			DoAdd();
			return;
		}
		else if (sender == mReplace)
		{
			DoReplace();
			return;
		}
		else if (sender == mReplaceAll)
		{
			DoReplace();
			return;
		}
		else if (sender == mSkip)
		{
			DoSkip();
			return;
		}
		else if (sender == mSkipAll)
		{
			DoSkipAll();
			return;
		}
		else if (sender == mOptions)
		{
			DoOptions();
			return;
		}
		else if (sender == mSendNow)
		{
			EndDialog(kDialogClosed_Btn3);
			return;
		}
	}

	// Pass up
	CDialogDirector::Receive(sender, message);
}

// Set the details
void CSpellCheckDialog::SetSpeller(CSpellPlugin* speller, JXWindowDirector* wnd, CTextDisplay* text, long start, cdstring* check, bool sending)
{
	mSpeller = speller;
	mWindow = wnd;
	mWindowMoved = false;
	mText = text;
	mTextSelStart = start;
	mCheckText = check;

	// Hide send button if not sending
	if (!sending)
		mSendNow->Hide();
}

// Set the details
void CSpellCheckDialog::Activate()
{
	// Do inherited
	CDialogDirector::Activate();

	// Do window state if not auto
	if (mSpeller->GetPreferences()->mAutoPositionDialog.GetValue())
	{
		// Determine position of text display
		if (mWindow)
			mWindowPos = mWindow->GetWindow()->GetDesktopLocation();
		JRect txt_bounds = mText->GetBoundsGlobal();
		txt_bounds.Shift(mWindowPos);

		// Get dialog bounds
		JPoint p = GetWindow()->GetDesktopLocation();
		JCoordinate w = GetWindow()->GetFrameWidth();
		JCoordinate h = GetWindow()->GetFrameHeight();
		JRect dlog_bounds(p.y, p.x, p.y + h, p.x + w);

		// Set new_bounds
		JCoordinate move_x = txt_bounds.left - dlog_bounds.left;
		JCoordinate move_y = txt_bounds.top - 8 - dlog_bounds.bottom;

		// Clip to top of screen
		if (dlog_bounds.top + move_y < 0)
		{
			JCoordinate wnd_move = - (dlog_bounds.top + move_y);
			move_y += wnd_move;

			if (mWindow)
			{
				// Shrink by offset
				mWindowPos.y += wnd_move;

				// Set new position and reset top previous
				mWindow->GetWindow()->Place(mWindowPos.x, mWindowPos.y);
				mWindowPos.y -= wnd_move;

				mWindowMoved = true;
			}
		}

		// Adjust horiz & vert
		dlog_bounds.left += move_x;
		dlog_bounds.right += move_x;
		dlog_bounds.top += move_y;
		dlog_bounds.bottom += move_y;

		GetWindow()->Place(dlog_bounds.left, dlog_bounds.top);
	}
	else
		ResetState();

	// Make sure text thinks its active so selection gets full hilite
	mText->TEActivate();
	mText->TEActivateSelection();

	// Start with first error
	NextError();

}

// Set the details
void CSpellCheckDialog::NextError(void)
{
	// Process sequence of errors
	const CSpellPlugin::SpError* sperr = nil;
	do
	{
		// Get next error on speller
		sperr = mSpeller->NextError(*mCheckText);

		// Check for any left
		if (!sperr)
		{
			// Terminate if no more
			EndDialog(kDialogClosed_Btn1);
			return;
		}

		// Check for replacement
		if (sperr->do_replace)
		{
			// Do visual select
			SelectWord(sperr);

			// Must tell speller to do replacement to keep word ptrs in sync
			mSpeller->Replace();

			// Do visual replace
			ReplaceWord(sperr);
		}

	} while (!sperr->ask_user);

	// Must select it
	SelectWord(sperr);

	// Get wrong word
	mWord->SetText(sperr->word);

	// Check for punctuation based error
	if (mSpeller->ErrorIsPunct())
	{
		mAdd->Deactivate();
		mReplaceAll->Deactivate();
		mSkipAll->Deactivate();
		mList->Deactivate();

		// Clear suggestion
		mSuggestion->SetText(sperr->replacement ? sperr->replacement : cdstring::null_str.c_str());
		mSuggestion->Deactivate();

		// Clear list
		mList->RemoveAllRows(true);
	}
	else if (mSpeller->ErrorIsDoubleWord())
	{
		mAdd->Deactivate();
		mReplaceAll->Deactivate();
		mSkipAll->Deactivate();
		mList->Activate();
		mSuggestion->Activate();

		// Display items in list
		DisplayDictionary(true);
	}
	else
	{
		mAdd->Activate();
		mReplaceAll->Activate();
		mSkipAll->Activate();
		mList->Activate();
		mSuggestion->Activate();

		// Display items in list
		if (mSuggestions)
			DisplaySuggestions();
		else
			DisplayDictionary(true);
	}

	// Make sure replace disabled for read-only
	if (mText->IsReadOnly())
	{
		mReplace->Deactivate();
		mReplaceAll->Deactivate();
	}

	// Skip button is always default
	//SetDefaultButton(paneid_SpellCheckSkip);
}

// Do visual select of word
void CSpellCheckDialog::SelectWord(const CSpellPlugin::SpError* sperr)
{
	// Convert utf8 sperr offsets to utf16
	long usel_start = UTF8OffsetToUTF16Offset(*mCheckText, sperr->sel_start);
	long usel_end = UTF8OffsetToUTF16Offset(*mCheckText, sperr->sel_end);

	mText->SetSel(mTextSelStart + usel_start, mTextSelStart + usel_end);
	mText->TEScrollToSelection(kTrue);
}

// Do visual replace of word
void CSpellCheckDialog::ReplaceWord(const CSpellPlugin::SpError* sperr)
{
	// Update memory buffer
	size_t replace_len = ::strlen(sperr->replacement);
	long difflen = replace_len - ::strlen(sperr->word);
	char* new_txt = new char[mCheckText->length() + difflen + 1];

	::strncpy(new_txt, *mCheckText, sperr->sel_start);
	::strncpy(new_txt + sperr->sel_start, sperr->replacement, replace_len);
	::strcpy(new_txt + sperr->sel_start + replace_len, mCheckText->c_str() + sperr->sel_end);

	*mCheckText = new_txt;

	// Now do visual replace
	mText->InsertUTF8(sperr->replacement);
}

// Add word to dictionary
void CSpellCheckDialog::DoAdd(void)
{
	cdstring add_word = mWord->GetText();

	// Must contain something
	if (add_word.empty())
		return;

	// Must not be in dictionary
	if (mSpeller->ContainsWord(add_word))
	{
		// Put error alert
		//CErrorHandler::PutNoteAlert(STRx_Spell, str_CannotAddDuplicate);
		return;
	}

	// Just add it as is
	mSpeller->AddWord(add_word);

#if 0
	// Do the dialog
	if (CSpellAddDialog::PoseDialog(mSpeller, add_word))
		OnSpellCheckSkipAll();
#endif
}

// Replace word
void CSpellCheckDialog::DoReplace(void)
{
	// Get current replacement word
	cdstring repl_word = mSuggestion->GetText();

	// First check that replacement word exists
	if (!mSpeller->ErrorIsPunct() && repl_word.length() && !mSpeller->ContainsWord(repl_word))
	{
		if (CErrorHandler::PutCautionAlert(true, IDS_SPELL_ReplaceNotInDictionary) == CErrorHandler::Cancel)
			return;
	}

	// Get speller to do it
	((CSpellPlugin::SpError*) mSpeller->CurrentError())->replacement = repl_word;
	mSpeller->Replace();

	// Now do visual replacement
	ReplaceWord(mSpeller->CurrentError());

	// Now go to next error
	NextError();
}

// Replace all words
void CSpellCheckDialog::DoReplaceAll(void)
{
	// Get current replacement word
	cdstring repl_word = mSuggestion->GetText();

	// First check that replacement word exists
	if (!mSpeller->ErrorIsPunct() && repl_word.length() && !mSpeller->ContainsWord(repl_word))
	{
		if (CErrorHandler::PutCautionAlert(true, IDS_SPELL_ReplaceNotInDictionary) == CErrorHandler::Cancel)
			return;
	}

	// Get speller to do it
	((CSpellPlugin::SpError*) mSpeller->CurrentError())->replacement = repl_word;
	mSpeller->ReplaceAll();

	// Now do visual replacement
	ReplaceWord(mSpeller->CurrentError());

	// Now go to next error
	NextError();
}

// Skip error
void CSpellCheckDialog::DoSkip(void)
{
	// Get speller to do it
	mSpeller->Skip();

	// Now go to next error
	NextError();
}

// Skip all errors
void CSpellCheckDialog::DoSkipAll(void)
{
	// Get speller to do it
	mSpeller->SkipAll();

	// Now go to next error
	NextError();
}

// Options dialog
void CSpellCheckDialog::DoOptions(void)
{
	CSpellOptionsDialog::PoseDialog(mSpeller);
}

// Display suggestions in list
void CSpellCheckDialog::DisplaySuggestions(void)
{
	// Remove any existing
	mList->RemoveAllRows(true);

	cdstrvect suggestions;
	mSpeller->GetSuggestions(mSpeller->CurrentError()->word, mExtendedGuess, suggestions);

	{
		mList->SetContents(suggestions);
		if (suggestions.size() != 0)
		{
			// Select first cell and copy to suggestion
			mList->SelectRow(1);
		}
		else
		{
			char empty = 0;
			SetSuggestion(&empty);
		}
	}
	mList->Refresh();

	mSuggestions = true;
}

// Display dictionary in list
void CSpellCheckDialog::DisplayDictionary(bool select, bool force_draw)
{
	mSuggestions = false;
}

// Set suggestion field
void CSpellCheckDialog::SetSuggestion(char* suggest)
{
	if (*suggest)
	{
		mSpeller->ProcessReplaceString(suggest);
		mSuggestion->SetText(suggest);
		//SetDefaultButton(paneid_SpellCheckReplace);
	}
	else
	{
		mSuggestion->SetText(suggest);
		//SetDefaultButton(paneid_SpellCheckSkip);
	}
}
