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

#include "CDialogDirector.h"

#include "CPreferenceValue.h"
#include "CSpellPlugin.h"
#include "CStaticText.h"
#include "CWindowStates.h"

// Classes
class	CStaticText;
class	CTextDisplay;
class	CTextTable;
class	CTextInputField;
class	JXTextButton;
class	JXWindowDirector;

class CSpellCheckDialog : public CDialogDirector
{
public:
	static CPreferenceValue<CWindowState>*	sWindowState;

					CSpellCheckDialog(JXDirector* supervisor);
	virtual 		~CSpellCheckDialog();

	void OnCreate();

protected:
	virtual void Receive(JBroadcaster* sender, const Message& message);

public:
	virtual void	Activate();
			void	SetSpeller(CSpellPlugin* speller,
								JXWindowDirector* wnd,
								CTextDisplay* text,
								long start,
								cdstring* check,
								bool sending);				// Set the speller & text

			void	RotateDefault(void);							// Rotate default button

			void	NextError(void);								// Go to next error
			void	SelectWord(const CSpellPlugin::SpError* sperr);		// Do visual select of word
			void	ReplaceWord(const CSpellPlugin::SpError* sperr);	// Do visual replace of word
			
			void	DoAdd(void);								// Add word to dictionary

			void	DoReplace(void);							// Replace word
			void	DoReplaceAll(void);							// Replace all words

			void	DoSkip(void);								// Skip error
			void	DoSkipAll(void);							// Skip all errors

			void	DoOptions(void);							// Options dialog

			void	DisplaySuggestions(void);					// Display suggestions in list
			void	DisplayDictionary(bool select,
									bool force_draw = false);	// Display dictionary in list

			void	SetSuggestion(char* suggest);				// Set suggestion field

private:
// begin JXLayout

    CTextInputField* mSuggestion;
    JXTextButton*    mSendNow;
    JXTextButton*    mCancelBtn;
    JXTextButton*    mSkip;
    JXTextButton*    mAdd;
    JXTextButton*    mOptions;
    JXTextButton*    mReplace;
    JXTextButton*    mReplaceAll;
    JXTextButton*    mSkipAll;
    CStaticText*     mWord;

// end JXLayout

	CTextTable*			mList;
	CSpellPlugin*		mSpeller;
	JXWindowDirector*	mWindow;
	JPoint				mWindowPos;
	bool				mWindowMoved;
	CTextDisplay*		mText;
	long				mTextSelStart;
	cdstring*			mCheckText;
	bool				mSuggestions;
	bool				mExtendedGuess;

	virtual void	ResetState(void);						// Reset state from prefs
	virtual void	SaveState(void);						// Save state in prefs

};

#endif
