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

// Header for CSpellCheckDialog class

#ifndef __CSPELLCHECKDIALOG__MULBERRY__
#define __CSPELLCHECKDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CPreferenceValue.h"
#include "CSpellPlugin.h"
#include "CWindowStates.h"

// Constants

// Panes
const	PaneIDT		paneid_SpellCheckDialog = 20100;
const	PaneIDT		paneid_SpellCheckDictName = 'DICT';
const	PaneIDT		paneid_SpellCheckScroller = 'HSCR';
const	PaneIDT		paneid_SpellCheckWordTop = 'WTOP';
const	PaneIDT		paneid_SpellCheckWordBottom = 'WBTM';
const	PaneIDT		paneid_SpellCheckList = 'LIST';
const	PaneIDT		paneid_SpellCheckWord = 'WORD';
const	PaneIDT		paneid_SpellCheckSuggestion = 'SUGG';
const	PaneIDT		paneid_SpellCheckAdd = 'ADDW';
const	PaneIDT		paneid_SpellCheckEdit = 'EDIT';
const	PaneIDT		paneid_SpellCheckReplace = 'REPL';
const	PaneIDT		paneid_SpellCheckReplaceAll = 'REPA';
const	PaneIDT		paneid_SpellCheckSkip = 'SKIP';
const	PaneIDT		paneid_SpellCheckSkipAll = 'SKPA';
const	PaneIDT		paneid_SpellCheckSuggestMore = 'MORE';
const	PaneIDT		paneid_SpellCheckOptions = 'OPTS';
const	PaneIDT		paneid_SpellCheckSendNow = 'SEND';
const	PaneIDT		paneid_SpellCheckCancel = 'CANC';

// Mesages
const	MessageT	msg_SpellCheckList = 'LIST';
const	MessageT	msg_SpellCheckHScroll = 'SCRL';
const	MessageT	msg_SpellCheckDblClkList = 'LISD';
const	MessageT	msg_SpellCheckWord = 'WORD';
const	MessageT	msg_SpellCheckSuggestion = 'SUGG';
const	MessageT	msg_SpellCheckAdd = 'ADDW';
const	MessageT	msg_SpellCheckEdit = 'EDIT';
const	MessageT	msg_SpellCheckReplace = 'REPL';
const	MessageT	msg_SpellCheckReplaceAll = 'REPA';
const	MessageT	msg_SpellCheckSkip = 'SKIP';
const	MessageT	msg_SpellCheckSkipAll = 'SKPA';
const	MessageT	msg_SpellCheckSuggestMore = 'MORE';
const	MessageT	msg_SpellCheckOptions = 'OPTS';
const	MessageT	msg_SpellCheckSendNow = 'SEND';

// Resources
const	ResIDT		RidL_CSpellCheckDialogBtns = 20100;

// Classes
class	CDictionaryPageScroller;
class	CStaticText;
class	LCheckBox;
class	CTextFieldX;
class	LPushButton;
class	LRadioButton;
class	CTextTable;
class	LWindow;
class	CTextDisplay;

class	CSpellCheckDialog : public LDialogBox {

	friend class CDictionaryPageScroller;

public:
	static CPreferenceValue<CWindowState>*	sWindowState;

private:
	CStaticText*		mDictName;
	CDictionaryPageScroller*	mScroller;
	CStaticText*		mWordTop;
	CStaticText*		mWordBottom;
	CTextTable*			mList;
	CStaticText*		mWord;
	CTextFieldX*		mSuggestion;
	LPushButton*		mAdd;
	LPushButton*		mEdit;
	LPushButton*		mReplace;
	LPushButton*		mReplaceAll;
	LPushButton*		mSkip;
	LPushButton*		mSkipAll;
	LPushButton*		mSuggestMore;
	LPushButton*		mOptions;
	LPushButton*		mSendNow;
	LPushButton*		mCancel;

	CSpellPlugin*		mSpeller;
	LWindow*			mWindow;
	Rect				mWindowBounds;
	bool				mWindowMoved;
	CTextDisplay*		mText;
	long				mTextSelStart;
	cdstring*			mCheckText;
	long				mSuggestionCtr;
	bool				mDone;

public:
	enum { class_ID = 'SpDl' };

					CSpellCheckDialog();
					CSpellCheckDialog(LStream *inStream);
	virtual 		~CSpellCheckDialog();


protected:
	virtual void	FinishCreateSelf();					// Do odds & ends

public:
	virtual Boolean	HandleKeyPress(const EventRecord &inKeyEvent);
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

			bool	IsDone()
				{ return mDone; }

			void	SetSpeller(CSpellPlugin* speller,
								LWindow* wnd,
								CTextDisplay* text,
								long start,
								cdstring* check,
								bool sending);				// Set the speller & text

			void	RotateDefault();							// Rotate default button

			void	NextError();								// Go to next error
			void	SelectWord(const CSpellPlugin::SpError* sperr);		// Do visual select of word
			void	ReplaceWord(const CSpellPlugin::SpError* sperr);	// Do visual replace of word
			
			void	DoAdd();								// Add word to dictionary
			void	DoEdit();								// Remove word from dicitonary

			void	DoReplace();							// Replace word
			void	DoReplaceAll();							// Replace all words

			void	DoSkip();								// Skip error
			void	DoSkipAll();							// Skip all errors

			void	DoSuggestMore();						// Options dialog
			void	DoOptions();							// Options dialog

			void	DisplaySuggestions();					// Display suggestions in list
			void	DisplayDictionary(bool select,
									bool force_draw = false);	// Display dictionary in list

			void	SetSuggestion(char* suggest);				// Set suggestion field

private:
	virtual void	ResetState();						// Reset state from prefs
	virtual void	SaveState();						// Save state in prefs

};

#endif
