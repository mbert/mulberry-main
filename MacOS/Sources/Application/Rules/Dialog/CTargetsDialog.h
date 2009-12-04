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


// Header for CTargetsDialog class

#ifndef __CTARGETSDIALOG__MULBERRY__
#define __CTARGETSDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CTargetItem.h"

// Constants
const	PaneIDT		paneid_TargetsDialog = 1215;
const	PaneIDT		paneid_TargetsName = 'NAME';
const	PaneIDT		paneid_TargetsEnabled = 'ENBL';
const	PaneIDT		paneid_TargetsApply = 'APPL';
const	PaneIDT		paneid_TargetsScroller = 'SCRL';
const	PaneIDT		paneid_TargetsTargets = 'TARS';
const	PaneIDT		paneid_TargetsTargetMove = 'TAMV';
const	PaneIDT		paneid_TargetsMoreTarget = 'MTAR';
const	PaneIDT		paneid_TargetsFewerTarget = 'FTAR';
const	PaneIDT		paneid_TargetsClearTarget = 'CTAR';

// Messages
const	MessageT	msg_TargetsApply = 'APPL';
const	MessageT	msg_TargetsMoreTargets = 'MTAR';
const	MessageT	msg_TargetsFewerTargets = 'FTAR';
const	MessageT	msg_TargetsClearTargets = 'CTAR';

// Resources
const	ResIDT		RidL_CTargetsDialogBtns = 1215;

// Classes

class CTextFieldX;
class LCheckBox;
class LPushButton;
class LPopupButton;

class CTargetsDialog : public LDialogBox
{
public:
	enum { class_ID = 'Targ' };

					CTargetsDialog();
					CTargetsDialog(LStream *inStream);
	virtual 		~CTargetsDialog();

	static bool PoseDialog(CTargetItem* spec);

protected:
	static CTargetsDialog* sTargetsDialog;

	CTextFieldX*	mName;
	LCheckBox*		mEnabled;
	LPopupButton*	mApply;
	LView*			mScroller;
	LView*			mTargets;
	LView*			mTargetsMove;
	LPushButton*	mMoreTargetsBtn;
	LPushButton*	mFewerTargetsBtn;
	LPushButton*	mClearTargetsBtn;

	unsigned long	mTargetsInitial;

	virtual void	FinishCreateSelf(void);

	virtual void	ListenToMessage(MessageT inMessage,
										void *ioParam);				// Respond to clicks in the icon buttons

	void	OnSetSchedule(long item);
	void	OnMoreTargets();
	void	OnFewerTargets();
	void	OnClearTargets();
	void	OnClearAllTargets();

	void	SetTarget(CTargetItem* target = NULL);

	void	InitTargets(const CFilterTargetList* targets);
	void	AddTarget(const CFilterTarget* target = NULL);
	void	RemoveTarget();
	void	ResizedTargets(int dy);

	void	ConstructTarget(CTargetItem* spec) const;
	CFilterTargetList*	ConstructTargets() const;
};

#endif
