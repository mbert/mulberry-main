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


// Header for CRulesTarget class

#ifndef __CRULESTARGET__MULBERRY__
#define __CRULESTARGET__MULBERRY__

#include "CCriteriaBase.h"

#include "CFilterTarget.h"

// Constants

// Panes
const	PaneIDT		paneid_RulesTarget = 1216;
const	PaneIDT		paneid_RulesTargetPopup1 = 'POP1';
const	PaneIDT		paneid_RulesTargetPopup2 = 'POP2';
const	PaneIDT		paneid_RulesTargetPopup3 = 'POP3';
const	PaneIDT		paneid_RulesTargetText = 'TXT1';
const	PaneIDT		paneid_RulesTargetMailboxPopup = 'MPOP';

// Mesages
const	MessageT	msg_RulesTargetPopup1 = 'POP1';
const	MessageT	msg_RulesTargetMailboxPopup = 'MPOP';

// Resources
const	ResIDT		RidL_CRulesTargetBtns = 1216;

// Classes
class CMailboxPopup;
class CTextFieldX;
class LPopupButton;

class CRulesTarget : public CCriteriaBase
{
private:
	LPopupButton*		mPopup1;
	LPopupButton*		mPopup2;
	LPopupButton*		mPopup3;
	CTextFieldX*		mText;
	CMailboxPopup*		mMailboxPopup;

public:
	enum { class_ID = 'RulT' };

					CRulesTarget();
					CRulesTarget(LStream *inStream);
	virtual 		~CRulesTarget();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	CFilterTarget*	GetFilterTarget() const;
	void			SetFilterTarget(const CFilterTarget* spec); 

	bool			DoActivate();

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	void	OnSetTarget(long item);

	virtual void	SwitchWith(CCriteriaBase* other);

private:
			void	InitCabinetMenu();			// Set up cabinet menu
			void	InitAccountMenu();			// Set up account menu
};

#endif
