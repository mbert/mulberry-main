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


// Source for CTextBase class

#include "CTextBase.h"

#include "CCommands.h"
#include "CPreferences.h"
#include "CWindowStatesFwd.h"
#include "StValueChanger.h"

#include <JXButton.h>
#include <JXColormap.h>
#include <JXDialogDirector.h>
#include <JXDisplay.h>
#include <JXFontManager.h>
#include <JXTextMenu.h>
#include <JXTextSelection16.h>
#include <JXWindow.h>
#include <jASCIIConstants.h>
#include <jXGlobals.h>

#include <algorithm>
#include <cassert>

CTextBase::StPreserveSelection::StPreserveSelection(CTextBase* txt)
{
	mTxt = txt;
	txt->GetSel(mSelStart, mSelEnd);
}

CTextBase::StPreserveSelection::~StPreserveSelection()
{
	// Make sure new selection is still in range
	mSelStart = std::min(mSelStart, mTxt->GetTextLength());
	mSelEnd = std::min(mSelEnd, mTxt->GetTextLength());
	mTxt->SetSel(mSelStart, mSelEnd);
}

// __________________________________________________________________________________________________
// C L A S S __ C T E X T D I S P L A Y
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTextBase::CTextBase(const JCharacter *text,
						JXScrollbarSet* sbs,
						JXContainer* enclosure,
						JXTextMenu *menu,	     
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h,
						Type editorType)
	: CCommander(enclosure),
		JXTEBase16(editorType, kFalse, kFalse, sbs, enclosure, hSizing, vSizing, x, y, w, h),
		CContextMenu(this)
{
	CTextBaseX(text, menu, w, h);
}

CTextBase::CTextBase(JXScrollbarSet* sbs,
						JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h,
						Type editorType)
	: CCommander(enclosure),
		JXTEBase16(editorType, kFalse, kFalse, sbs, enclosure, hSizing, vSizing, x, y, w, h),
		CContextMenu(this)
{
	CTextBaseX("", NULL, w, h);
}

void CTextBase::CTextBaseX(const JCharacter* text, JXTextMenu* menu,
							JCoordinate w, JCoordinate h)
{
	mDirty = false;
	mUseCommandClipboard = false;

	// Initialise to the standard text font
	const SFontInfo* actual_font = &CPreferences::sPrefs->mDisplayTextFontInfo.GetValue();
	ResetFont(*actual_font, 0);

	// Check charset for iso-8859-15 locale
	JIndex iso_charset = 0;
	if (JXGetLatinCharacterSetIndex(&iso_charset) && (iso_charset == 15))
		ResetCharset(eISO8859_15, 0);

	SetDefaultFontSize(kJXDefaultFontSize);
	TESetLeftMarginWidth(kMinLeftMarginWidth);
	SetText(text);

	ShouldAllowDragAndDrop(kTrue);

	SetBackgroundColor(GetColormap()->GetWhiteColor());

	bool doSet = false;
	if (w == 0)
	{
		w = GetMinBoundsWidth() + 2*GetBorderWidth();
		doSet = true;
	}
	if (h == 0)
	{
		h = GetMinBoundsHeight() + 2*GetBorderWidth();
		doSet = true;
	}
	if (doSet)
		SetSize(w,h);

	// Use nultiple undo/redo
	UseMultipleUndo();
}

// Default destructor
CTextBase::~CTextBase()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

void CTextBase::OnCreate(CMainMenu::EContextMenu context)
{
	// Context menu
	CreateContextMenu(context);
}

void CTextBase::Receive(JBroadcaster* sender, const Message& message) 
{
	// Check for context menu
	if (!ReceiveMenu(sender, message))
		// Do default action
		JXTEBase16::Receive(sender, message);
}

bool CTextBase::ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu)
{
	switch(cmd)
	{
	case CCommand::eEditUndo:
		Undo();
		return true;
	case CCommand::eEditRedo:
		Redo();
		return true;
	case CCommand::eEditCut:
	{
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Cut();
		return true;
	}
	case CCommand::eEditCopy:
	{
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Copy();
		return true;
	}
	case CCommand::eEditPaste:
	{
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Paste();
		return true;
	}
	case CCommand::eEditDelete:
		DeleteSelection();
		return true;
	case CCommand::eEditSelectAll:
		EditSelectAll();
		return true;
	}

	return CCommander::ObeyCommand(cmd, menu);
}

// Handle menus our way
void CTextBase::UpdateCommand(unsigned long cmd, CCmdUI* cmdui)
{
	switch(cmd)
	{
	case CCommand::eEditUndo:
	case CCommand::eEditRedo:
	case CCommand::eEditCut:
	case CCommand::eEditCopy:
	case CCommand::eEditPaste:
	case CCommand::eEditDelete:
	case CCommand::eEditSelectAll:
		{
			JString action1;
			JString action2;
			JBoolean readonly;
			JArray<JBoolean> cmds = GetCmdStatus(&action1, &action2, &readonly);
			CmdIndex test_it;
			switch(cmd)
			{
			case CCommand::eEditUndo:
				test_it = kUndoCmd;
				break;
			case CCommand::eEditRedo:
				test_it = kRedoCmd;
				break;
			case CCommand::eEditCut:
				test_it = kCutCmd;
				break;
			case CCommand::eEditCopy:
				test_it = kCopyCmd;
				break;
			case CCommand::eEditPaste:
				test_it = kPasteCmd;
				break;
			case CCommand::eEditDelete:
				test_it = kDeleteSelCmd;
				break;
			case CCommand::eEditSelectAll:
				test_it = kSelectAllCmd;
				break;
			}
			cmdui->Enable(cmds.GetElement(test_it));
			return;
		}
	}

	CCommander::UpdateCommand(cmd, cmdui);
}

// Speical processing of keyboard initiated clipboard actions
void CTextBase::HandleKeyPress(const int origKey, const JXKeyModifiers& origModifiers)
{
	if (!HandleChar(origKey, origModifiers))
		return JXTEBase16::HandleKeyPress(origKey, origModifiers);
}

// Speical processing of keyboard initiated clipboard actions
bool CTextBase::HandleChar(const int origKey, const JXKeyModifiers& origModifiers)
{
	JXKeyModifiers passModifiers(origModifiers);

	const JBoolean controlOn = origModifiers.control();
	const JBoolean metaOn    = origModifiers.meta();
	const JBoolean shiftOn   = origModifiers.shift();

	const Type type = GetType();

	if (type == kFullEditor &&
		 (((origKey == 'x' || origKey == 'X') && !controlOn &&  metaOn && !shiftOn) ||
		  (origKey == JXCtrl('X')         &&  controlOn && !metaOn && !shiftOn)))
	{
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Cut();
		return true;
	}
	else if (((origKey == 'c' || origKey == 'C') && !controlOn &&  metaOn && !shiftOn) ||
			 (origKey == JXCtrl('C')         &&  controlOn && !metaOn && !shiftOn))
	{
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Copy();
		return true;
	}
	else if (type == kFullEditor &&
			 (((origKey == 'v' || origKey == 'V') && !controlOn &&  metaOn && !shiftOn) ||
			  (origKey == JXCtrl('V')         &&  controlOn && !metaOn && !shiftOn)))
	{
		StValueChanger<bool> _change(mUseCommandClipboard, true);
		Paste();
		return true;
	}

	// Special for ctrl-Backspace/Delete
	else if (type == kFullEditor && !HasSelection() &&
			 (origKey == kJDeleteKey) && controlOn && !metaOn && !shiftOn)
	{
		// Fake crtl-left
		JXKeyModifiers newMods(GetDisplay());
		newMods.SetState(kJXShiftKeyIndex, kTrue);
		newMods.SetState(kJXControlKeyIndex, kTrue);
		JXTEBase16::HandleKeyPress(kJLeftArrow, newMods);

		// Now pretend we got keypress without modifier
		passModifiers.SetState(kJXControlKeyIndex, kFalse);
	}
	else if (type == kFullEditor && !HasSelection() &&
			 (origKey == kJForwardDeleteKey) && controlOn && !metaOn && !shiftOn)
	{
		// Fake crtl-right
		JXKeyModifiers newMods(GetDisplay());
		newMods.SetState(kJXShiftKeyIndex, kTrue);
		newMods.SetState(kJXControlKeyIndex, kTrue);
		JXTEBase16::HandleKeyPress(kJRightArrow, newMods);

		// Now pretend we got keypress without modifier
		passModifiers.SetState(kJXControlKeyIndex, kFalse);
	}

	// emacs key bindings
	else if (controlOn && !metaOn && !shiftOn)
	{
		JXKeyModifiers newMods(GetDisplay());
		switch(origKey + '@')
		{
		case 'F': // forward one char
			JXTEBase16::HandleKeyPress(kJRightArrow, newMods);
			return true;
		case 'B': // backward one char
			JXTEBase16::HandleKeyPress(kJLeftArrow, newMods);
			return true;
		case 'P': // previous line
			JXTEBase16::HandleKeyPress(kJUpArrow, newMods);
			return true;
		case 'N': // next line
			JXTEBase16::HandleKeyPress(kJDownArrow, newMods);
			return true;
		case 'A': // move to start of line
			GoToBeginningOfLine();
			return true;
		case 'E': // move to end of line
			GoToEndOfLine();
			return true;
		case 'D': // forward delete one char or selection
			// Must be editable
			if (type == kFullEditor)
				JXTEBase16::HandleKeyPress(kJForwardDeleteKey, newMods);
			return true;
		case 'K': // kill to end of line
			// Must be editable
			if (type == kFullEditor)
			{
				// Fake meta-shift-right
				JXKeyModifiers newMods(GetDisplay());
				newMods.SetState(kJXShiftKeyIndex, kTrue);
				newMods.SetState(kJXMetaKeyIndex, kTrue);
				JXTEBase16::HandleKeyPress(kJRightArrow, newMods);

				// Check for start of blank line
				if (!HasSelection())
				{
					JIndex charPos;
					GetCaretLocation(&charPos);
					JIndex lineIndex = GetLineForChar(charPos);
					JIndex line_start = GetLineStart(lineIndex);
					
					// Extend selection over the line end
					if (charPos == line_start)
					{
						// Fake shift-right
						JXKeyModifiers newMods(GetDisplay());
						newMods.SetState(kJXShiftKeyIndex, kTrue);
						JXTEBase16::HandleKeyPress(kJRightArrow, newMods);
					}
				}

				// Cut the selection
				if (HasSelection())
					DeleteSelection();

			}
			return true;
		}
	}

	// Make sure caret moves for page up/down home & end
	JXTEBase16::CaretShouldFollowScroll(kTrue);
	JXTEBase16::HandleKeyPress(origKey, passModifiers);
	JXTEBase16::CaretShouldFollowScroll(kFalse);
	
	return true;
}

void CTextBase::HandleMouseDown(const JPoint& pt, 
							      const JXMouseButton button,
							      const JSize clickCount, 
							      const JXButtonStates& buttonStates,
							      const JXKeyModifiers& modifiers)
{
	// Handle context menus
	if (button == kJXRightButton)
	{
		ContextEvent(pt, buttonStates, modifiers);
		return;
	}

	JXTEBase16::HandleMouseDown(pt, button, clickCount, buttonStates, modifiers);
}

Atom CTextBase::GetClipboardAtom() const
{
	return (mUseCommandClipboard ? XInternAtom(*GetDisplay(), "CLIPBOARD", False) : XA_PRIMARY);
}

// Exact copy of JXTEBase16 code except that it uses clipboard that this class determines
void CTextBase::TEClipboardChanged()
{
	if (HasSelection())
		{
		JString16* text = new JString16;
		assert( text != NULL );

		JRunArray<Font>* style = new JRunArray<Font>;
		assert( style != NULL );

		const JBoolean ok = GetSelection(text, style);
		assert( ok );

		JXTextSelection16* data = new JXTextSelection16(GetDisplay(), text, GetColormap(), style);
		assert( data != NULL );

		if (!(GetSelectionManager())->SetData(GetClipboardAtom(), data))
			{
			(JGetUserNotification())->ReportError("Unable to copy to the X Clipboard.");
			}
		}
}

// Exact copy of JXTEBase16 code except that it uses clipboard that this class determines
JBoolean CTextBase::TEGetExternalClipboard(JString16* text, JRunArray<Font>* style) const
{
	const JError err = GetSelectionData(GetClipboardAtom(), CurrentTime, text, style);
	if (err.OK())
		{
		return kTrue;
		}
	else
		{
		if (err != kNoData)
			{
			err.ReportIfError();
			}
		return kFalse;
		}
}

// Override to handle iso-8859-1 8-bit characters
JBoolean CTextBase::IsCharacterInWord(const JString16& text, const JIndex charIndex) const
{
	// For now we assume iso-8859-1 and use our own map
	static unsigned char isalnum_iso88591[256] =
	{
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x00
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x10
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x20
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x30
		0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	// 0x40
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x50
		0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	// 0x60
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x70
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x80
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0x90
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0xA0
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,	// 0xB0
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	// 0xC0
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	// 0xD0
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,	// 0xE0
		0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01	// 0xF0
	};
		
	return (text.GetCharacter(charIndex) < 256) && JConvertToBoolean( isalnum_iso88591[text.GetCharacter(charIndex)] == 1 );
}

// Selection utils

void CTextBase::GetSel(JIndex& sel_start, JIndex& sel_end) const
{
	if (HasSelection())
	{
		GetSelection(&sel_start, &sel_end);
		sel_start--;
	}
	else
	{
		GetCaretLocation(&sel_start);
		sel_end = --sel_start;
	}
}

void CTextBase::SetSel(const JIndex& sel_start, const JIndex& sel_end, bool do_copyselect)
{
	if (sel_start == sel_end)
		SetCaretLocation(sel_start + 1);
	else
		SetSelection(sel_start + 1, sel_end);
}

void CTextBase::GetSelectedText(cdstring& selected) const
{
	// get as UTF16 first
	cdustring utf16;
	GetSelectedText(utf16);
	selected = utf16.ToUTF8();
}

void CTextBase::GetSelectedText(cdustring& selected) const
{
	JString16 quote;
	if (HasSelection())
		GetSelection(&quote);
	selected = quote;
}

// UTF8 in
void CTextBase::SetText(const cdstring& all)
{
	cdustring utf16(all);
	SetText(utf16);
}

// UTF16 in
void CTextBase::SetText(const cdustring& all)
{
	JString16 temp(all);
	SetText16(all);
}

// Insert unstyled utf8 at selection
void CTextBase::InsertUTF8(const cdstring& txt)
{
	cdustring utf16(txt);
	InsertText(utf16);
}

// Insert unstyled utf8 at selection
void CTextBase::InsertUTF8(const char* txt, size_t len)
{
	cdstring utf8(txt, len);
	InsertUTF8(utf8);
}

// Insert unstyled utf16 at selection
void CTextBase::InsertText(const cdustring& utxt)
{
	Paste(utxt);
}

// Insert unstyled utf16 at selection
void CTextBase::InsertText(const unichar_t* utxt, size_t len)
{
	cdustring utf16(utxt, len);
	InsertText(utf16);
}

void CTextBase::GetText(cdstring& all) const
{
	cdustring utf16;
	GetText(utf16);
	all = utf16.ToUTF8();
}

cdstring CTextBase::GetText() const
{
	cdstring result;
	GetText(result);
	return result;
}

void CTextBase::GetText(cdustring& all) const
{
	// Get all the text
	JString16 txt = GetText16();
	all = txt;
}

// Set read only state
void CTextBase::SetReadOnly(bool read_only)
{
	SetType(read_only ? kSelectableText : kFullEditor);
}

#pragma mark ____________________________Font/charset

void CTextBase::ResetFont(const SFontInfo& font, long scale)
{
	StPreserveSelection _save(this);

	SetFontName(font.fontname);
	SetFontSize((JSize)std::max(1.0, pow(1.2, scale) * font.size));
	SetBackgroundColor(GetColormap()->GetDefaultFocusColor());
}

void CTextBase::SetFontName(const JCharacter* name)
{
	SelectAll();
	SetCurrentFontName(name);
	SetDefaultFontName(name);
}

void CTextBase::SetFontSize(const JSize size)
{
	SelectAll();
	SetCurrentFontSize(size);
	SetDefaultFontSize(size);
}

void CTextBase::SetFontStyle(const JFontStyle& style)
{
	SelectAll();
	SetCurrentFontStyle(style);
	SetDefaultFontStyle(style);
}

void CTextBase::SetFont(const JCharacter* name, const
					JSize size, const JFontStyle& style )
{ 
	SelectAll();
	SetCurrentFont(name, size, style);
	SetDefaultFont(name, size, style);
}

void CTextBase::SetBackgroundColor ( const JColorIndex color)
{
	SetBackColor(color);
	SetFocusColor(color);
}

#if __dest_os == __linux_os
const char* cXCharsets[] = {"", "", "iso8859-1", "iso8859-2", "iso8859-3", "iso8859-4",
								"iso8859-5", "iso8859-6", "iso8859-7", "iso8859-8", "iso8859-9",
								"iso8859-10", "iso8859-11", "iso8859-12", "iso8859-13", "iso8859-14", "iso8859-15"};

cdstring CharsetToXFontSpec(ECharset charset)
{
	if (charset <= eISO8859_15)
		return cXCharsets[charset];
	else
	{
		switch(charset)
		{
		case eKOI8_r:
		case eKOI8_ru:
		case eKOI8_u:
			return cXCharsets[eISO8859_5];
		default:;
		}
	}

	return cXCharsets[eUSAscii];
}
#endif

void CTextBase::ResetCharset(ECharset charset, long scale)
{
	// Get current font
	JString			name;
	JSize			size;
	JFontStyle		style;
	AlignmentType	align;
	GetCurrentFont(&name, &size, &style, &align);

	// Convert charset to charset name
	cdstring xcharset = CharsetToXFontSpec(charset);

	// Append charset spec to font name
	JString combined_name = GetFontManager()->CombineNameAndCharacterSet(name, xcharset);

	// Now set in entire control (only change the name, not size or style)
	SetFontName(combined_name);
}

ECharset CTextBase::GetCharset() const
{
	return eUSAscii;
}

void CTextBase::HandleFocusEvent()
{
	// Do inherited
	JXTEBase16::HandleFocusEvent();

	// Dialogs must not grab to returns if this is editable
	if (!IsReadOnly())
	{
		// Now see whether we are in a dialog
		JXWindow* wnd = GetWindow();
		JXDialogDirector* dir = dynamic_cast<JXDialogDirector*>(wnd->GetDirector());
		if (dir)
			wnd->ClearShortcuts(dir->GetOKButton());
	}

	// Make it the commander target
	SetTarget(this);
}

void CTextBase::HandleUnfocusEvent()
{
	// Do inherited
	JXTEBase16::HandleUnfocusEvent();

	// Dialogs must not grab to returns if this is editable
	if (!IsReadOnly())
	{
		// Now see whether we are in a dialog
		JXWindow* wnd = GetWindow();
		JXDialogDirector* dir = dynamic_cast<JXDialogDirector*>(wnd->GetDirector());
		if (dir)
			wnd->InstallShortcuts(dir->GetOKButton(), "^M");
	}

	// Remove the commander target
	//SetTarget(GetSuperCommander());
}

void CTextBase::HandleWindowFocusEvent()
{
	// Do inherited
	JXTEBase16::HandleWindowFocusEvent();

	// Make it the commander target
	SetTarget(this);
}

void CTextBase::HandleWindowUnfocusEvent()
{
	// Do inherited
	JXTEBase16::HandleWindowUnfocusEvent();
}
