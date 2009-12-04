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


// Header for CRulesActionLocal class

#ifndef __CRULESACTIONLOCAL__MULBERRY__
#define __CRULESACTIONLOCAL__MULBERRY__

#include "CRulesAction.h"

// Constants

// Panes
const	PaneIDT		paneid_RulesAction = 1211;
const	PaneIDT		paneid_RulesActionPopup1 = 'POP1';
const	PaneIDT		paneid_RulesActionPopup2 = 'POP2';
const	PaneIDT		paneid_RulesActionText = 'TXT1';
const	PaneIDT		paneid_RulesActionMailboxPopup = 'MPOP';
const	PaneIDT		paneid_RulesActionSaveSingle = 'CHCK';
const	PaneIDT		paneid_RulesActionTextBtn = 'TBTN';
const	PaneIDT		paneid_RulesActionSoundPopup = 'SND1';

// Mesages
const	MessageT	msg_RulesActionPopup1 = 'POP1';
const	MessageT	msg_RulesActionMailboxPopup = 'MPOP';
const	MessageT	msg_RulesActionTextBtn = 'TBTN';
const	MessageT	msg_RulesActionSoundPopup = 'SND1';

// Resources
const	ResIDT		RidL_CRulesActionLocalBtns = 1211;

// Classes
class CMailboxPopup;
class CSoundPopup;
class CTextFieldX;
class LCheckBox;
class LPopupButton;
class LCheckBox;
class LPushButton;

class CRulesActionLocal : public CRulesAction
{
private:
	LPopupButton*		mPopup1;
	LPopupButton*		mPopup2;
	CTextFieldX*		mText;
	CMailboxPopup*		mMailboxPopup;
	LCheckBox*			mSaveSingle;
	LPushButton*		mOptionsBtn;
	CSoundPopup*		mSoundPopup;

	CActionItem::CActionReply	mActionReply;
	CActionItem::CActionForward	mActionForward;
	CActionItem::CActionBounce	mActionBounce;
	CActionItem::CActionReject	mActionReject;
	mutable COSStringMap		mActionSound;

public:
	enum { class_ID = 'RulA' };

					CRulesActionLocal();
					CRulesActionLocal(LStream *inStream);
	virtual 		~CRulesActionLocal();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	CActionItem*	GetActionItem() const;
	void			SetActionItem(const CActionItem* spec); 

	bool			DoActivate();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void	InitLabelNames();

	void	OnSetAction(long item);
	void	OnSetOptions();
};

#endif
