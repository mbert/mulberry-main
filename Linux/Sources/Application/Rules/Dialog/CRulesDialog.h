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


// CRulesDialog.h : header file
//

#ifndef __CRULESDIALOG__MULBERRY__
#define __CRULESDIALOG__MULBERRY__

#include "CDialogDirector.h"
#include "CSearchBase.h"

#include "CActionItem.h"

#include <JXEngravedRect.h>
#include <JXScrollableWidget.h>

/////////////////////////////////////////////////////////////////////////////
// CRulesDialog dialog

class CFilterItem;
class CTextInputDisplay;
class CTextInputField;
class JXEngravedRect;
class CBetterScrollbarSet;
class CBlankScrollable;
class JXScrollableWidget;
class JXTextButton;
class JXTextCheckbox;
class JXWidgetSet;
class CMultibitPopup;

class CCriteriaBase;
typedef std::vector<CCriteriaBase*> CCriteriaBaseList;

class CRulesDialog : public CDialogDirector,
						public CSearchBase
{
	friend class CRulesAction;

// Construction
public:
	CRulesDialog(JXDirector* supervisor);   // standard constructor
	virtual ~CRulesDialog();

	static bool PoseDialog(CFilterItem* spec, bool& trigger_change);

	virtual void Activate();

// Implementation
protected:
// begin JXLayout

    CTextInputField*     mName;
    CMultibitPopup*      mTriggers;
    JXTextCheckbox*      mEditScript;
    CTextInputDisplay*   mScriptEdit;
    CBetterScrollbarSet* mScroller;
    CBlankScrollable*    mScrollPane;
    JXEngravedRect*      mCriteria;
    JXWidgetSet*         mCriteriaMove;
    JXTextButton*        mMoreBtn;
    JXTextButton*        mFewerBtn;
    JXTextButton*        mClearBtn;
    JXEngravedRect*      mActions;
    JXTextButton*        mMoreActionsBtn;
    JXTextButton*        mFewerActionsBtn;
    JXTextCheckbox*      mStop;
    JXTextButton*        mClearActionsBtn;
    JXWidgetSet*         mBottomArea;
    JXTextButton*        mCancelBtn;
    JXTextButton*        mOKBtn;

// end JXLayout

	CCriteriaBaseList		mActionItems;
	CFilterItem*			mFilter;
	bool					mChangedTriggers;
	int						mScrollerImageHeight;

	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

	void	SetFilter(CFilterItem* filter = NULL);
	void	InitTriggers(CFilterItem* filter);

	virtual JXWidget* GetContainerWnd()
		{ return mCriteria; }
	virtual CWindow* GetParentView()
		{ return NULL; }
	virtual JXTextButton* GetFewerBtn()
		{ return mFewerBtn; }

	virtual void Resized(int dy);

	CCriteriaBaseList& GetActions()
		{ return mActionItems; }

	void	InitActions(const CActionItemList* actions);
	void	AddAction(const CActionItem* action = NULL);
	void	RemoveAction(unsigned long num = 1);
	void	ResizedActions(int dy);

	void				ConstructFilter(CFilterItem* spec, bool script, bool change_triggers = true) const;
	CActionItemList*	ConstructActions() const;

	void OnSetTrigger(JIndex index);
	void OnEditScript(bool edit);
	void OnMoreActions();
	void OnFewerActions();
	void OnClearActions();
	void OnEditScript();

	virtual void	ResetState();					// Reset state from prefs
	virtual void	SaveDefaultState();				// Save state in prefs
};
#endif
