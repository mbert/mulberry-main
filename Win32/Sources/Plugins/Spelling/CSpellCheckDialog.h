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

// CSpellCheckDialog.h : header file
//

#ifndef __CSPELLCHECKDIALOG__MULBERRY__
#define __CSPELLCHECKDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CDictionaryPageList.h"
#include "CSpellPlugin.h"
#include "CWindowStates.h"

/////////////////////////////////////////////////////////////////////////////
// CSpellCheckDialog dialog

class CCmdEditView;
class cdstring;

class CSpellCheckDialog : public CHelpDialog
{
// Construction
public:
	CSpellCheckDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSpellCheckDialog)
	enum { IDD = IDD_SPELL_CHECK };
	CStatic		mPageCount;
	CStatic		mWordBottom;
	CStatic		mWordTop;
	CDictionaryPageList	mList;
	CEdit		mSuggestion;
	CStatic		mWord;
	CButton		mSkipAll;
	CButton		mReplaceAll;
	CButton		mSkip;
	CButton		mReplace;
	CButton		mEdit;
	CButton		mAdd;
	CButton		mSendNow;
	CButton		mSuggestMore;
	CStatic		mDictName;
	//}}AFX_DATA

	static CPreferenceValue<CWindowState>*	sWindowState;
	CSpellPlugin*		mSpeller;
	CWnd*				mWindow;
	CRect				mWindowBounds;
	bool				mWindowMoved;
	CCmdEditView*		mText;
	long				mTextSelStart;
	cdstring*			mCheckText;
	long				mSuggestionCtr;
	bool				mWillSend;

			void	SetSpeller(CSpellPlugin* speller,
								CWnd* wnd,
								CCmdEditView* text,
								long start,
								cdstring* check,
								bool sending);				// Set the speller & text

			void	NextError(void);								// Go to next error
			void	SelectWord(const CSpellPlugin::SpError* sperr);		// Do visual select of word
			void	ReplaceWord(const CSpellPlugin::SpError* sperr);	// Do visual replace of word

			void	DisplaySuggestions(void);					// Display suggestions in list
			void	DisplayDictionary(bool select,
									bool force_draw = false);	// Display dictionary in list

			void	SetSuggestion(char* suggest);				// Set suggestion field

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSpellCheckDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSpellCheckDialog)
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy(void);
	afx_msg void OnSpellCheckAdd();
	afx_msg void OnSpellCheckEdit();
	afx_msg void OnSpellCheckReplace();
	afx_msg void OnSpellCheckSkip();
	afx_msg void OnSpellCheckReplaceAll();
	afx_msg void OnSpellCheckSkipAll();
	afx_msg void OnSpellCheckOptions();
	afx_msg void OnSpellCheckSuggestMore();
	afx_msg void OnSpellCheckSendNow();
	afx_msg void OnSelchangeSpellCheckList();
	afx_msg void OnSelcancelSpellCheckList();
	afx_msg void OnDblclkSpellCheckList();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	virtual void	ResetState(void);						// Reset state from prefs
	virtual void	SaveState(void);						// Save state in prefs
};

#endif
