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


// Header for CPrefsSpellingSuggestions class

#ifndef __CPrefsSpellingSuggestions__MULBERRY__
#define __CPrefsSpellingSuggestions__MULBERRY__

#include "CPrefsTabSubPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsSpellingSuggestions = 5098;
const	PaneIDT		paneid_PrefsSpellOptionsPhoneticSuggestions = 'SPHO';
const	PaneIDT		paneid_PrefsSpellOptionsTypoSuggestions = 'STYP';
const	PaneIDT		paneid_PrefsSpellOptionsSuggestSplit = 'SSPL';
const	PaneIDT		paneid_PrefsSpellOptionsSpellSuggestFast = 'FAST';
const	PaneIDT		paneid_PrefsSpellOptionsSpellSuggestMedium = 'MEDM';
const	PaneIDT		paneid_PrefsSpellOptionsSpellSuggestSlow = 'SLOW';

// Mesages

// Resources

// Classes
class LCheckBox;
class LRadioButton;

class	CPrefsSpellingSuggestions : public CPrefsTabSubPanel
{
private:
	LCheckBox*		mPhoneticSuggestions;
	LCheckBox*		mTypoSuggestions;
	LCheckBox*		mSuggestSplit;
	LRadioButton*	mSuggestFast;
	LRadioButton*	mSuggestMedium;
	LRadioButton*	mSuggestSlow;

public:
	enum { class_ID = 'SPsu' };

					CPrefsSpellingSuggestions();
					CPrefsSpellingSuggestions(LStream *inStream);
	virtual 		~CPrefsSpellingSuggestions();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	SetData(void* data);				// Set data
	virtual void	UpdateData(void* data);				// Force update of data
};

#endif
