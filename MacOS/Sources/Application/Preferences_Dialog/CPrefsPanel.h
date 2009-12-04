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


// Header for CPrefsPanel class

#ifndef __CPREFSPANEL__MULBERRY__
#define __CPREFSPANEL__MULBERRY__


// Constants

// Panes
const	ClassIDT	class_PrefsPanel = 'Ppan';

// Classes
class CPreferences;

class CPrefsPanel : public LView
{
public:
	enum { class_ID = class_PrefsPanel };

					CPrefsPanel() { mCopyPrefs = nil; }
					CPrefsPanel(LStream *inStream) : LView(inStream) { mCopyPrefs = nil; }
	virtual 		~CPrefsPanel() {}

	virtual void	ToggleICDisplay(void) = 0;				// Toggle display of IC - pure virtual

	virtual void	SetPrefs(CPreferences* copyPrefs) = 0;	// Set prefs - pure virtual
															// so must override
	virtual void	UpdatePrefs(void) = 0;					// Force update of prefs

	virtual CPreferences* GetCopyPrefs(void)				// Get local copy
		{ return mCopyPrefs; }

protected:
	CPreferences*	mCopyPrefs;
};

#endif
