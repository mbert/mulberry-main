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

#include "HPopupMenu.h"

// Classes
class JXTextButton;
class CMailboxPopupButton;
class CTextInputField;

class CRulesActionSIEVE : public CRulesAction
{
public:
					CRulesActionSIEVE(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CRulesActionSIEVE();

	virtual void	OnCreate(CRulesDialog* dlog);

	virtual CActionItem*	GetActionItem() const;
	virtual void			SetActionItem(const CActionItem* spec); 

private:
// begin JXLayout1

    JXMultiImageButton*  mUp;
    JXMultiImageButton*  mDown;
    HPopupMenu*          mPopup1;
    JXTextButton*        mOptionsBtn;
    CTextInputField*     mText;
    CMailboxPopupButton* mMailboxPopup;

// end JXLayout1

	CActionItem::CActionVacation	mActionVacation;

	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	OnSetAction(JIndex nID);
			void	OnMailboxPopup(JIndex nID);
			void	OnOptionsBtn();
};

#endif
