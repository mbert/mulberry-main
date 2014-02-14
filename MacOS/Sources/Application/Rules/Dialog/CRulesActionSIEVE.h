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


// Header for CRulesActionSIEVE class

#ifndef __CRULESACTIONSIEVE__MULBERRY__
#define __CRULESACTIONSIEVE__MULBERRY__

#include "CRulesAction.h"

// Constants

// Panes
const	PaneIDT		paneid_RulesActionS = 1212;
const	PaneIDT		paneid_RulesActionSPopup1 = 'POP1';
const	PaneIDT		paneid_RulesActionSPopup2 = 'POP2';
const	PaneIDT		paneid_RulesActionSText = 'TXT1';
const	PaneIDT		paneid_RulesActionSMailboxPopup = 'MPOP';
const	PaneIDT		paneid_RulesActionSTextBtn = 'TBTN';

// Mesages
const	MessageT	msg_RulesActionSPopup1 = 'POP1';
const	MessageT	msg_RulesActionSPopup2 = 'POP2';
const	MessageT	msg_RulesActionSMailboxPopup = 'MPOP';
const	MessageT	msg_RulesActionSTextBtn = 'TBTN';

// Resources
const	ResIDT		RidL_CRulesActionSIEVEBtns = 1212;

// Classes
class CMailboxPopup;
class CTextFieldX;
class LPopupButton;
class LPushButton;

class CRulesActionSIEVE : public CRulesAction
{
private:
	LPopupButton*		mPopup1;
	LPopupButton*		mPopup2;
	CTextFieldX*		mText;
	CMailboxPopup*		mMailboxPopup;
	LPushButton*		mOptionsBtn;

	CActionItem::CActionVacation	mActionVacation;

public:
	enum { class_ID = 'RulS' };

					CRulesActionSIEVE();
					CRulesActionSIEVE(LStream *inStream);
	virtual 		~CRulesActionSIEVE();

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
