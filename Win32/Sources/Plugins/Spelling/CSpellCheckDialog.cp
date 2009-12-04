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

// CSpellCheckDialog.cpp : implementation file
//

#include "CSpellCheckDialog.h"

#include "CErrorHandler.h"
#include "CCmdEditView.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CSpellAddDialog.h"
#include "CSpellEditDialog.h"
#include "CSpellOptionsDialog.h"
#include "CUnicodeUtils.h"
#include "CUStringUtils.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellCheckDialog dialog

CPreferenceValue<CWindowState>* CSpellCheckDialog::sWindowState = NULL;

CSpellCheckDialog::CSpellCheckDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CSpellCheckDialog::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSpellCheckDialog)
	//}}AFX_DATA_INIT
	mSpeller = NULL;
	mSuggestionCtr = 0;

	if (!sWindowState)
		sWindowState = new CPreferenceValue<CWindowState>;
}


void CSpellCheckDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSpellCheckDialog)
	DDX_Control(pDX, IDC_SPELLCHECK_PAGE, mPageCount);
	DDX_Control(pDX, IDC_SPELLCHECK_WORDBOTTOM, mWordBottom);
	DDX_Control(pDX, IDC_SPELLCHECK_WORDTOP, mWordTop);
	DDX_Control(pDX, IDC_SPELLCHECK_LIST, mList);
	DDX_Control(pDX, IDC_SPELLCHECK_SUGGEST, mSuggestion);
	DDX_Control(pDX, IDC_SPELLCHECK_WORD, mWord);
	DDX_Control(pDX, IDC_SPELLCHECK_SKIPALL, mSkipAll);
	DDX_Control(pDX, IDC_SPELLCHECK_REPLACEALL, mReplaceAll);
	DDX_Control(pDX, IDC_SPELLCHECK_SKIP, mSkip);
	DDX_Control(pDX, IDC_SPELLCHECK_REPLACE, mReplace);
	DDX_Control(pDX, IDC_SPELLCHECK_EDIT, mEdit);
	DDX_Control(pDX, IDC_SPELLCHECK_ADD, mAdd);
	DDX_Control(pDX, IDC_SPELLCHECK_DICT, mDictName);
	DDX_Control(pDX, IDC_SPELLCHECK_SENDNOW, mSendNow);
	DDX_Control(pDX, IDC_SPELLCHECK_SUGGESTMORE, mSuggestMore);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSpellCheckDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CSpellCheckDialog)
	ON_WM_DESTROY()

	ON_BN_CLICKED(IDC_SPELLCHECK_ADD, OnSpellCheckAdd)
	ON_BN_CLICKED(IDC_SPELLCHECK_EDIT, OnSpellCheckEdit)
	ON_BN_CLICKED(IDC_SPELLCHECK_REPLACE, OnSpellCheckReplace)
	ON_BN_CLICKED(IDC_SPELLCHECK_SKIP, OnSpellCheckSkip)
	ON_BN_CLICKED(IDC_SPELLCHECK_REPLACEALL, OnSpellCheckReplaceAll)
	ON_BN_CLICKED(IDC_SPELLCHECK_SKIPALL, OnSpellCheckSkipAll)
	ON_BN_CLICKED(IDC_SPELLCHECK_OPTIONS, OnSpellCheckOptions)
	ON_BN_CLICKED(IDC_SPELLCHECK_SUGGESTMORE, OnSpellCheckSuggestMore)
	ON_BN_CLICKED(IDC_SPELLCHECK_SENDNOW, OnSpellCheckSendNow)
	ON_LBN_SELCHANGE(IDC_SPELLCHECK_LIST, OnSelchangeSpellCheckList)
	ON_LBN_SELCANCEL(IDC_SPELLCHECK_LIST, OnSelcancelSpellCheckList)
	ON_LBN_DBLCLK(IDC_SPELLCHECK_LIST, OnDblclkSpellCheckList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSpellCheckDialog message handlers

BOOL CSpellCheckDialog::OnInitDialog() 
{
	CHelpDialog::OnInitDialog();
	
	// Set details in scroller
	mList.SubclassDlgItem(IDC_SPELLCHECK_LIST, this);
	mList.SetDetails(mSpeller, &mPageCount);
	mList.SetRangeCaptions(&mWordTop, &mWordBottom);
	mList.DisplayPages(false);

	// Start at first page
	mList.InitPage();

	// Set dictionary name
	CString dname = cdustring(mSpeller->GetPreferences()->mDictionaryName.GetValue());
	mDictName.SetWindowText(dname);
	
	// Set details in scroller
	mList.SetDetails(mSpeller, &mPageCount);

	// Do window state if not auto
	if (mSpeller->GetPreferences()->mAutoPositionDialog.GetValue())
	{
		// Determine position of text display
		CRect txt_bounds;
		mText->GetWindowRect(txt_bounds);
		
		// Get dialog bounds
		CRect dlog_bounds;
		GetWindowRect(dlog_bounds);
		
		// Set new_bounds
		short move_x = txt_bounds.left - dlog_bounds.left;
		short move_y = txt_bounds.top - 4 - dlog_bounds.bottom;

		// Get task bar pos and see if at top
		CRect work_area;
		::SystemParametersInfo(SPI_GETWORKAREA, 0, (Rect*) work_area, 0);

		// Clip to top of screen
		if (dlog_bounds.top + move_y < work_area.top)
		{
			short wnd_move = work_area.top - (dlog_bounds.top + move_y);
			move_y += wnd_move;
			
			if (mWindow)
			{
				WINDOWPLACEMENT wp;
				mWindow->GetParentFrame()->GetWindowPlacement(&wp);

				// Get original position of window
				mWindowBounds = wp.rcNormalPosition;
				
				// Shrink by offset
				mWindowBounds.top += wnd_move;
				
				// Set new position and reset top previous
				mWindow->GetParentFrame()->SetWindowPos(NULL, mWindowBounds.left, mWindowBounds.top, mWindowBounds.Width(), mWindowBounds.Height(), SWP_NOZORDER);
				mWindowBounds.top -= wnd_move;
				
				mWindowMoved = true;
			}
		}

		// Adjust horiz & vert
		dlog_bounds.left += move_x;
		dlog_bounds.right += move_x;
		dlog_bounds.top += move_y;
		dlog_bounds.bottom += move_y;
		
		SetWindowPos(NULL, dlog_bounds.left, dlog_bounds.top, dlog_bounds.Width(), dlog_bounds.Height(), SWP_NOZORDER);
		 
	}
	else
		ResetState();

	// Remove send now button if not sending
	if (!mWillSend)
		mSendNow.ShowWindow(SW_HIDE);

	// Start with first error
	NextError();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Hide instead of close
void CSpellCheckDialog::OnDestroy()
{
	// Save window state
	SaveState();
	
	// Reset window being checked if it was moved during auto position
	if (mWindowMoved && mWindow)
		mWindow->GetParentFrame()->SetWindowPos(NULL, mWindowBounds.left, mWindowBounds.top,
												mWindowBounds.right - mWindowBounds.left, mWindowBounds.bottom - mWindowBounds.top, SWP_NOZORDER);

}

// Reset state from prefs
void CSpellCheckDialog::ResetState(void)
{
	// Do not set if empty
#ifdef __MULBERRY_V2
	CRect set_rect = sWindowState->Value().GetBestRect(sWindowState->GetValue());
#else
	CRect set_rect = sWindowState.GetValue().mWindow_bounds;
#endif
	if (!set_rect.IsRectNull())
	{
		// Clip to screen
		::RectOnScreen(set_rect, NULL);

		// Reset position - keep width the same as dlog units may result in scaling
		SetWindowPos(NULL, set_rect.left, set_rect.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
	}
}

// Save state in prefs
void CSpellCheckDialog::SaveState(void)
{
	// Get name as cstr
	cdstring name;
	
	// Get bounds
	CRect bounds;
	GetWindowRect(bounds);

	// Add info to prefs
#ifdef __MULBERRY_V2
	if (sWindowState->Value().Merge(CWindowState(name, &bounds, eWindowStateNormal)))
		sWindowState->SetDirty();
#else
	sWindowState.SetValue(CWindowState(name, &bounds));
#endif
}

// Set the details
void CSpellCheckDialog::SetSpeller(CSpellPlugin* speller, CWnd* wnd, CCmdEditView* text, long start, cdstring* check, bool sending)
{
	mSpeller = speller;
	mWindow = wnd;
	mWindowMoved = false;
	mText = text;
	mTextSelStart = start;
	mCheckText = check;
	mWillSend = sending;

	// OnInitDialog will do the rest
}

// Set the details
void CSpellCheckDialog::NextError(void)
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
			EndDialog(IDOK);
			return;
		}

		// Check for replacement
		if (sperr->do_replace)
		{
			// Select it first then do replacement
			SelectWord(sperr);
			ReplaceWord(sperr);
		}

	} while (!sperr->ask_user);

	// Must select it
	SelectWord(sperr);

	// Reset suggest more
	mSuggestionCtr = 0;
	mSuggestMore.EnableWindow(true);

	// Get wrong word
	cdstring str = sperr->word;
	CUnicodeUtils::SetWindowTextUTF8(&mWord, str);

	// Check for punctuation based error
	if (mSpeller->ErrorIsPunct())
	{
		mAdd.EnableWindow(false);
		mReplaceAll.EnableWindow(false);
		mSkipAll.EnableWindow(false);
		mSuggestMore.EnableWindow(false);
		mList.EnableWindow(false);

		// Clear suggestion
		if (sperr->replacement)
			str = sperr->replacement;
		else
			str = cdstring::null_str;
		CUnicodeUtils::SetWindowTextUTF8(&mSuggestion, str);
		mSuggestion.EnableWindow(false);

		// Clear list
		mList.ResetContent();
	}
	else if (mSpeller->ErrorIsDoubleWord())
	{
		mAdd.EnableWindow(false);
		mReplaceAll.EnableWindow(false);
		mSkipAll.EnableWindow(false);
		mSuggestMore.EnableWindow(false);
		mList.EnableWindow(true);
		mSuggestion.EnableWindow(true);

		// Display items in list
		DisplaySuggestions();
	}
	else
	{
		mAdd.EnableWindow(true);
		mReplaceAll.EnableWindow(true);
		mSkipAll.EnableWindow(true);
		mSuggestMore.EnableWindow(true);
		mList.EnableWindow(true);
		mSuggestion.EnableWindow(true);

		// Display items in list
		DisplaySuggestions();
	}

	// Make sure replace disabled for read-only
	if (mText->GetRichEditCtrl().GetStyle() & ES_READONLY)
	{
		mReplace.EnableWindow(false);
		mReplaceAll.EnableWindow(false);
	}

	// Skip button is always default
	GotoDlgCtrl(&mSkip);
	SetDefID(IDC_SPELLCHECK_SKIP);
}

// Do visual select of word
void CSpellCheckDialog::SelectWord(const CSpellPlugin::SpError* sperr)
{
	// Convert utf8 sperr offsets to utf16
	long usel_start = UTF8OffsetToUTF16Offset(*mCheckText, sperr->sel_start);
	long usel_end = UTF8OffsetToUTF16Offset(*mCheckText, sperr->sel_end);

	mText->SetSelectionRange(mTextSelStart + usel_start, mTextSelStart + usel_end);
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
	{
		// Filter out specials
		char* p = (char*) sperr->replacement;
		while(*p)
		{
			if (*((unsigned char*) p) == 0xA4) *p = ' ';
			p++;
		}
		
		// Do visual update
		mText->InsertUTF8(sperr->replacement);
	}
}

void CSpellCheckDialog::OnSpellCheckAdd() 
{
	cdstring add_word = CUnicodeUtils::GetWindowTextUTF8(&mWord);
	
	// Must contain something
	if (add_word.empty())
		return;

	// Must not be in dictionary
	if (mSpeller->ContainsWord(add_word))
	{
		// Put error alert
		
		return;
	}
	
	// Do the dialog
	if (CSpellAddDialog::PoseDialog(mSpeller, add_word))
		OnSpellCheckSkipAll();
}

void CSpellCheckDialog::OnSpellCheckEdit() 
{
	// Create the dialog
	CSpellEditDialog::PoseDialog(mSpeller);
}

void CSpellCheckDialog::OnSpellCheckReplace() 
{
	// Get current replacement word
	cdstring repl_word = CUnicodeUtils::GetWindowTextUTF8(&mSuggestion);
	
	// First check that replacement word exists
	if (!mSpeller->ErrorIsPunct() && !repl_word.empty() && !mSpeller->ContainsWord(repl_word))
	{
		if (CErrorHandler::PutCautionAlert(true, IDS_SPELL_ReplaceNotInDictionary) == CErrorHandler::Cancel)
			return;
	}

	// Substitute user replacement for suggested replacement
	((CSpellPlugin::SpError*) mSpeller->CurrentError())->replacement = repl_word;

	// Get speller to do it
	mSpeller->Replace();
	
	// Now do visual replacement
	ReplaceWord(mSpeller->CurrentError());

	// Now go to next error
	NextError();
}

void CSpellCheckDialog::OnSpellCheckSkip() 
{
	// Get speller to do it
	mSpeller->Skip();
	
	// Now go to next error
	NextError();
}

void CSpellCheckDialog::OnSpellCheckReplaceAll() 
{
	// Get current replacement word
	cdstring repl_word = CUnicodeUtils::GetWindowTextUTF8(&mSuggestion);
	
	// First check that replacement word exists
	if (!mSpeller->ErrorIsPunct() && !repl_word.empty() && !mSpeller->ContainsWord(repl_word))
	{
		if (CErrorHandler::PutCautionAlert(true, IDS_SPELL_ReplaceNotInDictionary) == CErrorHandler::Cancel)
			return;
	}

	// Substitute user replacement for suggested replacement
	((CSpellPlugin::SpError*) mSpeller->CurrentError())->replacement = repl_word;

	// Get speller to do it
	mSpeller->ReplaceAll();
	
	// Now do visual replacement
	ReplaceWord(mSpeller->CurrentError());

	// Now go to next error
	NextError();
}

void CSpellCheckDialog::OnSpellCheckSkipAll() 
{
	// Get speller to do it
	mSpeller->SkipAll();
	
	// Now go to next error
	NextError();
}

void CSpellCheckDialog::OnSpellCheckSuggestMore() 
{
	// Bump up suggestion counter and do suggestions again
	mSuggestionCtr++;
	DisplaySuggestions();
	
	// Check whether can do any more
	if (!mSpeller->MoreSuggestions(mSuggestionCtr))
		mSuggestMore.EnableWindow(false);
}

void CSpellCheckDialog::OnSpellCheckOptions() 
{
	// Create the dialog
	CSpellOptionsDialog::PoseDialog(mSpeller);

	// Reset dictionary name
	CString dname = cdustring(mSpeller->GetPreferences()->mDictionaryName.GetValue());
	mDictName.SetWindowText(dname);
}

void CSpellCheckDialog::OnSpellCheckSendNow() 
{
	EndDialog(IDC_SPELLCHECK_SENDNOW);
}

void CSpellCheckDialog::OnSelchangeSpellCheckList() 
{
	CString str;
	mList.GetText(mList.GetCurSel(), str);
	SetSuggestion(cdstring(str));
}

void CSpellCheckDialog::OnSelcancelSpellCheckList() 
{
	// TODO: Add your control notification handler code here
	
}

void CSpellCheckDialog::OnDblclkSpellCheckList() 
{
	// Do item select then replace
	OnSelchangeSpellCheckList();
	OnSpellCheckReplace();
}

// Display suggestions in list
void CSpellCheckDialog::DisplaySuggestions(void)
{
	// Switch display
	mList.DisplayPages(false);
	
	cdstrvect suggestions;
	mSpeller->GetSuggestions(mSpeller->CurrentError()->word, mSuggestionCtr, suggestions);
	mList.SetRedraw(false);
	mList.ResetContent();

	// Do for each word
	for(cdstrvect::const_iterator iter = suggestions.begin(); iter != suggestions.end(); iter++)
	{
		mList.AddString((*iter).win_str());
	}

	if (suggestions.size() != 0)
	{
		// Select first cell
		mList.SetCurSel(0);
		OnSelchangeSpellCheckList();
	}
	else
	{
		char empty = 0;
		SetSuggestion(&empty);
	}

	mList.SetRedraw(true);
	CUnicodeUtils::SetWindowTextUTF8(&mWordTop, cdstring((unsigned long) suggestions.size()));
}

// Display dictionary in list
void CSpellCheckDialog::DisplayDictionary(bool select, bool force_draw)
{
	// Do change to list
	mList.DisplayPages(true);
	bool matched = mList.DisplayDictionary(select ? mSpeller->CurrentError()->word : NULL);

	// Get page buffer and find closest location if switching from guesses
	if (!matched && select)
	{
		char empty = 0;
		SetSuggestion(&empty);
	}
	else
		OnSelchangeSpellCheckList();
}

// Set suggestion field
void CSpellCheckDialog::SetSuggestion(char* suggest)
{
	if (*suggest)
	{
		mSpeller->ProcessReplaceString(suggest);
		CUnicodeUtils::SetWindowTextUTF8(&mSuggestion, suggest);
		GotoDlgCtrl(&mReplace);
		SetDefID(IDC_SPELLCHECK_REPLACE);
	}
	else
	{
		CUnicodeUtils::SetWindowTextUTF8(&mSuggestion, suggest);
		GotoDlgCtrl(&mSkip);
		SetDefID(IDC_SPELLCHECK_SKIP);
	}
}