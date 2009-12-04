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


// CPrefsAccountPanel.h : header file
//

#ifndef __CPREFSACCOUNTPANEL__MULBERRY__
#define __CPREFSACCOUNTPANEL__MULBERRY__

#include "CTabPanel.h"

#include "CTabController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAccountPanel dialog

class CPreferences;

class CPrefsAccountPanel : public CTabPanel
{
// Construction
public:
	CPrefsAccountPanel(int id, int tab_id);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAccountPanel)
	CTabController	mTabs;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual void	SetPrefs(CPreferences* prefs)	// Set data
		{ mCopyPrefs = prefs; }
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAccountPanel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	int mTabID;
	CPreferences* mCopyPrefs;

	// Generated message map functions
	//{{AFX_MSG(CPrefsAccountPanel)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelChangePrefsAccountTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
	
	virtual void InitTabs() = 0;
};

#endif
