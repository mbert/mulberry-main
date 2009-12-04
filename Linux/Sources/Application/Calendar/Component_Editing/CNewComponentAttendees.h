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

#include "CNewComponentPanel.h"
#include "CListener.h"

// ===========================================================================
//	CNewComponentAttendees

class CAttendeeTable;
class CIdentity;
class CIdentityPopup;
class CSimpleTitleTable;

class JXFlatRect;
class JXStaticText;
class JXTextButton;

class CNewComponentAttendees : public CNewComponentPanel,
								public CListener
{
public:

	CNewComponentAttendees(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h);
	virtual				~CNewComponentAttendees() {}

	virtual void	OnCreate();

	virtual void	ListenTo_Message(long msg, void* param);

	virtual void	SetComponent(const iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);

	virtual void	SetReadOnly(bool read_only);

	void	ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status);

protected:

	// UI Objects
// begin JXLayout1

    JXFlatRect*   mAddAttendeesPane;
    JXTextButton* mAddAttendees;

// end JXLayout1
// begin JXLayout2

    JXFlatRect*     mAttendeesPane;
    JXStaticText*   mOrganiser;
    CIdentityPopup* mIdentity;
    JXTextButton*   mNewBtn;
    JXTextButton*   mChangeBtn;
    JXTextButton*   mDeleteBtn;
    JXTextButton*   mAvailabilityBtn;

// end JXLayout2

	CSimpleTitleTable*		mTitles;
	CAttendeeTable*			mTable;

	bool							mHasOrganizer;
	bool							mOrganizerIsMe;
	bool							mOrganizerChanged;
	bool							mUseIdentity;
	const CIdentity*				mOldIdentity;
	iCal::CICalendarProperty		mOrganizer;
	iCal::CICalendarPropertyList	mAttendees;
	
	virtual void Receive(JBroadcaster* sender, const Message& message);

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
