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

#include <LDialogBox.h>

#include "CMessageWindow.h"
#include "cdstring.h"

// Constants

// Panes
const	PaneIDT		paneid_ForwardOptionsDialog = 2012;
const	PaneIDT		paneid_ForwardOptionsQuote = 'QUOT';
const	PaneIDT		paneid_ForwardOptionsHeaders = 'HDRS';
const	PaneIDT		paneid_ForwardOptionsAttachment = 'ATCH';
const	PaneIDT		paneid_ForwardOptionsRFC822 = 'RFC8';
const	PaneIDT		paneid_ForwardOptionsQuoteSelection = 'QSEL';
const	PaneIDT		paneid_ForwardOptionsQuoteAll = 'QALL';
const	PaneIDT		paneid_ForwardOptionsQuoteNone = 'QNON';
const	PaneIDT		paneid_ForwardOptionsSave = 'SAVE';

// Messages
const	MessageT	msg_ForwardOptionsQuote = 'QUOT';
const	MessageT	msg_ForwardOptionsAttachment = 'ATCH';

// Resources
const	ResIDT		RidL_CForwardOptionsDialogBtns = 2012;

// Classes
class LCheckBox;
class LRadioButton;

class CForwardOptionsDialog : public LDialogBox
{
public:
	enum { class_ID = 'FoOp' };

					CForwardOptionsDialog();
					CForwardOptionsDialog(LStream *inStream);
	virtual 		~CForwardOptionsDialog();

	static bool PoseDialog(EForwardOptions& forward, EReplyQuote& quote, bool has_selection);

	void	SetDetails(EReplyQuote quote, bool has_selection);				// Set the dialogs info
	void	GetDetails(EReplyQuote& quote, bool& save);						// Set the dialogs return info

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

private:
	static EForwardOptions sForward;
	static bool sForwardInit;

	LCheckBox*			mForwardQuote;
	LCheckBox*			mForwardHeaders;
	LCheckBox*			mForwardAttachment;
	LCheckBox*			mForwardRFC822;
	LRadioButton*		mQuoteSelection;
	LRadioButton*		mQuoteAll;
	LRadioButton*		mQuoteNone;
	LCheckBox*			mSave;
	
	void InitOptions();
};

#endif
