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

#include "CNewComponentAlarm.h"

#include "CDateTimeZoneSelect.h"
#include "CDurationSelect.h"
#include "CNewEventDialog.h"
#include "CNewToDoDialog.h"
#include "CNumberEdit.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CTextDisplay.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarRecurrence.h"

#include <LCheckBox.h>
#include <LLittleArrows.h>
#include <LPopupButton.h>
#include <LRadioButton.h>
#include <LTabsControl.h>

// ---------------------------------------------------------------------------
//	CNewComponentAlarm														  [public]
/**
	Default constructor */

CNewComponentAlarm::CNewComponentAlarm(LStream *inStream) :
	CNewComponentPanel(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CNewComponentAlarm														  [public]
/**
	Destructor */

CNewComponentAlarm::~CNewComponentAlarm()
{
}

#pragma mark -

void CNewComponentAlarm::FinishCreateSelf()
{
	// Get UI items
	mUseAlert = dynamic_cast<LCheckBox*>(FindPaneByID(eUseAlert_ID));
	mUseAlertView = dynamic_cast<LView*>(FindPaneByID(eUseAlertView_ID));

	mStatus = dynamic_cast<LPopupButton*>(FindPaneByID(eStatus_ID));

	mTriggerOn = dynamic_cast<LRadioButton*>(FindPaneByID(eTriggerOn_ID));
	mTriggerDuration = dynamic_cast<LRadioButton*>(FindPaneByID(eTriggerDuration_ID));
	mTriggerDuration->SetValue(1);

	mTriggerDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eTriggerDateTimeZone_ID)));

	mDuration = CDurationSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eDuration_ID)));

	mRelated = dynamic_cast<LPopupButton*>(FindPaneByID(eRelated_ID));

	mTabs = dynamic_cast<LTabsControl*>(FindPaneByID(eTabs_ID));

	mActionView = dynamic_cast<LView*>(FindPaneByID(eActionView_ID));
	mAction = dynamic_cast<LPopupButton*>(FindPaneByID(eAction_ID));
	mDescription = dynamic_cast<CTextFieldX*>(FindPaneByID(eAction_CaptionDescription_ID));;
	mMessage = dynamic_cast<CTextFieldX*>(FindPaneByID(eAction_CaptionMessage_ID));;
	mAttendees = dynamic_cast<CTextDisplay*>(FindPaneByID(eAction_CaptionAttendees_ID));;

	mRepeatView = dynamic_cast<LView*>(FindPaneByID(eRepeatView_ID));
	mRepeatCount = dynamic_cast<CNumberEdit*>(FindPaneByID(eRepeat_Count_ID));
	mRepeatCountSpin = dynamic_cast<LLittleArrows*>(FindPaneByID(eRepeat_Count_Spin_ID));;
	mRepeatCount->SetArrows(mRepeatCountSpin, 0, 1000, 0);
	mInterval = CDurationSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eRepeat_Interval_ID)));

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
	
	// Init controls
	DoUseAlert(false);
	DoStatus(eStatusActive);
	DoTrigger(eTrigger_Duration);
	DoTabs(eTab_Action);
	DoActionPopup(eActionAudioSound);
}

// Respond to clicks in the icon buttons
void CNewComponentAlarm::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eUseAlert_ID:
		DoUseAlert(mUseAlert->GetValue() == 1);
		break;
	case eStatus_ID:
		DoStatus(mStatus->GetValue());
		break;
	case eTriggerOn_ID:
		DoTrigger(false);
		break;
	case eTriggerDuration_ID:
		DoTrigger(true);
		break;
	case eTabs_ID:
		DoTabs(mTabs->GetValue());
		break;
	case eAction_ID:
		DoActionPopup(mAction->GetValue());
		break;
	}
}

void CNewComponentAlarm::DoUseAlert(bool use_alert)
{
	mStatus->SetEnabled(use_alert);
	DoStatus(mStatus->GetValue());
}

void CNewComponentAlarm::DoStatus(UInt32 value)
{
	switch(value)
	{
	case eStatusActive:
		mUseAlertView->SetEnabled(mUseAlert->GetValue() == 1);
		break;
	case eStatusCompleted:
	case eStatusDisabled:
		mUseAlertView->SetEnabled(false);
		break;
	}
}

void CNewComponentAlarm::DoTrigger(bool use_duration)
{
	mTriggerDateTimeZone->SetEnabled(!use_duration);
	mDuration->SetEnabled(use_duration);
	mRelated->SetEnabled(use_duration);
}

void CNewComponentAlarm::DoTabs(UInt32 value)
{
	if (value == eTab_Action)
	{
		mRepeatView->Hide();
		mActionView->Show();
	}
	else
	{
		mActionView->Hide();
		mRepeatView->Show();
	}
}

void CNewComponentAlarm::DoActionPopup(UInt32 value)
{
	CTextFieldX* field_update = NULL;
	switch(value)
	{
	case eActionAudioSound:
	case eActionNotSupported:
		// Hide all captions and fields
		FindPaneByID(eAction_CaptionAudio_ID)->Hide();
		FindPaneByID(eAction_CaptionDisplay_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail1_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail2_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail3_ID)->Hide();
		FindPaneByID(eAction_CaptionDescription_ID)->Hide();
		FindPaneByID(eAction_CaptionMessage_ID)->Hide();
		FindPaneByID(eAction_CaptionAttendeesFocus_ID)->Hide();
		break;
	case eActionAudioSpeak:
		// Show audio caption and input field only
		FindPaneByID(eAction_CaptionAudio_ID)->Show();
		FindPaneByID(eAction_CaptionDisplay_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail1_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail2_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail3_ID)->Hide();
		FindPaneByID(eAction_CaptionDescription_ID)->Show();
		FindPaneByID(eAction_CaptionMessage_ID)->Hide();
		FindPaneByID(eAction_CaptionAttendeesFocus_ID)->Hide();
		field_update = mDescription;
		break;
	case eActionDisplay:
		// Show display caption and input field only
		FindPaneByID(eAction_CaptionAudio_ID)->Hide();
		FindPaneByID(eAction_CaptionDisplay_ID)->Show();
		FindPaneByID(eAction_CaptionEmail1_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail2_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail3_ID)->Hide();
		FindPaneByID(eAction_CaptionDescription_ID)->Show();
		FindPaneByID(eAction_CaptionMessage_ID)->Hide();
		FindPaneByID(eAction_CaptionAttendeesFocus_ID)->Hide();
		field_update = mDescription;
		break;
	case eActionEmail:
		// Show email captions and input fields only
		FindPaneByID(eAction_CaptionAudio_ID)->Hide();
		FindPaneByID(eAction_CaptionDisplay_ID)->Hide();
		FindPaneByID(eAction_CaptionEmail1_ID)->Show();
		FindPaneByID(eAction_CaptionEmail2_ID)->Show();
		FindPaneByID(eAction_CaptionEmail3_ID)->Show();
		FindPaneByID(eAction_CaptionDescription_ID)->Show();
		FindPaneByID(eAction_CaptionMessage_ID)->Show();
		FindPaneByID(eAction_CaptionAttendeesFocus_ID)->Show();
		field_update = mDescription;
		break;
	}
	
	// Update text field if empty
	if ((field_update != NULL) && field_update->GetText().empty())
	{
		// Look for parent item
		LView* super = GetSuperView();
		while(super && !dynamic_cast<CModelessDialog*>(super))
			super = super->GetSuperView();
		CModelessDialog* dlg = dynamic_cast<CModelessDialog*>(super);
		cdstring summary;
		if (dynamic_cast<CNewEventDialog*>(dlg))
			summary = static_cast<CNewEventDialog*>(dlg)->GetCurrentSummary();
		else if (dynamic_cast<CNewToDoDialog*>(dlg))
			summary = static_cast<CNewToDoDialog*>(dlg)->GetCurrentSummary();
		field_update->SetText(summary);
	}
}

void CNewComponentAlarm::SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded)
{
	// Get the first embedded alarm and display that
	const iCal::CICalendarVAlarm* alarm = dynamic_cast<const iCal::CICalendarVAlarm*>(vevent.GetFirstEmbeddedComponent(iCal::CICalendarComponent::eVALARM));
	SetAlarm(alarm);
}

void CNewComponentAlarm::SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded)
{
	// Get the first embedded alarm and display that
	const iCal::CICalendarVAlarm* alarm = dynamic_cast<const iCal::CICalendarVAlarm*>(vtodo.GetFirstEmbeddedComponent(iCal::CICalendarComponent::eVALARM));
	SetAlarm(alarm);
}

void CNewComponentAlarm::SetAlarm(const iCal::CICalendarVAlarm* valarm)
{
	if (valarm == NULL)
	{
		mUseAlert->SetValue(0);
	}
	else
	{
		mUseAlert->SetValue(1);
		
		// Status
		mStatus->SetValue(valarm->GetAlarmStatus() + eStatusActive);

		// Determine trigger type
		if (valarm->IsTriggerAbsolute())
		{
			mTriggerOn->SetValue(1);

			// Set start date-time (convert from UTC to current global timezone setting)
			iCal::CICalendarDateTime alarm_on = valarm->GetTriggerOn();
			iCal::CICalendarTimezone user_tzid;
			alarm_on.AdjustTimezone(user_tzid);
			mTriggerDateTimeZone->SetDateTimeZone(alarm_on, false);
		}
		else
		{
			mTriggerDuration->SetValue(1);

			// Set duration (note whether negative or positive for Before/After determination)
			bool after = true;
			iCal::CICalendarDuration duration = valarm->GetTriggerDuration();
			if (duration.GetTotalSeconds() <= 0)
			{
				after = false;
				duration.SetDuration(-duration.GetTotalSeconds());
			}
			mDuration->SetDuration(duration, false);
			
			// Set ralated
			if (valarm->IsTriggerOnStart())
				mRelated->SetValue(after ? eRelatedAfterStart : eRelatedBeforeStart);
			else
				mRelated->SetValue(after ? eRelatedAfterEnd : eRelatedBeforeEnd);
		}

		// Set action - only do DISPLAY & EMAIL for now
		switch(valarm->GetAction())
		{
		case iCal::eAction_VAlarm_Audio:
		{
			const iCal::CICalendarVAlarm::CICalendarVAlarmAudio* audio = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmAudio*>(valarm->GetActionData());
			if (audio)
			{
				// Switch between audio/speech action based on presence of speak text
				if (!audio->IsSpeakText())
					mAction->SetValue(eActionAudioSound);
				else
				{
					mAction->SetValue(eActionAudioSpeak);
					mDescription->SetText(audio->GetSpeakText());
				}
			}
			else
				mAction->SetValue(eActionAudioSound);
			break;
		}
		case iCal::eAction_VAlarm_Display:
			mAction->SetValue(eActionDisplay);
			const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay* display = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay*>(valarm->GetActionData());
			if (display)
				mDescription->SetText(display->GetDescription());
			break;
		case iCal::eAction_VAlarm_Email:
			mAction->SetValue(eActionEmail);
			const iCal::CICalendarVAlarm::CICalendarVAlarmEmail* email = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmEmail*>(valarm->GetActionData());
			if (email)
			{
				mDescription->SetText(email->GetSummary());
				mMessage->SetText(email->GetDescription());
				mAttendees->SetTextList(email->GetAttendees());
			}
			break;
		default:
			mAction->SetValue(eActionNotSupported);
			break;
		}
		
		// Do repeat & interval
		mRepeatCount->SetNumberValue(valarm->GetRepeats());
		mInterval->SetDuration(valarm->GetInterval(), false);
	}
}

void CNewComponentAlarm::GetEvent(iCal::CICalendarVEvent& vevent)
{
	GetAlarm(vevent);
}

void CNewComponentAlarm::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	GetAlarm(vtodo);
}

void CNewComponentAlarm::GetAlarm(iCal::CICalendarComponent& owner)
{
	// Get any existing alarm
	iCal::CICalendarVAlarm* alarm = dynamic_cast<iCal::CICalendarVAlarm*>(owner.GetFirstEmbeddedComponent(iCal::CICalendarComponent::eVALARM));

	// Check for no alarm
	if (mUseAlert->GetValue() == 0)
	{
		// Delete existing alarm if present
		if (alarm != NULL)
			owner.RemoveComponent(alarm);
	}
	else
	{
		// Create a new one if needed
		if (alarm == NULL)
		{
			alarm = new iCal::CICalendarVAlarm(owner.GetCalendar());

			owner.AddComponent(alarm);
		}
		
		// Now update based on fields
		
		// Status
		alarm->EditStatus(static_cast<iCal::EAlarm_Status>(mStatus->GetValue() - eStatusActive));

		// Action
		switch(mAction->GetValue())
		{
		case eActionAudioSound:
			alarm->EditAction(iCal::eAction_VAlarm_Audio, new iCal::CICalendarVAlarm::CICalendarVAlarmAudio(cdstring::null_str));
			break;
		case eActionAudioSpeak:
		{
			cdstring speak_text = mDescription->GetText();
			alarm->EditAction(iCal::eAction_VAlarm_Audio, new iCal::CICalendarVAlarm::CICalendarVAlarmAudio(speak_text));
			break;
		}
		case eActionDisplay:
		{
			cdstring description = mDescription->GetText();
			alarm->EditAction(iCal::eAction_VAlarm_Display, new iCal::CICalendarVAlarm::CICalendarVAlarmDisplay(description));
			break;
		}
		case eActionEmail:
		{
			cdstring description = mDescription->GetText();
			cdstring summary = mMessage->GetText();
			cdstrvect attendees;
			mAttendees->GetTextList(attendees);
			alarm->EditAction(iCal::eAction_VAlarm_Email, new iCal::CICalendarVAlarm::CICalendarVAlarmEmail(description, summary, attendees));
			break;
		}
		default:;	// Do not make any changes
		}
		
		// Trigger fields
		if (mTriggerOn->GetValue() == 1)
		{
			iCal::CICalendarDateTime trigger;
			mTriggerDateTimeZone->GetDateTimeZone(trigger, false);

			// Always convert time to UTC as that is what iCal requires
			trigger.AdjustToUTC();

			// Now set data in alarm
			alarm->EditTrigger(trigger);
		}
		else
		{
			iCal::CICalendarDuration duration;
			mDuration->GetDuration(duration, false);

			bool after = (mRelated->GetValue() == eRelatedAfterStart) || (mRelated->GetValue() == eRelatedAfterEnd);
			bool start = (mRelated->GetValue() == eRelatedAfterStart) || (mRelated->GetValue() == eRelatedBeforeStart);

			// Negate duration if before
			if (!after)
			{
				duration.SetDuration(-duration.GetTotalSeconds());
			}

			// Now set data in alarm
			alarm->EditTrigger(duration, start);
		}

		// Do repeat & interval
		{
			unsigned long count = mRepeatCount->GetNumberValue();
			
			iCal::CICalendarDuration interval;
			mInterval->GetDuration(interval, false);
			alarm->EditRepeats(count, interval);
		}
	}
}

void CNewComponentAlarm::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	mUseAlert->SetEnabled(!read_only);

	mStatus->SetEnabled(!read_only && (mUseAlert->GetValue() == 1));
	DoStatus(mStatus->GetValue());
}
