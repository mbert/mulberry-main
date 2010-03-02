/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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


// CRulesDialog.h : header file
//

#ifndef __CRULESDIALOG__MULBERRY__
#define __CRULESDIALOG__MULBERRY__

#include "CHelpDialog.h"
#include "CSearchBase.h"
#include "CWndAligner.h"

#include "CActionItem.h"

#include "CCmdEdit.h"
#include "CCommanderProtect.h"
#include "CGrayBackground.h"
#include "CGrayBorder.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CRulesDialog dialog

class CFilterItem;
class CCriteriaBase;
typedef std::vector<CCriteriaBase*> CCriteriaBaseList;


class CRulesDialog : public CHelpDialog, public CSearchBase, public CWndAligner
{
	friend class CRulesAction;

// Construction
public:
	CRulesDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CRulesDialog();

// Dialog Data
	//{{AFX_DATA(CRulesDialog)
	enum { IDD = IDD_RULESDIALOG };
	//}}AFX_DATA

	static  bool PoseDialog(CFilterItem* spec, bool& trigger_change);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRulesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CGrayBackground			mHeader;						// Header pane
	CStatic					mNameTitle;
	CCmdEdit				mName;
	CStatic					mTriggeredByTitle;
	CPopupButton			mTriggeredBy;
	CButton					mEditScript;
	CGrayBackground			mScroller;
	CGrayBorder				mCriteria;
	CGrayBackground			mCriteriaMove;
	CButton					mMoreBtn;
	CButton					mFewerBtn;
	CButton					mClearBtn;
	CStatic					mCriteriaDivider;
	CGrayBorder				mActions;
	CGrayBackground			mActionsMove;
	CButton					mMoreActionsBtn;
	CButton					mFewerActionsBtn;
	CButton					mClearActionsBtn;
	CButton					mStop;
	CStatic					mActionsDivider;
	CCmdEdit				mScriptEdit;
	CButton					mOKBtn;
	CButton					mCancelBtn;

	CCriteriaBaseList		mActionItems;

	CFilterItem*			mFilter;
	bool					mTriggerChange;

	int						mScrollerImageHeight;

	CCommanderProtect		mCmdProtect;					// Protect commands

	void	SetFilter(CFilterItem* filter = NULL);
	void	InitTriggers(CFilterItem* filter);

	virtual CWnd* GetContainerWnd()
		{ return &mCriteria; }
	virtual CView* GetParentView()
		{ return NULL; }
	virtual CButton* GetFewerBtn()
		{ return &mFewerBtn; }

	virtual void Resized(int dy);

	CCriteriaBaseList& GetActions()
		{ return mActionItems; }

	void	InitActions(const CActionItemList* actions);
	void	AddAction(const CActionItem* action = NULL);
	void	RemoveAction();
	void	ResizedActions(int dy);

	void				ConstructFilter(CFilterItem* spec, bool script, bool change_triggers = true) const;
	CActionItemList*	ConstructActions() const;

	virtual void	ResetState();		// Reset state from prefs

	// Generated message map functions
	//{{AFX_MSG(CRulesDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy(void);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetTrigger(UINT nID);
	afx_msg void OnMoreBtn();
	afx_msg void OnFewerBtn();
	afx_msg void OnClearBtn();
	afx_msg void OnMoreActions();
	afx_msg void OnFewerActions();
	afx_msg void OnClearActions();
	afx_msg void OnEditScript();
	afx_msg void OnSaveDefaultState(void);				// Save state in prefs
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif