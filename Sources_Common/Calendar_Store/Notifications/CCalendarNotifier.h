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
	CCalendarNotifier.h

	Author:			
	Description:	Represents a node in a calendar store calendar hierarchy
*/

#ifndef CCalendarNotifier_H
#define CCalendarNotifier_H

#include "CCalendarNotification.h"

#include "CICalendarDateTime.h"
#include "CICalendarVAlarm.h"

namespace iCal 
{
};

namespace calstore {

class CCalendarNotifier
{
public:
	static CCalendarNotifier sCalendarNotifier;

	CCalendarNotifier();
	~CCalendarNotifier();

	void Start();
	void Stop();
	void Pause(bool pause);

	void AddAlarm(iCal::CICalendarVAlarm* alarm);
	void RemoveAlarm(iCal::CICalendarVAlarm* alarm);
	void ChangedAlarm(iCal::CICalendarVAlarm* alarm);

	void Poll();

protected:
	CCalendarNotificationMap			mAlarms;			// All alarms known to system
	CCalendarNotificationSet			mNotifications;		// All notifications for 'active' alarms

	CCalendarNotification* GenerateNotification(iCal::CICalendarVAlarm* alarm) const;
	void UpdateNotification(CCalendarNotification_ptr& notification);	// Update to next notification in alarm set or remove it
	
	void ExecuteNotification();									// Always the one on the top of the stack
	void ExecuteAlarm(iCal::CICalendarVAlarm* alarm);		// Do alarm
};

}	// namespace calstore

#endif	// CCalendarNotifier_H
