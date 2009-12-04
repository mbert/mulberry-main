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


// CPrefsMailboxBasic.h : header file
//

#ifndef __CPREFSMAILBOXBASIC__MULBERRY__
#define __CPREFSMAILBOXBASIC__MULBERRY__

#include "CTabPanel.h"
#include "CMailboxPopup.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxBasic dialog

class CPrefsMailboxBasic : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsMailboxBasic)

// Construction
public:
	CPrefsMailboxBasic();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsMailboxBasic)
	enum { IDD = IDD_PREFS_MAILBOX_BASIC };
	int		openAtFirst;
	BOOL	mNoOpenPreview;
	int		mOldestToNewest;
	BOOL	expungeOnClose;
	BOOL	warnOnExpunge;
	BOOL	mWarnPuntUnseen;
	BOOL	mDoRollover;
	BOOL	mRolloverWarn;
	BOOL	mScrollForUnseen;

	BOOL		mDoMailboxClear;
	cdstring	clear_mailbox;
	BOOL		clear_warning;

	CButton	mDoMailboxClearCtrl;
	CEdit	mMoveMailboxCtrl;
	CMailboxPopup	mMoveToPopup;
	CButton	mWarnMoveCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsMailboxBasic)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsMailboxBasic)
	virtual BOOL OnInitDialog();
	afx_msg void OnMoveRead();
	afx_msg void OnChangeMoveTo(UINT nID);				// Change move to menu
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
