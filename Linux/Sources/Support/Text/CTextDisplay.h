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


// Header for CTextDisplay class

#ifndef __CTEXTDISPLAY__MULBERRY__
#define __CTEXTDISPLAY__MULBERRY__

#include "CTextBase.h"
#include "CBroadcaster.h"

#include "HPopupMenu.h"

// Classes

class CTextDisplay : public CTextBase,
						public CBroadcaster
{
	friend class CFindReplaceWindow;
	friend class CSpellCheckDialog;
	friend class CSpellPlugin;

public:
	enum
	{
		eBroadcast_Activate = 'TXac',
		eBroadcast_Deactivate = 'TXda',
		eBroadcast_SelectionChanged = 'TXsc'
	};

	CTextDisplay(const JCharacter *text, JXContainer* enclosure,
						 JXTextMenu *menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	CTextDisplay(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	CTextDisplay(JXScrollbarSet* sbs, JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kSelectableText);
	virtual 		~CTextDisplay();

	virtual void	OnCreate();

	JXScrollbarSet* GetScroller()
		{ return mSbs; }

	virtual void 	Activate();
	virtual void	Deactivate();

	virtual void 	HandleFocusEvent();
	virtual void 	HandleUnfocusEvent();
	virtual void	HandleWindowFocusEvent();
	virtual void	HandleWindowUnfocusEvent();

	virtual bool	HandleChar(const int key, const JXKeyModifiers& modifiers);

	virtual bool	ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void	UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
								
	virtual void	SetSpacesPerTab(short numSpacesPerTab);				// Set spaces per tab
	virtual short	GetSpacesPerTab(void) const
						{return mSpacesPerTab;}							// Get spaces per tab

	virtual void	SetWrap(short wrap);								// Set wrap width
	virtual short	GetWrap(void) const
						{return mWrap;}									// Get wrap width

	virtual void	SetHardWrap(bool hard_wrap);						// Set wrapping mode
	virtual void	ResetHardWrap();									// Reset wrapping mode
	virtual bool	GetHardWrap(void) const
						{return mHardWrap;}								// Get wrapping mode

	virtual void	SetQuotation(const cdstring& quotation)				// Set quotation
		{ mQuotation = quotation; }
	virtual const cdstring&	GetQuotation(void) const					// Get quotation
		{ return mQuotation; }

	virtual void	SetTabSelectAll(bool tab_select_all)
						{ mTabSelectAll = tab_select_all; }				// Set tab select all
	virtual bool	GetTabSelectAll(void) const {return mTabSelectAll;}	// Get tab select all

	virtual void	SetWrapAllowed(bool wrap_allowed)
						{ mWrapAllowed = wrap_allowed; }				// Set wrap allowed
	virtual bool	GetWrapAllowed(void) const							// Get wrap allowed
		{ return mWrapAllowed; }

	virtual void	SetFindAllowed(bool find_allowed)
						{ mFindAllowed = find_allowed; }				// Set find allowed
	virtual bool	GetFindAllowed(void) const							// Get find allowed
		{ return mFindAllowed; }

			void	AllowStyles(bool styles)							// Turn on/off styles
		{ ShouldPasteStyledText(JBoolean(styles)); }

			void	GetTextList(cdstrvect& all) const;
			void	SetTextList(const cdstrvect& all);

			void	GetScrollPos(long& h, long& v) const;				// Get scrollbar pos
			void	SetScrollPos(long h, long v);						// Set scrollbar pos

protected:
	JXScrollbarSet*			mSbs;
	short					mSpacesPerTab;								// Number of spaces to use in place of TAB
	short					mWrap;										// Size to hard wrap to
	bool					mHardWrap;									// Whether to hard wrap
	cdstring				mQuotation;									// String to use/recognise as prefix
	bool					mTabSelectAll;								// Tab selects all
	bool					mWrapAllowed;								// Allowed to wrap/shift etc
	bool					mFindAllowed;								// Allowed to do find & replace

	virtual void	Receive(JBroadcaster* sender, const Message& message);
	virtual void 	ContextEvent(const JPoint& pt,
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);

	// Text processing commands
	virtual void	OnWrapLines();
	virtual void	OnUnwrapLines();
	virtual void	OnQuoteLines();
	virtual void	OnUnquoteLines();
	virtual void	OnRequoteLines();
	virtual void	OnShiftLeft();
	virtual void	OnShiftRight();

	virtual bool	ExpandMacro();

	virtual void	PrepareTextProcessing(JIndex& sel_start, JIndex& sel_end, cdstring& selected);
	virtual void	EndTextProcessing(const char* insert_text, JIndex& sel_start, JIndex& sel_end);
	virtual void	SelectFullLines();

	// Find & Replace commands
	virtual void	OnFindText();
	virtual void	OnFindNextText(bool backwards);
	virtual void	OnFindSelectionText(bool backwards);
	virtual void	OnReplaceText();
	virtual void	OnReplaceFindText(bool backwards);
	virtual void	OnReplaceAllText();

	virtual bool	DoFindNextText(bool backwards, bool silent = false);
	virtual void	DoFindSelectionText(bool backwards);
	virtual void	DoReplaceText();
	virtual void	DoReplaceFindText(bool backwards);
	virtual void	DoReplaceAllText();

	// Spelling commands

	// Real-time spelling checker state information:
	struct RTSpell
	{
		// Current state
		enum ESpellState {eNotInWord, eInUnmarkedWord, eInMarkedWord};
		ESpellState state;
		JIndex cursorPos;			// Last know cursor position
		JIndex charCount;			// used to detect insertions
		JIndex wordStartPos;		// current word starting position
		JIndex contextWordPos;	// position of context menu word
		bool busy;				// TRUE if already processing events
		bool enabled;			// TRUE if real-time speller enabled
		bool pending;			// TRUE if spell check text is pending
	};

			class StPauseSpelling
			{
			public:
				StPauseSpelling(CTextDisplay* text)
					{ mText = text;
					  mWasEnabled = mText->PauseSpelling(true); }
				~StPauseSpelling()
					{ if (mWasEnabled) mText->PauseSpelling(false); }
			private:
				CTextDisplay*	mText;
				bool			mWasEnabled;
			};
			friend class StPauseSpelling;

			void	SpellAutoCheck(bool auto_check);
			void	SpellContext(JIndex item);
			bool	IsMisspelled(JIndex sel_start, JIndex sel_end) const;
			JSize	GetWordAt(JIndex pos, cdustring& word, JIndex* startPos, bool word_start) const;
			unichar_t GetTextChar(JIndex pos) const;
			JSize	GetSpellTextRange(JIndex start, JIndex end, cdustring& text) const;
			bool	GetMisspelled(JIndex pos, cdustring& word, JIndex& word_start) const;
			void	MarkMisspelled(JIndex sel_start, JIndex sel_end, bool misspelled);
			bool	CheckWord(const cdustring& word, JIndex start);
			void	RecheckAllWords(const cdustring& word);
			void	SpellInsertText(JIndex start, JSize numchars);
			void	SpellTextRequired()
				{ mRTSpell.pending = true; }
			void	SpellTextRequiredCheck()
				{ if (mRTSpell.pending) SpellTextChange(); }
			void	SpellTextChange();
			bool	PauseSpelling(bool pause);

private:
	bool*			mDeleted;
	RTSpell			mRTSpell;
	HPopupMenu*		mSpellPopup;
	cdstring		mSpellContextWord;
	cdstrvect		mSpellSuggestions;

			void	CTextDisplayX();
};

// Version that is a full text editor - handy when doing jxlayout
class CTextInputDisplay : public CTextDisplay
{
public:
	CTextInputDisplay(const JCharacter *text, JXContainer* enclosure,
						 JXTextMenu *menu,	     
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kFullEditor)
	: CTextDisplay(text, enclosure, menu, hSizing, vSizing, x, y, w, h, editorType) {}
	CTextInputDisplay(JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kFullEditor)
	: CTextDisplay(enclosure, hSizing, vSizing, x, y, w, h, editorType) {}
	CTextInputDisplay(JXScrollbarSet* sbs, JXContainer* enclosure,
						 const HSizingOption hSizing, const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h,
						 Type editorType = kFullEditor)
	: CTextDisplay(sbs, enclosure, hSizing, vSizing, x, y, w, h, editorType) {}
};

// Dummy class for cross-platform support
// Unix does not have to do this as redraw won't happen?
class StStopRedraw
{
public:
	StStopRedraw(CTextDisplay* view)
		{ }
	~StStopRedraw()
		{ }
};

#endif
