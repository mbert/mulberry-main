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


// Header for CPropCalendarACL class

#ifndef __CPropCalendarACL__MULBERRY__
#define __CPropCalendarACL__MULBERRY__

#include "CCalendarPropPanel.h"
#include "CListener.h"

#include "CCalendarACL.h"
#include "CCalendarStoreNode.h"

// Constants

// Panes
const	PaneIDT		paneid_PropCalendarACL = 1845;
const	PaneIDT		paneid_CalendarACLCalendarPopup = 'CALE';
const	PaneIDT		paneid_CalendarACLTable = 'TABL';
const	PaneIDT		paneid_CalendarACLReadFreeBusyMark = 'IND1';
const	PaneIDT		paneid_CalendarACLReadMark = 'IND2';
const	PaneIDT		paneid_CalendarACLWriteMark = 'IND3';
const	PaneIDT		paneid_CalendarACLScheduleMark = 'IND4';
const	PaneIDT		paneid_CalendarACLCreateMark = 'IND5';
const	PaneIDT		paneid_CalendarACLDeleteMark = 'IND6';
const	PaneIDT		paneid_CalendarACLAdminMark = 'IND7';
const	PaneIDT		paneid_CalendarACLNewUserBtn = 'NEWU';
const	PaneIDT		paneid_CalendarACLDeleteUserBtn = 'DELU';
const	PaneIDT		paneid_CalendarACLStylePopup = 'STYL';
const	PaneIDT		paneid_CalendarACLChangeAllBtn = 'CALL';

// Resources
const	ResIDT		RidL_CPropCalendarACLBtns = 1845;

// Mesages
const	MessageT	msg_CalACLCalendar = 'CALE';
const	MessageT	msg_CalACLNewUser = 'NEWU';
const	MessageT	msg_CalACLDeleteUser = 'DELU';
const	MessageT	msg_CalACLStylePopup = 'STYL';
const	MessageT	msg_CalACLChangeAll = 'CALL';

enum
{
	eCalACLStyleNewUser = 1,
	eCalACLStyleDeleteUser,
	eCalACLStyleDummy,
	eCalACLStyles
};

// Classes
class CACLStylePopup;
class CACLTable;
class LIconControl;
class LPopupButton;
class LPushButton;

class CPropCalendarACL : public CCalendarPropPanel,
						public LListener,
						public CListener
{
public:
	enum { class_ID = 'CAcl' };

					CPropCalendarACL(LStream *inStream);
	virtual 		~CPropCalendarACL();

	virtual void	ListenTo_Message(long msg, void* param);

	virtual void	SetCalList(calstore::CCalendarStoreNodeList* cal_list);		// Set cal list
	virtual void	SetProtocol(calstore::CCalendarProtocol* proto);			// Set protocol
	virtual void	DoChangeACL(CCalendarACL::ECalACL acl, bool set);	// User clicked a button
	virtual void	ApplyChanges(void);						// Force update of mboxes

protected:
	virtual void	FinishCreateSelf(void);					// Do odds & ends
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);
	
private:
	LPopupButton*	mCalendarPopup;
	LIconControl*	mReadFreeBusyMark;
	LIconControl*	mReadMark;
	LIconControl*	mWriteMark;
	LIconControl*	mScheduleMark;
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
	
	calstore::CCalendarStoreNode*	mActionCal;

	virtual void		SetCal(calstore::CCalendarStoreNode* cal);		// Set mbox item
	virtual void		SetMyRights(SACLRight rights);					// Set indicators from rights
	virtual void		SetButtons(bool enable, bool alloff);			// Enable/disable buttons

	// Handle actions
	virtual void	DoChangeRights(SACLRight rights);			// User clicked a button
	virtual void	DoNewUser(void);							// Do new user dialog
	virtual void	DoDeleteUser(void);							// Delete user from list
	virtual void	DoStylePopup(long index);					// Handle popup command
	virtual void	DoChangeAll();								// Change rights
};

#endif
