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


// CPrefsDisplayMessage.h : header file
//

#ifndef __CPREFSDISPLAYMESSAGE__MULBERRY__
#define __CPREFSDISPLAYMESSAGE__MULBERRY__

#include "CTabPanel.h"

#include "CColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayMessage dialog

class CPrefsDisplayMessage : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsDisplayMessage)

// Construction
public:
	CPrefsDisplayMessage();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsDisplayMessage)
	enum { IDD = IDD_PREFS_DISPLAY_MESSAGE };
	CColorPickerButton	mDeletedColour;
	CColorPickerButton	mImportantColour;
	CColorPickerButton	mAnsweredColour;
	CColorPickerButton	mSeenColour;
	CColorPickerButton	mUnseenColour;
	CColorPickerButton	mMatchColour;
	CColorPickerButton	mNonMatchColour;
	CButton				mUseMatchBtn;
	CButton				mUseNonMatchBtn;
	BOOL	mUnseenBold;
	BOOL	mSeenBold;
	BOOL	mAnsweredBold;
	BOOL	mImportantBold;
	BOOL	mDeletedBold;
	BOOL	mUnseenItalic;
	BOOL	mSeenItalic;
	BOOL	mAnsweredItalic;
	BOOL	mImportantItalic;
	BOOL	mDeletedItalic;
	BOOL	mUnseenStrike;
	BOOL	mSeenStrike;
	BOOL	mAnsweredStrike;
	BOOL	mImportantStrike;
	BOOL	mDeletedStrike;
	BOOL	mUnseenUnderline;
	BOOL	mSeenUnderline;
	BOOL	mAnsweredUnderline;
	BOOL	mImportantUnderline;
	BOOL	mDeletedUnderline;
	BOOL	mMultiAddressBold;
	BOOL	mMultiAddressItalic;
	BOOL	mMultiAddressStrike;
	BOOL	mMultiAddressUnderline;
	BOOL	mUseMatch;
	BOOL	mUseNonMatch;
	BOOL	mUseLocalTimezone;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDisplayMessage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsDisplayMessage)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseMatch();
	afx_msg void OnUseNonMatch();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
