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


// Header for CPropCalendarWebcal class

#ifndef __CPropCalendarWebcal__MULBERRY__
#define __CPropCalendarWebcal__MULBERRY__

#include "CCalendarPropPanel.h"

// Constants

// Panes
const	PaneIDT		paneid_PropCalendarWebcal = 1844;
const	PaneIDT		paneid_CalendarWebcalState = 'ICON';
const	PaneIDT		paneid_CalendarWebcalRemoteURL = 'URL ';
const	PaneIDT		paneid_CalendarWebcalRevertURL = 'REVR';
const	PaneIDT		paneid_CalendarWebcalAllowChanges = 'READ';
const	PaneIDT		paneid_CalendarWebcalAutoPublish = 'PUBL';
const	PaneIDT		paneid_CalendarWebcalRefreshNever = 'CHKN';
const	PaneIDT		paneid_CalendarWebcalRefreshEvery = 'CHKE';
const	PaneIDT		paneid_CalendarWebcalRefreshInterval = 'REFR';

// Resources
const ResIDT	RidL_CPropCalendarWebcalBtns = 1844;

// Mesages
const	MessageT	msg_CWRevertURL = 'REVR';
const	MessageT	msg_CWAllowChanges = 'READ';
const	MessageT	msg_CWAutoPublish = 'PUBL';
const	MessageT	msg_CWRefreshNever = 'CHKN';
const	MessageT	msg_CWRefreshEvery = 'CHKE';

// Classes
class CTextFieldX;
class LCheckBox;
class LIconControl;
class LRadioButton;

class CPropCalendarWebcal : public CCalendarPropPanel,
								public LListener
{
private:
	LIconControl*		mIconState;
	CTextFieldX*		mRemoteURL;
	LCheckBox*			mAllowChanges;
	LCheckBox*			mAutoPublish;
	LRadioButton*		mRefreshNever;
	LRadioButton*		mRefreshEvery;
	CTextFieldX*		mRefreshInterval;

	cdstring			mOriginalURL;

public:
	enum { class_ID = 'CWeb' };

					CPropCalendarWebcal(LStream *inStream);
	virtual 		~CPropCalendarWebcal();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	ApplyChanges(void);											// Force update of calendars

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	
			void	OnRevertURL();
			void	OnAllowChanges();
			void	OnAutoPublish();
			void	OnRefreshNever();
			void	OnRefreshEvery();
};

#endif
