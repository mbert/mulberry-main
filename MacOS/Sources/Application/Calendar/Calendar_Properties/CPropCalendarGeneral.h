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


// Header for CPropCalendarGeneral class

#ifndef __CPropCalendarGeneral__MULBERRY__
#define __CPropCalendarGeneral__MULBERRY__

#include "CCalendarPropPanel.h"


// Constants

// Panes
const	PaneIDT		paneid_PropCalendarGeneral = 1842;
const	PaneIDT		paneid_CalendarGeneralState = 'ICON';
const	PaneIDT		paneid_CalendarGeneralName = 'NAME';
const	PaneIDT		paneid_CalendarGeneralServer = 'SERV';
const	PaneIDT		paneid_CalendarGeneralHierarchy = 'HIER';
const	PaneIDT		paneid_CalendarGeneralSeparator = 'SEPR';
const	PaneIDT		paneid_CalendarGeneralSize = 'SIZE';
const	PaneIDT		paneid_CalendarGeneralCalculate = 'CALC';
const	PaneIDT		paneid_CalendarGeneralStatus = 'STAT';
const	PaneIDT		paneid_CalendarGeneralLastSync = 'SYNC';

// Resources
const ResIDT	RidL_CPropCalendarGeneralBtns = 1842;

// Mesages
const	MessageT	msg_CalendarGeneralCalculate = 'CALC';

// Classes
namespace calstore
{
class CCalendarStoreNode;
}

class CTextFieldX;
class LIconControl;

class CPropCalendarGeneral : public CCalendarPropPanel,
								public LListener
{
private:
	LIconControl*			mIconState;
	CTextFieldX*			mName;
	CTextFieldX*			mServer;
	CTextFieldX*			mHierarchy;
	CTextFieldX*			mSeparator;
	CTextFieldX*			mSize;
	CTextFieldX*			mStatus;
	CTextFieldX*			mLastSync;

public:
	enum { class_ID = 'CGen' };

					CPropCalendarGeneral(LStream *inStream);
	virtual 		~CPropCalendarGeneral();

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	
private:
	virtual void	SetCalendar(calstore::CCalendarStoreNode* node);					// Set mbox item
			void	CheckSize();
};

#endif
