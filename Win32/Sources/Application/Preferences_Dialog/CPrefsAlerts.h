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


// CPrefsAlerts.h : header file
//

#ifndef __CPREFSALERTS__MULBERRY__
#define __CPREFSALERTS__MULBERRY__

#include "CPrefsPanel.h"
#include "CTabController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAlerts dialog

class CPreferences;

class CPrefsAlerts : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsAlerts)

// Construction
public:
	CPrefsAlerts();
	~CPrefsAlerts();

// Dialog Data
	//{{AFX_DATA(CPrefsAlerts)
	enum { IDD = IDD_PREFS_ALERTS };
	CTabController	mTabs;
	
	//}}AFX_DATA

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAlerts)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAlerts)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnSelChangeMailboxTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
