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

#ifndef H_CNewComponentAttendees
#define H_CNewComponentAttendees
#pragma once

#include "CNewComponentPanel.h"

// ===========================================================================
//	CNewComponentAttendees

class CAttendeeTable;
class CIdentity;
class CIdentityPopup;
class CStaticText;
class LPushButton;

class CNewComponentAttendees : public CNewComponentPanel,
public LListener
{
public:
	enum { class_ID = 'Ncat', pane_ID = 1819 };
	
	CNewComponentAttendees(LStream *inStream);
	virtual				~CNewComponentAttendees();
	
	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons
	
	virtual void	SetComponent(const iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);
	
	virtual void	SetReadOnly(bool read_only);
	
	void	ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status);
	
protected:
	enum
	{
		eAddAttendeesPane_ID = 'ORGA',
		eAddAttendeesBtn_ID = 'ADDA',
		eAttendeesPane_ID = 'ATTD',
		eOrganiser_ID = 'ORGN',
		eIdentity_ID = 'IDEN',
		eTable_ID = 'TABL',
		eNewBtn_ID = 'NEWB',
		eChangeBtn_ID = 'CHNB',
		eDeleteBtn_ID = 'DELB',
		eAvailabilityBtn_ID = 'AVAI'
	};
	
	// UI Objects
	LView*				mAddAttendeesPane;
	LView*				mAttendeesPane;
	CStaticText*		mOrganiser;
	CIdentityPopup*		mIdentity;
	CAttendeeTable*		mTable;
	LPushButton*		mNewBtn;
	LPushButton*		mChangeBtn;
	LPushButton*		mDeleteBtn;
	LPushButton*		mAvailabilityBtn;
	
	bool							mHasOrganizer;
	bool							mOrganizerIsMe;
	bool							mOrganizerChanged;
	bool							mUseIdentity;
	const CIdentity*				mOldIdentity;
	iCal::CICalendarProperty		mOrganizer;
	iCal::CICalendarPropertyList	mAttendees;
	
	virtual void		FinishCreateSelf();
	
	void		SetIdentity(const CIdentity* id);
	
	void		OnAddAttendees();
	void		OnSelectionChange();
	void		OnNew();
	void		OnChange();
	void		OnDelete();
	void		OnAvailability();
	
	void		SortAttendees();
};

#endif
