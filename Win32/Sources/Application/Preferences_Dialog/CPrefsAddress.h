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


// CPrefsAddress.h : header file
//

#ifndef __CPREFSADDRESS__MULBERRY__
#define __CPREFSADDRESS__MULBERRY__

#include "CPrefsPanel.h"
#include "CTabController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAddress dialog

class CPreferences;

class CPrefsAddress : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsAddress)

// Construction
public:
	CPrefsAddress();
	~CPrefsAddress();

// Dialog Data
	//{{AFX_DATA(CPrefsAddress)
	enum { IDD = IDD_PREFS_ADDRESS };
	int mOptionKeyAddressDialog;
	CButton	mAltDownCtrl;
	CButton	mAltUpCtrl;
	CTabController	mTabs;
	//}}AFX_DATA

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAddress)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAddress)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnAltDown();
	afx_msg void OnAltUp();
	afx_msg void OnSelChangeAddressTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
