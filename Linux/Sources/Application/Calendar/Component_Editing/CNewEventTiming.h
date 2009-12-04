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

#include "CNewTimingPanel.h"

class CDateTimeZoneSelect;
class CDurationSelect;

class JXRadioGroup;
class JXTextCheckbox;
class JXTextRadioButton;

// ===========================================================================
//	CNewEventTiming

class CNewEventTiming : public CNewTimingPanel
{
public:

	CNewEventTiming(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h)
		: CNewTimingPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual				~CNewEventTiming() {}

	virtual void	OnCreate();

	virtual bool	GetAllDay() const;
	virtual void	GetTimezone(iCal::CICalendarTimezone& tz) const;

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);
			void    GetPeriod(iCal::CICalendarPeriod& period);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eEnd_End = 1,
		eEnd_Duration
	};

	// UI Objects
// begin JXLayout1

    JXTextCheckbox*      mAllDay;
    CDateTimeZoneSelect* mStartDateTimeZone;
    JXRadioGroup*        mUseGroup;
    JXTextRadioButton*   mUseEnd;
    CDateTimeZoneSelect* mEndDateTimeZone;
    JXTextRadioButton*   mUseDuration;
    CDurationSelect*     mDuration;

// end JXLayout1

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
			void	DoAllDay(bool set);
			void	DoEnds(bool use_duration);
			
			void	SyncEnd();
			void	SyncDuration();
};

#endif
