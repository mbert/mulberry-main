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


// CPrefsDisplayStyles.h : header file
//

#ifndef __CPREFSDISPLAYSTYLES__MULBERRY__
#define __CPREFSDISPLAYSTYLES__MULBERRY__

#include "CTabPanel.h"

#include "CColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayStyles dialog

class CPrefsDisplayStyles : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsDisplayStyles)

// Construction
public:
	CPrefsDisplayStyles();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsDisplayStyles)
	enum { IDD = IDD_PREFS_DISPLAY_STYLES };
	CColorPickerButton	mURLColourCtrl;
	BOOL	mURLBold;
	BOOL	mURLItalic;
	BOOL	mURLUnder;
	CColorPickerButton	mURLSeenColourCtrl;
	BOOL	mURLSeenBold;
	BOOL	mURLSeenItalic;
	BOOL	mURLSeenUnder;
	CColorPickerButton	mHeaderColourCtrl;
	BOOL	mHeaderBold;
	BOOL	mHeaderItalic;
	BOOL	mHeaderUnder;
	CColorPickerButton	mTagColourCtrl;
	BOOL	mTagBold;
	BOOL	mTagItalic;
	BOOL	mTagUnder;
	cdstring	mRecogniseURLs;
	CEdit		mRecogniseURLsCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDisplayStyles)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsDisplayStyles)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
