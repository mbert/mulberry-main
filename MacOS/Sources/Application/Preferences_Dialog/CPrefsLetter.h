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


// Header for CPrefsLetter class

#ifndef __CPREFSLETTER__MULBERRY__
#define __CPREFSLETTER__MULBERRY__

#include "CPrefsPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsLetter = 5005;
const	PaneIDT		paneid_LetterTabs = 'TABS';

// Resources
const	ResIDT		RidL_CPrefsLetterBtns = 5005;

// Mesages
const	MessageT	msg_LetterTabs = 'TABS';

// Classes
class CPrefsSubTab;

class	CPrefsLetter : public CPrefsPanel,
						public LListener
{
private:
	CPrefsSubTab*		mTabs;

public:
	enum { class_ID = 'Pltr' };

					CPrefsLetter();
					CPrefsLetter(LStream *inStream);
	virtual 		~CPrefsLetter();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs
};

#endif
