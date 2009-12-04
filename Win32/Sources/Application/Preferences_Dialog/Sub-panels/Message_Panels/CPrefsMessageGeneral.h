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


// CPrefsMessageGeneral.h : header file
//

#ifndef __CPREFSMESSAGEGENERALGENERAL__MULBERRY__
#define __CPREFSMESSAGEGENERALGENERAL__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessageGeneral dialog

class CPreferences;

class CPrefsMessageGeneral : public CTabPanel
{
	DECLARE_DYNCREATE(CPrefsMessageGeneral)

// Construction
public:
	CPrefsMessageGeneral();

// Dialog Data
	//{{AFX_DATA(CPrefsMessageGeneral)
	enum { IDD = IDD_PREFS_MESSAGE_GENERAL };
	cdstring	saveCreator;
	int			mDoSizeWarn;
	int			warnMessageSize;
	int			optionKeyReplyDialog;
	int			mForwardChoice;
	BOOL		mForwardQuoteOriginal;
	BOOL		mForwardHeaders;
	BOOL		mForwardAttachment;
	BOOL		mForwardRFC822;

	CEdit		mWarnSizeCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsMessageGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CPreferences* mCopyPrefs;

	// Generated message map functions
	//{{AFX_MSG(CPrefsMessageGeneral)
	afx_msg void OnLimitSize();
	afx_msg void OnNoLimit();
	afx_msg void OnPrintCaption();
	afx_msg void OnForwardChoice();
	afx_msg void OnForwardOptions();
	afx_msg void OnForwardQuoteOriginal();
	afx_msg void OnForwardAttachment();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
