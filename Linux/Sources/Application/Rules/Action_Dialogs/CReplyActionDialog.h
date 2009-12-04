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


// Header for CReplyActionDialog class

#ifndef __CREPLYACTIONDIALOG__MULBERRY__
#define __CREPLYACTIONDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CActionItem.h"

// Constants

// Type
class CIdentityPopup;
class JXRadioGroup;
class JXTextButton;
class JXTextCheckbox;

class CReplyActionDialog : public CDialogDirector
{
public:
					CReplyActionDialog(JXDirector* supervisor);
	virtual 		~CReplyActionDialog();

	static bool PoseDialog(CActionItem::CActionReply& details);

protected:
// begin JXLayout

    JXRadioGroup*   mReplyGroup;
    JXTextCheckbox* mQuote;
    JXTextButton*   mTextBtn;
    JXRadioGroup*   mIdentityGroup;
    CIdentityPopup* mIdentityPopup;
    JXTextCheckbox* mCreateDraft;
    JXTextButton*   mCancelBtn;
    JXTextButton*   mOKBtn;

// end JXLayout

	cdstring			mText;
	cdstring			mCurrentIdentity;

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	SetDetails(CActionItem::CActionReply& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionReply& details);		// Get the dialogs return info

			void	SetText();
};

#endif
