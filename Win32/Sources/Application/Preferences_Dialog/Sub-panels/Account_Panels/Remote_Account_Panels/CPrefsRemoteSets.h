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


// CPrefsRemoteSets.h : header file
//

#ifndef __CPREFSREMOTESETS__MULBERRY__
#define __CPREFSREMOTESETS__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsRemoteSets dialog

class CPrefsRemoteSets : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsRemoteSets)

// Construction
public:
	CPrefsRemoteSets();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsRemoteSets)
	enum { IDD = IDD_PREFS_ACCOUNT_Remote_Sets };
	CListBox	mList;
	CButton		mRefreshBtn;
	CButton		mRenameBtn;
	CButton		mDeleteBtn;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsRemoteSets)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	virtual void	UpdateList(void);						// Update list

	// Generated message map functions
	//{{AFX_MSG(CPrefsRemoteSets)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeRemoteSets();
	afx_msg void OnRefreshRemote();
	afx_msg void OnDeleteRemote();
	afx_msg void OnRenameRemote();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
