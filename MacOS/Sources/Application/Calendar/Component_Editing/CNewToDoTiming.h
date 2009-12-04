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

#ifndef H_CNewToDoTiming
#define H_CNewToDoTiming
#pragma once

#include "CNewTimingPanel.h"
#include <LListener.h>

class CDateTimeZoneSelect;
class CDurationSelect;

class LCheckBox;
class LRadioButton;

// ===========================================================================
//	CNewToDoTiming

class CNewToDoTiming : public CNewTimingPanel,
						public LListener
{
public:
	enum { class_ID = 'Ntod', pane_ID = 1816 };

						CNewToDoTiming(LStream *inStream);
	virtual				~CNewToDoTiming();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual bool	GetAllDay() const;
	virtual void	GetTimezone(iCal::CICalendarTimezone& tz) const;

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eAllDay_ID = 'ALLD',

		eNoDue_ID = 'NDUE',
		eDueBy_ID = 'DUEB',
		eStarts_ID = 'STAR',

		eEndDateTimeZone_ID = 'DTDS',
		eStartDateTimeZone_ID = 'DTSS',
		eDurationSelect_ID = 'DURS'

	};

	enum
	{
		eDue_NoDue = 1,
		eDue_DueBy,
		eDue_Start
	};

	// UI Objects
	LCheckBox*				mAllDay;

	LRadioButton*			mNoDue;
	LRadioButton*			mDueBy;
	LRadioButton*			mStarts;

	CDateTimeZoneSelect*	mDueDateTimeZone;
	CDateTimeZoneSelect*	mStartDateTimeZone;
	CDurationSelect*		mDuration;

	virtual void		FinishCreateSelf();
	
			void	DoAllDay(bool set);
			void	DoDueGroup(uint32_t group);
};

#endif
