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


// CTargetsDialog.h : header file
//

#ifndef __CTARGETSDIALOG__MULBERRY__
#define __CTARGETSDIALOG__MULBERRY__

#include "CHelpDialog.h"

#include "CCriteriaBase.h"

#include "CTargetItem.h"

#include "CCmdEdit.h"
#include "CCommanderProtect.h"
#include "CGrayBackground.h"
#include "CGrayBorder.h"
#include "CPopupButton.h"

/////////////////////////////////////////////////////////////////////////////
// CTargetsDialog dialog

class CTargetsDialog : public CHelpDialog
{
	friend class CRulesTarget;

// Construction
public:
	CTargetsDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTargetsDialog();

// Dialog Data
	//{{AFX_DATA(CTargetsDialog)
	enum { IDD = IDD_TARGETSDIALOG };
	//}}AFX_DATA

	static  bool PoseDialog(CTargetItem* spec);

	virtual BOOL	OnCmdMsg(UINT nID, int nCode, void* pExtra,
     							 AFX_CMDHANDLERINFO* pHandlerInfo);		// Special chain of command

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTargetsDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CGrayBackground			mHeader;						// Header pane
	CStatic					mNameTitle;
	CCmdEdit				mName;
	CButton					mEnabled;
	CStatic					mApplyTitle;
	CPopupButton			mApplyPopup;
	CGrayBackground			mScroller;
	CGrayBorder				mTargets;
	CGrayBackground			mTargetsMove;
	CButton					mMoreTargetsBtn;
	CButton					mFewerTargetsBtn;
	CButton					mClearTargetsBtn;
	CStatic					mTargetsDivider;
	CButton					mOKBtn;
	CButton					mCancelBtn;

	CCriteriaBaseList		mTargetItems;

	CTargetItem*			mTarget;
	
	int						mScrollerImageHeight;
	CCommanderProtect		mCmdProtect;					// Protect commands

	void	SetTarget(CTargetItem* target = NULL);

	CCriteriaBaseList& GetTargets()
		{ return mTargetItems; }

	void	InitTargets(const CFilterTargetList* targets);
	void	AddTarget(const CFilterTarget* target = NULL);
	void	RemoveTarget();
	void	ResizedTargets(int dy);

	void	ConstructTarget(CTargetItem* spec) const;
	CFilterTargetList*	ConstructTargets() const;

	// Generated message map functions
	//{{AFX_MSG(CTargetsDialog)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetSchedule(UINT nID);
	afx_msg void OnMoreTargets();
	afx_msg void OnFewerTargets();
	afx_msg void OnClearTargets();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif