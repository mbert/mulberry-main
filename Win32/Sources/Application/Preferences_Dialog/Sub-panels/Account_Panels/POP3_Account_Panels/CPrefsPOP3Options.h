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


// CPrefsPOP3Options.h : header file
//

#ifndef __CPREFSPOP3OPTIONS__MULBERRY__
#define __CPREFSPOP3OPTIONS__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsPOP3Options dialog

class CPrefsPOP3Options : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsPOP3Options)

// Construction
public:
	CPrefsPOP3Options();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsPOP3Options)
	enum { IDD = IDD_PREFS_ACCOUNT_POP3_Options };
	BOOL	mLeaveOnServer;
	CButton	mLeaveOnServerCtrl;
	BOOL	mDeleteAfter;
	CButton	mDeleteAfterCtrl;
	UINT	mDeleteDays;
	CEdit	mDeleteDaysCtrl;
	BOOL	mDeleteExpunged;
	CButton	mDeleteExpungedCtrl;
	BOOL	mUseMaxSize;
	CButton	mUseMaxSizeCtrl;
	UINT	mMaxSize;
	CEdit	mMaxSizeCtrl;
	BOOL	mUseAPOP;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsPOP3Options)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsPOP3Options)
	afx_msg void OnPrefsAccountPOP3LeaveMail();
	afx_msg void OnPrefsAccountPOP3DeleteAfter();
	afx_msg void OnPrefsAccountPOP3UseMaxSize();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	virtual void	SetLeaveOnServer(bool on);

};

#endif
