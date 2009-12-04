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

/* 
	CCalendarNotification.h

	Author:			
	Description:	Represents a node in a calendar store calendar hierarchy
*/

#ifndef CCalendarNotification_H
#define CCalendarNotification_H

#include "CICalendarDateTime.h"

#include "cdsharedptr.h"

#include <map>
#include <set>

namespace iCal 
{
	class CICalendarVAlarm;
};

namespace calstore {

class CCalendarNotification
{
public:
	
	struct compare_triggers
	{
		bool operator()(const cdsharedptr<CCalendarNotification>& a, const cdsharedptr<CCalendarNotification>& b) const
		{
			return a.get()->GetTime() < b.get()->GetTime();
		}
	};

	CCalendarNotification(const iCal::CICalendarDateTime& trigger_on, iCal::CICalendarVAlarm* alarm)
	{
		mTriggerOn = trigger_on;
		mTriggerOn.AdjustToUTC();	// Always use UTC times
		mAlarm = alarm;
	}
	virtual ~CCalendarNotification();

	int operator<(const CCalendarNotification& comp) const	// Compare with same
	{
		return mTriggerOn < comp.mTriggerOn;
	}

	const iCal::CICalendarDateTime& GetTime() const
	{
		return mTriggerOn;
	}
 	iCal::CICalendarDateTime& GetTime()
	{
		return mTriggerOn;
	}
	
	iCal::CICalendarVAlarm* GetAlarm() const
	{
		return mAlarm;
	}

protected:
	iCal::CICalendarDateTime		mTriggerOn;
	iCal::CICalendarVAlarm*	mAlarm;
};

typedef cdsharedptr<CCalendarNotification> CCalendarNotification_ptr;
typedef std::multiset<CCalendarNotification_ptr, CCalendarNotification::compare_triggers>	CCalendarNotificationSet;
typedef std::map<const iCal::CICalendarVAlarm*, CCalendarNotification_ptr >					CCalendarNotificationMap;

}	// namespace calstore

#endif	// CCalendarNotification_H
