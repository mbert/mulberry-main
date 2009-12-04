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


// Source for CTextDisplay class

#include "CTextDisplay.h"

#include "CCharsetManager.h"
#include "CCommands.h"
#include "CFindReplaceWindow.h"
#include "CFocusBorder.h"
#include "CLog.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CScrollbarSet.h"
#include "CSpellPlugin.h"
#include "CTextEngine.h"
#include "CWindowStatesFwd.h"

#include "TPopupMenu.h"

#include "strfind.h"
#include "ustrfind.h"

#include "StValueChanger.h"

#include <JXDisplay.h>
#include <JXFontManager.h>
#include <JXTextMenu.h>
#include <jXGlobals.h>
#include <jXKeysym.h>

#include <algorithm>

// __________________________________________________________________________________________________
// C L A S S __ C T E X T D I S P L A Y
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextDisplay::CTextDisplay(const JCharacter *text, JXContainer* enclosure,
						 JXTextMenu *menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType)
	: CTextBase(text, mSbs = new CScrollbarSet(enclosure, hSizing, vSizing, x, y, w, h),
					enclosure, menu, hSizing, vSizing, 0, 0, w, h, editorType)
{
	// Move text into scroll enclosure
	SetEnclosure(mSbs->GetScrollEnclosure());
	FitToEnclosure();

	CTextDisplayX();
	
	// Set scroller border to zero if enclosure is a focus border
	CFocusBorder* focus = dynamic_cast<CFocusBorder*>(enclosure);
	if (focus)
		mSbs->SetBorderWidth(0);
}

CTextDisplay::CTextDisplay(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType)
	: CTextBase(mSbs = new CScrollbarSet(enclosure, hSizing, vSizing, x, y, w, h),
					enclosure, hSizing, vSizing, 0, 0, w, h, editorType)
{
	// Move text into scroll enclosure
	SetEnclosure(mSbs->GetScrollEnclosure());
	FitToEnclosure();

	CTextDisplayX();
	
	// Set scroller border to zero if enclosure is a focus border
	CFocusBorder* focus = dynamic_cast<CFocusBorder*>(enclosure);
	if (focus)
		mSbs->SetBorderWidth(0);
}

CTextDisplay::CTextDisplay(JXScrollbarSet* sbs, JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType)
	: CTextBase(mSbs = sbs, sbs ? sbs->GetScrollEnclosure() : enclosure,
					hSizing, vSizing, 0, 0, w, h, editorType)
{
	CTextDisplayX();
	
	// Set scroller border to zero if enclosure is a focus border
	CFocusBorder* focus = dynamic_cast<CFocusBorder*>(enclosure);
	if (focus && sbs)
		mSbs->SetBorderWidth(0);
}

void CTextDisplay::CTextDisplayX()
{
	mDeleted = NULL;
	mSpacesPerTab = 0;
	mHardWrap = false;
	mWrap = 76;
	mTabSelectAll = true;
	mWrapAllowed = false;
	mFindAllowed = false;
	
	// Init spell auto check - off
	mRTSpell.busy = false;
	mRTSpell.enabled = false;
	mRTSpell.pending = false;
	mSpellPopup = NULL;
}

// Default destructor
CTextDisplay::~CTextDisplay()
{
	if (mDeleted)
		*mDeleted = true;
	delete mSpellPopup;
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup D&D
void CTextDisplay::OnCreate()
{
	// Set default context edit menu
	// Derived classes that want their own context menu
	// should set it BEFORE calling this inherited method
	CTextBase::OnCreate(CMainMenu::eContextEdit);
}

// Handle keys our way
bool CTextDisplay::HandleChar(const int key, const JXKeyModifiers& modifiers)
{
	bool result = false;

	// Read-only - pass-up
	if (IsReadOnly() && (key < 256) && isalnum(key))
		CCommander::HandleChar(key, modifiers);

	// Look for macro trigger
	CKeyAction keyact(key, CKeyModifiers(modifiers));
	if (keyact == CPreferences::sPrefs->mTextMacros.GetValue().GetKeyAction())
	{
		// Do macro expansion
		if (ExpandMacro())
		{
			// Do spell check
			SpellTextChange();

			return true;
		}
	}
	
	bool selection_extend = false;
	switch(key)
	{
	case '\t':
		{
			// Ignore locks in modifiers
			JXKeyModifiers mod_nolocks(modifiers);
			mod_nolocks.SetState(kJXShiftLockKeyIndex, kFalse);
			mod_nolocks.SetState(kJXNumLockKeyIndex, kFalse);
			mod_nolocks.SetState(kJXScrollLockKeyIndex, kFalse);
			
			// Only do if non-zero space substitutions and edits allowed
			if (CPreferences::sPrefs->mTabSpace.GetValue() &&
				(mSpacesPerTab > 0) && mod_nolocks.AllOff() && (GetType() == kFullEditor))
			{
				// Determine column position of start of selection
				JIndex selStart;
				JIndex selEnd;
				GetSel(selStart, selEnd);
				JIndex tab_column = GetColumnForChar(selStart + 1) - 1;

				// Add in the spaces
				short add_space = mSpacesPerTab - (tab_column % mSpacesPerTab);
				cdstring spaces(' ', add_space);
				InsertUTF8(spaces);
		
				return true;
			}
		}
	
	// Shift page-up/down handling
	case XK_Page_Up:
	case XK_KP_Page_Up:
	case XK_Page_Down:
	case XK_KP_Page_Down:
	case XK_Home:
	case XK_KP_Home:
	case XK_End:
	case XK_KP_End:
		selection_extend = modifiers.shift();
		break;
	}

	// Pause spell check notifications then restart them
	{
		StValueChanger<bool> _save(mRTSpell.busy, true);
		
		// Get current selection if we need to extend it
		JIndex sel_start = 0;
		JIndex sel_end = 0;
		if (selection_extend)
			GetSelectionRange(sel_start, sel_end);
	
		// Do normal key press
		result = CTextBase::HandleChar(key, modifiers);
		
		// Extend selection if required
		if (selection_extend)
		{
			// get current selection
			JIndex new_sel_start = 0;
			JIndex new_sel_end = 0;
			GetSelectionRange(new_sel_start, new_sel_end);
			
			// Extend to encompass full range
			SetSelectionRange(std::min(new_sel_start, sel_start), std::max(new_sel_end, sel_end));
		}
	}
	SpellTextChange();
	
	return result;
}

void CTextDisplay::Activate()
{
	// Do inherited
	CTextBase::Activate();
}

void CTextDisplay::Deactivate()
{
	// Do inherited
	CTextBase::Deactivate();
}

void CTextDisplay::HandleFocusEvent()
{
	// Do inherited
	CTextBase::HandleFocusEvent();

	Broadcast_Message(eBroadcast_Activate, this);
	Broadcast_Message(eBroadcast_SelectionChanged, this);
	
	// If there is a focus border, tell it to focus
	const CFocusBorder* focus = dynamic_cast<const CFocusBorder*>(mSbs ? mSbs->GetEnclosure() : GetEnclosure());
	if (focus)
		const_cast<CFocusBorder*>(focus)->SetFocus(true);
}

void CTextDisplay::HandleUnfocusEvent()
{
	// Do inherited
	CTextBase::HandleUnfocusEvent();

	Broadcast_Message(eBroadcast_Deactivate, this);
	
	// If there is a focus border, tell it to unfocus
	const CFocusBorder* focus = dynamic_cast<const CFocusBorder*>(mSbs ? mSbs->GetEnclosure() : GetEnclosure());
	if (focus)
		const_cast<CFocusBorder*>(focus)->SetFocus(false);
}

void CTextDisplay::HandleWindowFocusEvent()
{
	// Do inherited
	CTextBase::HandleWindowFocusEvent();

	Broadcast_Message(eBroadcast_Activate, this);
	Broadcast_Message(eBroadcast_SelectionChanged, this);
	
	// If there is a focus border, tell it to focus
	const CFocusBorder* focus = dynamic_cast<const CFocusBorder*>(mSbs ? mSbs->GetEnclosure() : GetEnclosure());
	if (focus)
		const_cast<CFocusBorder*>(focus)->SetFocus(true);
}

void CTextDisplay::HandleWindowUnfocusEvent()
{
	// Do inherited
	CTextBase::HandleWindowUnfocusEvent();
}

bool CTextDisplay::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	bool result = true;
	bool deleted = false;
	StValueChanger<bool*> _deleted_ptr(mDeleted, &deleted);
	{
		// Pause spell check notifications then restart them
		StValueChanger<bool> _save(mRTSpell.busy, true);

		switch(cmd)
		{
		case CCommand::eEditFind:
			OnFindText();
			break;

		case CCommand::eEditFindNext:
			OnFindNextText(GetDisplay()->GetLatestKeyModifiers().shift());
			break;

		case CCommand::eEditFindSel:
			OnFindSelectionText(GetDisplay()->GetLatestKeyModifiers().shift());
			break;

		case CCommand::eEditReplace:
			OnReplaceText();
			break;

		case CCommand::eEditReplaceFindNext:
			OnReplaceFindText(GetDisplay()->GetLatestKeyModifiers().shift());
			break;

		case CCommand::eEditReplaceAll:
			OnReplaceAllText();
			break;

		case CCommand::eDraftWrap:
			OnWrapLines();
			break;

		case CCommand::eDraftUnwrap:
			OnUnwrapLines();
			break;

		case CCommand::eDraftQuote:
			OnQuoteLines();
			break;

		case CCommand::eDraftUnquote:
			OnUnquoteLines();
			break;

		case CCommand::eDraftRequote:
			OnRequoteLines();
			break;

		case CCommand::eDraftShiftLeft:
			OnShiftLeft();
			break;

		case CCommand::eDraftShiftRight:
			OnShiftRight();
			break;
		
		default:
			result = CTextBase::ObeyCommand(cmd, menu);
		}
	}
	
	// Now trigger spell check test only if not deleted
	if (!deleted)
		SpellTextChange();

	return result;
}

// Handle menus our way
void CTextDisplay::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	bool has_editable_selection = JNegate(IsReadOnly()) && HasSelection();

	switch(cmd)
	{
	case CCommand::eEditFind:
		cmdui->Enable(mFindAllowed);
		return;
	case CCommand::eEditFindNext:
		cmdui->Enable(mFindAllowed && !CFindReplaceWindow::sFindText.empty());
		return;
	case CCommand::eEditFindSel:
		cmdui->Enable(mFindAllowed && HasSelection());
		return;
	case CCommand::eEditReplace:
		cmdui->Enable(mFindAllowed && has_editable_selection);
		return;
	case CCommand::eEditReplaceFindNext:
		cmdui->Enable(mFindAllowed && has_editable_selection &&
					!CFindReplaceWindow::sFindText.empty());
		return;
	case CCommand::eEditReplaceAll:
		cmdui->Enable(mFindAllowed && JNegate(IsReadOnly()) && !CFindReplaceWindow::sFindText.empty());
		return;
	case CCommand::eDraftWrap:
		cmdui->Enable(mWrapAllowed && has_editable_selection);
		return;
	case CCommand::eDraftUnwrap:
		cmdui->Enable(mWrapAllowed && has_editable_selection);
		return;
	case CCommand::eDraftQuote:
		cmdui->Enable(mWrapAllowed && has_editable_selection);
		return;
	case CCommand::eDraftUnquote:
		cmdui->Enable(mWrapAllowed && has_editable_selection);
		return;
	case CCommand::eDraftRequote:
		cmdui->Enable(mWrapAllowed && has_editable_selection);
		return;
	case CCommand::eDraftShiftLeft:
		cmdui->Enable(mWrapAllowed && JNegate(IsReadOnly()));
		return;
	case CCommand::eDraftShiftRight:
		cmdui->Enable(mWrapAllowed && JNegate(IsReadOnly()));
		return;
	}

	CTextBase::UpdateCommand(cmd, cmdui);
}

void CTextDisplay::Receive(JBroadcaster* sender, const Message& message)
{
	// Look for cursor/text changes to trigger spell-as-you-type
	if ((sender == this) &&
		(message.Is(JTextEditor16::kTextChanged) ||
		 message.Is(JTextEditor16::kCaretLocationChanged)))
		SpellTextChange();

	// Must special case SetText command to do check of all text now in the control
	else if ((sender == this) &&
				message.Is(JTextEditor16::kTextSet) &&
				mRTSpell.enabled && !mRTSpell.busy && (GetTextLength() > 0))
		SpellInsertText(0, GetTextLength());

	else if ((sender == mSpellPopup) && message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		SpellContext(is->GetIndex());
		return;
	}

	// Pass up
	CTextBase::Receive(sender, message);
}

void CTextDisplay::ContextEvent(const JPoint& pt, const JXButtonStates& buttonStates, const JXKeyModifiers& modifiers)
{
	bool result = false;

	// Check whether word under cursor has spell error hilite mode

	// Find character offset at event point
	CaretLocation caret = CalcCaretLocation(pt);
	JIndex over_char = caret.charIndex - 1;
	
	// Check the style of text at the event point
	if (IsMisspelled(over_char, over_char + 1))
	{
		// Get the error word
		cdustring text;
		GetMisspelled(over_char, text, mRTSpell.contextWordPos);
		size_t tlen = text.length();
		mSpellContextWord = text.ToUTF8();
		
		// Set selection to error text
		SetSelectionRange(mRTSpell.contextWordPos, mRTSpell.contextWordPos + tlen);
		
		// Get the spell checker plugin
		CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
		CSpellPlugin::StLoadSpellPlugin _load(speller);

		// Get suggestions for error word
		mSpellSuggestions.clear();
		cdstring utf8 = text.ToUTF8();
		speller->GetQuickSuggestions(utf8, mSpellSuggestions);

		if (!mSpellPopup)
		{
			mSpellPopup = new HPopupMenu("", this, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 10, 10);
			mSpellPopup->SetToHiddenPopupMenu(kTrue);
			mSpellPopup->SetUpdateAction(JXMenu::kDisableNone);
			mSpellPopup->Hide();
			ListenTo(mSpellPopup);
		}
		else
			mSpellPopup->RemoveAllItems();

		// Add up to 15 suggestions
		JIndex menu_pos = 1;
		for(cdstrvect::const_iterator iter = mSpellSuggestions.begin(); (menu_pos <= 15) && (iter != mSpellSuggestions.end()); iter++, menu_pos++)
			mSpellPopup->AppendItem(*iter, kTrue);
		if (menu_pos > 1)
			mSpellPopup->ShowSeparatorAfter(menu_pos - 1);
		mSpellPopup->AppendItem("Add to Dictionary");

		// Do popup menu of suggestions
		mSpellPopup->PopUp(this, pt, buttonStates, modifiers);

		// Stop further context menu processing
		return;
	}
	
	CContextMenu::ContextEvent(pt, buttonStates, modifiers);
}

#pragma mark ____________________________Wrapping

// Set wrap width
void CTextDisplay::SetWrap(short wrap)
{
	mWrap = (wrap == 0) ? 1000 : wrap;
	ResetHardWrap();
}

// Set wrapping mode
void CTextDisplay::SetHardWrap(bool hard_wrap)
{
	if (hard_wrap != mHardWrap)
	{
		mHardWrap = hard_wrap;
		if (mHardWrap)
			ResetHardWrap();
		else
		{
			// Fit to full width of enclosure and make it elastic
			FitToEnclosure(kTrue, kFalse);
			SetSizing(JXWidget::kHElastic, JXWidget::kVElastic);
		}
	}
}

// Set wrapping mode
void CTextDisplay::ResetHardWrap()
{
	if (mHardWrap)
	{
		// Turn of elastic resizing of right margin
		SetSizing(JXWidget::kFixedLeft, JXWidget::kVElastic);

		// Force image size to width of font * wrap length
		JString name;
		JSize size;
		JFontStyle style;
		AlignmentType align;
		GetDefaultFont(&name, &size, &style, &align);

		JFontID fontID = GetFontManager()->GetFontID(name, size, style);

		JSize width = (mWrap * GetFontManager()->GetStringWidth(fontID, size, style, "1234567890im", 12)) / 12 + TEGetLeftMarginWidth() + 2;

		JSize old_width = GetBoundsWidth();

		AdjustSize(width - old_width, 0);
	}
}

void CTextDisplay::SetSpacesPerTab(short numSpacesPerTab)
{
	JString name;
	JSize size;
	JFontStyle style;
	AlignmentType align;
	GetDefaultFont(&name, &size, &style, &align);

	JFontID fontID = GetFontManager()->GetFontID(name, size, style);

	JSize width = GetFontManager()->GetStringWidth(fontID, size, style, "1234567890im", 12) / 12;

	SetDefaultTabWidth(numSpacesPerTab * width);
	mSpacesPerTab = numSpacesPerTab;

	// Set for tab input
	WantInput(kTrue, mSpacesPerTab ? kTrue : kFalse, kFalse, kFalse);
}

// Get each line as utf8 array
void CTextDisplay::GetTextList(cdstrvect& all) const
{
	// Get text string
	cdstring temp = GetText();
	
	// Tokenise on CRLF
	const char* p = ::strtok(temp.c_str_mod(), "\r\n");
	while(p != NULL)
	{
		if (*p)
			all.push_back(p);
		p = ::strtok(NULL, "\r\n");
	}
}

// Set each line from utf8 array
void CTextDisplay::SetTextList(const cdstrvect& all)
{
	// Create single string of all lines
	cdstring temp;
	for(cdstrvect::const_iterator iter = all.begin(); iter != all.end(); iter++)
	{
		if (iter != all.begin())
			temp += os_endl;
		temp += *iter;
	}
	
	SetText(temp);
}

// Get horiz scrollbar pos
void CTextDisplay::GetScrollPos(long& h, long& v) const
{
	const JRect ap = GetAperture();

	// Get view position of top item
	h = ap.left;
	v = ap.top;
}

// Get horiz scrollbar pos
void CTextDisplay::SetScrollPos(long h, long v)
{
	// Scroll to move top item to new location
	ScrollTo(h, v);
}

#pragma mark ____________________________Text Processing

void CTextDisplay::OnWrapLines()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do wrap
	const char* wrapped = CTextEngine::WrapLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(), CPreferences::sPrefs->mFormatFlowed.GetValue());
	
	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

void CTextDisplay::OnUnwrapLines()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do wrap
	const char* unwrapped = CTextEngine::UnwrapLines(selected.c_str(), selected.length());
	
	// Replace selection
	EndTextProcessing(unwrapped, sel_start, sel_end);
}

void CTextDisplay::OnQuoteLines()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do wrap with quote
	const char* wrapped = CTextEngine::QuoteLines(selected.c_str(),
								selected.length(), CRFC822::GetWrapLength(),
								mQuotation, &CPreferences::sPrefs->mRecognizeQuotes.GetValue());
	
	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

void CTextDisplay::OnUnquoteLines()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do unquote
	const char* unquoted = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), mQuotation.c_str());
	
	// Replace selection
	EndTextProcessing(unquoted, sel_start, sel_end);
}

void CTextDisplay::OnRequoteLines()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Now do requote
	const char* txt = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), mQuotation.c_str());
	selected.steal(const_cast<char*>(txt));
	txt = CTextEngine::UnwrapLines(selected.c_str(), selected.length());
	selected.steal(const_cast<char*>(txt));
	txt = CTextEngine::QuoteLines(selected.c_str(), selected.length(),
									CRFC822::GetWrapLength(), mQuotation,
									&CPreferences::sPrefs->mRecognizeQuotes.GetValue());
	
	// Replace selection
	EndTextProcessing(txt, sel_start, sel_end);
}

void CTextDisplay::OnShiftLeft()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Pretend that shift chars are quotes!
	cdstring shift;
	if (CPreferences::sPrefs->mTabSpace.GetValue())
	{
		// Fill with required number of spaces
		for(short i = 0; i < mSpacesPerTab; i++)
			shift += ' ';
	}
	else
		shift = '\t';

	// Now do unquote
	const char* unquoted = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), shift.c_str());
	
	// Replace selection
	EndTextProcessing(unquoted, sel_start, sel_end);
}

void CTextDisplay::OnShiftRight()
{
	// Prepare for line based processing
	JIndex sel_start;
	JIndex sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);
	
	// Pretend that shift chars are quotes!
	cdstring shift;
	if (CPreferences::sPrefs->mTabSpace.GetValue())
	{
		// Fill with required number of spaces
		for(short i = 0; i < mSpacesPerTab; i++)
			shift += ' ';
	}
	else
		shift = '\t';

	// Now do wrap with quote
	const char* wrapped = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(), shift);
	
	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

bool CTextDisplay::ExpandMacro()
{
	// Look for selection
	JIndex selStart;
	JIndex selEnd;
	GetSelectionRange(selStart, selEnd);
	JIndex origStart = selStart;
	cdstring selected;

	if (selStart == selEnd)
	{
		// Scan backwards looking for word break
		while(selStart != 0)
		{
			selStart--;

			// Get the currently selected text
			SetSelectionRange(selStart, selEnd);
			GetSelectedText(selected);

			// Test for word break
			if (isspace(selected[0UL]) || ispunct(selected[0UL]))
			{
				// Bump up to start of word itself
				selStart++;
				SetSelectionRange(selStart, selEnd);
				GetSelectedText(selected);
				break;
			}
		}
	}
	else
		// Get the currently selected text
		GetSelectedText(selected);

	// Get the selected text
	selected.trimspace();
	if (selected.empty())
		return false;
	
	// Lookup the macro
	const cdstring& value = CPreferences::sPrefs->mTextMacros.GetValue().Lookup(selected);
	if (value.empty())
	{
		// Reset to old selection
		SetSelectionRange(origStart, selEnd);
		return false;
	}
	else
	{
		// Check whether spelling is active
		if (mRTSpell.enabled && !mRTSpell.busy)
		{
			// Always mark as not mispelled
			MarkMisspelled(selStart, selEnd, false);
		}

		// Make sure smart insert does not apply
		InsertUTF8(value);
		
		return true;
	}
}

void CTextDisplay::PrepareTextProcessing(JIndex& sel_start, JIndex& sel_end, cdstring& selected)
{
	// Make sure complete lines are selected
	SelectFullLines();
	GetSel(sel_start, sel_end);

	// Get the selected text
	GetSelectedText(selected);
}

void CTextDisplay::EndTextProcessing(const char* insert_text, JIndex& sel_start, JIndex& sel_end)
{
	// Replace selection
	if (insert_text)
	{
		size_t insert_length = ::strlen(insert_text);
		InsertUTF8(insert_text);

		sel_end = sel_start + insert_length;
	}
	SetSel(sel_start, sel_end);
	delete insert_text;
}

void CTextDisplay::SelectFullLines()
{
	// Get current selection and text
	JIndex sel_start;
	JIndex sel_end;
	GetSel(sel_start, sel_end);
	
	// Get offsets to lines
	JIndex line_start = GetLineForChar(sel_start + 1);
	JIndex line_end = GetLineForChar(sel_end);
	
	// Get range for new selection;
	JIndex new_sel_start = GetLineStart(line_start) - 1;
	JIndex new_sel_end = GetLineEnd(line_end);
	
	// Do new selection
	SetSel(new_sel_start, new_sel_end);
}

#pragma mark ____________________________Find & Replace

void CTextDisplay::OnFindText()
{
	// Bring find window to front or create it
	// Find window will 'drive' find operation
	CFindReplaceWindow::CreateFindReplaceWindow(this);
}

void CTextDisplay::OnFindNextText(bool backwards)
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoFindNextText(backwards);
}

void CTextDisplay::OnFindSelectionText(bool backwards)
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoFindSelectionText(backwards);
}

void CTextDisplay::OnReplaceText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceText();
}

void CTextDisplay::OnReplaceFindText(bool backwards)
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceFindText(backwards);
}

void CTextDisplay::OnReplaceAllText()
{
	// Make sure find & replace details are up to date
	CFindReplaceWindow::UpdateFindReplace();

	DoReplaceAllText();
}

bool CTextDisplay::DoFindNextText(bool backwards, bool silent)
{
	bool result = false;

	// Get current selection and text
	JIndex sel_start;
	JIndex sel_end;
	GetSel(sel_start, sel_end);
	
	// Get current text and lock while dereferences
	const char* text = GetText();
	
	// Find starts after current selection (forward) or before it (backwards)
	if (sel_start != sel_end)
	{
		if (backwards)
			sel_end = sel_start;
		else
			sel_start = sel_end;
	}

	// Do find
	EFindMode mode = (EFindMode) (CFindReplaceWindow::sFindMode & ~eBackwards);
	if (backwards)
		mode = (EFindMode) (mode | eBackwards);

	const char* found = ::strfind(text, GetTextLength(), sel_start,
									CFindReplaceWindow::sFindText.c_str(), mode);
	
	// Was it found
	if (found)
	{
		// Determine new selection
		sel_start = found - text;
		sel_end = sel_start + CFindReplaceWindow::sFindText.length();
		SetSel(sel_start, sel_end);
		TEScrollToSelection(kTrue);
		result = true;
	}
	else if (!silent)
		::MessageBeep(1);
	
	return result;
}

void CTextDisplay::DoFindSelectionText(bool backwards)
{
	// Get selected text and give it to find & replace window
	cdstring selected;
	GetSelectedText(selected);
	CFindReplaceWindow::SetFindText(selected);

	// Do normal find operation
	DoFindNextText(backwards);
}

void CTextDisplay::DoReplaceText()
{
	// Make sure smart insert does not apply
	InsertUTF8(CFindReplaceWindow::sReplaceText.c_str());
}

void CTextDisplay::DoReplaceFindText(bool backwards)
{
	DoReplaceText();
	DoFindNextText(backwards);
}

void CTextDisplay::DoReplaceAllText()
{
	// Reset selection to first character to allow replace within the selection itself
	JIndex sel_start;
	JIndex sel_end;
	GetSel(sel_start, sel_end);
	SetSel(sel_start, sel_start);

	// Loop until no more found (silently) and do replace
	bool found_once = false;
	while(DoFindNextText(CFindReplaceWindow::sFindMode & eBackwards, true))
	{
		found_once = true;
		DoReplaceText();
	}
	
	// Reset selection if no change
	if (!found_once)
	{
		SetSel(sel_start, sel_end);
		::MessageBeep(1);
	}
}

#pragma mark ____________________________Spelling commands

void CTextDisplay::SpellAutoCheck(bool auto_check)
{
	if (auto_check && CPluginManager::sPluginManager.HasSpelling())
	{
		// Spell check load may fail - e.g. missing dictionary etc
		try
		{
			// Make sure spell check plugin is loaded
			CSpellPlugin* plugin = CPluginManager::sPluginManager.GetSpelling();
			plugin->AutoLoad();
			
			mRTSpell.state = RTSpell::eNotInWord;
			mRTSpell.cursorPos = 0;
			mRTSpell.charCount = 0;
			mRTSpell.wordStartPos = 0;
			mRTSpell.contextWordPos = 0;
			mRTSpell.busy = false;
			mRTSpell.enabled = true;
			mRTSpell.pending = false;
			
			// Turn on change notifications
			ShouldBroadcastCaretLocationChanged(kTrue);
			ListenTo(this);
		}
		catch(...)
		{
			CLOG_LOGCATCH(...);
		}
	}
	else
	{
		mRTSpell.busy = false;
		mRTSpell.enabled = false;
		mRTSpell.pending = false;
	}
}

void CTextDisplay::SpellContext(JIndex item)
{
	// Get the spell checker plugin
	CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
	CSpellPlugin::StLoadSpellPlugin _load(speller);

	// Look for Add item
	if (item == mSpellPopup->GetItemCount())
	{
		// Add current word to dictionary
		speller->AddWord(mSpellContextWord);
		
		// Recheck this word everywhere else so that corrections are made
		RecheckAllWords(cdustring(mSpellContextWord));
	}

	// Replace error word with chosen one provided text is editable
	else if ((item != 0) && !IsReadOnly())
	{
		// Get the error word
		size_t tlen = mSpellContextWord.length();
		
		// Do replacement
		cdstring replace(mSpellSuggestions.at(item - 1));
		size_t rlen = replace.length();
		InsertUTF8(replace.c_str());

		// Recheck it
		JSize length_checked = 0;
		JIndex sel_start = mRTSpell.contextWordPos;
		while(true)
		{
			// Get the new word
			cdustring text;
			JSize wlen = GetWordAt(sel_start, text, &sel_start, true);
			CheckWord(text, sel_start);
			
			// Make sure we checked all of it
			if (wlen + length_checked < rlen)
			{
				// Update word start to next word
				sel_start += wlen + 1;
				length_checked += wlen + 1;
				
				// Make sure gap between words is cleared
				MarkMisspelled(sel_start - 1, sel_start, false);
			}
			else
				break;
		}
	}
	
	mSpellSuggestions.clear();
}

bool CTextDisplay::IsMisspelled(JIndex sel_start, JIndex sel_end) const
{
	return (sel_start < GetTextLength()) && (GetFontStyle(sel_start + 1).underlineType == JFontStyle::redwavy_Underline);
}

// Definitions from ssce.h etc

#define FANCY_APOSTROPHE 0x2019

/* Character-class masks: */
#define A_ 1
#define D_ 1
#define P_ 0
#define S_ 0
#define L_ 0
#define U_ 0
#define V_ 0

/** WS_Latin1_CharClass:
 * Used to classify Latin1 characters.
 */
const unsigned char WS_Latin1_CharClass[256] = {
	/* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, S_, S_, 0, 0, S_, 0, 0,
	/* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 20 */ S_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_,
	/* 30 */ D_, D_, D_, D_, D_, D_, D_, D_, D_, D_, P_, P_, P_, P_, P_, P_,
	/* 40 */ P_, A_|U_|V_, A_|U_, A_|U_, A_|U_, A_|U_|V_, A_|U_, A_|U_,
	/* 48 */ A_|U_, A_|U_|V_, A_|U_, A_|U_, A_|U_, A_|U_, A_|U_, A_|U_|V_,
	/* 50 */ A_|U_, A_|U_, A_|U_, A_|U_, A_|U_, A_|U_|V_, A_|U_, A_|U_,
	/* 58 */ A_|U_, A_|U_, A_|U_, P_, P_, P_, P_, P_,
	/* 60 */ P_, A_|L_|V_, A_|L_, A_|L_, A_|L_, A_|L_|V_, A_|L_, A_|L_,
	/* 68 */ A_|L_, A_|L_|V_, A_|L_, A_|L_, A_|L_, A_|L_, A_|L_, A_|L_|V_,
	/* 70 */ A_|L_, A_|L_, A_|L_, A_|L_, A_|L_, A_|L_|V_, A_|L_, A_|L_,
	/* 78 */ A_|L_, A_|L_, A_|L_, P_, P_, P_, P_, P_,
	/* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 90 */ 0, 0, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, 0, 0, 0,
	/* A0 */ 0, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_,
	/* B0 */ P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_,
	/* C0 */ A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_,
	/* C8 */ A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_,
	/* D0 */ A_|U_, A_|U_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, P_,
	/* D8 */ A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_|V_, A_|U_, A_|U_, A_|L_,
	/* E0 */ A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_,
	/* E8 */ A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, 
	/* F0 */ A_|L_|V_, A_|L_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, P_,
	/* F8 */ A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_|V_, A_|L_, A_|L_, A_|L_
};


// Return TRUE if a character could be part of a word:
#define IS_WORD_CHAR(c) ((c < 0x0100) && (WS_Latin1_CharClass[(unsigned char) c] == 1))

JSize CTextDisplay::GetWordAt(JIndex pos, cdustring& word, JIndex* startPos, bool word_start) const
{
	const unsigned long maxWordSize = 256;
	
	// Determine sensible buffer range around start pos
	JIndex rstart = (pos > maxWordSize) ? pos - maxWordSize : 0UL;
	JIndex rend = pos + maxWordSize;
	rend = std::min(rend, GetTextLength());

	// Get the text in that range
	cdustring buf;
	GetSpellTextRange(rstart, rend, buf);
	const unichar_t* _buf = buf.c_str();
	
	// Locate beginning of word by stepping back
	pos = std::min(pos, rstart + buf.length());
	JIndex i = std::min(pos, maxWordSize);
	while((i > 0) &&
			(IS_WORD_CHAR(_buf[i - 1]) ||
			 (_buf[i - 1] == '.') ||
			 (_buf[i - 1] == '\'') ||
			 (_buf[i - 1] == FANCY_APOSTROPHE)))
		i--;
		
	// May not be a word
	if (i < 0)
	{
		*startPos = ULONG_MAX;
		return 0;
	}
	
	// Advance past any periods and apostrophes at the beginning of the word
	while((_buf[i] != 0) && !IS_WORD_CHAR(_buf[i]))
		i++;
	*startPos = rstart + i;

	// Collect the word
	bool embeddedPeriods = false;
	while(IS_WORD_CHAR(_buf[i]) ||
			(_buf[i] == '.') || 
			(_buf[i] == '\'') ||
			(_buf[i] == FANCY_APOSTROPHE))
	{
		if ((_buf[i] == '.') && (i > 0) &&
			IS_WORD_CHAR(_buf[i - 1]) && IS_WORD_CHAR(_buf[i + 1]))
			embeddedPeriods = true;
		i++;
	}
	long n = rstart + i - *startPos;

	// Copy the word
	if (n > 0)
		word.assign(buf.c_str() + *startPos - rstart, n);
	
	// Accept a terminating period only if the word is an initialism
	// (contains embedded periods, no more than 2 consecutive alpha-numerics)
	if ((n > 0) && (word[(unsigned long)n - 1] == '.'))
	{
		bool initialism = false;
		if (embeddedPeriods)
		{
			long nAlphaNumerics = 0;

			initialism = true;
			for (const unichar_t* p = word.c_str(); *p != 0; p++)
			{
				if (IS_WORD_CHAR(*p))
				{
					if (++nAlphaNumerics > 2)
					{
						initialism = false;
						break;
					}
				}
				else
					nAlphaNumerics = 0;
			}
		}

		if (!initialism)
		{
			// Exclude all trailing periods
			while ((n > 0) && (word[(unsigned long)n - 1] == '.'))
				word[(unsigned long)--n] = 0;
		}
	}

	return n;
}

unichar_t CTextDisplay::GetTextChar(JIndex pos) const
{
	cdustring txt;
	long len = GetSpellTextRange(pos, pos + 1, txt);
	return (len > 0) ? txt[0UL] : 0;
}

JSize CTextDisplay::GetSpellTextRange(JIndex start, JIndex end, cdustring& text) const
{
	JIndex rstart = start;
	if (rstart < 0)
		rstart = 0;
	JIndex rend = end;
	if (rend > GetTextLength())
		rend = GetTextLength();
	if (rend < rstart)
		rend = rstart;
	
	text.assign(GetText16().GetCString() + rstart, rend - rstart);
	
	return text.length();
}

bool CTextDisplay::GetMisspelled(JIndex pos, cdustring& word, JIndex& word_start) const
{
	if (!IsMisspelled(pos, pos + 1))
		return false;

	// Locate the extent of the marked text
	for (word_start = pos; (word_start > 0) && IsMisspelled(word_start - 1, word_start) && !isspace(GetTextChar(word_start - 1)); word_start--)
	{
	}

	JIndex word_end;
	for (word_end = word_start; IsMisspelled(word_end, word_end + 1) && !isspace(GetTextChar(word_end)); ++word_end)
	{
		// Do nothing.
	}

	if (word_start < word_end)
	{
		GetSpellTextRange(word_start, word_end, word);
		return true;
	}
	else
		return false;
}

void CTextDisplay::MarkMisspelled(JIndex sel_start, JIndex sel_end, bool misspelled)
{
	// Determine whether current item has a plain underline
	JFontStyle style = GetFontStyle(sel_start + 1);
	bool reset_underline = (style.underlineCount > 0) && (style.underlineType == JFontStyle::plain_Underline);

	// Do red-wave hilite of text
	SetRangeFontUnderlineType(sel_start + 1, sel_end, misspelled ? JFontStyle::redwavy_Underline : JFontStyle::plain_Underline);
	if (!misspelled && reset_underline)
		SetRangeFontUnderline(sel_start + 1, sel_end, 1);
	if (misspelled)
		SetRangeFontBackColor(sel_start + 1, sel_end, CPluginManager::sPluginManager.GetSpelling()->SpellBackgroundColour());
	else
		SetRangeFontBackColor(sel_start + 1, sel_end, 0);
}

bool CTextDisplay::CheckWord(const cdustring& word, JIndex start)
{
	bool result = false;

	// Ignore if no spell checker
	if (!CPluginManager::sPluginManager.HasSpelling())
		return true;

	// Canonicalise word
	cdustring temp(word);
	size_t wlen = temp.length();
	if ((wlen > 0) && (temp[wlen - 1] == '\''))
	{
		if ((wlen > 1) && (temp[wlen - 2] != 's') && (temp[wlen - 2] != 'S'))
		{
			temp[wlen - 1] = '\0';
			wlen--;
		}
	}
	
	// Check word
	cdstring utf8 = temp.ToUTF8();
	const CSpellPlugin::SpError* sperr = CPluginManager::sPluginManager.GetSpelling()->CheckWord(utf8);
	
	// Clear mark if no error
	if (sperr == NULL)
		MarkMisspelled(start, start + wlen, false);
	
	// Check for error with auto-correct
	else if (sperr->do_replace && sperr->replacement && *sperr->replacement)
	{
		// Correct the word
		StPreserveSelection _preserve(this);
		SetSelectionRange(start, start + wlen);

		cdustring utf16_replace(sperr->replacement);
		size_t new_wlen = utf16_replace.length();
		InsertText(utf16_replace);
		
		// Correct selection for change in length
		_preserve.Offset(start + wlen, new_wlen - wlen);
		result = true;
	}
	
	// Mark the error
	else
		MarkMisspelled(start, start + wlen, true);
	
	return result;
}

void CTextDisplay::RecheckAllWords(const cdustring& word)
{
	StStopRedraw noDraw(this);
	CTextDisplay::StPreserveSelection _selection(this);

	const unichar_t* ustart = GetText16().GetCString();
	size_t ulen = GetTextLength();
	const unichar_t* p = ustart;
	unsigned long pos = 0;
	while((p = ::ustrfind(ustart, ulen, pos, word.c_str(), eEntireWord)) != NULL)
	{
		// Update pos
		pos = p - ustart;

		// Recheck the word found
		cdustring old_word;
		JIndex startPos = 0;
		GetWordAt(pos, old_word, &startPos, true);
		CheckWord(old_word, startPos);
		
		// Resume searching after this word
		pos++;
	}
}

void CTextDisplay::SpellInsertText(JIndex start, JSize numchars)
{
	// Ignore if no spell checker
	if (!CPluginManager::sPluginManager.HasSpelling())
		return;

	// Save current selection pos
	JIndex sel_start;
	JIndex sel_end;
	GetSelectionRange(sel_start, sel_end);

	// Determine reasonable buffer around cursor
	JIndex cursor_pos = start + numchars;
	JIndex text_start = 0;
	if (cursor_pos >= (numchars + 256))
		text_start = cursor_pos - (numchars + 256);
	else
		text_start = 0UL;
	JSize text_len = cursor_pos - text_start;
	text_len += std::max(text_len / 5UL, 256UL);
	text_len = std::min(text_len, GetTextLength() - text_start);

	// Copy text into buffer
	cdustring text;
	GetSpellTextRange(text_start, text_start + text_len, text);

	// Locate the beginning of the word around the first inserted character.
	unsigned long i = (cursor_pos - text_start) - numchars;
	while ((i > 0) && (IS_WORD_CHAR(text[i - 1]) || (text[i - 1] == '.') || (text[i - 1] == '\'') || (text[i - 1] == FANCY_APOSTROPHE)))
	{
		i--;
	}

	// Locate the end of the word around the last inserted character and terminate buffer there.
	unsigned long j = cursor_pos - text_start;
	while ((j < text_len) && IS_WORD_CHAR(text[j]) || (text[j] == '.') || (text[j] == '\'') || (text[j] == FANCY_APOSTROPHE))
	{
		j++;
	}
	text[j] = 0;

	JIndex text_cursor = i;
	cdustring utf16_check(text, text_cursor);
	cdstring utf8_check = utf16_check.ToUTF8();
	CPluginManager::sPluginManager.GetSpelling()->CheckText(utf8_check);

	if (CPluginManager::sPluginManager.GetSpelling()->HasErrors())
	{
		for(const CSpellPlugin::SpError* sperr = CPluginManager::sPluginManager.GetSpelling()->NextError(utf8_check); 
					sperr != NULL; sperr = CPluginManager::sPluginManager.GetSpelling()->NextError(utf8_check))
		{
			// Do auto replace
			if (sperr->do_replace && sperr->replacement && *sperr->replacement)
			{
				// Convert utf8 sperr offsets to utf16
				long usel_start = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_start);
				long usel_end = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_end);

				// Replace misspelled word
				StPreserveSelection _preserve(this);
				SetSelectionRange(text_start + text_cursor + usel_start, text_start + text_cursor + usel_end);
				cdustring utf16_replace(sperr->replacement);
				InsertText(utf16_replace);

				// Replaced current error
				CPluginManager::sPluginManager.GetSpelling()->Replace();
				
				// Need to adjust text being tested for the replacement
				cdstring temp(utf8_check, 0, sperr->sel_start);
				temp += sperr->replacement;
				temp.append(utf8_check, sperr->sel_end, cdstring::npos);
				utf8_check.steal(temp.grab_c_str());
			}
			else if (CPluginManager::sPluginManager.GetSpelling()->ErrorIsPunct())
			{
				// Skip current error
				CPluginManager::sPluginManager.GetSpelling()->Skip();
			}
			else
			{
				// Convert utf8 sperr offsets to utf16
				long usel_start = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_start);
				long usel_end = UTF8OffsetToUTF16Offset(utf8_check, sperr->sel_end);

				// Mark as misspelled
				MarkMisspelled(text_start + text_cursor + usel_start, text_start + text_cursor + usel_end, true);

				// Skip current error
				CPluginManager::sPluginManager.GetSpelling()->Skip();
			}
		}
	}
}

void CTextDisplay::SpellTextChange()
{
	// Check whether processing can be done
	if (TEIsDragging() || !mRTSpell.enabled || mRTSpell.busy || (GetTextLength() == 0))
		return;

	StValueChanger<bool> _change(mRTSpell.busy, true);
	mRTSpell.pending = false;

	JSize newCharCount = GetTextLength();
	JIndex newWordStartPos = mRTSpell.wordStartPos;

	// Determine the new cursor position
	JIndex selStart;
	JIndex selEnd;
	GetSelectionRange(selStart, selEnd);
	JIndex newCursorPos = selEnd;

	// Determine the dimensions of the current word
	cdustring word;
	JSize wordLen = 0;
	if (mRTSpell.wordStartPos != ULONG_MAX)
	{
		// Adjust cached start pos if change occurs in front of cacched word pos
		if (newCursorPos < mRTSpell.wordStartPos)
		{
			if (newCharCount > mRTSpell.charCount)
				mRTSpell.wordStartPos += newCharCount - mRTSpell.charCount;
			else if (newCharCount < mRTSpell.charCount)
			{
				JIndex adjust = mRTSpell.charCount - newCharCount;
				if (adjust > mRTSpell.wordStartPos)
					mRTSpell.wordStartPos = ULONG_MAX;
				else
					mRTSpell.wordStartPos -= adjust;
			}
		}
	
		// Now get cached word at new position
		if (mRTSpell.wordStartPos != ULONG_MAX)
		{
			wordLen = GetWordAt(mRTSpell.wordStartPos, word, &newWordStartPos, true);
			mRTSpell.wordStartPos = newWordStartPos;
		}
	}

	// Determine what events occurred, and process them according to the current state
	if (newCharCount - mRTSpell.charCount == 1)
	{
		// Single character inserted.
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Do nothing.
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			// A marked word was edited, which might change it from misspelled to OK
			if (wordLen && (mRTSpell.wordStartPos != ULONG_MAX))
				MarkMisspelled(mRTSpell.wordStartPos, mRTSpell.wordStartPos + wordLen, false);
			break;
		}
	}
	else if (newCharCount < mRTSpell.charCount)
	{
		// One or more characters deleted
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Do nothing
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			// A marked word was edited, which might change it from misspelled to OK
			if (mRTSpell.wordStartPos != ULONG_MAX)
				MarkMisspelled(mRTSpell.wordStartPos, mRTSpell.wordStartPos + wordLen, false);
			break;
		}
	}
	else if (newCharCount - mRTSpell.charCount > 1)
	{
		// Multiple characters inserted
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			if (newCursorPos >= newCharCount - mRTSpell.charCount)
				SpellInsertText(newCursorPos - (newCharCount - mRTSpell.charCount), newCharCount - mRTSpell.charCount);
			break;
		case RTSpell::eInUnmarkedWord:
			// Check any words inserted after the current word. We don't
			// want to check the current word here because it may not be complete
			if (newCursorPos > (mRTSpell.wordStartPos + wordLen))
				SpellInsertText(mRTSpell.wordStartPos + wordLen,
									newCursorPos - (mRTSpell.wordStartPos + wordLen));
			break;
		case RTSpell::eInMarkedWord:
			// Check all inserted text including the current word. The
			// insertion may have changed the current word from misspelled to OK
			SpellInsertText(mRTSpell.wordStartPos, newCursorPos - mRTSpell.wordStartPos);
		}
	}

	if ((mRTSpell.wordStartPos != ULONG_MAX) && (wordLen > 0) &&
	  	((newCursorPos < mRTSpell.wordStartPos) || (newCursorPos > mRTSpell.wordStartPos + wordLen)))
	{
		// Cursor moved outside current word
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Can't happen
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			{
				bool wordChanged = CheckWord(word, mRTSpell.wordStartPos);

				if (wordChanged)
				{
					long new_wordLen = GetWordAt(mRTSpell.wordStartPos, word, &mRTSpell.wordStartPos, true);
					newCursorPos += new_wordLen - wordLen;
					mRTSpell.cursorPos += new_wordLen - wordLen;
				}
			}
			break;
		}

		// Must reset style in some cases
		if (newCursorPos == mRTSpell.cursorPos + 1)
		{
			MarkMisspelled(newCursorPos - 1, newCursorPos, false);
		}
	}

	// Update the current state.
	wordLen = GetWordAt(newCursorPos, word, &mRTSpell.wordStartPos, false);
	if (wordLen >= 0)
	{
		mRTSpell.state = RTSpell::eInUnmarkedWord;

		// If the selection is being extended by shift/arrow or dragging
		// the mouse, calling isMarkedMisspelled will interfere with the
		// extension (because isMarkedMisspelled must change the selection to
		// sample the character format). If the selection is being extended,
		// we won't	determine if the current word is marked, to avoid
		// interfering with	the selection; we'll assume the word is unmarked.
		// This means that a correction to the marked word (by deleting the
		// selected characters, for example) might be missed so the word
		// would remain marked, but that's not too serious since the user
		// can force a re-check by passing the cursor through the word.
		if (selStart == selEnd)
		{
			// Determine if the current word is marked.
			if ((newCursorPos < GetTextLength()) && IsMisspelled(newCursorPos, newCursorPos))
				mRTSpell.state = RTSpell::eInMarkedWord;
		}
	}
	else
		mRTSpell.state = RTSpell::eNotInWord;
	mRTSpell.cursorPos = newCursorPos;
	mRTSpell.charCount = newCharCount;
}

bool CTextDisplay::PauseSpelling(bool pause)
{
	if (pause)
	{
		bool was_enabled = mRTSpell.enabled;
		mRTSpell.enabled = false;
		return was_enabled;
	}
	else if (!mRTSpell.enabled)
	{
		mRTSpell.enabled = true;
		
		// Determine the new cursor position
		JIndex selStart;
		JIndex selEnd;
		GetSelectionRange(selStart, selEnd);

		// Update state
		mRTSpell.state = RTSpell::eNotInWord;
		mRTSpell.wordStartPos = selEnd;
		mRTSpell.cursorPos = selEnd;
		mRTSpell.charCount = GetTextLength();
		
		return true;
	}
	
	return true;
}
