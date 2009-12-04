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


// Header for CBounceActionDialog class

#ifndef __CBOUNCEACTIONDIALOG__MULBERRY__
#define __CBOUNCEACTIONDIALOG__MULBERRY__

#include <LDialogBox.h>

#include "CActionItem.h"

// Constants

// Panes
const	PaneIDT		paneid_BounceActionDialog = 1222;
const	PaneIDT		paneid_BounceActionTo = 'ADTO';
const	PaneIDT		paneid_BounceActionCC = 'ADCC';
const	PaneIDT		paneid_BounceActionBcc = 'ADBC';
const	PaneIDT		paneid_BounceActionUseTied = 'TIED';
const	PaneIDT		paneid_BounceActionUseIdentity = 'UIDN';
const	PaneIDT		paneid_BounceActionIdentity = 'IDEN';
const	PaneIDT		paneid_BounceActionCreateDraft = 'DRFT';

// Mesages
const	MessageT	msg_BounceActionUseTied = 'TIED';
const	MessageT	msg_BounceActionUseIdentity = 'UIDN';
const	MessageT	msg_BounceActionIdentity = 'IDEN';

// Resources
const	ResIDT		RidL_CBounceActionDialogBtns = 1222;

// Type
class CIdentityPopup;
class CTextFieldX;
class LCheckBox;
class LRadioButton;

class CBounceActionDialog : public LDialogBox
{
private:
	CTextFieldX*		mTo;
	CTextFieldX*		mCC;
	CTextFieldX*		mBcc;
	LRadioButton*		mUseTied;
	LRadioButton*		mUseIdentity;
	CIdentityPopup*		mIdentityPopup;
	LCheckBox*			mCreateDraft;

	cdstring			mCurrentIdentity;

public:
	enum { class_ID = 'Bact' };

					CBounceActionDialog();
					CBounceActionDialog(LStream *inStream);
	virtual 		~CBounceActionDialog();

	static bool PoseDialog(CActionItem::CActionBounce& details);

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
			void	SetDetails(CActionItem::CActionBounce& details);		// Set the dialogs info
			void	GetDetails(CActionItem::CActionBounce& details);		// Get the dialogs return info
};

#endif
