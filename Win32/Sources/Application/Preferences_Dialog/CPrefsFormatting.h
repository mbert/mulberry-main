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


// CPrefsFormatting.h : header file
//

#ifndef __CPREFSFORMATTING__MULBERRY__
#define __CPREFSFORMATTING__MULBERRY__

#include "CPrefsPanel.h"

#include "CSizeMenu.h"
#include "CWindowStatesFwd.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsFormatting dialog

class CPreferences;

class CPrefsFormatting : public CPrefsPanel
{
	DECLARE_DYNCREATE(CPrefsFormatting)

// Construction
public:
	CPrefsFormatting();   // standard constructor
	~CPrefsFormatting();

// Dialog Data
	//{{AFX_DATA(CPrefsFormatting)
	enum { IDD = IDD_PREFS_FORMATTING };
	cdstring	mListFont;
	CStatic		mListFontCtrl;
	cdstring	mListFontSize;
	CStatic		mListFontSizeCtrl;
	SLogFont	mListTextFont;
	cdstring	mDisplayFont;
	CStatic		mDisplayFontCtrl;
	cdstring	mDisplayFontSize;
	CStatic		mDisplayFontSizeCtrl;
	SLogFont	mDisplayTextFont;
	cdstring	mPrinterFont;
	CStatic		mPrinterFontCtrl;
	cdstring	mPrinterFontSize;
	CStatic		mPrinterFontSizeCtrl;
	SLogFont	mPrintTextFont;
	cdstring	mCaptionFont;
	CStatic		mCaptionFontCtrl;
	cdstring	mCaptionFontSize;
	CStatic		mCaptionFontSizeCtrl;
	SLogFont	mCaptionTextFont;
	cdstring	mFixedFont;
	CStatic		mFixedFontCtrl;
	cdstring	mFixedFontSize;
	CStatic		mFixedFontSizeCtrl;
	SLogFont	mFixedTextFont;
	cdstring	mHTMLFont;
	CStatic		mHTMLFontCtrl;
	cdstring	mHTMLFontSize;
	CStatic		mHTMLFontSizeCtrl;
	SLogFont	mHTMLTextFont;
	CSizePopup	mMinimumFontCtrl;
	BOOL		mUseStyles;
	//}}AFX_DATA

	void SetPrefs(CPreferences* prefs);					// Set up params for DDX
	void UpdatePrefs(CPreferences* prefs);				// Get params from DDX

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsFormatting)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

			void DoFontChange(SLogFont& font, CStatic& font_ctrl, CStatic& size_ctrl);

	// Generated message map functions
	//{{AFX_MSG(CPrefsFormatting)
	virtual void InitControls();
	virtual void SetControls();
	afx_msg void OnListFontBtn();
	afx_msg void OnDisplayFontBtn();
	afx_msg void OnPrinterFontBtn();
	afx_msg void OnCaptionFontBtn();
	afx_msg void OnFixedFontBtn();
	afx_msg void OnHTMLFontBtn();
	afx_msg void OnChangeMinimumSize(UINT nID);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
