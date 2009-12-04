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


// CPrefsDisplayServer.h : header file
//

#ifndef __CPREFSDISPLAYSERVER__MULBERRY__
#define __CPREFSDISPLAYSERVER__MULBERRY__

#include "CTabPanel.h"

#include "CColorButton.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsDisplayServer dialog

class CPrefsDisplayServer : public CTabPanel
{
	DECLARE_DYNAMIC(CPrefsDisplayServer)

// Construction
public:
	CPrefsDisplayServer();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsDisplayServer)
	enum { IDD = IDD_PREFS_DISPLAY_SERVER };
	CColorPickerButton	mOpenColour;
	CColorPickerButton	mClosedColour;
	CColorPickerButton	mServerColour;
	CColorPickerButton	mFavouriteColour;
	CColorPickerButton	mHierarchyColour;
	CButton				mUseServerBtn;
	CButton				mUseFavouriteBtn;
	CButton				mUseHierarchyBtn;
	BOOL	mOpenBold;
	BOOL	mClosedBold;
	BOOL	mOpenItalic;
	BOOL	mClosedItalic;
	BOOL	mOpenStrike;
	BOOL	mClosedStrike;
	BOOL	mOpenUnderline;
	BOOL	mClosedUnderline;
	BOOL	mUseServer;
	BOOL	mUseFavourite;
	BOOL	mUseHierarchy;
	//}}AFX_DATA

	virtual void	SetContent(void* data);			// Set data
	virtual bool	UpdateContent(void* data);			// Force update of data

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsDisplayServer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsDisplayServer)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseServer();
	afx_msg void OnUseFavourite();
	afx_msg void OnUseHierarchy();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
