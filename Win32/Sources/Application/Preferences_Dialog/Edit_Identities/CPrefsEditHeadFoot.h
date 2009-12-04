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


// CPrefsEditHeadFoot.h : header file
//

#ifndef __CPREFSEDITHEADFOOT__MULBERRY__
#define __CPREFSEDITHEADFOOT__MULBERRY__

#include "CHelpDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CPrefsEditHeadFoot dialog

class CPreferences;

class CPrefsEditHeadFoot : public CHelpDialog
{
// Construction
public:
	CPrefsEditHeadFoot(bool allow_file, CWnd* pParent = NULL);   // standard constructor

	static bool PoseDialog(CPreferences* prefs, cdstring& txt, bool allow_file);

			void SetCurrentWrap(short wrap)
					{ mWrap = wrap; }
			void SetCurrentSpacesPerTab(short num)
					{ mSpacesPerTab = num; }
			void SetFont(CFont* font)
					{ mFont = font; }

// Dialog Data
	//{{AFX_DATA(CPrefsEditHeadFoot)
	enum { IDD1 = IDD_EDITHEADFOOT,  IDD2 = IDD_EDITHEADFOOTFILE};
	int		mUseFile;
	cdstring	mFileName;
	CEdit		mFileNameCtrl;
	cdstring	mEditHeadFoot;
	CEdit		mEditHeadFootCtrl;
	CStatic		mEditRuler;
	CButton		mOKBtn;
	CButton		mCancelBtn;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsEditHeadFoot)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CReplyChooseDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnUseFile();
	afx_msg void OnUseText();
	afx_msg void OnChooseFile();
	//}}AFX_MSG

private:
	bool			mAllowFile;								// Allow choice of file
	cdstring		mOldText;								// Text used prior to switching to file
	short			mSpacesPerTab;
	short			mWrap;
	CFont*			mFont;

	void	ResizeToWrap();								// Resize Window to wrap length
	void	SetEditorText(const cdstring& text);		// Set text in editor
	void	GetEditorText(cdstring& text);				// Get text from editor

	void	UpdateFile();
	void	SetUseFile(bool use_file);

	DECLARE_MESSAGE_MAP()
};

#endif
