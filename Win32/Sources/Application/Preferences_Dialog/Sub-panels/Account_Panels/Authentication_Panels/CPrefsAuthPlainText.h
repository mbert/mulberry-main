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


// CPrefsAuthPlainText.h : header file
//

#ifndef __CPREFSAUTHPLAINTEXT__MULBERRY__
#define __CPREFSAUTHPLAINTEXT__MULBERRY__

#include "CTabPanel.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsAuthPlainText dialog

class CPrefsAuthPlainText : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsAuthPlainText)

// Construction
public:
	CPrefsAuthPlainText();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsAuthPlainText)
	enum { IDD = IDD_PREFS_AUTHENTICATE_PLAIN };
	cdstring	mUID;
	BOOL		mSaveUser;
	BOOL		mSavePswd;
	CButton		mSavePswdCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsAuthPlainText)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsAuthPlainText)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
