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


// CDisplayPreferences.h : header file
//

#ifndef __CPREFSDISPLAY__MULBERRY__
#define __CPREFSDISPLAY__MULBERRY__

#include "CPrefsPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplay dialog

class CTabController;
class JXTextButton;
class JXTextCheckbox;

class CPrefsDisplay : public CPrefsPanel
{
// Construction
public:
	CPrefsDisplay(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
// begin JXLayout1

    CTabController* mTabs;
    JXTextButton*   mDisplayResetBtn;
    JXTextCheckbox* mAllowKeyboardShortcuts;

// end JXLayout1

	virtual void Receive(JBroadcaster* sender, const Message& message);

	void OnDisplayReset();
};

#endif
