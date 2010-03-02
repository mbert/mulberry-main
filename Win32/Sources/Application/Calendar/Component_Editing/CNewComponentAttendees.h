/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CAttendeeTable.h"
#include "CIdentityPopup.h"
#include "CSimpleTitleTable.h"

// ===========================================================================
//	CNewComponentAttendees

class CNewComponentAttendees : public CNewComponentPanel
{
public:
						CNewComponentAttendees();
	virtual				~CNewComponentAttendees();

	virtual void	SetComponent(const iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);

	virtual void	SetReadOnly(bool read_only);

	void	ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status);

protected:
// Dialog Data
	//{{AFX_DATA(CNewComponentAttendees)
	enum { IDD = IDD_CALENDAR_NEW_ATTENDEES };
	CStatic					mOrganiser;
	CIdentityPopup			mIdentity;
	CSimpleTitleTable		mTitles;
	CAttendeeTable			mTable;
	CButton					mNewBtn;
	CButton					mChangeBtn;
	CButton					mDeleteBtn;
	CButton					mAvailabilityBtn;
	//}}AFX_DATA
	bool							mHasOrganizer;
	bool							mOrganizerIsMe;
	bool							mOrganizerChanged;
	bool							mUseIdentity;
	const CIdentity*				mOldIdentity;
	iCal::CICalendarProperty		mOrganizer;
	iCal::CICalendarPropertyList	mAttendees;

			void		ShowAttendees(bool show);

			void		SetIdentity(const CIdentity* id);

			void		SortAttendees();

	// Generated message map functions
	//{{AFX_MSG(CNewComponentAttendees)
	virtual BOOL	OnInitDialog();
	afx_msg void		OnAddAttendees();
	afx_msg void		OnIdentityPopup(UINT nID);
	afx_msg void		OnSelectionChange();
	afx_msg void		OnNew();
	afx_msg void		OnChange();
	afx_msg void		OnDelete();
	afx_msg void		OnAvailability();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

#endif
