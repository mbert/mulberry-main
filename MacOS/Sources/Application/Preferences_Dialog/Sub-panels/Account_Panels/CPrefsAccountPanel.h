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


// Header for CPrefsAccountPanel class

#ifndef __CPREFSACCOUNTPANEL__MULBERRY__
#define __CPREFSACCOUNTPANEL__MULBERRY__


// Constants

// Panes
const	PaneIDT		paneid_PrefsAccountTabs = 'TABS';

// Mesages
const	MessageT	msg_PrefsAccountTabs = 'TABS';

// Classes
class CPrefsSubTab;
class CPreferences;

class CPrefsAccountPanel : public LView, public LListener
{
public:
					CPrefsAccountPanel();
					CPrefsAccountPanel(LStream *inStream);
	virtual 		~CPrefsAccountPanel();

	virtual void	SetState(unsigned long state)
		{ mState = state; }

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	ToggleICDisplay(bool IC_on);	// Toggle display of IC
	virtual void	SetData(void* data);			// Set data
	virtual void	SetPrefs(CPreferences* prefs)	// Set data
		{ mCopyPrefs = prefs; }
	virtual void	UpdateData(void* data);			// Force update of data

protected:
	unsigned long mState;
	CPrefsSubTab* mTabs;
	CPreferences* mCopyPrefs;

	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	InitTabs() = 0;
};

#endif
