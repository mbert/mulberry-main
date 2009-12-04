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


// CPrefsMailboxAdvanced.h : header file
//

#ifndef __CPREFSMAILBOXADVANCED__MULBERRY__
#define __CPREFSMAILBOXADVANCED__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMailboxAdvanced dialog

class CPrefsMailboxAdvanced : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsMailboxAdvanced)

// Construction
public:
	CPrefsMailboxAdvanced();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsMailboxAdvanced)
	enum { IDD = IDD_PREFS_MAILBOX_ADVANCED };
	int		mNewMessage;

	BOOL	mUseCopyToCabinet;
	CButton	mUseCopyToCabinetCtrl;
	BOOL	mUseAppendToCabinet;
	CButton	mUseAppendToCabinetCtrl;
	int		mMaximumRecent;

	int		mRLoCache;
	int		mRNoHiCache;
	int		mRHiCache;
	int		mRAutoIncrement;
	int		mRCacheIncrement;
	int		mRSortCache;

	CEdit	mRHiCacheCtrl;
	CEdit	mRCacheIncrementCtrl;

	int		mLLoCache;
	int		mLNoHiCache;
	int		mLHiCache;
	int		mLAutoIncrement;
	int		mLCacheIncrement;
	int		mLSortCache;

	CEdit	mLHiCacheCtrl;
	CEdit	mLCacheIncrementCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsMailboxAdvanced)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsMailboxAdvanced)
	virtual BOOL OnInitDialog();
	afx_msg void OnForceMailboxListReset();
	afx_msg void OnRNoHiCache();
	afx_msg void OnRUseHiCache();
	afx_msg void OnRAutoIncrement();
	afx_msg void OnRUseCacheIncrement();
	afx_msg void OnLNoHiCache();
	afx_msg void OnLUseHiCache();
	afx_msg void OnLAutoIncrement();
	afx_msg void OnLUseCacheIncrement();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
