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

#include "CDictionaryPageScroller.h"
#include "CErrorHandler.h"
#include "CMulberryCommon.h"
#include "CSpellAddDialog.h"
#include "CSpellEditDialog.h"
#include "CSpellOptionsDialog.h"
#include "CStaticText.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"
#include "CTextTable.h"
#include "CUStringUtils.h"
#include "CWindowStates.h"

#include <LActiveScroller.h>
#include <LCheckBox.h>
#include <LPushButton.h>
#include <LRadioButton.h>
#include <LScrollBar.h>

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

CPreferenceValue<CWindowState>* CSpellCheckDialog::sWindowState = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CSpellCheckDialog::CSpellCheckDialog()
{
	mSpeller = NULL;
	mSuggestionCtr = 0;
	mDone = false;

	if (!sWindowState)
		sWindowState = new CPreferenceValue<CWindowState>;
}

// Constructor from stream
CSpellCheckDialog::CSpellCheckDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	mSpeller = NULL;
	mSuggestionCtr = 0;
	mDone = false;

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
		mWindow->DoSetBounds(mWindowBounds);
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CSpellCheckDialog::FinishCreateSelf()
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get items
	mDictName = (CStaticText*) FindPaneByID(paneid_SpellCheckDictName);
	mScroller = (CDictionaryPageScroller*) FindPaneByID(paneid_SpellCheckScroller);
	mWordTop = (CStaticText*) FindPaneByID(paneid_SpellCheckWordTop);
	mWordBottom = (CStaticText*) FindPaneByID(paneid_SpellCheckWordBottom);
	mScroller->SetRangeCaptions(mWordTop, mWordBottom);
	mList = (CTextTable*) FindPaneByID(paneid_SpellCheckList);
	mList->SetSingleSelection();
	mList->AddListener(this);
	mWord = (CStaticText*) FindPaneByID(paneid_SpellCheckWord);
	mSuggestion = (CTextFieldX*) FindPaneByID(paneid_SpellCheckSuggestion);
	mSuggestion->SetBroadcastReturn(false);
	mAdd = (LPushButton*) FindPaneByID(paneid_SpellCheckAdd);
	mEdit = (LPushButton*) FindPaneByID(paneid_SpellCheckEdit);
	mReplace = (LPushButton*) FindPaneByID(paneid_SpellCheckReplace);
	mReplaceAll = (LPushButton*) FindPaneByID(paneid_SpellCheckReplaceAll);
	mSkip = (LPushButton*) FindPaneByID(paneid_SpellCheckSkip);
	mSkipAll = (LPushButton*) FindPaneByID(paneid_SpellCheckSkipAll);
	mSuggestMore = (LPushButton*) FindPaneByID(paneid_SpellCheckSuggestMore);
	mOptions = (LPushButton*) FindPaneByID(paneid_SpellCheckOptions);
	mSendNow = (LPushButton*) FindPaneByID(paneid_SpellCheckSendNow);
	mCancel = (LPushButton*) FindPaneByID(paneid_SpellCheckCancel);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CSpellCheckDialogBtns);

}

// Reset state from prefs
void CSpellCheckDialog::ResetState()
{
	// Do not set if empty
#ifdef __MULBERRY_V2
	Rect set_rect = sWindowState->Value().GetBestRect(sWindowState->GetValue());
#else
	Rect set_rect = sWindowState.GetValue().mWindow_bounds;
#endif
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}
}

// Save state in prefs
void CSpellCheckDialog::SaveState()
{
	// Get name as cstr
	cdstring name;

	// Get bounds
	Rect bounds = mUserBounds;

	// Add info to prefs
#ifdef __MULBERRY_V2
	if (sWindowState->Value().Merge(CWindowState(name, &bounds, eWindowStateNormal)))
		sWindowState->SetDirty();
#else
	sWindowState.SetValue(CWindowState(name, &bounds));
#endif
}

Boolean CSpellCheckDialog::HandleKeyPress ( const EventRecord	&inKeyEvent )
{
	bool		keyHandled = false;
	LControl*	keyButton = NULL;

	switch (inKeyEvent.message & charCodeMask)
	{

	case char_Tab:
		// Check for command key
		if (inKeyEvent.modifiers & cmdKey)
		{
			RotateDefault();
			keyHandled = true;
			break;
		}

		// Fall through

	default:
		keyHandled = LDialogBox::HandleKeyPress(inKeyEvent);
		break;
	}

	return keyHandled;

}	//	LDialogBox::HandleKeyPress

// Handle OK button
void CSpellCheckDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{

		case msg_SpellCheckList:
			// Transfer selected word to suggestion and default to replace
			{
				STableCell aCell = mList->GetFirstSelectedCell();
				if (aCell.row)
				{
					char str[256];
					UInt32	len = sizeof(str);
					mList->GetCellData(aCell, str, len);
					SetSuggestion(str);
				}
			}
			break;

		case msg_SpellCheckDblClkList:
			// Do replace action with selected item
			{
				STableCell aCell = mList->GetFirstSelectedCell();
				if (aCell.row)
				{
					char str[256];
					UInt32	len = sizeof(str);
					mList->GetCellData(aCell, str, len);
					SetSuggestion(str);
					DoReplace();
				}
			}
			break;

		case msg_SpellCheckWord:
			// Transfer unknown word to suggestion and delault to replace
			{
				mSuggestion->SetText(mWord->GetText());
				SetDefaultButton(paneid_SpellCheckReplace);
			}
			break;

		case msg_SpellCheckSuggestion:
			// Any typing makes replace btn default
			SetDefaultButton(paneid_SpellCheckReplace);
			break;

		case msg_SpellCheckAdd:
			DoAdd();
			break;

		case msg_SpellCheckEdit:
			DoEdit();
			break;

		case msg_SpellCheckReplace:
			DoReplace();
			break;

		case msg_SpellCheckReplaceAll:
			DoReplaceAll();
			break;

		case msg_SpellCheckSkip:
			DoSkip();
			break;

		case msg_SpellCheckSkipAll:
			DoSkipAll();
			break;

		case msg_SpellCheckSuggestMore:
			DoSuggestMore();
			break;

		case msg_SpellCheckOptions:
			DoOptions();
			break;

		default:
			LDialogBox::ListenToMessage(inMessage, ioParam);
			break;
	}
}

// Set the details
void CSpellCheckDialog::SetSpeller(CSpellPlugin* speller, LWindow* wnd, CTextDisplay* text, long start, cdstring* check, bool sending)
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

	// Set details in scroller
	mScroller->SetDetails(mSpeller, this);

	// Do window state if not auto
	if (mSpeller->GetPreferences()->mAutoPositionDialog.GetValue())
	{
		// Determine position of text display
		Rect txt_bounds;
		mText->CalcPortFrameRect(txt_bounds);
		mText->PortToGlobalPoint(topLeft(txt_bounds));
		mText->PortToGlobalPoint(botRight(txt_bounds));

		// Get dialog bounds
		Rect dlog_bounds = UWindows::GetWindowStructureRect(mMacWindowP);

		// Set new_bounds
		short move_x = txt_bounds.left - dlog_bounds.left;
		short move_y = txt_bounds.top - 4 - dlog_bounds.bottom;

		// Clip to top of screen
		if (dlog_bounds.top + move_y < ::GetMBarHeight())
		{
			short wnd_move = ::GetMBarHeight() - (dlog_bounds.top + move_y);
			move_y += wnd_move;

			if (mWindow)
			{
				// Get original position of window
				mWindow->CalcPortFrameRect(mWindowBounds);
				mWindow->PortToGlobalPoint(topLeft(mWindowBounds));
				mWindow->PortToGlobalPoint(botRight(mWindowBounds));

				// Shrink by offset
				mWindowBounds.top += wnd_move;

				// Set new position and reset top previous
				mWindow->DoSetBounds(mWindowBounds);
				mWindowBounds.top -= wnd_move;

				mWindowMoved = true;
			}
		}

		// Adjust horiz & vert
		Rect new_bounds;
		CalcPortFrameRect(new_bounds);
		PortToGlobalPoint(topLeft(new_bounds));
		PortToGlobalPoint(botRight(new_bounds));
		new_bounds.left += move_x;
		new_bounds.right += move_x;
		new_bounds.top += move_y;
		new_bounds.bottom += move_y;

		DoSetBounds(new_bounds);

	}
	else
		ResetState();

	// Set dictionary name
	mDictName->SetText(mSpeller->GetPreferences()->mDictionaryName.GetValue());

	// Now show the window
	Show();

	// Make sure text thinks its active so selection gets full hilite
	mText->Activate();

	// Start with first error
	NextError();

}

// Rotate default button
void CSpellCheckDialog::RotateDefault()
{
	switch(mDefaultButtonID)
	{
	case paneid_SpellCheckAdd:
		SetDefaultButton(paneid_SpellCheckEdit);
		break;
	case paneid_SpellCheckEdit:
		if (mReplace->IsEnabled())
		{
			SetDefaultButton(paneid_SpellCheckReplace);
			break;
		}
	case paneid_SpellCheckReplace:
		if (mSkip->IsEnabled())
		{
			SetDefaultButton(paneid_SpellCheckSkip);
			break;
		}
	case paneid_SpellCheckSkip:
		if (mReplace->IsEnabled())
		{
			SetDefaultButton(paneid_SpellCheckReplaceAll);
			break;
		}
	case paneid_SpellCheckReplaceAll:
		if (mSkipAll->IsEnabled())
		{
			SetDefaultButton(paneid_SpellCheckSkipAll);
			break;
		}
	case paneid_SpellCheckSkipAll:
		SetDefaultButton(paneid_SpellCheckOptions);
		break;
	case paneid_SpellCheckOptions:
		SetDefaultButton(paneid_SpellCheckCancel);
		break;
	case paneid_SpellCheckCancel:
		if (mAdd->IsEnabled())
		{
			SetDefaultButton(paneid_SpellCheckAdd);
			break;
		}
	}
}

// Set the details
void CSpellCheckDialog::NextError()
{
	// Process sequence of errors
	const CSpellPlugin::SpError* sperr = NULL;
	do
	{
		// Get next error on speller
		sperr = mSpeller->NextError(*mCheckText);

		// Check for any left
		if (!sperr)
		{
			// Terminate if no more
			mDone = true;
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

	// Reset suggest more
	mSuggestionCtr = 0;
	mSuggestMore->Enable();

	// Get wrong word
	cdstring str = sperr->word;
	mWord->SetText(str);

	// Check for punctuation based error
	if (mSpeller->ErrorIsPunct())
	{
		mAdd->Disable();
		mReplaceAll->Disable();
		mSkipAll->Disable();
		mSuggestMore->Disable();
		mList->Disable();

		// Clear suggestion
		if (sperr->replacement)
			str = sperr->replacement;
		else
			str = cdstring::null_str;
		mSuggestion->SetText(str);
		mSuggestion->Disable();

		// Clear list
		mList->RemoveAllRows(true);
	}
	else if (mSpeller->ErrorIsDoubleWord())
	{
		mAdd->Disable();
		mReplaceAll->Disable();
		mSkipAll->Disable();
		mSuggestMore->Disable();
		mList->Enable();
		mSuggestion->Enable();

		// Display items in list
		DisplaySuggestions();
	}
	else
	{
		mAdd->Enable();
		mReplaceAll->Enable();
		mSkipAll->Enable();
		mSuggestMore->Enable();
		mList->Enable();
		mSuggestion->Enable();

		// Display items in list
		DisplaySuggestions();
	}

	// Make sure replace disabled for read-only
	if (mText->IsReadOnly())
	{
		mReplace->Disable();
		mReplaceAll->Disable();
	}

	// Skip button is always default
	SetDefaultButton(paneid_SpellCheckSkip);
}

// Do visual select of word
void CSpellCheckDialog::SelectWord(const CSpellPlugin::SpError* sperr)
{
	StGrafPortSaver graf_port_save;

	// Convert utf8 sperr offsets to utf16
	long usel_start = UTF8OffsetToUTF16Offset(*mCheckText, sperr->sel_start);
	long usel_end = UTF8OffsetToUTF16Offset(*mCheckText, sperr->sel_end);

	mText->FocusDraw();
	mText->MoveSelection(mTextSelStart + usel_start, mTextSelStart + usel_end);
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
	StGrafPortSaver graf_port_save;

	// Filter out specials
	char* p = (char*) sperr->replacement;
	while(*p)
	{
		if (*p == '¡') *p = ' ';
		p++;
	}

	// Do visual update (must turn off smart text insert)
	mText->FocusDraw();
	mText->InsertUTF8(sperr->replacement);
}

// Add word to dictionary
void CSpellCheckDialog::DoAdd()
{
	cdstring add_word = mWord->GetText();

	// Must contain something
	if (add_word.empty())
		return;

	// Must not be in dictionary
	if (mSpeller->ContainsWord(add_word))
	{
		// Put error alert
		CErrorHandler::PutNoteAlert(STRx_Spell, str_CannotAddDuplicate);
		return;
	}

	// Do the dialog
	if (CSpellAddDialog::PoseDialog(mSpeller, add_word))
	{
		// Words will be added by add dialog

		// Force to skip all
		DoSkipAll();
	}
}

// Remove word from dicitonary
void CSpellCheckDialog::DoEdit()
{
	// Create the dialog
	CSpellEditDialog::PoseDialog(mSpeller);
}

// Replace word
void CSpellCheckDialog::DoReplace()
{
	// Get current replacement word
	cdstring repl_word = mSuggestion->GetText();

	// First check that replacement word exists
	if (!mSpeller->ErrorIsPunct() && ::strlen(repl_word) && !mSpeller->ContainsWord(repl_word))
	{
		if (CErrorHandler::PutCautionAlert(true, STRx_Spell, str_ReplaceNotInDictionary) == CErrorHandler::Cancel)
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
void CSpellCheckDialog::DoReplaceAll()
{
	// Get current replacement word
	cdstring repl_word = mSuggestion->GetText();

	// First check that replacement word exists
	if (!mSpeller->ErrorIsPunct() && ::strlen(repl_word) && !mSpeller->ContainsWord(repl_word))
	{
		if (CErrorHandler::PutCautionAlert(true, STRx_Spell, str_ReplaceNotInDictionary) == CErrorHandler::Cancel)
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
void CSpellCheckDialog::DoSkip()
{
	// Get speller to do it
	mSpeller->Skip();

	// Now go to next error
	NextError();
}

// Skip all errors
void CSpellCheckDialog::DoSkipAll()
{
	// Get speller to do it
	mSpeller->SkipAll();

	// Now go to next error
	NextError();
}

// Options dialog
void CSpellCheckDialog::DoSuggestMore()
{
	// Bump up suggestion counter and do suggestions again
	mSuggestionCtr++;
	DisplaySuggestions();
	
	// Check whether can do any more
	if (!mSpeller->MoreSuggestions(mSuggestionCtr))
		mSuggestMore->Disable();
}

// Options dialog
void CSpellCheckDialog::DoOptions()
{
	// Create the dialog
	CSpellOptionsDialog::PoseDialog(mSpeller);

	// Reset dictionary name
	mDictName->SetText(mSpeller->GetPreferences()->mDictionaryName.GetValue());
}

// Display suggestions in list
void CSpellCheckDialog::DisplaySuggestions()
{
	// Remove any existing
	mList->RemoveAllRows(false);
	mScroller->DisplayPages(false);

	cdstrvect suggestions;
	mSpeller->GetSuggestions(mSpeller->CurrentError()->word, mSuggestionCtr, suggestions);
	mList->SetContents(suggestions);
	if (mList->GetItemCount())
		mList->SelectCell(STableCell(1, 1));
	else
	{
		char empty = 0;
		SetSuggestion(&empty);
	}

	mScroller->SetCount(mList->GetItemCount());
}

// Display dictionary in list
void CSpellCheckDialog::DisplayDictionary(bool select, bool force_draw)
{
	// Do change to list
	mScroller->DisplayPages(true);
	bool matched = mScroller->DisplayDictionary(select ? mSpeller->CurrentError()->word : NULL, force_draw);

	// Get page buffer and find closest location if switching from guesses
	if (!matched && select)
	{
		char empty = 0;
		SetSuggestion(&empty);
	}
}

// Set suggestion field
void CSpellCheckDialog::SetSuggestion(char* suggest)
{
	if (*suggest)
	{
		mSpeller->ProcessReplaceString(suggest);
		mSuggestion->SetText(suggest);
		SetDefaultButton(paneid_SpellCheckReplace);
	}
	else
	{
		mSuggestion->SetText(suggest);
		SetDefaultButton(paneid_SpellCheckSkip);
	}
}
