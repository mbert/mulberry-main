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
	CCalendarNotifier.cpp

	Author:			
	Description:	<describe the CCalendarNotifier class here>
*/

#include "CCalendarNotifier.h"

#include "CICalendarComponentRecur.h"

#ifdef __MULBERRY
#include "CErrorHandler.h"
#include "CMulberryCommon.h"
#endif
#ifdef __use_speech
#include "CSpeechSynthesis.h"
#endif

using namespace calstore; 

CCalendarNotifier CCalendarNotifier::sCalendarNotifier;

// This constructs the calendar manager root
CCalendarNotifier::CCalendarNotifier()
{
}

CCalendarNotifier::~CCalendarNotifier()
{
}

void CCalendarNotifier::Start()
{
	
}

void CCalendarNotifier::Stop()
{
	
}

void CCalendarNotifier::Pause(bool pause)
{
	
}

void CCalendarNotifier::AddAlarm(iCal::CICalendarVAlarm* alarm)
{
	// Only add if not already present
	if (mAlarms.count(alarm) == 0)
	{
		// Get notifier for alarm (may be NULL)
		cdsharedptr<CCalendarNotification> notify(GenerateNotification(alarm));
		
		// Add alarm and notifier
		mAlarms.insert(CCalendarNotificationMap::value_type(alarm, notify));
		
		// Add notifier if not NULL
		if (notify.get() != NULL)
			mNotifications.insert(notify);
	}
}

void CCalendarNotifier::RemoveAlarm(iCal::CICalendarVAlarm* alarm)
{
	// Find notifications
	CCalendarNotificationMap::iterator found = mAlarms.find(alarm);
	if (found != mAlarms.end())
	{
		// Delete/remove notification if present
		if ((*found).second.get() != NULL)
			mNotifications.erase((*found).second);
		
		// Now remove the alarm
		mAlarms.erase(found);
	}
}

void CCalendarNotifier::ChangedAlarm(iCal::CICalendarVAlarm* alarm)
{
	// Find notifications
	CCalendarNotificationMap::iterator found = mAlarms.find(alarm);
	if (found != mAlarms.end())
	{
		// Remove notification if present
		if ((*found).second.get() != NULL)
			mNotifications.erase((*found).second);

		// Get new notifier for alarm (may be NULL)
		cdsharedptr<CCalendarNotification> notify(GenerateNotification(alarm));
		
		// Replace old one with new one
		(*found).second = notify;
		
		// Add notifier if not NULL
		if (notify.get() != NULL)
			mNotifications.insert(notify);
	}
}

CCalendarNotification* CCalendarNotifier::GenerateNotification(iCal::CICalendarVAlarm* alarm) const
{
	// Must be active
	if (alarm->GetAlarmStatus() == iCal::eAlarm_Status_Pending)
	{
		iCal::CICalendarDateTime next;
		alarm->GetNextTrigger(next);
		
		// Check that it is still pending as the trigger update may determine the alarm is too old
		if (alarm->GetAlarmStatus() == iCal::eAlarm_Status_Pending)
			return new CCalendarNotification(next, alarm);
	}
	
	return NULL;
}

void CCalendarNotifier::Poll()
{
	// Get now
	iCal::CICalendarDateTime now;
	now.SetNowUTC();
	
	// Now look at first item in order set
	while(mNotifications.size() && ((*mNotifications.begin())->GetTime() <= now))
	{
		ExecuteNotification();
	}
	
}

// Always the one on the top of the stack
void CCalendarNotifier::ExecuteNotification()
{
	// Pop off the stack
	CCalendarNotification_ptr notification(*mNotifications.begin());
	mNotifications.erase(mNotifications.begin());
	
	// Now run it
	ExecuteAlarm(notification->GetAlarm());
	
	// Update it (will add back to stack if its still active, otherwise
	// will remove from map
	UpdateNotification(notification);
}

// Do alarm
void CCalendarNotifier::ExecuteAlarm(iCal::CICalendarVAlarm* alarm)
{
	switch(alarm->GetAction())
	{
	case iCal::eAction_VAlarm_Audio:
		{
			const iCal::CICalendarVAlarm::CICalendarVAlarmAudio* audio = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmAudio*>(alarm->GetActionData());
			if (audio != NULL)
			{
				if (audio->IsSpeakText())
				{
#ifdef __use_speech
					// If speech requested, queue before anything else
					if (CSpeechSynthesis::Available())
					{
						// Speak user string as is
						CSpeechSynthesis::SpeakString(audio->GetSpeakText());
					}
					else
#endif
					{
						::PlayNamedSound(cdstring::null_str);
					}
				}
				else
				{
					::PlayNamedSound(cdstring::null_str);
				}
			}
		}
		break;
	case iCal::eAction_VAlarm_Display:
		{
			const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay* display = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay*>(alarm->GetActionData());
			if (display != NULL)
			{
				CErrorHandler::PutNoteAlertRsrcTxt("UI::Calendar::AlertDisplay", display->GetDescription());
			}
		}
		break;
	case iCal::eAction_VAlarm_Email:
		break;
	default:;
	}
}

// Update to next notification in alarm set or remove it
void CCalendarNotifier::UpdateNotification(CCalendarNotification_ptr& notification)
{
	// Update alarm trigger time and see if more to come
	if (notification->GetAlarm()->AlarmTriggered(notification->GetTime()))
	{
		// Add it back into notifications list
		mNotifications.insert(notification);
	}
	else
	{
		// Remove notification from alarm-notification map
		CCalendarNotificationMap::iterator found = mAlarms.find(notification->GetAlarm());
		(*found).second.reset(NULL);
	}
};
