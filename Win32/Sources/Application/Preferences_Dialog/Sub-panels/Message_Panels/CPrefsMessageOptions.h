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


// CPrefsMessageOptionsOptions.h : header file
//

#ifndef __CPrefsMessageOptionsOPTIONS__MULBERRY__
#define __CPrefsMessageOptionsOPTIONS__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsMessageOptions dialog

class CPreferences;

class CPrefsMessageOptions : public CTabPanel
{
	DECLARE_DYNCREATE(CPrefsMessageOptions)

// Construction
public:
	CPrefsMessageOptions();

// Dialog Data
	//{{AFX_DATA(CPrefsMessageOptions)
	enum { IDD = IDD_PREFS_MESSAGE_OPTIONS };
	BOOL	showMessageHeader;
	BOOL	mShowStyledText;
	BOOL	saveMessageHeader;
	BOOL	deleteAfterCopy;
	BOOL	mOpenDeleted;
	BOOL	mCloseDeleted;
	BOOL	mOpenReuse;
	BOOL	mQuoteSelection;
	BOOL	alwaysQuote;
	BOOL	mAutoDigest;
	BOOL	mExpandHeader;
	BOOL	mExpandParts;
	int		mMDNOptions;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CPrefsMessageOptions)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CPrefsMessageOptions)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
