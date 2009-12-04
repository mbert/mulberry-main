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


// CPrefsPanel.h : header file
//

#ifndef __CPREFSPANEL__MULBERRY__
#define __CPREFSPANEL__MULBERRY__

#include "CHelpPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsPanel dialog

class CPreferences;

class CPrefsPanel : public CHelpPropertyPage
{
	DECLARE_DYNCREATE(CPrefsPanel)

// Construction
public:
	CPrefsPanel();
	CPrefsPanel(UINT nID);
	virtual ~CPrefsPanel();

// Dialog Data
	//{{AFX_DATA(CPrefsPanel)
	//}}AFX_DATA

	virtual void SetPrefs(CPreferences* prefs)				// Set up params for DDX
		{ mCopyPrefs = prefs; }
	virtual void ResetPrefs(CPreferences* prefs);			// Set up params for DDX
	virtual void UpdatePrefs(CPreferences* prefs) {}			// Get params from DDX
	virtual CPreferences* GetCopyPrefs(void)				// Get local copy
		{ return mCopyPrefs; }

// Implementation
protected:
	bool mInited;
	CPreferences* mCopyPrefs;

	// Generated message map functions
	//{{AFX_MSG(CPrefsPanel)
	virtual BOOL OnInitDialog();
	virtual void InitControls(void) {}
	virtual void SetControls(void) {}
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
