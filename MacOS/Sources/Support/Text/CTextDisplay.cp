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

#include "CATSUIStyle.h"
#include "CBetterScrollerX.h"
#include "CGUtils.h"
#include "CCharsetManager.h"
#include "CCommands.h"
#include "CContextMenu.h"
#include "CFindReplaceWindow.h"
#include "CMulberryCommon.h"
#include "CPluginManager.h"
#include "CPreferences.h"
#include "CRFC822.h"
#include "CSpellPlugin.h"
#include "CTextEngine.h"
#include "CTextMacros.h"

#include "ustrfind.h"

#include <UGAColorRamp.h>

#include <limits.h>

//#define PROFILING
#ifdef PROFILING
#include <UProfiler.h>
#endif

// __________________________________________________________________________________________________
// C L A S S __ C T E X T D I S P L A Y
// __________________________________________________________________________________________________

CTextDisplay* CTextDisplay::sTextClicking = NULL;
MenuHandle CTextDisplay::sSpellChanger = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CTextDisplay::CTextDisplay(LStream *inStream)
		: CTextBase(inStream)
{
	mSpacesPerTab = 0;
	mHardWrap = false;
	mWrap = 76;
	mTabSelectAll = true;
	mFindAllowed = false;
	
	// Init spell auto check - off
	SpellAutoCheck(false);

	ResIDT	inTextTraitsID;
	*inStream >> inTextTraitsID;

	TextTraitsPtr	traitsPtr = nil;
	
	if (inTextTraitsID != Txtr_SystemFont)
	{
		TextTraitsH	traitsH = UTextTraits::LoadTextTraits(inTextTraitsID);
		
		if (traitsH != nil) {
			StHandleLocker	lock((Handle)traitsH);
			SetTextTraits(**traitsH);
		}
	}
}

// Default destructor
CTextDisplay::~CTextDisplay()
{
}


// O T H E R  M E T H O D S ____________________________________________________________________________

// Setup D&D
void CTextDisplay::FinishCreateSelf(void)
{
	// Do inherited
	CTextBase::FinishCreateSelf();

	// Get menu handle if not already here
	if (!sSpellChanger)
	{
		sSpellChanger = ::GetMenu(3026);
		ThrowIfResFail_(sSpellChanger);
		::InsertMenu(sSpellChanger, hierMenu);
	}

	CContextMenuAttachment::AddUniqueContext(this, 2500, this, false);

	// Hack to set frame style of scroller
	if (dynamic_cast<CBetterScrollerX*>(GetSuperView()) && IsReadOnly())
		static_cast<CBetterScrollerX*>(GetSuperView())->SetReadOnly(true);
}

// Handle keys our way
Boolean CTextDisplay::HandleKeyPress(const EventRecord& inKeyEvent)
{
	// Indicate pending spell check
	SpellTextRequired();

	bool		keyHandled = true;
	EKeyStatus	theKeyStatus = keyStatus_Input;
	SInt16		theKey = inKeyEvent.message & charCodeMask;

	FocusDraw();

	// Look for macro trigger
	CKeyAction keyact(theKey, CKeyModifiers(inKeyEvent.modifiers));
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
	
	// Get key status
	if (inKeyEvent.modifiers & cmdKey) {	// Always pass up when the command
		theKeyStatus = keyStatus_PassUp;	//   key is down
	} else {

		theKeyStatus = UKeyFilters::PrintingCharField(inKeyEvent);
	}

	// Special case for extra navigation key
	if ((theKeyStatus == keyStatus_PassUp) &&
		((theKey == char_LeftArrow) ||
		 (theKey == char_RightArrow) ||
		 (theKey == char_UpArrow) ||
		 (theKey == char_DownArrow)))
	{
		theKeyStatus = keyStatus_TECursor;
	}

	// Switch on key status
	switch (theKeyStatus)
	{
	case keyStatus_PassUp:
		if (theKey == char_Return)
		{
			// Only do if edits allowed
			if (!IsReadOnly())
			{
				keyHandled = CTextBase::HandleKeyPress(inKeyEvent);
			}
		}
		// Check for TAB and replace with spaces if required and edits allowed
		else if ((theKey == char_Tab) && (mSpacesPerTab > 0) && !IsReadOnly() &&
				 ((inKeyEvent.modifiers & (shiftKey | optionKey | controlKey)) == 0))
		{
			// Only do if edits allowed
			if (!IsReadOnly())
			{
				// Check whether space substituion allowed
				if (CPreferences::sPrefs->mTabSpace.GetValue())
				{
					SInt32 selStart;
					SInt32 selEnd;
					GetSelection(&selStart, &selEnd);

					SInt32 line = OffsetToLine(selStart);

					SInt32 lineStart;
					SInt32 lineEnd;
					FindLineRange(line, &lineStart, &lineEnd);
					
					// Adjust for last line
					if ((selStart > lineEnd) && (mTextPtr[selStart - 1] == '\r'))
						lineStart = lineEnd = selStart;

					SInt32 tab_pos = selStart - lineStart;
					short add_space = mSpacesPerTab - (tab_pos % mSpacesPerTab);
					cdustring utxt(' ', add_space);
					InsertText(utxt);
				}
				else
					// Just allow normal tab typing
					keyHandled = CTextBase::HandleKeyPress(inKeyEvent);;
			}

		}
		else if ((theKey == char_Tab) && (mSpacesPerTab > 0) &&
				((inKeyEvent.modifiers & (shiftKey | controlKey)) != 0))
		{
			// Allow modified tab to do tab selection
			EventRecord modKeyEvent = inKeyEvent;
			modKeyEvent.modifiers &= ~(optionKey | controlKey);
			keyHandled = LCommander::HandleKeyPress(modKeyEvent);
		}
		else
			keyHandled = LCommander::HandleKeyPress(inKeyEvent);
		break;

	// Pass ordinary keys up to super-commander if text is not editable
	case keyStatus_Input:
	case keyStatus_TEDelete:
		// Only do if edits allowed
		if (IsReadOnly())
		{
			keyHandled = LCommander::HandleKeyPress(inKeyEvent);
			break;
		}

		// Fall through for key processing

	default:
		keyHandled = CTextBase::HandleKeyPress(inKeyEvent);
		break;
	}
	
	// Do pending spell check
	SpellTextRequiredCheck();
	
	if (theKeyStatus == keyStatus_TECursor)
		mPendingDrawCaret = true;

	return keyHandled;
}

Boolean CTextDisplay::HandleKeyPress(const EventRecord& inKeyEvent, const UniChar* uc, UniCharCount ucount)
{
	// Indicate pending spell check
	SpellTextRequired();

	bool		keyHandled = true;
	SInt16		theKey = inKeyEvent.message & charCodeMask;

	FocusDraw();

	// Look for macro trigger
	CKeyAction keyact(theKey, CKeyModifiers(inKeyEvent.modifiers));
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
	
	// Do default
	keyHandled = CTextBase::HandleKeyPress(inKeyEvent, uc, ucount);
	
	// Do pending spell check
	SpellTextRequiredCheck();

	return keyHandled;
}

// Handle commands our way
Boolean CTextDisplay::ObeyCommand(CommandT inCommand,
					void* ioParam)
{
	Boolean		cmdHandled = true;

	switch (inCommand)
	{
	case msg_TabSelect:
		if (!IsEnabled())
		{
			cmdHandled = false;
			break;
		}
		else if (!mTabSelectAll)
		{
			FocusDraw();
			break;
		}
		SelectAll();
		break;

	case cmd_WrapLines:
		OnWrapLines();
		break;

	case cmd_UnwrapLines:
		OnUnwrapLines();
		break;

	case cmd_QuoteLines:
		OnQuoteLines();
		break;

	case cmd_UnquoteLines:
		OnUnquoteLines();
		break;

	case cmd_RequoteLines:
		OnRequoteLines();
		break;

	case cmd_ShiftLeft:
		OnShiftLeft();
		break;

	case cmd_ShiftRight:
		OnShiftRight();
		break;

	case cmd_FindText:
		OnFindText();
		break;

	case cmd_FindNextText:
		OnFindNextText(false);
		break;

	case cmd_FindNextBackText:
		OnFindNextText(true);
		break;

	case cmd_FindSelectionText:
		OnFindSelectionText(false);
		break;

	case cmd_FindSelectionBackText:
		OnFindSelectionText(true);
		break;

	case cmd_ReplaceText:
		OnReplaceText();
		break;

	case cmd_ReplaceFindText:
		OnReplaceFindText(false);
		break;

	case cmd_ReplaceFindBackText:
		OnReplaceFindText(true);
		break;

	case cmd_ReplaceAllText:
		OnReplaceAllText();
		break;

	default:
		cmdHandled = CTextBase::ObeyCommand(inCommand, ioParam);
		break;
	}

	return cmdHandled;
}

// Handle menus our way
void CTextDisplay::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	UInt16		&outMark,
	Str255		outName)
{
	outUsesMark = false;
	SInt32 sel_start;
	SInt32 sel_end;
	GetSelection(&sel_start, &sel_end);

	switch (inCommand) {

		case cmd_WrapLines:
		case cmd_UnwrapLines:
		case cmd_QuoteLines:
		case cmd_UnquoteLines:
		case cmd_RequoteLines:
			outEnabled = !IsReadOnly() && (sel_start != sel_end);
			break;

		case cmd_ShiftLeft:
		case cmd_ShiftRight:
			outEnabled = !IsReadOnly();
			break;

		case cmd_FindText:
			outEnabled = mFindAllowed;
			break;

		case cmd_FindNextText:
		case cmd_FindNextBackText:
			outEnabled = mFindAllowed && !CFindReplaceWindow::sFindText.empty();
			break;

		case cmd_FindSelectionText:
		case cmd_FindSelectionBackText:
			outEnabled = mFindAllowed && (sel_start != sel_end);
			break;

		case cmd_ReplaceText:
			outEnabled = mFindAllowed && !IsReadOnly() && (sel_start != sel_end);
			break;

		case cmd_ReplaceFindText:
		case cmd_ReplaceFindBackText:
			outEnabled = mFindAllowed && !IsReadOnly() &&
							(sel_start != sel_end) && !CFindReplaceWindow::sFindText.empty();
			break;

		case cmd_ReplaceAllText:
			outEnabled = mFindAllowed && !IsReadOnly() && !CFindReplaceWindow::sFindText.empty();
			break;

		default:
			CTextBase::FindCommandStatus(inCommand, outEnabled,
									outUsesMark, outMark, outName);
			break;
	}
}

void CTextDisplay::BeTarget()
{
	CTextBase::BeTarget();
	Broadcast_Message(eBroadcast_Activate, this);
	Broadcast_Message(eBroadcast_SelectionChanged, this);
}

void CTextDisplay::DontBeTarget()
{
	CTextBase::DontBeTarget();
	Broadcast_Message(eBroadcast_Deactivate, this);
}

// Make sure selection is visible
void CTextDisplay::FixSelectionDisplay(void)
{
	FocusDraw();
	ShowSelection(false);
}

// Allow background window D&D
void CTextDisplay::Click(SMouseDownEvent &inMouseDown)
{
	// Indicate pending spell check
	SpellTextRequired();

	// Call inherited for default behavior.
	CTextBase::Click(inMouseDown);
	
	// Do spell check if still pending
	SpellTextRequiredCheck();
}

// Do click and drag
void CTextDisplay::ClickSelf(const SMouseDownEvent &inMouseDown)
{
	// Cache this first
	sTextClicking = this;

	// Call inherited for default behavior.
	CTextBase::ClickSelf(inMouseDown);
}

// HandleContextMenuEvent:
//	A context menu event is detected. This method allows the text display to intercept
//	the default context menu handling and do its own thing.
//
// cmmEvent		in		event that triggered context menu call
//
// return				true: if this class handled it and no more context menu processing should be done
//						false: if not handled here and standard context menu processing should be done
//
bool CTextDisplay::HandleContextMenuEvent(const EventRecord& cmmEvent)
{
	bool result = false;

	// Check whether word under cursor has spell error hilite mode

	// Convert event point to a local point
	Point pt = cmmEvent.where;
	GlobalToPortPoint(pt);
	PortToLocalPoint(pt);

	bool dummy;
	UniCharArrayOffset over_char = PointToOffset(pt, dummy);
	
	// Check the style of text at the event point
	if (IsMisspelled(over_char, over_char + 1))
	{
		StPreserveSelection _preserve(this);

		// Get the error word
		SInt32 sel_start = 0;
		cdustring text;
		GetMisspelled(over_char, text, sel_start);
		size_t tlen = text.length();
		
		// Set selection to error text
		FocusDraw();
		SetSelection(sel_start, sel_start + tlen);
		
		// Get the spell checker plugin
		CSpellPlugin* speller = CPluginManager::sPluginManager.GetSpelling();
		CSpellPlugin::StLoadSpellPlugin _load(speller);

		// Get suggestions for error word
		cdstrvect suggestions;
		cdstring utf8 = text.ToUTF8();
		speller->GetQuickSuggestions(utf8, suggestions);

		// Delete previous menu items
		for(short i = ::CountMenuItems(sSpellChanger) - 2; i > 0 ; i--)
			::DeleteMenuItem(sSpellChanger, 1);

		// Add up to 15 suggestions
		short menu_pos = 0;
		for(cdstrvect::const_iterator iter = suggestions.begin(); (menu_pos < 15) && (iter != suggestions.end()); iter++, menu_pos++)
		{
			::InsertMenuItem(sSpellChanger, "\p?", menu_pos);
			::SetMenuItemTextUTF8(sSpellChanger, menu_pos + 1, *iter);
		}

		// Do popup menu of suggestions
		Point startPt = cmmEvent.where;
		SInt32 popup_result = ::PopUpMenuSelect(sSpellChanger, startPt.v, startPt.h, 0)  & 0x0000FFFF;

		// Look for Add item
		if (popup_result == menu_pos + 2)
		{
			// Add current word to dictionary
			speller->AddWord(text.ToUTF8());
			
			// Recheck this word everywhere else so that corrections are made
			RecheckAllWords(text);
		}

		// Replace error word with chosen one provided text is editable
		else if ((popup_result != 0) && !IsReadOnly())
		{
			// Do replacement
			cdstring utf8_replace(suggestions.at(popup_result - 1));
			cdustring utf16_replace(utf8_replace);
			size_t rlen = utf16_replace.length();
			InsertUTF8(utf8_replace);
			_preserve.Offset(sel_start + tlen, rlen - tlen);

			// Recheck it
			long length_checked = 0;
			while(true)
			{
				// Get the new word
				long wlen = GetWordAt(sel_start, text, &sel_start, true);
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
		
		// Stop further context menu processing
		result = true;
	}
	
	return result;
}

// Handle local text traits
Boolean CTextDisplay::FocusDraw(LPane* inSubPane)
{
	bool	focused = CTextBase::FocusDraw(inSubPane);
#ifdef _TODO
	if (focused) {
		StColorPenState::Normalize();
		if (mTextTraitsID >= 0)
			UTextTraits::SetPortTextTraits(mTextTraitsID);
		else
			UTextTraits::SetPortTextTraits(&mTextTraits);
	}
#endif

	// Replace background color with owning panes background
	RGBColor textColor;
	::GetForeColor(&textColor);

	ApplyForeAndBackColors();

#if 0
	// If disabled lighten fore color
	if (!IsActive())
		textColor = UGraphicsUtilities::Lighten(&textColor);
#endif
	::RGBForeColor(&textColor);

	return focused;
}

void CTextDisplay::WipeText(const TextTraitsRecord& aTextTrait)
{
	FocusDraw();

	// Now remove the text with the default style set correctly
	SetSelection(0, LONG_MAX);
	unichar_t text[1];
	text[0] = 0;
	SetTextPtr((const char*)&text, 1);

	// Set background to default
	SetBackground(mBackground);

	// Store new traits and invalidate traits id
	SetTextTraits(aTextTrait);
}

void CTextDisplay::SetTextTraits(const TextTraitsRecord& aTextTrait)
{
	// Store new traits and invalidate traits id
	mTextTraits = aTextTrait;
	if (mTextTraits.fontName[0] == 0)
		::GetFontName(mTextTraits.fontNumber, mTextTraits.fontName);

	// Update TE with new traits
	{
		StStopRedraw noDraw(this);
		StPreserveSelection _preserve(this);
		SetSelection(0, LONG_MAX);
		SetFontName(mTextTraits.fontName);
		SetFontSize(::Long2Fix(mTextTraits.size));
		SetFontStyle(mTextTraits.style);
		SetFontColor(mTextTraits.color);
	}

	ResetHardWrap();

	Refresh();
}

void CTextDisplay::SetFont(const TextTraitsRecord& aTextTrait)
{
	SetTextTraits(aTextTrait);
}

// Get each line as utf8 array
void CTextDisplay::GetTextList(cdstrvect& all) const
{
	// Get text string
	cdstring temp;
	GetText(temp);
	
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

// Force dirty
void CTextDisplay::UserChangedText(void)
{
	CTextBase::UserChangedText();
	
	// Do spell check
	SpellTextChange();
}

// Get horiz scrollbar pos
void CTextDisplay::GetScrollPos(long& h, long& v) const
{
	SPoint32 floc;
	GetFrameLocation(floc);

	SPoint32 iloc;
	GetImageLocation(iloc);

	h = floc.h - iloc.h;
	v = floc.v - iloc.v;
}

// Get horiz scrollbar pos
void CTextDisplay::SetScrollPos(long h, long v)
{
	ScrollPinnedImageTo(h, v, true);
}

void CTextDisplay::FindLine(SInt32 offset, SInt32* start, SInt32* end) const
{
	unsigned long line = OffsetToLine(offset);
	if (start != NULL)
	{
		if ((mLineCount == 0) || (GetTextLength() == 0) || (line == 0))
			*start = 0;
		else
			*start = mLineBreaks[line - 1];
	}
	if (end != NULL)
	{
		if ((mLineCount == 0) || (GetTextLength() == 0) || (line == 0))
			*end = 0;
		else if (line < mLineCount - 1)
			*end = mLineBreaks[line];
		else
			*end = GetTextLength() - 1;
	}
}

void CTextDisplay::FindLineRange(unsigned long line, SInt32* start, SInt32* end) const
{
	if (start != NULL)
	{
		if ((mLineCount == 0) || (GetTextLength() == 0))
			*start = 0;
		else if (line < mLineCount)
			*start = (line > 0) ? mLineBreaks[line - 1] : 0;
		else
			*start = GetTextLength() - 1;
	}

	if (end != NULL)
	{
		if ((mLineCount == 0) || (GetTextLength() == 0))
			*end = 0;
		else if (line < mLineCount - 1)
			*end = mLineBreaks[line];
		else
			*end = GetTextLength() - 1;
	}
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
		mWordWrap = !hard_wrap;
		mHardWrap = hard_wrap;
		if (mHardWrap)
			ResetHardWrap();
		
		// Now adjust layout to re-wrap lines
		AdjustTextLayout();
	}
}

long CTextDisplay::MeasureText(const char* txt)
{
	// Force image size to width of font * wrap length
	StTextState _save;
	UTextTraits::SetPortTextTraits(&CPreferences::sPrefs->mDisplayTextTraits.GetValue().traits);
	FocusDraw();
	
	StPreserveSelection _selection(this);
	cdustring utmp(txt);
	CTextWidget::InsertText(utmp.c_str(), utmp.length(), 0UL);
	Rect line;
	if (::ATSUMeasureTextImage( mTextLayout, 0, utmp.length(), 0, 0, &line ) != noErr)
	{
		line.left = line.right = 0;
	}
	DeleteText(0, utmp.length());
	
	return line.right - line.left;
}

// Set wrapping mode
void CTextDisplay::ResetHardWrap()
{
	if (mHardWrap)
	{
		SDimension32 image;
		GetImageSize(image);
		
		// Force image size to width of font * wrap length
		long tlen = MeasureText("1234567890im1234567890im1234567890im1234567890im1234567890im1234567890im");
		if (tlen != 0)
		{
			image.width = (mWrap * tlen) / 72 + mMargins.left + mMargins.right;
			ResizeImageTo(image.width, image.height, false);
		}
	}
}

void CTextDisplay::SetSpacesPerTab(short numSpacesPerTab)
{
	// Force image size to width of font * wrap length
	long char_width = MeasureText("1234567890im1234567890im1234567890im1234567890im1234567890im1234567890im");
	if (char_width != 0)
	{
		char_width = (numSpacesPerTab * char_width) / 72;
	}
	Fixed tab_width = IntToFixed(char_width);
	Fixed tab_pos = tab_width;

	// Create tab ruler
	const int cRulerSize = 30;
	ATSUTab tabs[cRulerSize];
	for(int i = 0; i < cRulerSize; i++)
	{
		tabs[i].tabPosition = tab_pos;
		tabs[i].tabType = kATSULeftTab;
		tab_pos += tab_width;
	}
	::ATSUSetTabArray(mTextLayout, tabs, cRulerSize);

	mSpacesPerTab = numSpacesPerTab;
}

#pragma mark ____________________________Text Processing

void CTextDisplay::OnWrapLines()
{
	// Prepare for line based processing
	SInt32 sel_start;
	SInt32 sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);

	// Now do wrap - in flowed mode
	const char* wrapped = CTextEngine::WrapLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(), CPreferences::sPrefs->mFormatFlowed.GetValue());

	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

void CTextDisplay::OnUnwrapLines()
{
	// Prepare for line based processing
	SInt32 sel_start;
	SInt32 sel_end;
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
	SInt32 sel_start;
	SInt32 sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);

	// Now do wrap with quote
	const char* wrapped = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(),
													mQuotation, &CPreferences::sPrefs->mRecognizeQuotes.GetValue(), true);

	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

void CTextDisplay::OnUnquoteLines()
{
	// Prepare for line based processing
	SInt32 sel_start;
	SInt32 sel_end;
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
	SInt32 sel_start;
	SInt32 sel_end;
	cdstring selected;
	PrepareTextProcessing(sel_start, sel_end, selected);

	// Now do requote
	const char* txt = CTextEngine::UnquoteLines(selected.c_str(), selected.length(), mQuotation.c_str());
	selected.steal(const_cast<char*>(txt));
	txt = CTextEngine::UnwrapLines(selected.c_str(), selected.length());
	selected.steal(const_cast<char*>(txt));
	txt = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(),
									mQuotation, &CPreferences::sPrefs->mRecognizeQuotes.GetValue(), true);

	// Replace selection
	EndTextProcessing(txt, sel_start, sel_end);
}

void CTextDisplay::OnShiftLeft()
{
	// Prepare for line based processing
	SInt32 sel_start;
	SInt32 sel_end;
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
	SInt32 sel_start;
	SInt32 sel_end;
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
	const char* wrapped = CTextEngine::QuoteLines(selected.c_str(), selected.length(), CRFC822::GetWrapLength(), shift, NULL, true);

	// Replace selection and reselect
	EndTextProcessing(wrapped, sel_start, sel_end);
}

bool CTextDisplay::ExpandMacro()
{
	FocusDraw();

	// Look for selection
	SInt32 selStart;
	SInt32 selEnd;
	GetSelectionRange(selStart, selEnd);
	cdustring utxt;
	GetText(utxt);
	if (selStart == selEnd)
	{
		// Scan backwards looking for word break
		while(selStart--)
		{
			// Test for word break
			if (isspace(utxt[selStart]) || ispunct(utxt[selStart]))
			{
				// Bump up to start of word itself
				selStart++;
				break;
			}
		}
		
		// Adjust for first char
		if (selStart < 0)
			selStart = 0;
	}

	// Get the selected text
	cdustring uselected;
	uselected.assign(utxt, selStart, selEnd - selStart);
	cdstring selected = uselected.ToUTF8();
	selected.trimspace();
	if (selected.empty())
		return false;
	
	// Lookup the macro
	const cdstring& value = CPreferences::sPrefs->mTextMacros.GetValue().Lookup(selected);
	if (!value.empty())
	{
		// Check whether spelling is active
		if (mRTSpell.enabled && !mRTSpell.busy)
		{
			// Always mark as not mispelled
			MarkMisspelled(selStart, selEnd, false);
		}

		// Make sure smart insert does not apply
		SetSelectionRange(selStart, selEnd);

		InsertUTF8(value);
		
		// Mark text as changed
		UserChangedText();
		//CheckScroll(true);
		
		return true;
	}
	else
		return false;
}

void CTextDisplay::PrepareTextProcessing(SInt32& sel_start, SInt32& sel_end, cdstring& selected)
{
	FocusDraw();

	// Make sure complete lines are selected
	SelectFullLines();
	GetSelection(&sel_start, &sel_end);

	// Get the selected text
	GetSelectedText(selected);
}

void CTextDisplay::EndTextProcessing(const char* insert_text, SInt32& sel_start, SInt32& sel_end)
{
	// Replace selection
	if (insert_text)
	{
		size_t insert_length = ::strlen(insert_text);
		InsertUTF8(insert_text, insert_length);

		FocusDraw();

		sel_end = sel_start + insert_length + ((insert_text[insert_length - 1] == '\r') ? 0 : 1);
	}
	SetSelection(sel_start, sel_end);
	delete insert_text;
}

void CTextDisplay::SelectFullLines()
{
#if 0
	// Get current selection and text
	SInt32 sel_start;
	SInt32 sel_end;
	GetSelection(&sel_start, &sel_end);

	// Get offsets to lines
	SInt32 line_start = WEOffsetToLine(sel_start);
	SInt32 line_end = WEOffsetToLine(sel_end);

	// Get range for new selection;
	SInt32 dummy;
	SInt32 new_sel_start;
	SInt32 new_sel_end;
	WEGetLineRange(line_start, &new_sel_start, &dummy);
	WEGetLineRange(line_end, &dummy, &new_sel_end);

	// Adjust for line end
	if (dummy == sel_end)
		new_sel_end = sel_end;

	// Do new selection
	WESetSelection(new_sel_start, new_sel_end);
#endif
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
	SInt32 sel_start;
	SInt32 sel_end;
	GetSelection(&sel_start, &sel_end);

	// Get current text
	cdustring text;
	GetText(text);

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

	cdustring findit(CFindReplaceWindow::sFindText);

	const unichar_t* found = ::ustrfind(text, text.length(), sel_start, findit, mode);

	// Was it found
	if (found)
	{
		// Determine new selection
		sel_start = found - text.c_str();
		sel_end = sel_start + findit.length();
		FocusDraw();
		MoveSelection(sel_start, sel_end);
		result = true;
	}
	else if (!silent)
		::SysBeep(1);

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
	FocusDraw();
	InsertUTF8(CFindReplaceWindow::sReplaceText);
}

void CTextDisplay::DoReplaceFindText(bool backwards)
{
	DoReplaceText();
	DoFindNextText(backwards);
}

void CTextDisplay::DoReplaceAllText()
{
	// Reset selection to first character to allow replace within the selection itself
	FocusDraw();
	SInt32 sel_start;
	SInt32 sel_end;
	GetSelection(&sel_start, &sel_end);
	SetSelection(sel_start, sel_start);

	// Loop until no more found (silently) and do replace
	bool found_once = false;
	SInt32 first_start;
	SInt32 first_end;

	while(DoFindNextText(CFindReplaceWindow::sFindMode & eBackwards, true))
	{
		if (!found_once)
		{
			// Get the first item found and cache selection range
			GetSelection(&first_start, &first_end);
			found_once = true;
		}
		else
		{
			// Make sure we don't repeat ourselves
			GetSelection(&sel_start, &sel_end);
			if (sel_start == first_start)
				break;
		}
		DoReplaceText();
	}

	// Reset selection if no change
	if (!found_once)
	{
		SetSelection(sel_start, sel_end);
		::SysBeep(1);
	}
}

#pragma mark ____________________________Spelling commands

void CTextDisplay::SpellAutoCheck(bool auto_check)
{
	if (auto_check && CPluginManager::sPluginManager.HasSpelling())
	{
		// Make sure spell check plugin is loaded
		CSpellPlugin* plugin = CPluginManager::sPluginManager.GetSpelling();
		if (plugin->IsLoaded())
			plugin->LoadPlugin();
		else
		{
			plugin->LoadPlugin();
			plugin->SpInitialise();
		}
		
		mRTSpell.state = RTSpell::eNotInWord;
		mRTSpell.cursorPos = 0;
		mRTSpell.charCount = 0;
		mRTSpell.wordStartPos = 0;
		mRTSpell.contextWordPos = 0;
		mRTSpell.busy = false;
		mRTSpell.enabled = true;
		mRTSpell.pending = false;
	}
	else
	{
		mRTSpell.busy = false;
		mRTSpell.enabled = false;
		mRTSpell.pending = false;
	}
}

bool CTextDisplay::IsMisspelled(SInt32 sel_start, SInt32 sel_end) const
{
	if (sel_end > GetTextLength())
		sel_end = GetTextLength();
	
	// Loop over style runs
	UniCharArrayOffset start = sel_start;
	UniCharCount length = sel_end - sel_start;
	
	bool spelling = false;

	while(length > 0)
	{
		// Get style run for start char
		ATSUStyle style;
		UniCharArrayOffset runStart;
		UniCharCount runLength;
		if (::ATSUGetRunStyle(mTextLayout, start, &style, &runStart, &runLength) != noErr)
			return false;
		
		// Get minimum length of change within this run
		UniCharCount applyLength = std::min(runLength - (start - runStart), length);
		
		// Now create new style
		CATSUIStyle newStyle;
		::ATSUCopyAttributes(style, newStyle.Get());
		spelling = spelling || newStyle.IsSpelling();
		
		// Adjust apply lengths
		start += applyLength;
		length -= applyLength;
	}
	
	return spelling;
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
#if (defined(_WIN16) || defined(_WIN32)) && !defined(SSCE_NO_WINDOWS)
	/* Additional alphabetic characters in the Latin1 Windows charset */
	/* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, A_|U_, 0, A_|U_|V_, 0, A_|U_, 0,
	/* 90 */ 0, 0, P_, P_, P_, P_, P_, P_, P_, P_, A_|L_, P_, A_|L_|V_, 0, A_|L_, A_|U_,
#else
	/* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	/* 90 */ 0, 0, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, P_, 0, 0, 0,
#endif
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
#define IS_WORD_CHAR(c) ((c < 0x0100) && (WS_Latin1_CharClass[c] == 1))


long CTextDisplay::GetWordAt(long pos, cdustring& word, long* startPos, bool word_start) const
{
	const long maxWordSize = 256;
	
	// Determine sensible buffer range around start pos
	long rstart = std::max(0L, pos - maxWordSize);
	long rend = pos + maxWordSize;
	
	// Get the text in that range
	cdustring buf;
	GetSpellTextRange(rstart, rend, buf);
	const unichar_t* _buf = buf.c_str();
	
	// Locate beginning of word by stepping back
	pos = std::min(pos, rstart + (long) buf.length());
	long i = std::min(pos, maxWordSize);
	while((i > 0) &&
			(IS_WORD_CHAR(_buf[(unsigned long) i - 1]) ||
			 (_buf[(unsigned long) i - 1] == '.') ||
			 (_buf[(unsigned long) i - 1] == '\'') ||
			 (_buf[(unsigned long) i - 1] == FANCY_APOSTROPHE)))
		i--;
		
	// May not be a word
	if (i < 0)
	{
		*startPos = -1;
		return 0;
	}
	
	// Advance past any periods and apostrophes at the beginning of the word
	while((_buf[(unsigned long) i] != 0) && !IS_WORD_CHAR(_buf[(unsigned long) i]))
		i++;
	*startPos = rstart + i;

	// Collect the word
	bool embeddedPeriods = false;
	while(IS_WORD_CHAR(_buf[(unsigned long) i]) ||
			(_buf[(unsigned long) i] == '.') || 
			(_buf[(unsigned long) i] == '\'') ||
			(_buf[(unsigned long) i] == FANCY_APOSTROPHE))
	{
		if ((_buf[(unsigned long) i] == '.') && (i > 0) &&
			IS_WORD_CHAR(_buf[(unsigned long) i - 1]) && IS_WORD_CHAR(_buf[(unsigned long) i + 1]))
			embeddedPeriods = true;
		i++;
	}
	long n = rstart + i - *startPos;

	// Copy the word
	if (n > 0)
		word.assign(buf.c_str() + *startPos - rstart, n);
	
	// Accept a terminating period only if the word is an initialism
	// (contains embedded periods, no more than 2 consecutive alpha-numerics)
	if ((n > 0) && (word[(unsigned long) n - 1] == '.'))
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
			/* Exclude all trailing periods. */
			while ((n > 0) && (word[(unsigned long) n - 1] == '.'))
				word[(unsigned long) --n] = 0;
		}
	}

	return n;
}

unichar_t CTextDisplay::GetTextChar(long pos) const
{
	cdustring txt;
	long len = GetSpellTextRange(pos, pos + 1, txt);
	return (len > 0) ? txt[0UL] : 0;
}

long CTextDisplay::GetSpellTextRange(long start, long end, cdustring& text) const
{
	UniCharArrayOffset rstart = start;
	if (rstart < 0)
		rstart = 0;
	UniCharArrayOffset rend = end;
	if (rend > GetTextLength())
		rend = GetTextLength();
	if (rend < rstart)
		rend = rstart;
	
	text.assign(mTextPtr + rstart, rend - rstart);
	
	return text.length();
}

bool CTextDisplay::GetMisspelled(SInt32 pos, cdustring& word, SInt32& word_start) const
{
	if (!IsMisspelled(pos, pos + 1))
		return false;

	// Locate the extent of the marked text
	for (word_start = pos; (word_start > 0) && IsMisspelled(word_start - 1, word_start) && !isspace(GetTextChar(word_start - 1)); word_start--)
	{
	}

	SInt32 word_end;
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

void CTextDisplay::MarkMisspelled(SInt32 sel_start, SInt32 sel_end, bool misspelled)
{
	SetSpellHighlight(sel_start, sel_end, misspelled);
}

bool CTextDisplay::CheckWord(const cdustring& word, SInt32 start)
{
	// Ignore if no spell checker
	if (!CPluginManager::sPluginManager.HasSpelling())
		return true;

	bool result = false;

	// Canonicalise word
	cdustring temp(word);
	size_t wlen = temp.length();
	if ((wlen > 0) && ((temp[wlen - 1] == '\'') || (temp[wlen - 1] == FANCY_APOSTROPHE)))
	{
		if ((wlen > 1) && (temp[wlen - 2] != 's') && (temp[wlen - 2] != 'S'))
		{
			temp[wlen - 1] = '\0';
			wlen--;
		}
	}
	
	// Check word (convert to local charset first)
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

		FocusDraw();
		
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
	const unichar_t* p = mTextPtr;
	unsigned long pos = 0;
	while((p = ::ustrfind(mTextPtr, mTextLength, pos, word.c_str(), eEntireWord)) != NULL)
	{
		// Update pos
		pos = p - mTextPtr;

		// Recheck the word found
		cdustring old_word;
		long startPos = 0;
		GetWordAt(pos, old_word, &startPos, true);
		CheckWord(old_word, startPos);
		
		// Resume searching after this word
		pos++;
	}
}

void CTextDisplay::SpellInsertText(long start, long numchars)
{
	// Ignore if no spell checker
	if (!CPluginManager::sPluginManager.HasSpelling())
		return;

	// Save current selection pos
	long sel_start;
	long sel_end;
	GetSelectionRange(sel_start, sel_end);

	// Determine reasonable buffer around cursor
	long cursor_pos = start + numchars;
	long text_start = cursor_pos - (numchars + 256);
	text_start = std::max(text_start, 0L);
	long text_len = cursor_pos - text_start;
	text_len += std::max(text_len / 5L, 256L);
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

	long text_cursor = i;
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
				FocusDraw();
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

#ifdef PROFILING
	StProfileSection profile("\pSpellTextChange-Profile", 2000, 50);
#endif

	// Check whether processing can be done
	if (!mRTSpell.enabled || mRTSpell.busy)
		return;

	StValueChanger<bool> _change(mRTSpell.busy, true);
	mRTSpell.pending = false;

	long newCharCount = GetTextLength();
	long newWordStartPos = mRTSpell.wordStartPos;

	// Determine the new cursor position
	long selStart;
	long selEnd;
	GetSelectionRange(selStart, selEnd);
	long newCursorPos = selEnd;

	// Determine the dimensions of the current word
	cdustring word;
	long wordLen = 0;
	if (mRTSpell.wordStartPos >= 0)
	{
		// Adjust cached start pos if change occurs in front of cacched word pos
		if (newCursorPos < mRTSpell.wordStartPos)
		{
			if (newCharCount > mRTSpell.charCount)
				mRTSpell.wordStartPos += newCharCount - mRTSpell.charCount;
			else if (newCharCount < mRTSpell.charCount)
				mRTSpell.wordStartPos -= mRTSpell.charCount - newCharCount;
		}
	
		// Now get cached word at new position
		wordLen = GetWordAt(mRTSpell.wordStartPos, word, &newWordStartPos, true);
		mRTSpell.wordStartPos = newWordStartPos;
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
			// Always unmark word
			if (wordLen)
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
			// Always unmark word
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
			SpellInsertText(newCursorPos - (newCharCount - mRTSpell.charCount), newCharCount - mRTSpell.charCount);
			break;
		case RTSpell::eInUnmarkedWord:
			// Check any words inserted after the current word. We don't
			// want to check the current word here because it may not be complete
			if (newCursorPos - (mRTSpell.wordStartPos + wordLen) > 0)
				SpellInsertText(mRTSpell.wordStartPos + wordLen,
									newCursorPos - (mRTSpell.wordStartPos + wordLen));
			break;
		case RTSpell::eInMarkedWord:
			// Check all inserted text including the current word. The
			// insertion may have changed the current word from misspelled to OK
			SpellInsertText(mRTSpell.wordStartPos, newCursorPos - mRTSpell.wordStartPos);
		}
	}

	if ((mRTSpell.wordStartPos >= 0) && (wordLen > 0) &&
	  	((newCursorPos < mRTSpell.wordStartPos) || (newCursorPos > mRTSpell.wordStartPos + wordLen)))
	{
		// Cursor moved outside current word
		bool wordChanged = false;
		switch (mRTSpell.state)
		{
		case RTSpell::eNotInWord:
			// Can't happen
			break;
		case RTSpell::eInUnmarkedWord:
		case RTSpell::eInMarkedWord:
			wordChanged = CheckWord(word, mRTSpell.wordStartPos);
			if (wordChanged)
			{
				// Adjust cached cursor positions for change in word
				long new_wordLen = GetWordAt(mRTSpell.wordStartPos, word, &mRTSpell.wordStartPos, true);
				newCursorPos += new_wordLen - wordLen;
				mRTSpell.cursorPos += new_wordLen - wordLen;
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
			if (IsMisspelled(newCursorPos, newCursorPos))
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
		long selStart;
		long selEnd;
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

#pragma mark -

StFocusTE::StFocusTE(LPane	*inPane)
{
	mClipRgn = NULL;

	bool	exposed = false;
	if ( (inPane->GetSuperView() != NULL) &&
		 inPane->GetSuperView()->EstablishPort() ) {
		exposed = inPane->FocusExposed(true);
	}

	if (!exposed || !inPane->IsVisible()) {					// Make an empty clip region
		mClipRgn = ::NewRgn();
		if (mClipRgn != NULL) {
			::GetClip(mClipRgn);
			Rect	zeroRect = {0, 0, 0, 0};
			::ClipRect(&zeroRect);
		}
	}
}


StFocusTE::~StFocusTE()
{
	if (mClipRgn != NULL) {					// Restore original clip region
		::SetClip(mClipRgn);
		::DisposeRgn(mClipRgn);
	}
}
