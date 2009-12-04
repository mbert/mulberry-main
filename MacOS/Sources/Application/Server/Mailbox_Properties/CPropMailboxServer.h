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


// Header for CPropMailboxServer class

#ifndef __CPROPMAILBOXSERVER__MULBERRY__
#define __CPROPMAILBOXSERVER__MULBERRY__

#include "CMailboxPropPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PropMailboxServer = 4015;
const	PaneIDT		paneid_MailboxServerState = 'ICON';
const	PaneIDT		paneid_MailboxServerName = 'NAME';
const	PaneIDT		paneid_MailboxServerType = 'TYPE';
const	PaneIDT		paneid_MailboxServerGreeting = 'GRET';
const	PaneIDT		paneid_MailboxServerCapability = 'CAPS';
const	PaneIDT		paneid_MailboxServerConnectionTitle = 'CONT';
const	PaneIDT		paneid_MailboxServerConnection = 'CONN';
const	PaneIDT		paneid_MailboxServerViewCert = 'CERT';

// Resources
const ResIDT	RidL_CPropMailboxServerBtns = 4015;

// Mesages
const	MessageT	msg_MailboxServerViewCert = 'CERT';


// Classes
class CMboxProtocol;
class CTextDisplay;
class CTextFieldX;
class LIconControl;
class LPushButton;
class LStaticText;

class CPropMailboxServer : public CMailboxPropPanel,
								public LListener
{
private:
	LIconControl*		mIconState;
	CTextFieldX*		mName;
	CTextFieldX*		mType;
	CTextDisplay*		mGreeting;
	CTextDisplay*		mCapability;
	LStaticText*		mConnectionTitle;
	CTextFieldX*		mConnection;
	LPushButton*		mViewCert;

	CMboxProtocol*		mProto;

public:
	enum { class_ID = 'MSer' };

					CPropMailboxServer();
					CPropMailboxServer(LStream *inStream);
	virtual 		~CPropMailboxServer();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetMboxList(CMboxList* mbox_list);		// Set mbox list
	virtual void	SetProtocol(CMboxProtocol* protocol);	// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
			void	OnViewCert();
};

#endif
