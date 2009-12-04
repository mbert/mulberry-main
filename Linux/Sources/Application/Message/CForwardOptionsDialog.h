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


// Header for CForwardOptionsDialog class

#ifndef __CFORWARDOPTIONSDIALOG__MULBERRY__
#define __CFORWARDOPTIONSDIALOG__MULBERRY__

#include "CDialogDirector.h"

#include "CMessageWindow.h"

// Classes
class JXRadioGroup;
class JXTextButton;
class JXTextCheckbox;
class JXTextRadioButton;

class CForwardOptionsDialog : public CDialogDirector
{
public:
					CForwardOptionsDialog(JXDirector* supervisor);
	virtual 		~CForwardOptionsDialog() {}

	static bool PoseDialog(EForwardOptions& forward, EReplyQuote& quote, bool has_selection);

	void	SetDetails(EReplyQuote quote, bool has_selection);			// Set the dialogs info
	void	GetDetails(EReplyQuote& quote, bool& save);					// Set the dialogs return info

protected:
	virtual void OnCreate();
	virtual void Receive(JBroadcaster* sender, const Message& message);

private:
	static EForwardOptions sForward;
	static bool sForwardInit;

// begin JXLayout

    JXTextCheckbox*    mForwardQuote;
    JXTextCheckbox*    mForwardHeaders;
    JXTextCheckbox*    mForwardAttachment;
    JXTextCheckbox*    mForwardRFC822;
    JXRadioGroup*      mQuote;
    JXTextRadioButton* mQuoteSelection;
    JXTextRadioButton* mQuoteAll;
    JXTextRadioButton* mQuoteNone;
    JXTextCheckbox*    mSave;
    JXTextButton*      mCancelBtn;
    JXTextButton*      mOKBtn;

// end JXLayout
	
	void InitOptions();
};

#endif
