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


// Source for CRulesDialog class

#include "CRulesDialog.h"

#include "CActionItem.h"
#include "CBalloonDialog.h"
#include "CFilterItem.h"
#include "CFilterManager.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CResources.h"
#include "CRulesAction.h"
#include "CRulesActionLocal.h"
#include "CRulesActionSIEVE.h"
#include "CSearchCriteria.h"
#include "CSearchItem.h"
#include "CTargetItem.h"
#include "CTextDisplay.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

enum
{
	eTriggersMenu_Apply = 1,
	eTriggersMenuSeparator,
	eTriggersFirst
};

// Static members

CRulesDialog* CRulesDialog::sRulesDialog = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CRulesDialog::CRulesDialog() :
	CSearchBase(true)
{
	mChangedTriggers = false;
}

// Constructor from stream
CRulesDialog::CRulesDialog(LStream *inStream) :
	LDialogBox(inStream),
	CSearchBase(true)
{
	sRulesDialog = this;
	mChangedTriggers = false;
}

// Default destructor
CRulesDialog::~CRulesDialog()
{
	sRulesDialog = NULL;

	// Make sure window position is saved
	SaveState();
}

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________Visual, Commands, Messages

// Do various bits
void CRulesDialog::FinishCreateSelf()
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get all controls
	mName = (CTextFieldX*) FindPaneByID(paneid_RulesName);
	mTriggers = (LPopupButton*) FindPaneByID(paneid_RulesTriggers);
	mEditScript = (LCheckBox*) FindPaneByID(paneid_RulesEditScript);
	mFocus1 = (LView*) FindPaneByID(paneid_RulesFocus1);
	mFocus2 = (LView*) FindPaneByID(paneid_RulesFocus2);
	mScroller = (LView*) FindPaneByID(paneid_RulesScroller);
	mCriteria = (LView*) FindPaneByID(paneid_RulesCriteria);
	mCriteriaMove = (LView*) FindPaneByID(paneid_RulesCriteriaMove);
	mMoreBtn = (LPushButton*) FindPaneByID(paneid_RulesMore);
	mFewerBtn = (LPushButton*) FindPaneByID(paneid_RulesFewer);
	mClearBtn = (LPushButton*) FindPaneByID(paneid_RulesClear);
	mActions = (LView*) FindPaneByID(paneid_RulesActions);
	mActionsInitial = mActions->GetSubPanes().GetCount();
	mActionsMove = (LView*) FindPaneByID(paneid_RulesActionMove);
	mMoreActionsBtn = (LPushButton*) FindPaneByID(paneid_RulesMoreAction);
	mFewerActionsBtn = (LPushButton*) FindPaneByID(paneid_RulesFewerAction);
	mClearActionsBtn = (LPushButton*) FindPaneByID(paneid_RulesClearAction);
	mStop = (LCheckBox*) FindPaneByID(paneid_RulesStop);
	mScriptEdit = (CTextDisplay*) FindPaneByID(paneid_RulesScriptEdit);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CRulesDialogBtns);

	// Reset window state
	ResetState();
}

// Respond to clicks in the icon buttons
void CRulesDialog::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case msg_RulesTriggers:
		OnSetTrigger(*(long*) ioParam);
		break;

	case msg_RulesEditScript:
		OnEditScript(*(long*) ioParam);
		break;

	case msg_RulesMore:
		OnMore();
		break;

	case msg_RulesFewer:
		OnFewer();
		break;

	case msg_RulesClear:
		OnClear();
		break;

	case msg_RulesMoreActions:
		OnMoreActions();
		break;

	case msg_RulesFewerActions:
		OnFewerActions();
		break;

	case msg_RulesClearActions:
		OnClearActions();
		break;
	}
}

#pragma mark ____________________________Commands

void CRulesDialog::OnSetTrigger(long index)
{
	short mark = 0;
	::GetItemMark(mTriggers->GetMacMenuH(), index, &mark);
	::SetItemMark(mTriggers->GetMacMenuH(), index, (mark != noMark) ? (UInt16)noMark : (UInt16)checkMark);

	mChangedTriggers = true;
}

void CRulesDialog::OnEditScript(bool edit)
{
	
	if (edit)
	{
		// Convert current items to script
		CFilterItem spec(CFilterItem::eSIEVE);
		ConstructFilter(&spec, false, false);
		spec.SetUseScript(true);

		// Set script in display
		mScriptEdit->SetText(spec.GetScript());

		// Switch to script display
		mFocus1->Hide();
		mFocus2->Show();
	}
	else
	{
		mFocus2->Hide();
		mFocus1->Show();
	}

	// Always force reset
	OnClear();
	OnClearActions();
}

void CRulesDialog::OnMoreActions()
{
	AddAction();
}

void CRulesDialog::OnFewerActions()
{
	RemoveAction();
}

void CRulesDialog::OnClearActions()
{
	// Remove all but the first one
	while(mActions->GetSubPanes().GetCount() > mActionsInitial)
		RemoveAction();

	// Reset the first one
	AddAction(NULL);
}

#pragma mark ____________________________Criteria Panels

void CRulesDialog::SetFilter(CFilterItem* filter)
{
	if (filter)
	{
		mName->SetText(filter->GetName());
		mFilterType = filter->GetType();
		if (filter->GetType() == CFilterItem::eSIEVE)
			mEditScript->Show();
		else
			mEditScript->Hide();
		StopListening();
		mEditScript->SetValue(filter->GetUseScript());
		StartListening();
		mStop->SetValue(filter->Stop());
	}

	// Remove existing
	RemoveAllCriteria();
	while(mActions->GetSubPanes().GetCount() > mActionsInitial)
		RemoveAction();

	InitTriggers(filter);
	InitCriteria(filter ? filter->GetCriteria() : NULL);
	InitActions(filter ? filter->GetActions() : NULL);

	if (filter && filter->GetUseScript())
	{
		// Set script in display
		mScriptEdit->SetText(filter->GetScript());

		// Switch to script display
		mFocus1->Hide();
		mFocus2->Show();
	}
}

void CRulesDialog::InitTriggers(CFilterItem* filter)
{
	bool has_manual = filter && (filter->GetType() == CFilterItem::eLocal);

	// Remove any existing items from main menu
	short num_menu = ::CountMenuItems(mTriggers->GetMacMenuH());
	for(short i = has_manual ? eTriggersFirst : eTriggersMenu_Apply; i <= num_menu; i++)
		::DeleteMenuItem(mTriggers->GetMacMenuH(), has_manual ? eTriggersFirst : eTriggersMenu_Apply);

	// Set Apply item check mark
	if (has_manual)
		::SetItemMark(mTriggers->GetMacMenuH(), eTriggersMenu_Apply, filter->GetManual() ? (UInt16)checkMark : (UInt16)noMark);

	// Add all triggers/scripts
	if (filter)
	{
		short index = (has_manual ? eTriggersFirst : eTriggersMenu_Apply);
		if (filter->GetType() == CFilterItem::eLocal)
		{
			for(CTargetItemList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetTargets(filter->GetType()).begin();
					iter != CPreferences::sPrefs->GetFilterManager()->GetTargets(filter->GetType()).end(); iter++, index++)
			{
				// Add menu item
				::AppendItemToMenu(mTriggers->GetMacMenuH(), index, (*iter)->GetName());
				
				// Determine if this rule is in the trigger
				::SetItemMark(mTriggers->GetMacMenuH(), index, (*iter)->ContainsFilter(filter) ? (UInt16)checkMark : (UInt16)noMark);
			}
		}
		else
		{
			for(CFilterScriptList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetScripts(filter->GetType()).begin();
					iter != CPreferences::sPrefs->GetFilterManager()->GetScripts(filter->GetType()).end(); iter++, index++)
			{
				// Add menu item
				::AppendItemToMenu(mTriggers->GetMacMenuH(), index, (*iter)->GetName());
				
				// Determine if this rule is in the script
				::SetItemMark(mTriggers->GetMacMenuH(), index, (*iter)->ContainsFilter(filter) ? (UInt16)checkMark : (UInt16)noMark);
			}
		}
	}

	// Force max/min update
	mTriggers->SetMenuMinMax();
	mTriggers->SetValue(0);
}

const long cMaxWindowHeight = 400L;

void CRulesDialog::Resized(int dy)
{
	// Resize internal bits
	mCriteria->ResizeFrameBy(0, dy, true);
	mCriteriaMove->MoveBy(0, dy, true);
	mScroller->ResizeImageBy(0, dy, true);
}

#pragma mark ____________________________Actions Panels

void CRulesDialog::InitActions(const CActionItemList* actions)
{
	// Add each action
	if (actions)
	{
		for(CActionItemList::const_iterator iter = actions->begin(); iter != actions->end(); iter++)
			AddAction(*iter);
	}
	else
		AddAction();
}

const int cCriteriaHOffset = 4;
const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 14;

void CRulesDialog::AddAction(const CActionItem* spec)
{
	// Reanimate a new search criteria panel
	LCommander* defCommander;
	GetSubCommanders().FetchItemAt(1, defCommander);
	SetDefaultCommander(defCommander);
	SetDefaultView(this);
	
	PaneIDT paneid = 0;
	switch(mFilterType)
	{
	case CFilterItem::eLocal:
	default:
		paneid = paneid_RulesAction;
		break;
	case CFilterItem::eSIEVE:
		paneid = paneid_RulesActionS;
		break;
	}
	CRulesAction* action = (CRulesAction*) UReanimator::ReadObjects('PPob', paneid);
	action->FinishCreate();
	if (spec)
		action->SetActionItem(spec);

	// Get last view in action bottom
	SPoint32 new_pos = {cCriteriaHOffset, cCriteriaVInitOffset};
	if (mActions->GetSubPanes().GetCount() > mActionsInitial)
	{
		CRulesAction* prev = (CRulesAction*) mActions->GetSubPanes()[mActions->GetSubPanes().GetCount()];
		prev->SetBottom(false);

		action->SetBottom(true);

		// Now get position
		SDimension16 prev_size;
		prev->GetFrameSize(prev_size);
		SPoint32 prev_pos32;
		prev->GetFrameLocation(prev_pos32);
		Point prev_pos = {prev_pos32.v, prev_pos32.h};
		mActions->PortToLocalPoint(prev_pos);
		new_pos.v = prev_pos.v + prev_size.height;
	}
	else
	{
		action->SetTop(true);
		action->SetBottom(true);
	}

	// Put inside panel
	action->PutInside(mActions);

	// Now adjust sizes
	SDimension16 size;
	action->GetFrameSize(size);

	// Get size to increase
	SDimension16 psize;
	mActions->GetFrameSize(psize);
	
	// Resize groups so that width first inside criteria
	action->ResizeFrameBy(psize.width - cCriteriaHWidthAdjust - size.width, 0, false);

	ResizedActions(size.height);
	
	// Position new sub-panel
	action->PlaceInSuperFrameAt(new_pos.h, new_pos.v, false);
	action->Show();

	// Do button state
	mFewerActionsBtn->Show();
}

void CRulesDialog::RemoveAction()
{
	// Get last view in criteria
	LPane* action = mActions->GetSubPanes()[mActions->GetSubPanes().GetCount()];
	action->PutInside(NULL);

	// Now adjust sizes
	SDimension16 size;
	action->GetFrameSize(size);
	ResizedActions(-size.height);

	// Now delete the pane
	delete action;

	// Make sure new bottom is set
	if (mActions->GetSubPanes().GetCount() > mActionsInitial)
	{
		CRulesAction* prev = (CRulesAction*) mActions->GetSubPanes()[mActions->GetSubPanes().GetCount()];
		prev->SetBottom(true);
	}

	// Do button state
	if (mActions->GetSubPanes().GetCount() < mActionsInitial + 1)
		mFewerActionsBtn->Hide();
}

void CRulesDialog::ResizedActions(int dy)
{
	// Resize internal bits
	mCriteriaMove->ResizeFrameBy(0, dy, true);
	mActions->ResizeFrameBy(0, dy, true);
	mScroller->ResizeImageBy(0, dy, true);
}

#pragma mark ____________________________Build Search

void CRulesDialog::ConstructFilter(CFilterItem* spec, bool script, bool change_triggers) const
{
	// Get name
	spec->SetName(mName->GetText());

	// Get triggers
	if (change_triggers)
	{
		bool has_manual = (spec->GetType() == CFilterItem::eLocal);
		if (has_manual)
		{
			short mark = 0;
			::GetItemMark(mTriggers->GetMacMenuH(), eTriggersMenu_Apply, &mark);
			spec->SetManual(mark != noMark);
		}
		short num_menu = ::CountMenuItems(mTriggers->GetMacMenuH());
		short index = 0;
		for(short i = has_manual ? eTriggersFirst : eTriggersMenu_Apply; i <= num_menu; i++, index++)
		{
			// Check mark state
			short mark = 0;
			::GetItemMark(mTriggers->GetMacMenuH(), i, &mark);
			
			// Set the appropriate item
			if (mark != noMark)
			{
				if (spec->GetType() == CFilterItem::eLocal)
					CPreferences::sPrefs->GetFilterManager()->GetTargets(spec->GetType()).at(index)->AddFilter(spec);
				else
					CPreferences::sPrefs->GetFilterManager()->GetScripts(spec->GetType()).at(index)->AddFilter(spec);
			}
			else
			{
				if (spec->GetType() == CFilterItem::eLocal)
					CPreferences::sPrefs->GetFilterManager()->GetTargets(spec->GetType()).at(index)->RemoveFilter(spec);
				else
					CPreferences::sPrefs->GetFilterManager()->GetScripts(spec->GetType()).at(index)->RemoveFilter(spec);
			}
		}
	}

	// Get details
	if (script)
	{
		spec->SetUseScript(script);

		// Copy info from panel into prefs
		cdstring txt;
		mScriptEdit->GetText(txt);
		spec->SetScript(txt);
	}
	else
	{
		spec->SetUseScript(false);
		spec->SetCriteria(ConstructSearch());
		spec->SetActions(ConstructActions());
		spec->SetStop(mStop->GetValue());
	}
}

CActionItemList* CRulesDialog::ConstructActions() const
{
	CActionItemList* actions = new CActionItemList;

	// Count items
	long num = mActions->GetSubPanes().GetCount();
	for(long i = mActionsInitial + 1; i <= num; i++)
		actions->push_back(static_cast<const CRulesAction*>(mActions->GetSubPanes()[i])->GetActionItem());

	return actions;
}

// Reset state from prefs
void CRulesDialog::ResetState(void)
{
	CWindowState& state = CPreferences::sPrefs->mRulesDialogDefault.Value();

	// Do not set if empty
	Rect set_rect = state.GetBestRect(state);
	if (!::EmptyRect(&set_rect))
	{
		// Clip to screen
		::RectOnScreen(set_rect, this);

		// Reset bounds
		DoSetBounds(set_rect);
	}
}

// Save state in prefs
void CRulesDialog::SaveState(void)
{
	// Get name as cstr
	cdstring name;

	// Get bounds
	Rect bounds;
	bool zoomed = CalcStandardBounds(bounds);

	// Add info to prefs
	CWindowState state(name, &mUserBounds, eWindowStateNormal);
	if (CPreferences::sPrefs->mRulesDialogDefault.Value().Merge(state))
		CPreferences::sPrefs->mRulesDialogDefault.SetDirty();
}

bool CRulesDialog::PoseDialog(CFilterItem* spec, bool& trigger_change)
{
	bool result = false;
	trigger_change = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_RulesDialog, CMulberryApp::sApp);
	CRulesDialog* dlog = (CRulesDialog*) theHandler.GetDialog();
	dlog->SetFilter(spec);
	theHandler.GetDialog()->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			// Lock to prevent filter manager changes whilst running
			cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

			dlog->ConstructFilter(spec, ((CRulesDialog*) theHandler.GetDialog())->mEditScript->GetValue());
			trigger_change = dlog->mChangedTriggers;
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
