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

#ifndef H_CNewEventTiming
#define H_CNewEventTiming
#pragma once

#include "CNewTimingPanel.h"
#include <LListener.h>

class CDateTimeZoneSelect;
class CDurationSelect;

class LCheckBox;
class LRadioButton;

// ===========================================================================
//	CNewEventTiming

class CNewEventTiming : public CNewTimingPanel,
						public LListener
{
public:
	enum { class_ID = 'Neti', pane_ID = 1815 };

						CNewEventTiming(LStream *inStream);
	virtual				~CNewEventTiming();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual bool	GetAllDay() const;
	virtual void	GetTimezone(iCal::CICalendarTimezone& tz) const;

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);
			void    GetPeriod(iCal::CICalendarPeriod& period);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eAllDay_ID = 'ALLD',

		eStartDateTimeZone_ID = 'DTSS',

		eEnds_ID = 'ENDS',
		eDuration_ID = 'DURA',

		eEndDateTimeZone_ID = 'DTES',

		eDurationSelect_ID = 'DURS'

	};

	enum
	{
		eEnd_End = 1,
		eEnd_Duration
	};

	// UI Objects
	LCheckBox*				mAllDay;

	CDateTimeZoneSelect*	mStartDateTimeZone;

	LRadioButton*			mUseEnd;
	LRadioButton*			mUseDuration;

	CDateTimeZoneSelect*	mEndDateTimeZone;

	CDurationSelect*		mDuration;

	virtual void		FinishCreateSelf();
	
			void	DoAllDay(bool set);
			void	DoEnds(bool use_duration);
			
			void	SyncEnd();
			void	SyncDuration();
};

#endif
