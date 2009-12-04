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


// Header for CVacationActionDialog class

#ifndef __CVACATIONACTIONDIALOG__MULBERRY__
#define __CVACATIONACTIONDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CActionItem.h"

// Constants

// Type
class JXTextButton;
class JXIntegerInput;
template <class T> class CInputField;
class CTextInputField;
class CTextDisplay;

class CVacationActionDialog : public CDialogDirector
{
public:
					CVacationActionDialog(JXDirector* supervisor);
	virtual 		~CVacationActionDialog();

	static bool PoseDialog(CActionItem::CActionVacation& details);

protected:
// begin JXLayout

    CInputField<JXIntegerInput>* mDays;
    CTextInputField*             mSubject;
    JXTextButton*                mTextBtn;
    JXTextButton*                mCancelBtn;
    JXTextButton*                mOKBtn;
    CTextDisplay*             	 mAddresses;

// end JXLayout

	cdstring			mText;

	virtual void	OnCreate();
	virtual void	Receive(JBroadcaster* sender, const Message& message);

			void	SetDetails(CActionItem::CActionVacation& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionVacation& details);		// Get the dialogs return info

			void	SetText();
};

#endif
