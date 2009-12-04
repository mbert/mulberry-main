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


// Header for CForwardActionDialog class

#ifndef __CFORWARDACTIONDIALOG__MULBERRY__
#define __CFORWARDACTIONDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CActionItem.h"

// Constants

// Type
class CIdentityPopup;
class JXRadioGroup;
class JXTextButton;
class JXTextCheckbox;
class CTextInputField;

class CForwardActionDialog : public CDialogDirector
{
public:
					CForwardActionDialog(JXDirector* supervisor);
	virtual 		~CForwardActionDialog();

	static bool PoseDialog(CActionItem::CActionForward& details);

protected:
// begin JXLayout

    CTextInputField* mTo;
    CTextInputField* mCC;
    CTextInputField* mBcc;
    JXTextCheckbox*  mQuote;
    JXTextButton*    mTextBtn;
    JXTextCheckbox*  mAttach;
    JXRadioGroup*    mIdentityGroup;
    CIdentityPopup*  mIdentityPopup;
    JXTextCheckbox*  mCreateDraft;
    JXTextButton*    mCancelBtn;
    JXTextButton*    mOKBtn;

// end JXLayout

	cdstring			mText;
	cdstring			mCurrentIdentity;

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	SetDetails(CActionItem::CActionForward& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionForward& details);		// Get the dialogs return info

			void	SetText();
};

#endif
