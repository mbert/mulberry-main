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

// Header for CSpellOptionsDialog class

#ifndef __CSPELLOPTIONSDIALOG__MULBERRY__
#define __CSPELLOPTIONSDIALOG__MULBERRY__

#include <LDialogBox.h>

// Constants

// Panes
const	PaneIDT		paneid_SpellOptionsDialog = 20300;
const	PaneIDT		paneid_SpellOptionsIgnoreCapitalised = 'ICAP';
const	PaneIDT		paneid_SpellOptionsIgnoreAllCaps = 'ACAP';
const	PaneIDT		paneid_SpellOptionsIgnoreWordsNumbers = 'IWNO';
const	PaneIDT		paneid_SpellOptionsIgnoreMixedCase = 'IMIX';
const	PaneIDT		paneid_SpellOptionsIgnoreDomainNames = 'IDOM';
const	PaneIDT		paneid_SpellOptionsReportDoubledWords = 'RTWO';
const	PaneIDT		paneid_SpellOptionsCaseSensitive = 'CASE';
const	PaneIDT		paneid_SpellOptionsPhoneticSuggestions = 'SPHO';
const	PaneIDT		paneid_SpellOptionsTypoSuggestions = 'STYP';
const	PaneIDT		paneid_SpellOptionsSuggestSplit = 'SSPL';
const	PaneIDT		paneid_SpellOptionsAutoCorrect = 'AUTO';
const	PaneIDT		paneid_SpellOptionsAutoPositionDialog = 'APOS';
const	PaneIDT		paneid_SpellOptionsSpellOnSend = 'CHCK';
const	PaneIDT		paneid_SpellOptionsSpellAsYouType = 'SAYT';
const	PaneIDT		paneid_SpellOptionsSpellColourBackground = 'SCOL';
const	PaneIDT		paneid_SpellOptionsSpellBackgroundColour = 'BCOL';
const	PaneIDT		paneid_SpellOptionsSpellSuggestFast = 'FAST';
const	PaneIDT		paneid_SpellOptionsSpellSuggestMedium = 'MEDM';
const	PaneIDT		paneid_SpellOptionsSpellSuggestSlow = 'SLOW';
const	PaneIDT		paneid_SpellOptionsDictionaries = 'DICT';

// Mesages
const	MessageT	msg_SpellOptionsSpellColourBackground = 'SCOL';

// Resources
const	ResIDT		RidL_CSpellOptionsDialog = 20300;

// Classes
class	LCheckBox;
class	LPopupButton;
class	LRadioButton;
class	LGAColorSwatchControl;
class	CSpellPlugin;

class	CSpellOptionsDialog : public LDialogBox
{
public:
	enum { class_ID = 'SpOp' };

					CSpellOptionsDialog();
					CSpellOptionsDialog(LStream *inStream);
	virtual 		~CSpellOptionsDialog();

	static bool PoseDialog(CSpellPlugin* speller);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	LCheckBox*		mIgnoreCapitalised;
	LCheckBox*		mIgnoreAllCaps;
	LCheckBox*		mIgnoreWordsNumbers;
	LCheckBox*		mIgnoreMixedCase;
	LCheckBox*		mIgnoreDomainNames;
	LCheckBox*		mReportDoubledWords;
	LCheckBox*		mCaseSensitive;
	LCheckBox*		mPhoneticSuggestions;
	LCheckBox*		mTypoSuggestions;
	LCheckBox*		mSuggestSplit;
	LCheckBox*		mAutoCorrect;
	LCheckBox*		mAutoPositionDialog;
	LCheckBox*		mSpellOnSend;
	LCheckBox*		mSpellAsYouType;
	LCheckBox*		mSpellColourBackground;
	LGAColorSwatchControl*	mSpellBackgroundColour;
	LRadioButton*	mSuggestFast;
	LRadioButton*	mSuggestMedium;
	LRadioButton*	mSuggestSlow;
	LPopupButton*	mDictionaries;

			void	SetSpeller(CSpellPlugin* speller);			// Set the speller
			void	GetOptions(CSpellPlugin* speller);			// Update options
			
			void	InitDictionaries(CSpellPlugin* speller);
};

#endif
