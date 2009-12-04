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

#include <LDialogBox.h>

#include "CActionItem.h"

// Constants

// Panes
const	PaneIDT		paneid_ReplyActionDialog = 1220;
const	PaneIDT		paneid_ReplyActionReplyTo = 'RRTO';
const	PaneIDT		paneid_ReplyActionSender = 'RSND';
const	PaneIDT		paneid_ReplyActionFrom = 'RFRM';
const	PaneIDT		paneid_ReplyActionAll = 'RALL';
const	PaneIDT		paneid_ReplyActionQuote = 'QUOT';
const	PaneIDT		paneid_ReplyActionText = 'TEXT';
const	PaneIDT		paneid_ReplyActionUseTied = 'TIED';
const	PaneIDT		paneid_ReplyActionUseIdentity = 'UIDN';
const	PaneIDT		paneid_ReplyActionIdentity = 'IDEN';
const	PaneIDT		paneid_ReplyActionCreateDraft = 'DRFT';

// Mesages
const	MessageT	msg_ReplyActionText = 'TEXT';
const	MessageT	msg_ReplyActionUseTied = 'TIED';
const	MessageT	msg_ReplyActionUseIdentity = 'UIDN';
const	MessageT	msg_ReplyActionIdentity = 'IDEN';

// Resources
const	ResIDT		RidL_CReplyActionDialogBtns = 1220;

// Type
class CIdentityPopup;
class LCheckBox;
class LRadioButton;

class CReplyActionDialog : public LDialogBox
{
private:
	LRadioButton*		mReplyTo;
	LRadioButton*		mReplySender;
	LRadioButton*		mReplyFrom;
	LRadioButton*		mReplyAll;
	LCheckBox*			mQuote;
	LRadioButton*		mUseTied;
	LRadioButton*		mUseIdentity;
	CIdentityPopup*		mIdentityPopup;
	LCheckBox*			mCreateDraft;

	cdstring			mText;
	cdstring			mCurrentIdentity;

public:
	enum { class_ID = 'Ract' };

					CReplyActionDialog();
					CReplyActionDialog(LStream *inStream);
	virtual 		~CReplyActionDialog();

	static bool PoseDialog(CActionItem::CActionReply& details);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
			void	SetDetails(CActionItem::CActionReply& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionReply& details);		// Get the dialogs return info

			void	SetText();
};

#endif
