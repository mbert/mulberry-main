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
#include "CTabController.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsSpelling dialog

class CPreferences;
class CSpellPlugin;

class CPrefsSpelling : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsSpelling)

// Construction
public:
	CPrefsSpelling();
	~CPrefsSpelling();

// Dialog Data
	//{{AFX_DATA(CPrefsSpelling)
	enum { IDD = IDD_PREFS_SPELLING };
	CTabController	mTabs;
	//}}AFX_DATA
	CPopupButton	mDictionaries;
	UINT			mDictionariesValue;
	cdstring		mDictionariesName;
	CSpellPlugin*	mSpeller;

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsSpelling)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void	InitDictionaries();

	// Generated message map functions
	//{{AFX_MSG(CPrefsSpelling)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnDictionaryPopup(UINT nID);
	afx_msg void OnSelChangeSpellingTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

};

#endif
