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

#ifndef H_CNewEventDialog
#define H_CNewEventDialog
#pragma once

#include "CNewComponentDialog.h"

#include "CICalendarVEvent.h"

// ===========================================================================
//	CNewEventDialog

class CNewEventDialog : public CNewComponentDialog
{
public:
	enum { class_ID = 'EdEv', pane_ID = 1810 };
	
	static void StartNew(const iCal::CICalendarDateTime& dtstart, const iCal::CICalendar* calin = NULL);
	static void StartEdit(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	static void StartDuplicate(const iCal::CICalendarVEvent& vevent);
	
	CNewEventDialog(LStream *inStream);
	virtual				~CNewEventDialog();
		
protected:
	enum
	{
		eStatus_ID = 'STAT',
		eAvailability_ID = 'AVAI',
	};
	
	// UI Objects
	LPopupButton*			mStatus;
	LCheckBox*				mAvailability;
	
	static void StartModeless(iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded, EModelessAction action);
	
	virtual void FinishCreateSelf();
	
	virtual void InitPanels();
	
	virtual void	SetComponent(iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);
	
	virtual void	SetReadOnly(bool read_only);
	
	virtual void	ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status);

	virtual bool	DoNewOK();
	virtual bool	DoEditOK();
	virtual void	DoCancel();
};

#endif
