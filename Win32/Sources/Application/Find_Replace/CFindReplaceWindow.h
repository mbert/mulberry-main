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


// CFindReplaceWindow.h : header file
//

#ifndef __CFINDREPLACEWINDOW__MULBERRY__
#define __CFINDREPLACEWINDOW__MULBERRY__

#include "CHelpDialog.h"

#include "CPopupButton.h"
#include "cdstring.h"
#include "strfind.h"

/////////////////////////////////////////////////////////////////////////////
// CFindReplaceWindow dialog

class CCmdEditView;

class CFindReplaceWindow : public CHelpDialog
{
// Construction
public:
	CFindReplaceWindow();   // standard constructor

// Dialog Data
	//{{AFX_DATA(CFindReplaceWindow)
	enum { IDD = IDD_FINDREPLACE };
	CEdit	mFindText;
	CPopupButton	mFindPopup;
	CEdit	mReplaceText;
	CPopupButton	mReplacePopup;
	CButton	mCaseSensitive;
	CButton	mBackwards;
	CButton	mWrap;
	CButton	mEntireWord;
	CButton	mFindBtn;
	CButton	mReplaceBtn;
	CButton	mReplaceFindBtn;
	CButton	mReplaceAllBtn;
	//}}AFX_DATA

	CCmdEditView*					mTargetDisplay;

	static CFindReplaceWindow*		sFindReplaceWindow;
	static cdstring					sFindText;
	static cdstring					sReplaceText;
	static EFindMode				sFindMode;
	static cdstrvect				sFindHistory;
	static cdstrvect				sReplaceHistory;

	static void CreateFindReplaceWindow(CCmdEditView* target);	// Create it or bring it to the front
	static void UpdateFindReplace();							// Update current details
	static void SetFindText(const char* text);					// Set find text and push into history

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFindReplaceWindow)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

	void InitItems();										// Initialise items
	void InitHistory(CPopupButton* popup,						// Initialise menus
						UINT cmd_start,
						const cdstrvect& history);
	void UpdateButtons();									// Update button state
	void SelectHistory(const cdstrvect& history,			// History item chosen
						long menu_pos,
						CEdit* field);
	void CaptureState();									// Copy state information to statics
	virtual void PostNcDestroy();

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFindReplaceWindow)
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnChangeFindText();
	afx_msg void OnFindPopup(UINT nID);
	afx_msg void OnReplacePopup(UINT nID);
	afx_msg void OnFindText();
	afx_msg void OnReplaceText();
	afx_msg void OnReplaceFindText();
	afx_msg void OnReplaceAllText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	void	PrepareTarget();								// Prepare target before execution

	void	ResetState(bool force = false);					// Reset state from prefs
	void	OnSaveDefaultState(void);						// Save state in prefs
};

#endif
