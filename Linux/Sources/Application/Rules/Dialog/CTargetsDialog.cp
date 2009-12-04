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

// CTargetsDialog.cpp : implementation file
//

#include "CTargetsDialog.h"

#include "CBetterScrollbarSet.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CResizeNotifier.h"
#include "CRulesTarget.h"
#include "CSearchWindow.h"
#include "CTextField.h"

#include "TPopupMenu.h"

#include <JXColormap.h>
#include <JXDisplay.h>
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
	eTarget_Never = 1,
	eTarget_Incoming,
	eTarget_OpenMailbox,
	eTarget_CloseMailbox,
	eTarget_Always
};

const char* cTargetAction = 
	"Never %r| Incoming Messages %r| Open Mailbox %r| Close Mailbox %r| Always %r";

/////////////////////////////////////////////////////////////////////////////
// CTargetsDialog dialog

CTargetsDialog::CTargetsDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
{
}

// Default destructor
CTargetsDialog::~CTargetsDialog()
{
}

/////////////////////////////////////////////////////////////////////////////
// CTargetsDialog message handlers


void CTargetsDialog::OnCreate()
{
// begin JXLayout

    JXWindow* window = new JXWindow(this, 500,175, "");
    assert( window != NULL );
    SetWindow(window);

    CResizeNotifier* container =
        new CResizeNotifier(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 500,175);
    assert( container != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Name:", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,12, 50,20);
    assert( obj1 != NULL );

    mName =
        new CTextInputField(container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 65,10, 240,20);
    assert( mName != NULL );

    mEnabled =
        new JXTextCheckbox("Enabled", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 325,10, 105,20);
    assert( mEnabled != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Trigger when:", container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,44, 80,20);
    assert( obj2 != NULL );

    mApplyPopup =
        new HPopupMenu("",container,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 95,40, 170,20);
    assert( mApplyPopup != NULL );

    mScroller =
        new CBetterScrollbarSet(container,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,65, 480,70);
    assert( mScroller != NULL );

    mScrollPane =
        new CBlankScrollable(mScroller, mScroller->GetScrollEnclosure(), // mScroller,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 460,60);
    assert( mScrollPane != NULL );

    mCriteria =
        new JXEngravedRect(mScrollPane,
                    JXWidget::kHElastic, JXWidget::kVElastic, 3,10, 445,10);
    assert( mCriteria != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Apply:", mScrollPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,1, 56,15);
    assert( obj3 != NULL );

    mMoreTargetsBtn =
        new JXTextButton("More Targets", mScrollPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 0,30, 105,25);
    assert( mMoreTargetsBtn != NULL );

    mFewerTargetsBtn =
        new JXTextButton("Fewer Targets", mScrollPane,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 115,30, 105,25);
    assert( mFewerTargetsBtn != NULL );

    mClearTargetsBtn =
        new JXTextButton("Reset", mScrollPane,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 385,30, 70,25);
    assert( mClearTargetsBtn != NULL );

    mBottomArea =
        new JXWidgetSet(container,
                    JXWidget::kHElastic, JXWidget::kFixedBottom, 2,140, 496,33);
    assert( mBottomArea != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 318,0, 70,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", mBottomArea,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 408,0, 70,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	// Do scroll pane init
	mScrollPane->Init();
	mScroller->SetAllowScroll(false, true);

	window->SetTitle("Edit Rule Target");
	SetButtons(mOKBtn, mCancelBtn);

	mApplyPopup->SetMenuItems(cTargetAction);
	mApplyPopup->SetValue(eTarget_Incoming);

	ListenTo(container);
	ListenTo(mApplyPopup);
	ListenTo(mMoreTargetsBtn);
	ListenTo(mFewerTargetsBtn);
	ListenTo(mClearTargetsBtn);
	
	// Turn off auto-place and lock
	UseModalPlacement(kFalse);

	// Set current width and height as minimum
	window->SetMinSize(500, 240);
}

// Handle buttons
void CTargetsDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXButton::kPushed))
	{
		if (sender == mMoreTargetsBtn)
		{
			OnMoreTargets();
			return;
		}
		else if (sender == mFewerTargetsBtn)
		{
			OnFewerTargets();
			return;
		}
		else if (sender == mClearTargetsBtn)
		{
			OnClearTargets();
			return;
		}
	}
	else if (message.Is(JXMenu::kItemSelected))
	{
		JIndex index = dynamic_cast<const JXMenu::ItemSelected*>(&message)->GetIndex();
    	if (sender == mApplyPopup)
    	{
			OnSetSchedule(index);
			return;
		}
	}

	CDialogDirector::Receive(sender, message);
}

void CTargetsDialog::Activate()
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

#pragma mark ____________________________Commands

void CTargetsDialog::OnSetSchedule(JIndex nID)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(nID - 1)
	{
	case CTargetItem::eNever:
	default:
		OnClearAllTargets();
		mCriteria->Deactivate();
		mMoreTargetsBtn->Deactivate();
		mFewerTargetsBtn->Deactivate();
		mClearTargetsBtn->Deactivate();
		break;
	case CTargetItem::eIncomingMailbox:
	case CTargetItem::eOpenMailbox:
	case CTargetItem::eCloseMailbox:
	case CTargetItem::eAll:
		mCriteria->Activate();
		mMoreTargetsBtn->Activate();
		mFewerTargetsBtn->Activate();
		mClearTargetsBtn->Activate();

		// Must have at least one target
		if (!mTargetItems.size())
			AddTarget();
		break;
	}
}

void CTargetsDialog::OnMoreTargets()
{
	AddTarget();
}

void CTargetsDialog::OnFewerTargets()
{
	RemoveTarget();
}

void CTargetsDialog::OnClearTargets()
{
	// Remove all but the first one
	if (mTargetItems.size() > 1)
		RemoveTarget(mTargetItems.size() - 1);
}

void CTargetsDialog::OnClearAllTargets()
{
	// Remove all but the first one
	if (mTargetItems.size())
		RemoveTarget(mTargetItems.size());
}

void CTargetsDialog::SetTarget(CTargetItem* target)
{
	if (target)
	{
		mName->SetText(target->GetName());
		mEnabled->SetState(JBoolean(target->IsEnabled()));

		mApplyPopup->SetValue(target->GetSchedule() + 1);
		OnSetSchedule(target->GetSchedule() + 1);
	}

	// Remove existing
	RemoveTarget(mTargetItems.size());

	InitTargets(target ? target->GetTargets() : NULL);
}

#pragma mark ____________________________Criteria

const long cMaxWindowHeight = 400L;

#pragma mark ____________________________Actions

void CTargetsDialog::InitTargets(const CFilterTargetList* targets)
{
	// Add each target
	if (targets)
	{
		for(CFilterTargetList::const_iterator iter = targets->begin(); iter != targets->end(); iter++)
			AddTarget(*iter);
	}
}

const int cTargetHOffset = 4;
const int cTargetVInitOffset = 5;
const int cTargetVOffset = 0;
const int cTargetHeight = 25;
const int cTargetWidth = 432;

void CTargetsDialog::AddTarget(const CFilterTarget* spec)
{
	// Create a new search criteria panel
	JRect r = mCriteria->GetFrame();
	r.Shift(-r.left, -r.top);
	r.left += cTargetHOffset;
	r.right -= cTargetHOffset;
	r.top = cTargetVInitOffset + mTargetItems.size() * (cTargetHeight + cTargetVOffset);
	r.bottom = r.top + cTargetHeight;
	
	// Create actual items
	CRulesTarget* target = new CRulesTarget(mCriteria, JXWidget::kHElastic, JXWidget::kFixedTop, r.left, r.top, cTargetWidth, cTargetHeight);
	target->OnCreate(this);

	// Adjust size to actual width of parent
	target->AdjustSize(r.width() - cTargetWidth, 0);
	
	if (spec)
		target->SetFilterTarget(spec);

	// Get last view in target bottom
	if (mTargetItems.size())
	{
		CRulesTarget* prev = static_cast<CRulesTarget*>(mTargetItems.back());
		prev->SetBottom(false);
		
		// Change current to add Or/And menu
		target->SetBottom(true);
	}
	else
	{
		target->SetTop(true);
		target->SetBottom(true);
	}

	// Add to list
	mTargetItems.push_back(target);

	// Now adjust sizes
	ResizedTargets(cTargetHeight + cTargetVOffset);
	
	// Do button state
	mFewerTargetsBtn->Show();
}

void CTargetsDialog::RemoveTarget(unsigned long num)
{
	for(unsigned long i = 1; i <= num; i++)
	{
		// Get last view in criteria
		JXWidget* target = mTargetItems.back();
		mTargetItems.pop_back();
		target->Hide();

		// Now delete the pane
		delete target;
	}

	// Now adjust sizes
	ResizedTargets((-cTargetHeight - cTargetVOffset) * num);
	
	if (mTargetItems.size())
	{
		CRulesTarget* prev = static_cast<CRulesTarget*>(mTargetItems.back());
		prev->SetBottom(true);
	}

	// Do button state
	if (mTargetItems.size() < 1)
		mFewerTargetsBtn->Hide();
}

const int cCriteriaInitialHeight = 10;
const int cCriteriaHeight = 25;
const int cCriteriaMaxHeight = cCriteriaInitialHeight + 5 * cCriteriaHeight;

void CTargetsDialog::ResizedTargets(int dy)
{
#if 0
	JRect frame = mCriteria->GetFrame();

	// Calculate the amount of window resizing which may be different than the amount of scroll resizing
	// due to clipping the window size
	int window_dy = 0;
	if (dy >= 0)
		// Increase in size up to max height always done
		window_dy = (frame.height() + dy <= cCriteriaMaxHeight) ? dy : 0;
	else
		window_dy = (frame.height() + dy >= cCriteriaMaxHeight) ? 0 :
						((frame.height() <= cCriteriaMaxHeight) ? dy :
							frame.height() + dy - cCriteriaMaxHeight);

	// Now adjust window size
	if (window_dy)
	{
		mScroller->AdjustSize(0, window_dy);
		AdjustSize(0, window_dy);
	}
#endif

	// Adjust scroll pane and scroll to bottom if scroller active
	mScrollPane->AdjustBounds(0, dy);
	if (dy > 0)
		mScrollPane->Scroll(0, -dy);
}

#pragma mark ____________________________Build

void CTargetsDialog::ConstructTarget(CTargetItem* spec) const
{
	spec->SetName(mName->GetText());
	spec->SetEnabled(mEnabled->IsChecked());

	spec->SetSchedule(static_cast<CTargetItem::ESchedule>(mApplyPopup->GetValue() - 1));
	spec->SetTargets(ConstructTargets());
}

CFilterTargetList* CTargetsDialog::ConstructTargets() const
{
	CFilterTargetList* targets = new CFilterTargetList;

	// Count items
	long num = mTargetItems.size();
	for(long i = 0; i < num; i++)
		targets->push_back(static_cast<const CRulesTarget*>(mTargetItems.at(i))->GetFilterTarget());

	return targets;
}

bool CTargetsDialog::PoseDialog(CTargetItem* spec)
{
	bool result = false;

	// Create the dialog
	CTargetsDialog* dlog = new CTargetsDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetTarget(spec);
	
	// Let DialogHandler process events
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->ConstructTarget(spec);
		result = true;
		dlog->Close();
	}

	return result;
}
