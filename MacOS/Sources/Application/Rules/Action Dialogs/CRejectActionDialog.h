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


// Header for CRejectActionDialog class

#ifndef __CREJECTACTIONDIALOG__MULBERRY__
#define __CREJECTACTIONDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CActionItem.h"

// Constants

// Panes
const	PaneIDT		paneid_RejectActionDialog = 1223;
const	PaneIDT		paneid_RejectActionReturnHeaders = 'HEAD';
const	PaneIDT		paneid_RejectActionReturnMessage = 'EMSG';
const	PaneIDT		paneid_RejectActionUseTied = 'TIED';
const	PaneIDT		paneid_RejectActionUseIdentity = 'UIDN';
const	PaneIDT		paneid_RejectActionIdentity = 'IDEN';
const	PaneIDT		paneid_RejectActionCreateDraft = 'DRFT';

// Mesages
const	MessageT	msg_RejectActionUseTied = 'TIED';
const	MessageT	msg_RejectActionUseIdentity = 'UIDN';
const	MessageT	msg_RejectActionIdentity = 'IDEN';

// Resources
const	ResIDT		RidL_CRejectActionDialogBtns = 1223;

// Type
class CIdentityPopup;
class LCheckBox;
class LRadioButton;

class CRejectActionDialog : public LDialogBox
{
private:
	LRadioButton*		mReturnHeaders;
	LRadioButton*		mReturnMessage;
	LRadioButton*		mUseTied;
	LRadioButton*		mUseIdentity;
	CIdentityPopup*		mIdentityPopup;
	LCheckBox*			mCreateDraft;

	cdstring			mText;
	cdstring			mCurrentIdentity;

public:
	enum { class_ID = 'Jact' };

					CRejectActionDialog();
					CRejectActionDialog(LStream *inStream);
	virtual 		~CRejectActionDialog();

	static bool PoseDialog(CActionItem::CActionReject& details);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
			void	SetDetails(CActionItem::CActionReject& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionReject& details);		// Get the dialogs return info
};

#endif
