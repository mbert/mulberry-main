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


// CRulesWindow.h

#ifndef __CRULESWINDOW__MULBERRY__
#define __CRULESWINDOW__MULBERRY__

#include "CTableWindow.h"

#include "CCommanderProtect.h"
#include "CGrayBackground.h"
#include "CIconButton.h"
#include "CPopupButton.h"
#include "CRulesTable.h"
#include "CScriptsTable.h"
#include "CSimpleTitleTable.h"
#include "CSplitterView.h"
#include "CTextButton.h"
#include "CWindowStatesFwd.h"

// Classes
class CRulesWindow : public CTableWindow
{
	DECLARE_DYNCREATE(CRulesWindow)

	friend class CRulesTable;
	friend class CScriptsTable;

public:
	static CMultiDocTemplate*	sRulesDocTemplate;
	static CRulesWindow*		sRulesWindow;

				CRulesWindow();
	virtual		~CRulesWindow();

	static CRulesWindow* ManualCreate(void);			// Manually create document
	static void CreateRulesWindow();		// Create it or bring it to the front
	static void DestroyRulesWindow();		// Destroy the window

	static void MakeRule(const CMessageList& msgs);	// Make rule from example messages

			CRulesTable* GetTable(void) const
						{ return (CRulesTable*) &mRulesTable; }
	virtual void	ResetTable(void);						// Reset the table

	virtual void	ResetFont(CFont* font);					// Reset list font

			void	FocusRules()						// Focus on rules
						{ mFocusRules = true; }
			void	FocusScripts()						// Focus on scripts
						{ mFocusRules = false; }

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

protected:
	CTabCtrl				mTabs;
	CGrayBackground			mHeader;						// Header pane
	CGrayBackground			mLocalBtns;
	CGrayBackground			mSIEVEBtns;
	CIconButton				mNewRuleBtn;
	CIconButton				mNewTargetBtn;
	CIconButton				mNewScriptBtn;
	CIconButton				mEditBtn;
	CIconButton				mDeleteBtn;
	CIconButton				mApplyBtn;
	CIconButton				mGenerateBtn;
	CTextButton				mShowTriggers;
	CSplitterView			mSplitterView;
	CGrayBackground			mRulesFocusRing;						// Focus ring
	CSimpleTitleTable		mRulesTitles;
	CRulesTable				mRulesTable;
	CGrayBackground			mScriptsFocusRing;						// Focus ring
	CSimpleTitleTable		mScriptsTitles;
	CScriptsTable			mScriptsTable;
	bool					mFocusRules;

	CFilterItem::EType		mType;
	CCommanderProtect		mCmdProtect;					// Protect commands

	virtual void	InitColumns(void);						// Init columns and text

	virtual int		GetSortBy(void) {return 0;}				// No sorting
	virtual void	SetSortBy(int sort) {}					// No sorting

			void	SetTriggerSplit();						// Show/hide triggers panel

	virtual void	ResetState(bool force = false);			// Reset state from prefs
	virtual void	SaveState(void);						// Save current state in prefs
	virtual void	SaveDefaultState(void);					// Save state in prefs

	// Generated message map functions
	//{{AFX_MSG(CEditIdentities)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);

	afx_msg void OnUpdateFileImport(CCmdUI* pCmdUI);
	afx_msg void OnUpdateFileExport(CCmdUI* pCmdUI);

	afx_msg void OnFileImport();
	afx_msg void OnFileExport();

	afx_msg void OnSelChangeRulesTabs(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnShowTriggers();
	afx_msg void OnNewRules();
	afx_msg void OnNewTrigger();
	afx_msg void OnNewScript();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
