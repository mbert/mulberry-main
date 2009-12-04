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


// CPrefsDisplayLabel.h : header file
//

#ifndef __CPREFSDISPLAYLABEL__MULBERRY__
#define __CPREFSDISPLAYLABEL__MULBERRY__

#include "CTabPanel.h"

#include "CColorButton.h"
#include "CMessageFwd.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayLabel dialog

class CPrefsDisplayLabel : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsDisplayLabel)

// Construction
public:
	CPrefsDisplayLabel();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsDisplayLabel)
	enum { IDD = IDD_PREFS_DISPLAY_LABEL };
	cdstring			mLabel[NMessage::eMaxLabels];
	BOOL				mUseColour[NMessage::eMaxLabels];
	CColorPickerButton	mColour[NMessage::eMaxLabels];
	BOOL				mUseBkgndColour[NMessage::eMaxLabels];
	CColorPickerButton	mBkgndColour[NMessage::eMaxLabels];
	BOOL				mBold[NMessage::eMaxLabels];
	BOOL				mItalic[NMessage::eMaxLabels];
	BOOL				mStrike[NMessage::eMaxLabels];
	BOOL				mUnderline[NMessage::eMaxLabels];
	//}}AFX_DATA
	cdstrvect			mIMAPLabels;

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDisplayLabel)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsDisplayLabel)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseColor(UINT nID);
	afx_msg void OnUseBkgndColor(UINT nID);
	afx_msg void OnIMAPLabels();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
