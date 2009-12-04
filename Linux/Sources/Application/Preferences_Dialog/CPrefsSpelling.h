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


// CPrefsSpelling.h : header file
//

#ifndef __CPrefsSpelling__MULBERRY__
#define __CPrefsSpelling__MULBERRY__

#include "CPrefsPanel.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpelling dialog

class CSpellPlugin;
class CTabController;
class JXTextButton;
class JXTextCheckbox;

class CPrefsSpelling : public CPrefsPanel
{
// Construction
public:
	CPrefsSpelling(JXContainer* enclosure,
							 const HSizingOption hSizing, const VSizingOption vSizing,
							 const JCoordinate x, const JCoordinate y,
							 const JCoordinate w, const JCoordinate h);

	virtual void OnCreate();

	virtual void SetPrefs(CPreferences* prefs);				
	virtual void UpdatePrefs(CPreferences* prefs);		

protected:
// begin JXLayout1

    CTabController* mTabs;
    HPopupMenu*     mDictionaries;

// end JXLayout1
	bool			mHasDictionaries;
			
	void	SetSpeller(CSpellPlugin* speller);			// Set the speller
	void	GetOptions(CSpellPlugin* speller);			// Update options

	void	InitDictionaries(CSpellPlugin* speller);
};

#endif
