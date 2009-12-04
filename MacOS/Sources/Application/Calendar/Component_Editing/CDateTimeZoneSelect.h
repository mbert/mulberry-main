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

#ifndef H_CDateTimeZoneSelect
#define H_CDateTimeZoneSelect
#pragma once

#include <LView.h>
#include <LListener.h>

#include "CClockEventHandler.h"

#include "CICalendarDateTime.h"

class CTimezonePopup;

class LBevelButton;
class LClock;

// ===========================================================================
//	CDateTimeZoneSelect

class CDateTimeZoneSelect : public LView,
							public LListener
{
public:
	enum { class_ID = 'Dtzs', pane_ID = 1811 };

	static CDateTimeZoneSelect* CreateInside(LView* parent);

						CDateTimeZoneSelect(LStream *inStream);
	virtual				~CDateTimeZoneSelect();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

			void	SetAllDay(bool all_day);
			
			void	SetDateTimeZone(const iCal::CICalendarDateTime& dt, bool all_day);
			void	GetDateTimeZone(iCal::CICalendarDateTime& dt, bool all_day);

			void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eDate_ID = 'DTZD',
		eDateBtn_ID = 'DTZB',
		eTime_ID = 'DTZT',
		eZone_ID = 'DTZZ'

	};

	// UI Objects
	CClockEventHandler	mClockHandler;
	LClock*				mDate;
	LBevelButton*		mDateBtn;
	LClock*				mTime;
	CTimezonePopup*		mZone;

	bool	mAllDay;

	virtual void		FinishCreateSelf();
	
	void	OnDateBtn();
};

#endif
