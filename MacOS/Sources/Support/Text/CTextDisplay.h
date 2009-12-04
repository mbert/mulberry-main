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

#include "cdstring.h"
#include "cdustring.h"

// Classes

class	cdustring;

class CTextDisplay : public CTextBase,
						public CBroadcaster
{

	friend class CFindReplaceWindow;
	friend class CSpellPlugin;

protected:
	short					mSpacesPerTab;								// Number of spaces to use in place of TAB
	short					mWrap;										// Size to hard wrap to
	bool					mHardWrap;									// Whether to hard wrap
	cdstring				mQuotation;									// String to use/recognise as prefix
	bool					mTabSelectAll;								// Tab selects all
	bool					mFindAllowed;								// Allowed to do find & replace
	static CTextDisplay*	sTextClicking;								// The text display being click looped
	TextTraitsRecord		mTextTraits;								// Text traits for general text

	static MenuHandle		sSpellChanger;								// Spell replace popup

public:
	enum { class_ID = 'TeDi' };

	enum
	{
		eBroadcast_Activate = 'TXac',
		eBroadcast_Deactivate = 'TXda',
		eBroadcast_SelectionChanged = 'TXsc'
	};

	class StPreserveSelection
	{
	public:
		StPreserveSelection(CTextDisplay* text)
			{ mText = text;
			  mText->GetSelection(&selStart, &selEnd); }
		~StPreserveSelection()
			{ mText->SetSelection(selStart, selEnd); }
		void Offset(SInt32 start, SInt32 delta)
			{ if (selStart > start) selStart += delta;
			  if (selEnd > start) selEnd += delta; }
	private:
		CTextDisplay* mText;
		SInt32 selStart;
		SInt32 selEnd;
	};

					CTextDisplay(LStream *inStream);
	virtual 		~CTextDisplay();
	
protected:
	virtual void		FinishCreateSelf(void);							// Setup D&D

public:
	virtual Boolean	HandleKeyPress(const EventRecord& inKeyEvent);		// Handle keys our way
	virtual Boolean	HandleKeyPress(const EventRecord& inKeyEvent, const UniChar* uc, UniCharCount ucount);
	virtual	Boolean ObeyCommand(CommandT inCommand,
								void* ioParam);							// Handle commands our way
	virtual void	FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							UInt16 &outMark, Str255 outName);			// Handle menus our way
	virtual bool	HandleContextMenuEvent(const EventRecord& cmmEvent);

protected:
	virtual void	BeTarget();
	virtual void	DontBeTarget();

	virtual void	Click(SMouseDownEvent &inMouseDown);		// Allow background window D&D
	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);

	// Text processing commands
	virtual void	OnWrapLines();
	virtual void	OnUnwrapLines();
	virtual void	OnQuoteLines();
	virtual void	OnUnquoteLines();
	virtual void	OnRequoteLines();
	virtual void	OnShiftLeft();
	virtual void	OnShiftRight();

	virtual bool	ExpandMacro();

	virtual void	PrepareTextProcessing(SInt32& sel_start, SInt32& sel_end, cdstring& selected);
	virtual void	EndTextProcessing(const char* insert_text, SInt32& sel_start, SInt32& sel_end);
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
		long cursorPos;			// Last know cursor position
		long charCount;			// used to detect insertions
		long wordStartPos;		// current word starting position
		long contextWordPos;	// position of context menu word
		bool busy;				// TRUE if already processing events
		bool enabled;			// TRUE if real-time speller enabled
		bool pending;			// TRUE if spell check text is pending
	};

	public:
			friend class StPauseSpelling;
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

			void	SpellAutoCheck(bool auto_check);
			bool	IsMisspelled(SInt32 sel_start, SInt32 sel_end) const;
			long	GetWordAt(long pos, cdustring& word, long* startPos, bool word_start) const;
			unichar_t GetTextChar(long pos) const;
			long	GetSpellTextRange(long start, long end, cdustring& text) const;
			bool	GetMisspelled(SInt32 pos, cdustring& word, SInt32& word_start) const;
			void	MarkMisspelled(SInt32 sel_start, SInt32 sel_end, bool misspelled);
			bool	CheckWord(const cdustring& word, SInt32 start);
			void	RecheckAllWords(const cdustring& word);
			void	SpellInsertText(long start, long numchars);
			void	SpellTextRequired()
				{ mRTSpell.pending = true; }
			void	SpellTextRequiredCheck()
				{ if (mRTSpell.pending) SpellTextChange(); }
			void	SpellTextChange();
			bool	PauseSpelling(bool pause);

public:
	virtual void	FixSelectionDisplay(void);							// Make sure selection is visible

protected:

public:
	virtual Boolean	FocusDraw(LPane* inSubPane = nil);					// Handle local text traits
	virtual void	SetTextTraits(const TextTraitsRecord& aTextTrait);	// Set text traits for general text
	virtual void	SetFont(const TextTraitsRecord& aTextTrait);		// Set font text traits for general text

	virtual void 	WipeText(const TextTraitsRecord& aTextTrait);				// Delete text and set traits

	virtual void	SetSpacesPerTab(short numSpacesPerTab);				// Set spaces per tab
	virtual short	GetSpacesPerTab(void) const
						{return 4;}										// Get spaces per tab

	virtual void	SetWrap(short wrap);								// Set wrap width
	virtual short	GetWrap(void) const
						{return mWrap;}									// Get wrap width

	virtual void	SetHardWrap(bool hard_wrap);						// Set wrapping mode
	virtual void	ResetHardWrap();									// Reset wrapping mode
	virtual bool	GetHardWrap(void) const
						{return mHardWrap;}								// Get wrapping mode
			long	MeasureText(const char* txt);

	virtual void	SetQuotation(const cdstring& quotation)				// Set quotation
		{ mQuotation = quotation; }
	virtual const cdstring&	GetQuotation(void) const					// Get quotation
		{ return mQuotation; }

	virtual void	SetTabSelectAll(bool tab_select_all)
						{ mTabSelectAll = tab_select_all; }				// Set tab select all
	virtual bool	GetTabSelectAll(void) const {return mTabSelectAll;}	// Get tab select all

	virtual void	SetFindAllowed(bool find_allowed)
						{ mFindAllowed = find_allowed; }				// Set find allowed
	virtual bool	GetFindAllowed(void) const							// Get find allowed
		{ return mFindAllowed; }

			void	GetTextList(cdstrvect& all) const;
			void	SetTextList(const cdstrvect& all);

			 void	FindLine(SInt32 offset, SInt32* start, SInt32* end) const;
			 void	FindLineRange(unsigned long line, SInt32* start, SInt32* end) const;

			void	GetScrollPos(long& h, long& v) const;			// Get scrollbar pos
			void	SetScrollPos(long h, long v);					// Set scrollbar pos

	virtual void	UserChangedText(void);								// Force dirty

private:
	RTSpell			mRTSpell;
};

class StFocusTE {
public:
					StFocusTE(LPane *inPane);
					~StFocusTE();
private:
	RgnHandle		mClipRgn;
};

#endif
