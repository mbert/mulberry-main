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


// Header for CPropMailboxGeneral class

#ifndef __CPROPMAILBOXGENERAL__MULBERRY__
#define __CPROPMAILBOXGENERAL__MULBERRY__

#include "CMailboxPropPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PropMailboxGeneral = 4012;
const	PaneIDT		paneid_MailboxGeneralState = 'ICON';
const	PaneIDT		paneid_MailboxGeneralName = 'NAME';
const	PaneIDT		paneid_MailboxGeneralServer = 'SERV';
const	PaneIDT		paneid_MailboxGeneralHierarchy = 'HIER';
const	PaneIDT		paneid_MailboxGeneralSeparator = 'SEPR';
const	PaneIDT		paneid_MailboxGeneralTotal = 'TOTL';
const	PaneIDT		paneid_MailboxGeneralRecent = 'RECT';
const	PaneIDT		paneid_MailboxGeneralUnseen = 'UNSN';
const	PaneIDT		paneid_MailboxGeneralDeleted = 'DELD';
const	PaneIDT		paneid_MailboxGeneralFullSync = 'FSYN';
const	PaneIDT		paneid_MailboxGeneralPartialSync = 'PSYN';
const	PaneIDT		paneid_MailboxGeneralMissingSync = 'MSYN';
const	PaneIDT		paneid_MailboxGeneralSize = 'SIZE';
const	PaneIDT		paneid_MailboxGeneralStatus = 'STAT';
const	PaneIDT		paneid_MailboxGeneralUIDValidity = 'UIDV';
const	PaneIDT		paneid_MailboxGeneralLastSync = 'SYNC';
const	PaneIDT		paneid_MailboxGeneralCalculate = 'CALC';

// Resources
const ResIDT	RidL_CPropMailboxGeneralBtns = 4012;

// Mesages
const	MessageT	msg_MailboxGeneralCalculate = 'CALC';

// Classes
class CMbox;
class CTextFieldX;
class LIconControl;

class CPropMailboxGeneral : public CMailboxPropPanel,
								public LListener
{
private:
	LIconControl*			mIconState;
	CTextFieldX*			mName;
	CTextFieldX*			mServer;
	CTextFieldX*			mHierarchy;
	CTextFieldX*			mSeparator;
	CTextFieldX*			mTotal;
	CTextFieldX*			mRecent;
	CTextFieldX*			mUnseen;
	CTextFieldX*			mDeleted;
	CTextFieldX*			mFullSync;
	CTextFieldX*			mPartialSync;
	CTextFieldX*			mMissingSync;
	CTextFieldX*			mSize;
	CTextFieldX*			mStatus;
	CTextFieldX*			mUIDValidity;
	CTextFieldX*			mLastSync;

public:
	enum { class_ID = 'MGen' };

					CPropMailboxGeneral();
					CPropMailboxGeneral(LStream *inStream);
	virtual 		~CPropMailboxGeneral();

	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	
private:
	virtual void	SetMbox(CMbox* mbox);					// Set mbox item
			void	CheckSize();
};

#endif
