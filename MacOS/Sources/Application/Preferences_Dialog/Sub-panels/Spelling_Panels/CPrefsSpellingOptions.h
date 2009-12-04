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


// Header for CPrefsSpellingOptions class

#ifndef __CPrefsSpellingOptions__MULBERRY__
#define __CPrefsSpellingOptions__MULBERRY__

#include "CPrefsTabSubPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsSpellingOptions = 5097;
const	PaneIDT		paneid_PrefsSpellOptionsCaseSensitive = 'CASE';
const	PaneIDT		paneid_PrefsSpellOptionsIgnoreCapitalised = 'ICAP';
const	PaneIDT		paneid_PrefsSpellOptionsIgnoreAllCaps = 'ACAP';
const	PaneIDT		paneid_PrefsSpellOptionsIgnoreWordsNumbers = 'IWNO';
const	PaneIDT		paneid_PrefsSpellOptionsIgnoreMixedCase = 'IMIX';
const	PaneIDT		paneid_PrefsSpellOptionsIgnoreDomainNames = 'IDOM';
const	PaneIDT		paneid_PrefsSpellOptionsReportDoubledWords = 'RTWO';
const	PaneIDT		paneid_PrefsSpellOptionsAutoCorrect = 'AUTO';
const	PaneIDT		paneid_PrefsSpellOptionsAutoPositionDialog = 'APOS';
const	PaneIDT		paneid_PrefsSpellOptionsSpellOnSend = 'CHCK';
const	PaneIDT		paneid_PrefsSpellOptionsSpellAsYouType = 'SAYT';
const	PaneIDT		paneid_PrefsSpellOptionsSpellColourBackground = 'SCOL';
const	PaneIDT		paneid_PrefsSpellOptionsSpellBackgroundColour = 'BCOL';

// Mesages
const	MessageT	msg_PrefsSpellOptionsSpellColourBackground = 'SCOL';

// Resources
const	ResIDT		RidL_CPrefsSpellingOptions = 5097;

// Classes
class LCheckBox;
class LGAColorSwatchControl;

class	CPrefsSpellingOptions : public CPrefsTabSubPanel,
								public LListener
{
private:
	LCheckBox*		mCaseSensitive;
	LCheckBox*		mIgnoreCapitalised;
	LCheckBox*		mIgnoreAllCaps;
	LCheckBox*		mIgnoreWordsNumbers;
	LCheckBox*		mIgnoreMixedCase;
	LCheckBox*		mIgnoreDomainNames;
	LCheckBox*		mReportDoubledWords;
	LCheckBox*		mAutoCorrect;
	LCheckBox*		mAutoPositionDialog;
	LCheckBox*		mSpellOnSend;
	LCheckBox*		mSpellAsYouType;
	LCheckBox*		mSpellColourBackground;
	LGAColorSwatchControl*	mSpellBackgroundColour;

public:
	enum { class_ID = 'SPop' };

					CPrefsSpellingOptions();
					CPrefsSpellingOptions(LStream *inStream);
	virtual 		~CPrefsSpellingOptions();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data
};

#endif
