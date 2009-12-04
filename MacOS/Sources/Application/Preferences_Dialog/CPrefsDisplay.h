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


// Header for CPrefsDisplay class

#ifndef __CPREFSDISPLAY__MULBERRY__
#define __CPREFSDISPLAY__MULBERRY__

#include "CPrefsPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PrefsDisplay = 5002;
const	PaneIDT		paneid_DisplayTabs = 'TABS';
const	PaneIDT		paneid_DisplayAllowKeyboardShortcuts = 'KEYS';
const	PaneIDT		paneid_DisplayResetBtn = 'RSET';

// Panes
const	MessageT	msg_DisplayTabs = 'TABS';
const	MessageT	msg_ResetWindows = 'RSET';

// Resources
const	ResIDT		RidL_CPrefsDisplayBtns = 5002;

// Classes
class CPrefsSubTab;
class LCheckBox;

class CPrefsDisplay : public CPrefsPanel,
						public LListener
{
private:
	CPrefsSubTab*		mTabs;
	LCheckBox*			mAllowKeyboardShortcuts;

public:
	enum { class_ID = 'Pdsp' };

					CPrefsDisplay();
					CPrefsDisplay(LStream *inStream);
	virtual 		~CPrefsDisplay();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs

};

#endif
