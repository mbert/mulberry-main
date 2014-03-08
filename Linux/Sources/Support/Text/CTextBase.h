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


// Header for CTextBase class

#ifndef __CTEXTBASE__MULBERRY__
#define __CTEXTBASE__MULBERRY__

#include <JXTEBase16.h>
#include "CCommander.h"
#include "CContextMenu.h"

#include "CCharSets.h"

#include "cdstring.h"
#include "cdustring.h"

// Classes
class SFontInfo;

class CTextBase : public CCommander,		// Commander must be first so it gets destroyed last
					public JXTEBase16,
					public CContextMenu
{
public:
	class StPreserveSelection
	{
	public:
		StPreserveSelection(CTextBase* txt);
		~StPreserveSelection();

		void Offset(JIndex start, long delta)
			{ if (mSelStart > start) mSelStart += delta;
			  if (mSelEnd > start) mSelEnd += delta; }
	private:
		CTextBase*	mTxt;
		JIndex		mSelStart;
		JIndex		mSelEnd;
	};

	CTextBase(const JCharacter *text,
						JXScrollbarSet* sbs,
						JXContainer* enclosure,
						JXTextMenu *menu,	     
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h,
						Type editorType = kSelectableText);
	CTextBase(JXScrollbarSet* sbs,
						JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType);
	virtual 		~CTextBase();

	virtual void OnCreate(CMainMenu::EContextMenu context);

	virtual void Receive(JBroadcaster* sender, const Message& message);
	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual void HandleKeyPress(const int key, const JXKeyModifiers& modifiers);
	virtual bool HandleChar(const int key, const JXKeyModifiers& modifiers);
								
	// Useful utils to work around JX oddities
	virtual void GetSel(JIndex& sel_start, JIndex& sel_end) const;
	virtual void SetSel(const JIndex& sel_start, const JIndex& sel_end, bool do_copyselect = false);
	virtual void	GetSelectionRange(JIndex& selStart, JIndex& selEnd) const
		{ GetSel(selStart, selEnd); }
	virtual void	SetSelectionRange(JIndex selStart, JIndex selEnd)
		{ SetSel(selStart, selEnd); }

	virtual void	GetSelectedText(cdstring& selected) const;
	virtual void	GetSelectedText(cdustring& selected) const;

			void	SetText(const cdstring& all);						// UTF8 in
			void	SetText(const cdustring& all);						// UTF16 in

			void	InsertUTF8(const cdstring& txt);							// Insert unstyled utf8 at selection
			void	InsertUTF8(const char* txt, size_t len = -1);				// Insert unstyled utf8 at selection
			void	InsertText(const cdustring& txt);							// Insert unstyled utf16 at selection
			void	InsertText(const unichar_t* utxt, size_t len = -1);			// Insert unstyled utf16 at selection

			void	GetText(cdstring& all) const;						// UTF8 out
		cdstring	GetText() const;									// UTF8 out
			void	GetText(cdustring& all) const;						// UTF16 out

	// Font related utils
	virtual void ResetFont(const SFontInfo& font, long scale);
	void SetFontName(const JCharacter* name);
	void SetFontSize(const JSize size);
	void SetFontStyle(const JFontStyle& style);
	void SetFont(const JCharacter* name, const
					JSize size, const JFontStyle& style );
	void SetBackgroundColor(const JColorIndex color);

	// Charset related utils
	virtual void ResetCharset(ECharset charset, long scale);
	virtual ECharset GetCharset() const;

	// Control state
	virtual void	SetReadOnly(bool read_only);					// Set read only state

	virtual void	SetDirty(bool dirty) { mDirty = dirty; } 		// Set dirty state
	virtual bool	IsDirty(void) { return mDirty; }				// Get dirty state

protected:
	bool			mDirty;										// Is text dirty
	bool			mUseCommandClipboard;						// Clipboard ops caused by menu commands

	virtual void	HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	virtual void	HandleFocusEvent();
	virtual void	HandleUnfocusEvent();
	virtual void	HandleWindowFocusEvent();
	virtual void	HandleWindowUnfocusEvent();

	// Special multi-clipboard handling
	virtual Atom		GetClipboardAtom() const;
	virtual void		TEClipboardChanged();
	virtual JBoolean	TEGetExternalClipboard(JString16* text, JRunArray<Font>* style) const;

	// Override to handle iso-8859-1 8-bit characters
	virtual JBoolean	IsCharacterInWord(const JString16& text,
										  const JIndex charIndex) const;

private:
			void	CTextBaseX(const JCharacter* text, 
									 JXTextMenu* menu,
									 JCoordinate w,
									 JCoordinate h);
};

#endif
