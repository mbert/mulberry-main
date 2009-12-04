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


// Header for CPropMailboxACL class

#ifndef __CPROPMAILBOXACL__MULBERRY__
#define __CPROPMAILBOXACL__MULBERRY__

#include "CMailboxPropPanel.h"
#include "CListener.h"

#include "CMboxACL.h"

// Constants

// Panes
const	PaneIDT		paneid_PropMailboxACL = 4013;
const	PaneIDT		paneid_MailboxACLMailboxPopup = 'MBOX';
const	PaneIDT		paneid_MailboxACLTable = 'TABL';
const	PaneIDT		paneid_MailboxACLLookupMark = 'IND1';
const	PaneIDT		paneid_MailboxACLReadMark = 'IND2';
const	PaneIDT		paneid_MailboxACLSeenMark = 'IND3';
const	PaneIDT		paneid_MailboxACLWriteMark = 'IND4';
const	PaneIDT		paneid_MailboxACLInsertMark = 'IND5';
const	PaneIDT		paneid_MailboxACLPostMark = 'IND6';
const	PaneIDT		paneid_MailboxACLCreateMark = 'IND7';
const	PaneIDT		paneid_MailboxACLDeleteMark = 'IND8';
const	PaneIDT		paneid_MailboxACLAdminMark = 'IND9';
const	PaneIDT		paneid_MailboxACLNewUserBtn = 'NEWU';
const	PaneIDT		paneid_MailboxACLDeleteUserBtn = 'DELU';
const	PaneIDT		paneid_MailboxACLStylePopup = 'STYL';
const	PaneIDT		paneid_MailboxACLChangeAllBtn = 'CALL';

// Resources
const	ResIDT		RidL_CPropMailboxACLBtns = 4013;

// Mesages
const	MessageT	msg_ACLMailbox = 'MBOX';
const	MessageT	msg_ACLNewUser = 4120;
const	MessageT	msg_ACLDeleteUser = 4121;
const	MessageT	msg_ACLStylePopup = 4122;
const	MessageT	msg_ACLChangeAll = 'CALL';

enum
{
	eACLStyleNewUser = 1,
	eACLStyleDeleteUser,
	eACLStyleDummy,
	eACLStyles
};

// Classes
class CACLStylePopup;
class CACLTable;
class CMbox;
class LIconControl;
class LPopupButton;
class LPushButton;

class CPropMailboxACL : public CMailboxPropPanel,
						public LListener,
						public CListener
{
public:
	enum { class_ID = 'MAcl' };

					CPropMailboxACL();
					CPropMailboxACL(LStream *inStream);
	virtual 		~CPropMailboxACL();

	virtual void	ListenTo_Message(long msg, void* param);

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	DoChangeACL(CMboxACL::EMboxACL acl, bool set);	// User clicked a button
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
private:
	LPopupButton*	mMailboxPopup;
	LIconControl*	mLookupMark;
	LIconControl*	mReadMark;
	LIconControl*	mSeenMark;
	LIconControl*	mWriteMark;
	LIconControl*	mInsertMark;
	LIconControl*	mPostMark;
	LIconControl*	mCreateMark;
	LIconControl*	mDeleteMark;
	LIconControl*	mAdminMark;
	CACLTable*		mACLTable;
	LPushButton*	mNewUserBtn;
	LPushButton*	mDeleteUserBtn;
	CACLStylePopup*	mStylePopup;
	LPushButton*	mChangeAllBtn;
	bool			mCanChange;
	bool			mNoChange;
	
	CMbox*			mActionMbox;

	virtual void		SetMbox(CMbox* mbox);					// Set mbox item
	virtual void		SetMyRights(SACLRight rights);			// Set indicators from rights
	virtual void		SetButtons(bool enable, bool alloff);	// Enable/disable buttons

	// Handle actions
	virtual void	DoChangeRights(SACLRight rights);			// User clicked a button
	virtual void	DoNewUser(void);							// Do new user dialog
	virtual void	DoDeleteUser(void);							// Delete user from list
	virtual void	DoStylePopup(long index);					// Handle popup command
	virtual void	DoChangeAll();								// Change rights
};

#endif
