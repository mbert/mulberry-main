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

#include "HPopupMenu.h"

// Classes
class JXTextButton;
class JXTextCheckbox;
class CMailboxPopupButton;
class CTextInputField;

class CRulesActionLocal : public CRulesAction
{
public:
					CRulesActionLocal(JXContainer* enclosure,
								const HSizingOption hSizing, const VSizingOption vSizing,
								const JCoordinate x, const JCoordinate y,
								const JCoordinate w, const JCoordinate h);
	virtual 		~CRulesActionLocal();

	virtual void	OnCreate(CRulesDialog* dlog);

	virtual CActionItem*	GetActionItem() const;
	virtual void			SetActionItem(const CActionItem* spec); 

private:
// begin JXLayout1

    JXMultiImageButton*  mUp;
    JXMultiImageButton*  mDown;
    HPopupMenu*          mPopup1;
    HPopupMenu*          mPopup2;
    JXTextButton*        mOptionsBtn;
    CTextInputField*     mText;
    CMailboxPopupButton* mMailboxPopup;
    JXTextCheckbox*      mSaveSingle;

// end JXLayout1

	CActionItem::CActionReply	mActionReply;
	CActionItem::CActionForward	mActionForward;
	CActionItem::CActionBounce	mActionBounce;
	CActionItem::CActionReject	mActionReject;
	mutable COSStringMap		mActionSound;

	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	InitLabelNames();

			void	OnSetAction(JIndex nID);
			void	OnMailboxPopup(JIndex nID);
			void	OnOptionsBtn();
};

#endif
