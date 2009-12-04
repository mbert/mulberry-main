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

#include "CDialogDirector.h"

#include "CCriteriaBase.h"

#include "CTargetItem.h"

#include "HPopupMenu.h"

/////////////////////////////////////////////////////////////////////////////
// CTargetsDialog dialog

class CTextInputField;
class JXEngravedRect;
class CBetterScrollbarSet;
class CBlankScrollable;
class JXScrollableWidget;
class JXTextButton;
class JXTextCheckbox;
class JXWidgetSet;

class CTargetsDialog : public CDialogDirector
{
	friend class CRulesTarget;

// Construction
public:
	CTargetsDialog(JXDirector* supervisor);   // standard constructor
	virtual ~CTargetsDialog();

	static  bool PoseDialog(CTargetItem* spec);

	virtual void Activate();

// Implementation
protected:
// begin JXLayout

    CTextInputField*     mName;
    JXTextCheckbox*      mEnabled;
    HPopupMenu*          mApplyPopup;
    CBetterScrollbarSet* mScroller;
    CBlankScrollable*    mScrollPane;
    JXEngravedRect*      mCriteria;
    JXTextButton*        mMoreTargetsBtn;
    JXTextButton*        mFewerTargetsBtn;
    JXTextButton*        mClearTargetsBtn;
    JXWidgetSet*         mBottomArea;
    JXTextButton*        mCancelBtn;
    JXTextButton*        mOKBtn;

// end JXLayout

	CCriteriaBaseList		mTargetItems;
	CTargetItem*			mTarget;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void	SetTarget(CTargetItem* target = NULL);

	CCriteriaBaseList& GetTargets()
		{ return mTargetItems; }

	void	InitTargets(const CFilterTargetList* targets);
	void	AddTarget(const CFilterTarget* target = NULL);
	void	RemoveTarget(unsigned long num = 1);
	void	ResizedTargets(int dy);

	void	ConstructTarget(CTargetItem* spec) const;
	CFilterTargetList*	ConstructTargets() const;

	void OnSetSchedule(JIndex nID);
	void OnMoreTargets();
	void OnFewerTargets();
	void OnClearTargets();
	void OnClearAllTargets();
};
#endif
