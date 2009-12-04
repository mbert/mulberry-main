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


// Header for CMessagePaneOptions class

#ifndef __CMESSAGEPANEOPTIONS__MULBERRY__
#define __CMESSAGEPANEOPTIONS__MULBERRY__

#include "CCommonViewOptions.h"

#include "C3PaneOptions.h"

// Constants
const	PaneIDT		paneid_MailViewOptions = 1792;

const	PaneIDT		paneid_MailViewMailboxUseTabs = 'USET';
const	PaneIDT		paneid_MailViewMailboxRestoreTabs = 'REST';
const	PaneIDT		paneid_MailViewMessageAddress = 'ADDR';
const	PaneIDT		paneid_MailViewMessageSummary = 'HDRS';
const	PaneIDT		paneid_MailViewMessageParts = 'PART';
const	PaneIDT		paneid_MailViewMessageMarkSeen = 'SEEN';
const	PaneIDT		paneid_MailViewMessageMarkSeenAfter = 'AFTR';
const	PaneIDT		paneid_MailViewMessageNoMarkSeen = 'NOSE';
const	PaneIDT		paneid_MailViewMessageSeenDelay = 'SECS';

// Messages
const	MessageT	msg_MailViewMailboxUseTabs = 'USET';
const	MessageT	msg_MailViewMessageMarkSeen = 'SEEN';
const	MessageT	msg_MailViewMessageMarkSeenAfter = 'AFTR';
const	MessageT	msg_MailViewMessageNoMarkSeen = 'NOSE';

// Resources
const	ResIDT		RidL_CMessagePaneOptionsBtns = 1792;

// Classes
class CMailViewOptions;
class CTextFieldX;
class LCheckBox;
class LRadioButton;

class CMessagePaneOptions : public CCommonViewOptions,
							public LListener
{
public:
	enum { class_ID = 'MVop' };

					CMessagePaneOptions(LStream *inStream);
	virtual 		~CMessagePaneOptions();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	void	SetData(const CUserAction& listPreview,
					const CUserAction& listFullView,
					const CUserAction& itemsPreview,
					const CUserAction& itemsFullView,
					const CMailViewOptions& options,
					bool is3pane);
	void	GetData(CUserAction& listPreview,
					CUserAction& listFullView,
					CUserAction& itemsPreview,
					CUserAction& itemsFullView,
					CMailViewOptions& options);

protected:
	LCheckBox*			mMailboxUseTabs;
	LCheckBox*			mMailboxRestoreTabs;
	LCheckBox*			mMessageAddress;
	LCheckBox*			mMessageSummary;
	LCheckBox*			mMessageParts;
	LRadioButton*		mMarkSeen;
	LRadioButton*		mMarkSeenAfter;
	LRadioButton*		mNoMarkSeen;
	CTextFieldX*		mSeenDelay;

	virtual void	FinishCreateSelf(void);
};

#endif
