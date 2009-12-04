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


// CPrefsDisplayQuotes.h : header file
//

#ifndef __CPREFSDISPLAYQUOTES__MULBERRY__
#define __CPREFSDISPLAYQUOTES__MULBERRY__

#include "CTabPanel.h"

#include "CColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayQuotes dialog

class CPrefsDisplayQuotes : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsDisplayQuotes)

// Construction
public:
	CPrefsDisplayQuotes();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsDisplayQuotes)
	enum { IDD = IDD_PREFS_DISPLAY_QUOTES };
	CColorPickerButton	mQuotation1ColourCtrl;
	BOOL	mQuotationBold;
	BOOL	mQuotationItalic;
	BOOL	mQuotationUnder;
	CColorPickerButton	mQuotation2ColourCtrl;
	CColorPickerButton	mQuotation3ColourCtrl;
	CColorPickerButton	mQuotation4ColourCtrl;
	CColorPickerButton	mQuotation5ColourCtrl;
	BOOL		mUseQuotation;
	CButton		mUseQuotationBtn;
	cdstring	mRecogniseQuotes;
	CEdit		mRecogniseQuotesCtrl;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDisplayQuotes)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsDisplayQuotes)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseQuotation();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
