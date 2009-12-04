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


// Header for CPropCalendarOptions class

#ifndef __CPropCalendarOptions__MULBERRY__
#define __CPropCalendarOptions__MULBERRY__

#include "CCalendarPropPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PropCalendarOptions = 1841;
const	PaneIDT		paneid_CalendarOptionsState = 'ICON';
const	PaneIDT		paneid_CalendarOptionsSubscribe = 'SUBS';
const	PaneIDT		paneid_CalendarOptionsFreeBusySet = 'FREE';
const	PaneIDT		paneid_CalendarOptionsTieIdentity = 'IDAC';
const	PaneIDT		paneid_CalendarOptionsIdentityPopup = 'IDEN';

// Resources
const ResIDT	RidL_CPropCalendarOptionsBtns = 1841;

// Mesages
const	MessageT	msg_COSubscribe = 'SUBS';
const	MessageT	msg_COFreeBusySet = 'FREE';
const	MessageT	msg_COTieIdentity = 'IDAC';
const	MessageT	msg_COIdentityPopup = 'IDEN';

// Classes
class CIdentityPopup;
class LCheckBox;
class LIconControl;
class LPopupButton;

class CPropCalendarOptions : public CCalendarPropPanel,
								public LListener
{
private:
	LIconControl*		mIconState;
	LCheckBox*			mSubscribe;
	LCheckBox*			mFreeBusySet;
	LCheckBox*			mTieIdentity;
	CIdentityPopup*		mIdentityPopup;

	cdstring			mCurrentIdentity;

public:
	enum { class_ID = 'COpt' };

					CPropCalendarOptions(LStream *inStream);
	virtual 		~CPropCalendarOptions();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	ApplyChanges(void);											// Force update of calendars

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
private:
			void	SetTiedIdentity(bool add);				// Add/remove tied identity
};

#endif
