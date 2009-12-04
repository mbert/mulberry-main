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


/////////////////////////////////////////////////////////////////////////////
// CPrefsEditCaption dialog

#ifndef __CPREFSEDITCAPTION__MULBERRY__
#define __CPREFSEDITCAPTION__MULBERRY__

#include "CHelpDialog.h"

template<class T> class CPreferenceValueMap;

class CPrefsEditCaption : public CHelpDialog
{
// Construction
public:
	CPrefsEditCaption(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CPrefsEditCaption)
	enum { IDD = IDD_EDITCAPTION };
	CButton	mPageBtn;
	CButton	mCursorTopCtrl;
	CButton	mMessage1Ctrl;
	CButton	mDraft1Ctrl;
	CButton	mUseBox1Ctrl;
	CButton	mSummaryCtrl;
	CButton	mMessage2Ctrl;
	CButton	mDraft2Ctrl;
	CButton	mUseBox2Ctrl;
	CEdit	mEditHeaderCtrl;
	CEdit	mEditFooterCtrl;
	//}}AFX_DATA
	CEdit*	mEditFocusCtrl;


	virtual void SetCurrentSpacesPerTab(short num)
					{ mSpacesPerTab = num; }
	virtual void SetFont(CFont* font)
					{ mFont = font; }
	virtual void	SetData(CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary);		// Set text in editor
	virtual void	GetEditorText(void);								// Get text from editor

	static void PoseDialog(CPreferenceValueMap<cdstring>* txt1,
							CPreferenceValueMap<cdstring>* txt2,
							CPreferenceValueMap<cdstring>* txt3,
							CPreferenceValueMap<cdstring>* txt4,
							CPreferenceValueMap<bool>* cursor_top,
							CPreferenceValueMap<bool>* use_box1,
							CPreferenceValueMap<bool>* use_box2,
							CPreferenceValueMap<bool>* summary);

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CPrefsEditCaption)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CPrefsEditCaption)
	virtual BOOL OnInitDialog();
	afx_msg void OnCaptionMyName();
	afx_msg void OnCaptionMyEmail();
	afx_msg void OnCaptionMyFull();
	afx_msg void OnCaptionMyFirst();
	afx_msg void OnCaptionSmartName();
	afx_msg void OnCaptionSmartEmail();
	afx_msg void OnCaptionSmartFull();
	afx_msg void OnCaptionSmartFirst();
	afx_msg void OnCaptionFromName();
	afx_msg void OnCaptionFromEmail();
	afx_msg void OnCaptionFromFull();
	afx_msg void OnCaptionFromFirst();
	afx_msg void OnCaptionToName();
	afx_msg void OnCaptionToEmail();
	afx_msg void OnCaptionToFull();
	afx_msg void OnCaptionToFirst();
	afx_msg void OnCaptionCCName();
	afx_msg void OnCaptionCCEmail();
	afx_msg void OnCaptionCCFull();
	afx_msg void OnCaptionCCFirst();
	afx_msg void OnCaptionSubject();
	afx_msg void OnCaptionSentShort();
	afx_msg void OnCaptionSentLong();
	afx_msg void OnCaptionDateNow();
	afx_msg void OnCaptionPage();
	afx_msg void OnCaptionRevert();
	afx_msg void OnCaptionMessage1();
	afx_msg void OnCaptionDraft1();
	afx_msg void OnCaptionMessage2();
	afx_msg void OnCaptionDraft2();
	afx_msg void OnHeaderSetFocus();
	afx_msg void OnFooterSetFocus();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	bool showing1;
	bool showing3;
	cdstring text1;
	cdstring text2;
	cdstring text3;
	cdstring text4;
	CPreferenceValueMap<cdstring>*		mText1Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText2Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText3Item;			// Prefs field
	CPreferenceValueMap<cdstring>*		mText4Item;			// Prefs field
	CPreferenceValueMap<bool>*			mCursorTopItem;		// Cursor top item
	CPreferenceValueMap<bool>*			mBox1Item;			// Box state
	CPreferenceValueMap<bool>*			mBox2Item;			// Box state
	CPreferenceValueMap<bool>*			mSummaryItem;		// Summary state

	short mSpacesPerTab;
	bool mUsePage;
	CFont* mFont;

			void	AddCaptionItem(const char* item);

			void	SetDisplay1(bool show1);
			void	UpdateDisplay1();
			void	SetDisplay2(bool show2);
			void	UpdateDisplay2();
};

#endif
