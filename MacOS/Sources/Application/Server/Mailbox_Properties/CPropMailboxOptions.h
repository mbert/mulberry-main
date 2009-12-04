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


// Header for CPropMailboxOptions class

#ifndef __CPROPMAILBOXOPTIONS__MULBERRY__
#define __CPROPMAILBOXOPTIONS__MULBERRY__

#include "CMailboxPropPanel.h"

#include "CMailAccountManager.h"

// Constants

// Panes
const	PaneIDT		paneid_PropMailboxOptions = 4011;
const	PaneIDT		paneid_MailboxOptionsState = 'ICON';
const	PaneIDT		paneid_MailboxOptionsCheck = 'CHCK';
const	PaneIDT		paneid_MailboxOptionsAlertStylePopup = 'STYL';
const	PaneIDT		paneid_MailboxOptionsOpen = 'OPEN';
const	PaneIDT		paneid_MailboxOptionsCopyTo = 'COPY';
const	PaneIDT		paneid_MailboxOptionsAppendTo = 'APPN';
const	PaneIDT		paneid_MailboxOptionsPuntOnClose = 'MARS';
const	PaneIDT		paneid_MailboxOptionsAutoSync = 'SYNC';
const	PaneIDT		paneid_MailboxOptionsTieIdentity = 'IDAC';
const	PaneIDT		paneid_MailboxOptionsIdentityPopup = 'IDEN';
const	PaneIDT		paneid_MailboxOptionsRebuildLocal = 'RBUI';

// Resources
const ResIDT	RidL_CPropMailboxOptionsBtns = 4011;

// Mesages
const	MessageT	msg_OpenAtStart = 'OPEN';
const	MessageT	msg_AutoCheck = 'CHCK';
const	MessageT	msg_AlertStyle = 'STYL';
const	MessageT	msg_MessageCopyTo = 'COPY';
const	MessageT	msg_DraftCopyTo = 'APPN';
const	MessageT	msg_PuntOnClose = 'MARS';
const	MessageT	msg_AutoSync = 'SYNC';
const	MessageT	msg_MOTieIdentity = 'IDAC';
const	MessageT	msg_MOIdentityPopup = 'IDEN';
const	MessageT	msg_MORebuildLocal = 'RBUI';

// Classes
class CIdentityPopup;
class CMbox;
class LCheckBox;
class LIconControl;
class LPopupButton;
class LPushButton;

class CPropMailboxOptions : public CMailboxPropPanel,
								public LListener
{
private:
	LIconControl*		mIconState;
	LCheckBox*			mCheck;
	LPopupButton*		mAlertStylePopup;
	LCheckBox*			mOpen;
	LCheckBox*			mCopyTo;
	LCheckBox*			mAppendTo;
	LCheckBox*			mPuntOnClose;
	LCheckBox*			mAutoSync;
	LCheckBox*			mTieIdentity;
	CIdentityPopup*		mIdentityPopup;
	LPushButton*		mRebuildLocal;

	cdstring			mCurrentIdentity;

public:
	enum { class_ID = 'MOpt' };

					CPropMailboxOptions();
					CPropMailboxOptions(LStream *inStream);
	virtual 		~CPropMailboxOptions();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
private:
			void	InitAlertStylePopup();
			void	SetFavourite(bool set, CMailAccountManager::EFavourite fav_type);
			void	SetTiedIdentity(bool add);				// Add/remove tied identity

			void	OnRebuildLocal();

			CMailAccountManager::EFavourite GetStyleType(unsigned long index) const;
};

#endif
