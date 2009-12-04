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

// CRulesDialog.cpp : implementation file
//

#include "CRulesDialog.h"

#include "CActionItem.h"
#include "CBetterScrollbarSet.h"
#include "CFilterItem.h"
#include "CFilterManager.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CMultibitPopup.h"
#include "CPreferences.h"
#include "CResizeNotifier.h"
#include "CRulesAction.h"
#include "CRulesActionLocal.h"
#include "CRulesActionSIEVE.h"
#include "CSearchCriteria.h"
#include "CSearchWindow.h"
#include "CTargetItem.h"
#include "CTextDisplay.h"
#include "CTextField.h"

#include <JXColormap.h>
#include <JXDisplay.h>
#include <JXDownRect.h>
#include <JXEngravedRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

enum
{
	eTriggersMenu_Apply = 1,
	//eTriggersMenuSeparator,
	eTriggersFirst
};

const char* cTriggers = 
	"Apply Rules Menu %b %l";

/////////////////////////////////////////////////////////////////////////////
// CRulesDialog dialog

CRulesDialog::CRulesDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor), CSearchBase(true)
{
	mChangedTriggers = false;
}

// Default destructor
CRulesDialog::~CRulesDialog()
{
	SaveDefaultState();
}

/////////////////////////////////////////////////////////////////////////////
// CRulesDialog message handlers


void CRulesDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 550,315, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* container =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 550,315);
    assert( container != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Name:", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,12, 50,20);
    assert( obj1 != NULL );

    mName =
        new CTextInputField(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 65,10, 170,20);
    assert( mName != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Triggered by:", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 250,10, 85,20);
    assert( obj2 != NULL );

    mTriggers =
        new CMultibitPopup("",container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 335,10, 30,20);
    assert( mTriggers != NULL );

    mEditScript =
        new JXTextCheckbox("Edit as Text", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 395,10, 95,20);
    assert( mEditScript != NULL );

    mScriptEdit =
        new CTextInputDisplay(container,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 530,240);
    assert( mScriptEdit != NULL );

    mScroller =
        new CBetterScrollbarSet(container,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,40, 530,240);
    assert( mScroller != NULL );

    mScrollPane =
        new CBlankScrollable(mScroller, mScroller->GetScrollEnclosure(), // mScriptEdit,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 510,135);
    assert( mScrollPane != NULL );

    mCriteria =
        new JXEngravedRect(mScrollPane,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 3,10, 495,10);
    assert( mCriteria != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Find Messages whose", mScrollPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,1, 136,14);
    assert( obj3 != NULL );

    mCriteriaMove =
        new JXWidgetSet(mScrollPane,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,30, 510,105);
    assert( mCriteriaMove != NULL );

    mMoreBtn =
        new JXTextButton("More Choices", mCriteriaMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,0, 105,26);
    assert( mMoreBtn != NULL );

    mFewerBtn =
        new JXTextButton("Fewer Choices", mCriteriaMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 120,0, 105,26);
    assert( mFewerBtn != NULL );

    mClearBtn =
        new JXTextButton("Reset", mCriteriaMove,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 430,0, 70,26);
    assert( mClearBtn != NULL );

    JXDownRect* obj4 =
        new JXDownRect(mCriteriaMove,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 5,30, 495,3);
    assert( obj4 != NULL );

    mActions =
        new JXEngravedRect(mCriteriaMove,
                    JXWidget::kHElastic, JXWidget::kVElastic, 3,49, 495,10);
    assert( mActions != NULL );

    JXStaticText* obj5 =
        new JXStaticText("Actions", mCriteriaMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,40, 56,14);
    assert( obj5 != NULL );

    mMoreActionsBtn =
        new JXTextButton("More Actions", mCriteriaMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 5,68, 105,26);
    assert( mMoreActionsBtn != NULL );

    mFewerActionsBtn =
        new JXTextButton("FewerActions", mCriteriaMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 120,68, 105,26);
    assert( mFewerActionsBtn != NULL );

    mStop =
        new JXTextCheckbox("Stop if Matched", mCriteriaMove,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 230,70, 125,20);
    assert( mStop != NULL );

    mClearActionsBtn =
        new JXTextButton("Reset", mCriteriaMove,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 430,68, 70,26);
    assert( mClearActionsBtn != NULL );

    JXDownRect* obj6 =
        new JXDownRect(mCriteriaMove,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 5,99, 495,3);
    assert( obj6 != NULL );

    mBottomArea =
        new JXWidgetSet(container,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 2,282, 546,33);
    assert( mBottomArea != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 368,0, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 458,0, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	// Do scroll pane init
	mScrollPane->Init();
	mScroller->SetAllowScroll(false, true);

	window->SetTitle("Edit Rules");
	SetButtons(mOKBtn, mCancelBtn);

	mTriggers->SetMenuItems(cTriggers);
	mTriggers->SetValue(eTriggersMenu_Apply);

	mScriptEdit->Hide();

	ListenTo(container);
	ListenTo(mTriggers);
	ListenTo(mEditScript);
	ListenTo(mMoreBtn);
	ListenTo(mFewerBtn);
	ListenTo(mClearBtn);
	ListenTo(mMoreActionsBtn);
	ListenTo(mFewerActionsBtn);
	ListenTo(mClearActionsBtn);
	
	// Turn off auto-place and lock
	UseModalPlacement(kFalse);

	// Set current width and height as minimum
	window->SetMinSize(550, 315);
	
	ResetState();
}

#pragma mark ____________________________Commands

void CRulesDialog::OnSetTrigger(JIndex index)
{
	// The multibit menu takes care of changing its own state
	mChangedTriggers = true;
}

// Handle buttons
void CRulesDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mMoreBtn)
		{
			OnMore();
			return;
		}
		else if (sender == mFewerBtn)
		{
			OnFewer();
			return;
		}
		else if (sender == mClearBtn)
		{
			OnClear();
			return;
		}
		else if (sender == mMoreActionsBtn)
		{
			OnMoreActions();
			return;
		}
		else if (sender == mFewerActionsBtn)
		{
			OnFewerActions();
			return;
		}
		else if (sender == mClearActionsBtn)
		{
			OnClearActions();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mEditScript)
		{
			OnEditScript(mEditScript->IsChecked());
			return;
			
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mTriggers)
    	{
			OnSetTrigger(index);
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

void CRulesDialog::Activate()
{
	// When activating do modal dialog window placement
	if (!IsActive())
	{
		JXWindow* window = GetWindow();
		window->PlaceAsDialogWindow();
	}

	// Do inherited
	CDialogDirector::Activate();
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
	if (mActionItems.size() > 1)
		RemoveAction(mActionItems.size() - 1);

	// Reset the first one
	CRulesAction* action = static_cast<CRulesAction*>(mActionItems.at(0));
	action->SetActionItem(NULL);
}

void CRulesDialog::OnEditScript(bool edit)
{
	if (mEditScript->IsChecked())
	{
		// Convert current items to script
		CFilterItem spec(CFilterItem::eSIEVE);
		ConstructFilter(&spec, false, false);
		spec.SetUseScript(true);

		// Set script in display
		mScriptEdit->SetText(spec.GetScript());

		// Switch to script display
		mScroller->Hide();
		mScriptEdit->Show();
	}
	else
	{
		mScriptEdit->Hide();
		mScroller->Show();
	}

	// Always force reset
	OnClear();
	OnClearActions();
}

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
		StopListening(mEditScript);
		mEditScript->SetState(JBoolean(filter->GetUseScript()));
		ListenTo(mEditScript);
		mStop->SetState(JBoolean(filter->Stop()));
	}

	// Remove existing
	RemoveAllCriteria();
	RemoveAction(mActionItems.size());

	InitTriggers(filter);
	InitCriteria(filter ? filter->GetCriteria() : NULL);
	InitActions(filter ? filter->GetActions() : NULL);

	if (filter && filter->GetUseScript())
	{
		// Set script in display
		mScriptEdit->SetText(filter->GetScript());

		// Switch to script display
		mScroller->Hide();
		mScriptEdit->Show();
	}
}

void CRulesDialog::InitTriggers(CFilterItem* filter)
{
	bool has_manual = filter && (filter->GetType() == CFilterItem::eLocal);

	// Remove any existing items from main menu
	mTriggers->RemoveAllItems();

	// Set Apply item check mark
	if (has_manual)
	{
		mTriggers->SetMenuItems(cTriggers);
		mTriggers->InitBits();
		if (filter->GetManual())
			mTriggers->SetBit(eTriggersMenu_Apply);
	}

	// Add all triggers/scripts
	if (filter)
	{
		JIndex index = (has_manual ? eTriggersFirst : eTriggersMenu_Apply);
		if (filter->GetType() == CFilterItem::eLocal)
		{
			for(CTargetItemList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetTargets(filter->GetType()).begin();
					iter != CPreferences::sPrefs->GetFilterManager()->GetTargets(filter->GetType()).end(); iter++, index++)
			{
				// Add menu item
				mTriggers->AppendItem((*iter)->GetName(), kTrue, kFalse);
				mTriggers->InitBits();
				
				// Determine if this rule is in the trigger
				if ((*iter)->ContainsFilter(filter))
					mTriggers->SetBit(index);
			}
		}
		else
		{
			for(CFilterScriptList::iterator iter = CPreferences::sPrefs->GetFilterManager()->GetScripts(filter->GetType()).begin();
					iter != CPreferences::sPrefs->GetFilterManager()->GetScripts(filter->GetType()).end(); iter++, index++)
			{
				// Add menu item
				mTriggers->AppendItem((*iter)->GetName(), kTrue, kFalse);
				mTriggers->InitBits();
				
				// Determine if this rule is in the trigger
				if ((*iter)->ContainsFilter(filter))
					mTriggers->SetBit(index);
			}
		}
	}

	// Force max/min update
	mTriggers->SetValue(0);
}

#pragma mark ____________________________Criteria

void CRulesDialog::Resized(int dy)
{
	// Adjust scroll pane and scroll to bottom if scroller active
	mScrollPane->AdjustBounds(0, dy);
	if (dy > 0)
		mScrollPane->Scroll(0, -dy);

	mCriteria->AdjustSize(0, dy);
	mCriteriaMove->Move(0, dy);
}

#pragma mark ____________________________Actions

void CRulesDialog::InitActions(const CActionItemList* actions)
{
	// Add each action
	if (actions && actions->size())
	{
		for(CActionItemList::const_iterator iter = actions->begin(); iter != actions->end(); iter++)
			AddAction(*iter);
	}
	else
		AddAction();
}

const int cActionHOffset = 4;
const int cActionVInitOffset = 5;
const int cActionVOffset = 0;
const int cActionHeight = 25;
const int cActionWidth = 492;

void CRulesDialog::AddAction(const CActionItem* spec)
{
	// Create a new search criteria panel
	JRect r = mActions->GetFrame();
	r.Shift(-r.left, -r.top);
	r.left += cActionHOffset;
	r.right -= cActionHOffset;
	r.top = cActionVInitOffset + mActionItems.size() * (cActionHeight + cActionVOffset);
	r.bottom = r.top + cActionHeight;
	CRulesAction* action = NULL;
	switch(mFilterType)
	{
	case CFilterItem::eLocal:
	default:
		action = new CRulesActionLocal(mActions, JXWidget::kHElastic, JXWidget::kFixedTop, r.left, r.top, cActionWidth, cActionHeight);
		break;
	case CFilterItem::eSIEVE:
		action = new CRulesActionSIEVE(mActions, JXWidget::kHElastic, JXWidget::kFixedTop, r.left, r.top, cActionWidth, cActionHeight);
		break;
	}

	// Create actual items
	action->OnCreate(this);
	
	// Adjust size to actual width of parent
	action->AdjustSize(r.width() - cActionWidth, 0);
	
	// Set any input spec
	if (spec)
		action->SetActionItem(spec);

	// Get last view in criteria bottom
	if (mActionItems.size())
	{
		CRulesAction* prev = static_cast<CRulesAction*>(mActionItems.back());
		prev->SetBottom(false);
		
		// Change current to add Or/And menu
		action->SetBottom(true);
	}
	else
	{
		action->SetTop(true);
		action->SetBottom(true);
	}

	// Add to list
	mActionItems.push_back(action);

	// Now adjust sizes
	ResizedActions(cActionHeight + cActionVOffset);
	
	// Do button state
	mFewerActionsBtn->Show();
}

void CRulesDialog::RemoveAction(unsigned long num)
{
	for(unsigned long i = 1; i <= num; i++)
	{
		// Get last view in criteria
		JXWidget* action = mActionItems.back();
		mActionItems.pop_back();
		action->Hide();

		// Now delete the pane
		delete action;
	}

	// Now adjust sizes
	ResizedActions((-cActionHeight - cActionVOffset) * num);

	// Set up/down button state
	if (mActionItems.size())
	{
		CRulesAction* prev = static_cast<CRulesAction*>(mActionItems.back());
		prev->SetBottom(true);
	}

	// Do button state
	if (mActionItems.size() == 0)
		mFewerActionsBtn->Hide();
}

void CRulesDialog::ResizedActions(int dy)
{
	// Adjust scroll pane and scroll to bottom if scroller active
	mScrollPane->AdjustBounds(0, dy);
	if (dy > 0)
		mScrollPane->Scroll(0, -dy);

	mCriteriaMove->AdjustSize(0, dy);
	//mActions->AdjustSize(0, dy);
}

#pragma mark ____________________________Build

void CRulesDialog::ConstructFilter(CFilterItem* spec, bool script, bool change_triggers) const
{
	spec->SetName(mName->GetText());

	// Get triggers
	if (change_triggers)
	{
		bool has_manual = (spec->GetType() == CFilterItem::eLocal);
		if (has_manual)
			spec->SetManual(mTriggers->GetBit(eTriggersMenu_Apply));

		JSize num_menu = mTriggers->GetItemCount();
		JIndex index = 0;
		for(JIndex i = has_manual ? eTriggersFirst : eTriggersMenu_Apply; i <= num_menu; i++, index++)
		{
			// Check mark state and set the appropriate item
			if (mTriggers->GetBit(i))
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
		spec->SetScript(mScriptEdit->GetText());
	}
	else
	{
		spec->SetUseScript(false);
		spec->SetCriteria(ConstructSearch());
		spec->SetActions(ConstructActions());
		spec->SetStop(mStop->IsChecked());
	}
}

CActionItemList* CRulesDialog::ConstructActions() const
{
	CActionItemList* actions = new CActionItemList;

	// Count items
	long num = mActionItems.size();
	for(long i = 0; i < num; i++)
		actions->push_back(static_cast<CRulesAction*>(mActionItems.at(i))->GetActionItem());

	return actions;
}

bool CRulesDialog::PoseDialog(CFilterItem* spec, bool& trigger_change)
{
	bool result = false;

	// Create the dialog
	CRulesDialog* dlog = new CRulesDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetFilter(spec);

	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		// Lock to prevent filter manager changes whilst running
		cdmutex::lock_cdmutex _lock(CPreferences::sPrefs->GetFilterManager()->RunningLock());

		dlog->ConstructFilter(spec, dlog->mEditScript->IsChecked());
		trigger_change = dlog->mChangedTriggers;
		result = true;
		dlog->Close();
	}

	return result;
}

#pragma mark ____________________________Window State

// Reset state from prefs
void CRulesDialog::ResetState()
{
	// Get name as cstr
	char name = 0;

	// Get default state
	CWindowState* state = &CPreferences::sPrefs->mRulesDialogDefault.Value();

	// Do not set if empty
	JRect set_rect = state->GetBestRect(*state);
	if (set_rect.left && set_rect.top)
	{
		// Clip to screen
		::RectOnScreen(set_rect);

		// Reset position
		GetWindow()->Place(set_rect.left, set_rect.top);
		GetWindow()->SetSize(set_rect.width(), set_rect.height());
	}
}

// Save current state in prefs
void CRulesDialog::SaveDefaultState(void)
{
	// Get bounds - convert to position only
	JPoint p = GetWindow()->GetDesktopLocation();
	JCoordinate w = GetWindow()->GetFrameWidth();
	JCoordinate h = GetWindow()->GetFrameHeight();
	JRect bounds(p.y, p.x, p.y + h, p.x + w);

	// Add info to prefs
	CWindowState state(NULL, &bounds, eWindowStateNormal);
	if (CPreferences::sPrefs->mRulesDialogDefault.Value().Merge(state))
		CPreferences::sPrefs->mRulesDialogDefault.SetDirty();
}
