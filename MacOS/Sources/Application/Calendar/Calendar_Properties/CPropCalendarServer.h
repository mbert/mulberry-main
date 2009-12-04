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


// Header for CPropCalendarServer class

#ifndef __CPropCalendarServer__MULBERRY__
#define __CPropCalendarServer__MULBERRY__

#include "CCalendarPropPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PropCalendarServer = 1843;
const	PaneIDT		paneid_CalendarServerState = 'ICON';
const	PaneIDT		paneid_CalendarServerName = 'NAME';
const	PaneIDT		paneid_CalendarServerType = 'TYPE';
const	PaneIDT		paneid_CalendarServerGreeting = 'GRET';
const	PaneIDT		paneid_CalendarServerCapability = 'CAPS';
const	PaneIDT		paneid_CalendarServerConnectionTitle = 'CONT';
const	PaneIDT		paneid_CalendarServerConnection = 'CONN';
const	PaneIDT		paneid_CalendarServerViewCert = 'CERT';

// Resources
const ResIDT	RidL_CPropCalendarServerBtns = 1843;

// Mesages
const	MessageT	msg_CalendarServerViewCert = 'CERT';


// Classes
class CTextDisplay;
class CTextFieldX;
class LIconControl;
class LPushButton;
class LIconControl;
class LStaticText;

class CPropCalendarServer : public CCalendarPropPanel,
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

	calstore::CCalendarProtocol*	mProto;

public:
	enum { class_ID = 'CSer' };

					CPropCalendarServer(LStream *inStream);
	virtual 		~CPropCalendarServer();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
			void	OnViewCert();
};

#endif
