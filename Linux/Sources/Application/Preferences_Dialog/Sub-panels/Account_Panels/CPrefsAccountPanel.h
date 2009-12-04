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

#include <JXWidgetSet.h>

// Classes
class CPreferences;
class CTabController;

class CPrefsAccountPanel : public JXWidgetSet
{
public:
	CPrefsAccountPanel(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: JXWidgetSet(enclosure, hSizing, vSizing, x, y, w, h) {}

	virtual void	OnCreate();
	virtual void	SetState(unsigned long state)
		{ mState = state; }

	virtual void	SetData(void* data);			// Set data
	virtual void	SetPrefs(CPreferences* prefs)	// Set data
		{ mCopyPrefs = prefs; }
	virtual void	UpdateData(void* data);			// Force update of data

protected:
	unsigned long mState;
	CTabController* mTabs;
	CPreferences* mCopyPrefs;

	virtual void	InitTabs() = 0;
};

#endif
