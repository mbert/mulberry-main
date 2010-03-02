/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


/////////////////////////////////////////////////////////////////////////////
// CLoggingOptionsDialog1 dialog

#ifndef __CLOGGINGOPTIONSDIALOG__MULBERRY__
#define __CLOGGINGOPTIONSDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CLog.h"

class CLoggingOptionsDialog : public CHelpDialog
{
// Construction
public:
	CLoggingOptionsDialog(CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CLog::SLogOptions& options);

// Dialog Data
	//{{AFX_DATA(CLoggingOptionsDialog)
	enum { IDD = IDD_LOGGING_OPTIONS };
	BOOL	mActivate;
	BOOL	mLogs[CLog::eLogTypeLast];
	BOOL	mAuthentication;
	BOOL	mPlayback;
	BOOL	mOverwrite;
	int		mUserCWD;
	//}}AFX_DATA

	void	SetOptions(const CLog::SLogOptions& options);		// Set options in dialog
	void	GetOptions(CLog::SLogOptions& options);				// Get options from dialog

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CLoggingOptionsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CLoggingOptionsDialog)
	afx_msg void OnLoggingClearBtn();
	afx_msg void OnLoggingFlushBtn();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
