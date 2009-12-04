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


// CPrefsAttachments.h : header file
//

#ifndef __CPREFSATTACHMENTS__MULBERRY__
#define __CPREFSATTACHMENTS__MULBERRY__

#include "CPrefsPanel.h"
#include "CTabController.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAttachments dialog

class CPreferences;

class CPrefsAttachments : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsAttachments)

// Construction
public:
	CPrefsAttachments();
	~CPrefsAttachments();

// Dialog Data
	//{{AFX_DATA(CPrefsAttachments)
	enum { IDD = IDD_PREFS_ATTACHMENTS };
	CTabController	mTabs;
	//}}AFX_DATA

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAttachments)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsAttachments)
	virtual void InitControls(void);
	virtual void SetControls(void);
	afx_msg void OnSelChangeAttachmentTabs(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
