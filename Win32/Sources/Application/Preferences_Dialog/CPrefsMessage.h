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


// CPrefsMessage.h : header file
//

#ifndef __CPREFSMESSAGE__MULBERRY__
#define __CPREFSMESSAGE__MULBERRY__

#include "CPrefsPanel.h"
#include "CTabController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessage dialog

class CPreferences;

class CPrefsMessage : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsMessage)

// Construction
public:
	CPrefsMessage();
	~CPrefsMessage();

// Dialog Data
	//{{AFX_DATA(CPrefsMessage)
	enum { IDD = IDD_PREFS_MESSAGE };
	CTabController	mTabs;
	//}}AFX_DATA

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsMessage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsMessage)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnSelChangeMessageTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
