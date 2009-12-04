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


// Header for CPrefsSpelling class

#ifndef __CPrefsSpelling__MULBERRY__
#define __CPrefsSpelling__MULBERRY__

#include "CPrefsPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsSpelling = 5015;
const	PaneIDT		paneid_SpellingTabs = 'TABS';
const	PaneIDT		paneid_SpellingDictionary = 'DICT';

// Panes
const	MessageT	msg_SpellingTabs = 'TABS';

// Resources
const	ResIDT		RidL_CPrefsSpellingBtns = 5015;

// Classes
class CSpellPlugin;
class CPrefsSubTab;
class LPopupButton;

class CPrefsSpelling : public CPrefsPanel,
						public LListener
{
private:
	CPrefsSubTab*		mTabs;
	LPopupButton*		mDictionaries;

public:
	enum { class_ID = 'Pspl' };

					CPrefsSpelling();
					CPrefsSpelling(LStream *inStream);
	virtual 		~CPrefsSpelling();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

protected:

			void	SetSpeller(CSpellPlugin* speller);			// Set the speller
			void	GetOptions(CSpellPlugin* speller);			// Update options
			
			void	InitDictionaries(CSpellPlugin* speller);

};

#endif
