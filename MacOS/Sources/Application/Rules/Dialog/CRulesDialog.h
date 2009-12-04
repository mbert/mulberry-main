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


// Header for CRulesDialog class

#ifndef __CRULESDIALOG__MULBERRY__
#define __CRULESDIALOG__MULBERRY__

#include <LDialogBox.h>
#include "CSearchBase.h"

#include "CActionItem.h"

#include <LPushButton.h>

// Constants
const	PaneIDT		paneid_RulesDialog = 1210;
const	PaneIDT		paneid_RulesName = 'NAME';
const	PaneIDT		paneid_RulesTriggers = 'TRIG';
const	PaneIDT		paneid_RulesEditScript = 'SCRP';
const	PaneIDT		paneid_RulesFocus1 = 'FOC1';
const	PaneIDT		paneid_RulesFocus2 = 'FOC2';
const	PaneIDT		paneid_RulesScroller = 'SCRL';
const	PaneIDT		paneid_RulesCriteria = 'CRIT';
const	PaneIDT		paneid_RulesCriteriaMove = 'CRMV';
const	PaneIDT		paneid_RulesMore = 'MCHS';
const	PaneIDT		paneid_RulesFewer = 'FCHS';
const	PaneIDT		paneid_RulesClear = 'CLRS';
const	PaneIDT		paneid_RulesActions = 'ACTS';
const	PaneIDT		paneid_RulesActionMove = 'ACMV';
const	PaneIDT		paneid_RulesMoreAction = 'MACT';
const	PaneIDT		paneid_RulesFewerAction = 'FACT';
const	PaneIDT		paneid_RulesClearAction = 'CACT';
const	PaneIDT		paneid_RulesStop = 'STOP';
const	PaneIDT		paneid_RulesScriptEdit = 'TEXT';

// Messages
const	MessageT	msg_RulesTriggers = 'TRIG';
const	MessageT	msg_RulesEditScript = 'SCRP';
const	MessageT	msg_RulesMore = 'MCHS';
const	MessageT	msg_RulesFewer = 'FCHS';
const	MessageT	msg_RulesClear = 'CLRS';
const	MessageT	msg_RulesMoreActions = 'MACT';
const	MessageT	msg_RulesFewerActions = 'FACT';
const	MessageT	msg_RulesClearActions = 'CACT';

// Resources
const	ResIDT		RidL_CRulesDialogBtns = 1210;

// Classes

class CActionItem;
class CFilterItem;
class CSearchItem;
class CTextDisplay;
class CTextFieldX;
class LCheckBox;
class LPopupButton;
class LPushButton;

class CRulesDialog : public LDialogBox,
						public CSearchBase
{
	friend class CRulesAction;

public:
	enum { class_ID = 'Rule' };

					CRulesDialog();
					CRulesDialog(LStream *inStream);
	virtual 		~CRulesDialog();

	static bool PoseDialog(CFilterItem* spec, bool& trigger_change);

protected:
	static CRulesDialog* sRulesDialog;

	CTextFieldX*	mName;
	LPopupButton*	mTriggers;
	LCheckBox*		mEditScript;
	LView*			mFocus1;
	LView*			mFocus2;
	LView*			mScroller;
	LView*			mCriteria;
	LView*			mCriteriaMove;
	LPushButton*	mMoreBtn;
	LPushButton*	mFewerBtn;
	LPushButton*	mClearBtn;
	LView*			mActions;
	LView*			mActionsMove;
	LPushButton*	mMoreActionsBtn;
	LPushButton*	mFewerActionsBtn;
	LPushButton*	mClearActionsBtn;
	LCheckBox*		mStop;
	CTextDisplay*	mScriptEdit;

	unsigned long	mActionsInitial;
	bool			mChangedTriggers;

	virtual void	FinishCreateSelf(void);

	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

	void	OnSetTrigger(long index);
	void	OnEditScript(bool edit);
	void	OnMoreActions();
	void	OnFewerActions();
	void	OnClearActions();

	void	SetFilter(CFilterItem* filter = NULL);
	void	InitTriggers(CFilterItem* filter);

	virtual LWindow* GetContainerWnd()
		{ return this; }
	virtual LView* GetParentView()
		{ return mCriteria; }
	virtual LPane* GetFewerBtn()
		{ return mFewerBtn; }

	virtual void Resized(int dy);

	void	InitActions(const CActionItemList* actions);
	void	AddAction(const CActionItem* action = NULL);
	void	RemoveAction();
	void	ResizedActions(int dy);

	void				ConstructFilter(CFilterItem* spec, bool script, bool change_triggers = true) const;
	CActionItemList*	ConstructActions() const;

private:
	virtual void	ResetState(void);					// Reset state from prefs
	virtual void	SaveState(void);					// Save state as default
};

#endif
