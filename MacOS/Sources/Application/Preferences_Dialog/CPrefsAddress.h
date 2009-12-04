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


// Header for CPrefsAddress class

#ifndef __CPREFSADDRESS__MULBERRY__
#define __CPREFSADDRESS__MULBERRY__

#include "CPrefsPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PrefsAddress = 5006;
const	PaneIDT		paneid_AddressOptionKeyDown = 'OPDW';
const	PaneIDT		paneid_AddressOptionKeyUp = 'OPUP';
const	PaneIDT		paneid_AddressTabs = 'TABS';

// Mesages
const	MessageT	msg_AddressTabs = 'TABS';

// Resources
const	ResIDT		RidL_CPrefsAddressBtns = 5006;

// Classes
class CPrefsSubTab;
class LRadioButton;

class	CPrefsAddress : public CPrefsPanel,
						public LListener
{
private:
	CPrefsSubTab*		mTabs;
	LRadioButton*		mOptionKeyDown;
	LRadioButton*		mOptionKeyUp;

public:
	enum { class_ID = 'Padr' };

					CPrefsAddress();
					CPrefsAddress(LStream *inStream);
	virtual 		~CPrefsAddress();


protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

public:
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(void);					// Toggle display of IC - pure virtual
	virtual void	SetPrefs(CPreferences* copyPrefs);		// Set prefs
	virtual void	UpdatePrefs(void);						// Force update of prefs
};

#endif
