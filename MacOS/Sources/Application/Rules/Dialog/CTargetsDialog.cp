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


// Source for CTargetsDialog class

#include "CTargetsDialog.h"

#include "CBalloonDialog.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CResources.h"
#include "CRulesTarget.h"
#include "CTextFieldX.h"

#include <LCheckBox.h>
#include <LPushButton.h>
#include <LPopupButton.h>

// __________________________________________________________________________________________________
// C L A S S __ C A D D R E S S S E A R C H W I N D O W
// __________________________________________________________________________________________________

// Static members

CTargetsDialog* CTargetsDialog::sTargetsDialog = NULL;

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
CTargetsDialog::CTargetsDialog()
{
}

// Constructor from stream
CTargetsDialog::CTargetsDialog(LStream *inStream)
		: LDialogBox(inStream)
{
	sTargetsDialog = this;
}

// Default destructor
CTargetsDialog::~CTargetsDialog()
{
	sTargetsDialog = NULL;
}

// O T H E R  M E T H O D S ____________________________________________________________________________

#pragma mark ____________________________Visual, Commands, Messages

// Do various bits
void CTargetsDialog::FinishCreateSelf()
{
	// Do inherited
	LDialogBox::FinishCreateSelf();

	// Get all controls
	mName = (CTextFieldX*) FindPaneByID(paneid_TargetsName);
	mEnabled = (LCheckBox*) FindPaneByID(paneid_TargetsEnabled);
	mApply = (LPopupButton*) FindPaneByID(paneid_TargetsApply);
	mScroller = (LView*) FindPaneByID(paneid_TargetsScroller);
	mTargets = (LView*) FindPaneByID(paneid_TargetsTargets);
	mTargetsInitial = mTargets->GetSubPanes().GetCount();
	mTargetsMove = (LView*) FindPaneByID(paneid_TargetsTargetMove);
	mMoreTargetsBtn = (LPushButton*) FindPaneByID(paneid_TargetsMoreTarget);
	mFewerTargetsBtn = (LPushButton*) FindPaneByID(paneid_TargetsFewerTarget);
	mClearTargetsBtn = (LPushButton*) FindPaneByID(paneid_TargetsClearTarget);

	// Link controls to this window
	UReanimator::LinkListenerToBroadcasters(this, this, RidL_CTargetsDialogBtns);

} // CTargetsDialog::FinishCreateSelf

// Respond to clicks in the icon buttons
void CTargetsDialog::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
		case msg_TargetsApply:
			OnSetSchedule(*(long*) ioParam);
			break;

		case msg_TargetsMoreTargets:
			OnMoreTargets();
			break;

		case msg_TargetsFewerTargets:
			OnFewerTargets();
			break;

		case msg_TargetsClearTargets:
			OnClearTargets();
			break;
	}
}

#pragma mark ____________________________Commands

void CTargetsDialog::OnSetSchedule(long item1)
{
	// Set popup menu for method and show/hide text field as approriate
	switch(item1 - 1)
	{
	case CTargetItem::eNever:
	default:
		OnClearAllTargets();
		mTargets->Disable();
		mMoreTargetsBtn->Disable();
		mFewerTargetsBtn->Disable();
		mClearTargetsBtn->Disable();
		break;
	case CTargetItem::eAll:
	case CTargetItem::eIncomingMailbox:
	case CTargetItem::eOpenMailbox:
	case CTargetItem::eCloseMailbox:
		mTargets->Enable();
		mMoreTargetsBtn->Enable();
		mFewerTargetsBtn->Enable();
		mClearTargetsBtn->Enable();
		
		// Must have at least one target
		if (mTargets->GetSubPanes().GetCount() == mTargetsInitial)
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
	while(mTargets->GetSubPanes().GetCount() > mTargetsInitial + 1)
		RemoveTarget();
}

void CTargetsDialog::OnClearAllTargets()
{
	// Remove all
	while(mTargets->GetSubPanes().GetCount() > mTargetsInitial)
		RemoveTarget();
}

#pragma mark ____________________________Criteria Panels

void CTargetsDialog::SetTarget(CTargetItem* target)
{
	if (target)
	{
		mName->SetText(target->GetName());
		mEnabled->SetValue(target->IsEnabled());

		mApply->SetValue(target->GetSchedule() + 1);
		OnSetSchedule(target->GetSchedule() + 1);
	}

	// Remove existing
	while(mTargets->GetSubPanes().GetCount() > mTargetsInitial)
		RemoveTarget();

	InitTargets(target ? target->GetTargets() : NULL);
}

const int cCriteriaHOffset = 4;
const int cCriteriaHWidthAdjust = 8;
const int cCriteriaVInitOffset = 14;

#pragma mark ____________________________Targets Panels

void CTargetsDialog::InitTargets(const CFilterTargetList* targets)
{
	// Add each action
	if (targets)
	{
		for(CFilterTargetList::const_iterator iter = targets->begin(); iter != targets->end(); iter++)
			AddTarget(*iter);
	}
}

void CTargetsDialog::AddTarget(const CFilterTarget* spec)
{
	// Reanimate a new search criteria panel
	LCommander* defCommander;
	GetSubCommanders().FetchItemAt(1, defCommander);
	SetDefaultCommander(defCommander);
	SetDefaultView(this);
	CRulesTarget* target = (CRulesTarget*) UReanimator::ReadObjects('PPob', paneid_RulesTarget);
	target->FinishCreate();
	if (spec)
		target->SetFilterTarget(spec);

	// Get last view in action bottom
	SPoint32 new_pos = {cCriteriaHOffset, cCriteriaVInitOffset};
	if (mTargets->GetSubPanes().GetCount() > mTargetsInitial)
	{
		CRulesTarget* prev = (CRulesTarget*) mTargets->GetSubPanes()[mTargets->GetSubPanes().GetCount()];
		prev->SetBottom(false);

		target->SetBottom(true);

		// Now get position
		SDimension16 prev_size;
		prev->GetFrameSize(prev_size);
		SPoint32 prev_pos32;
		prev->GetFrameLocation(prev_pos32);
		Point prev_pos = {prev_pos32.v, prev_pos32.h};
		mTargets->PortToLocalPoint(prev_pos);
		new_pos.v = prev_pos.v + prev_size.height;
	}
	else
	{
		target->SetTop(true);
		target->SetBottom(true);
	}

	// Put inside panel
	target->PutInside(mTargets);

	// Now adjust sizes
	SDimension16 size;
	target->GetFrameSize(size);

	// Get size to increase
	SDimension16 psize;
	mTargets->GetFrameSize(psize);
	
	// Resize groups so that width first inside criteria
	target->ResizeFrameBy(psize.width - cCriteriaHWidthAdjust - size.width, 0, false);

	ResizedTargets(size.height);
	
	// Position new sub-panel
	target->PlaceInSuperFrameAt(new_pos.h, new_pos.v, false);
	target->Show();

	// Do button state
	mFewerTargetsBtn->Show();
}

void CTargetsDialog::RemoveTarget()
{
	// Get last view in criteria
	LPane* target = mTargets->GetSubPanes()[mTargets->GetSubPanes().GetCount()];
	target->PutInside(NULL);

	// Now adjust sizes
	SDimension16 size;
	target->GetFrameSize(size);
	ResizedTargets(-size.height);

	// Now delete the pane
	delete target;

	if (mTargets->GetSubPanes().GetCount() > mTargetsInitial)
	{
		CRulesTarget* prev = (CRulesTarget*) mTargets->GetSubPanes()[mTargets->GetSubPanes().GetCount()];
		prev->SetBottom(true);
	}

	// Do button state
	if (mTargets->GetSubPanes().GetCount() < mTargetsInitial + 1)
		mFewerTargetsBtn->Hide();
}

const long cMaxWindowHeight = 400L;

void CTargetsDialog::ResizedTargets(int dy)
{
	// Resize internal bits
	mTargets->ResizeFrameBy(0, dy, true);
	mTargetsMove->MoveBy(0, dy, true);
	mScroller->ResizeImageBy(0, dy, true);
	
	// Check for resize of window
	SDimension32 image;
	mScroller->GetImageSize(image);
	SDimension16 frame;
	mScroller->GetFrameSize(frame);
	
	// Check for shrink
	if (image.height < frame.height)
		ResizeWindowBy(0, image.height - frame.height);
	else if (image.height > frame.height)
	{
		Rect bounds;
		GetGlobalBounds(bounds);
		
		if (bounds.bottom - bounds.top < cMaxWindowHeight)
			ResizeWindowBy(0, std::min(cMaxWindowHeight - (bounds.bottom - bounds.top), image.height - frame.height));
	}
	
	// Always scroll to bottom if enlarging
	if (dy > 0)
		mScroller->ScrollPinnedImageBy(0, dy, true);
}


#pragma mark ____________________________Build Search

void CTargetsDialog::ConstructTarget(CTargetItem* spec) const
{
	spec->SetName(mName->GetText());
	spec->SetEnabled(mEnabled->GetValue());

	spec->SetSchedule(static_cast<CTargetItem::ESchedule>(mApply->GetValue() - 1));

	spec->SetTargets(ConstructTargets());
}

CFilterTargetList* CTargetsDialog::ConstructTargets() const
{
	CFilterTargetList* targets = new CFilterTargetList;

	// Count items
	long num = mTargets->GetSubPanes().GetCount();
	for(long i = mTargetsInitial + 1; i <= num; i++)
		targets->push_back(static_cast<const CRulesTarget*>(mTargets->GetSubPanes()[i])->GetFilterTarget());

	return targets;
}

bool CTargetsDialog::PoseDialog(CTargetItem* spec)
{
	bool result = false;

	// Create the dialog
	CBalloonDialog	theHandler(paneid_TargetsDialog, CMulberryApp::sApp);
	((CTargetsDialog*) theHandler.GetDialog())->SetTarget(spec);
	theHandler.GetDialog()->Show();

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			((CTargetsDialog*) theHandler.GetDialog())->ConstructTarget(spec);
			result = true;
			break;
		}
		else if (hitMessage == msg_Cancel)
			break;
	}
	
	return result;
}
