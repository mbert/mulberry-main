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
#include "CDivider.h"
#include "CDurationSelect.h"
#include "CNewEventDialog.h"
#include "CNewToDoDialog.h"
#include "CNumberEdit.h"
#include "CTabController.h"
#include "CTextField.h"
#include "CTextDisplay.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarRecurrence.h"

#include "TPopupMenu.h"

#include <JXFlatRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXWindow.h>

#include <cassert>

#pragma mark -

void CNewComponentAlarm::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mUseAlert =
        new JXTextCheckbox("Use Alarm", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,10, 85,20);
    assert( mUseAlert != NULL );

    CDivider* obj1 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 100,20, 390,2);
    assert( obj1 != NULL );

    mUseAlertView =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,55, 500,245);
    assert( mUseAlertView != NULL );

    mTabs =
        new CTabController(mUseAlertView,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,95, 480,150);
    assert( mTabs != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Status:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 20,36, 60,20);
    assert( obj2 != NULL );

    mStatus =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 90,36, 150,20);
    assert( mStatus != NULL );

    mTriggerGroup =
        new JXRadioGroup(mUseAlertView,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 10,5, 80,60);
    assert( mTriggerGroup != NULL );
    mTriggerGroup->SetBorderWidth(0);

    mTriggerOn =
        new JXTextRadioButton(1, "Alarm on:", mTriggerGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,5, 80,20);
    assert( mTriggerOn != NULL );

    mTriggerDateTimeZone =
        new CDateTimeZoneSelect(mUseAlertView,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,5, 400,30);
    assert( mTriggerDateTimeZone != NULL );

    mTriggerDuration =
        new JXTextRadioButton(2, "Interval:", mTriggerGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,35, 80,20);
    assert( mTriggerDuration != NULL );

    mDuration =
        new CDurationSelect(mUseAlertView,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,35, 400,30);
    assert( mDuration != NULL );

    mRelated =
        new HPopupMenu("",mUseAlertView,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 90,70, 150,20);
    assert( mRelated != NULL );

// end JXLayout1

	mTriggerDateTimeZone->OnCreate();
	mDuration->OnCreate();
	mTriggerGroup->SelectItem(eTrigger_Duration);

	mAlarmBehaviourItems = new CNewComponentAlarmBehaviour(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 120);
	mTabs->AppendCard(mAlarmBehaviourItems, "Action");
	mAlarmRepeatsItems = new CNewComponentAlarmRepeats(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 120);
	mTabs->AppendCard(mAlarmRepeatsItems, "Repeats");
	mTabs->ShowCard(1);

	mStatus->SetMenuItems("Active %r | Completed %r | Cancelled %r");
	mStatus->SetValue(1);

	mRelated->SetMenuItems("Before the Start %r | Before the End %r | After the Start %r | After the End %r");
	mRelated->SetValue(1);

	ListenTo(mUseAlert);
	ListenTo(mStatus);
	ListenTo(mTriggerGroup);
	ListenTo(mAlarmBehaviourItems->mAction);

	// Init controls
	DoUseAlert(false);
	DoStatus(eStatusActive);
	DoTrigger(eTrigger_Duration);
	DoActionPopup(eActionAudioSound);
}

// Respond to clicks in the icon buttons
void CNewComponentAlarm::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mUseAlert)
		{
			DoUseAlert(mUseAlert->IsChecked());
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mStatus)
		{
			DoStatus(mStatus->GetValue());
			return;
		}
		else if (sender == mAlarmBehaviourItems->mAction)
		{
			DoActionPopup(mAlarmBehaviourItems->mAction->GetValue());
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mTriggerGroup)
			DoTrigger(index == eTrigger_Duration);
		return;
	}
}

void CNewComponentAlarm::DoUseAlert(bool use_alert)
{
	mStatus->SetActive(use_alert);
	DoStatus(mStatus->GetValue());
}

void CNewComponentAlarm::DoStatus(JIndex value)
{
	switch(value)
	{
	case eStatusActive:
		mUseAlertView->SetActive(mUseAlert->IsChecked());
		break;
	case eStatusCompleted:
	case eStatusDisabled:
		mUseAlertView->SetActive(false);
		break;
	}
}

void CNewComponentAlarm::DoTrigger(bool use_duration)
{
	mTriggerDateTimeZone->SetActive(!use_duration);
	mDuration->SetActive(use_duration);
	mRelated->SetActive(use_duration);
}

void CNewComponentAlarm::DoActionPopup(JIndex value)
{
	bool field_update = false;
	switch(value)
	{
	case eActionAudioSound:
	case eActionNotSupported:
		// Hide all captions and fields
		mAlarmBehaviourItems->mSpeakTxt->Hide();
		mAlarmBehaviourItems->mMessageTxt->Hide();
		mAlarmBehaviourItems->mSubjectTxt->Hide();
		mAlarmBehaviourItems->mBodyTxt->Hide();
		mAlarmBehaviourItems->mToTxt->Hide();
		mAlarmBehaviourItems->mDescription->Hide();
		mAlarmBehaviourItems->mMessage->Hide();
		mAlarmBehaviourItems->mAttendees->Hide();
		break;
	case eActionAudioSpeak:
		// Show audio caption and input field only
		mAlarmBehaviourItems->mSpeakTxt->Show();
		mAlarmBehaviourItems->mMessageTxt->Hide();
		mAlarmBehaviourItems->mSubjectTxt->Hide();
		mAlarmBehaviourItems->mBodyTxt->Hide();
		mAlarmBehaviourItems->mToTxt->Hide();
		mAlarmBehaviourItems->mDescription->Show();
		mAlarmBehaviourItems->mMessage->Hide();
		mAlarmBehaviourItems->mAttendees->Hide();
		field_update = true;
		break;
	case eActionDisplay:
		// Show display caption and input field only
		mAlarmBehaviourItems->mSpeakTxt->Hide();
		mAlarmBehaviourItems->mMessageTxt->Show();
		mAlarmBehaviourItems->mSubjectTxt->Hide();
		mAlarmBehaviourItems->mBodyTxt->Hide();
		mAlarmBehaviourItems->mToTxt->Hide();
		mAlarmBehaviourItems->mDescription->Show();
		mAlarmBehaviourItems->mMessage->Hide();
		mAlarmBehaviourItems->mAttendees->Hide();
		field_update = true;
		break;
	case eActionEmail:
		// Show email captions and input fields only
		mAlarmBehaviourItems->mSpeakTxt->Hide();
		mAlarmBehaviourItems->mMessageTxt->Hide();
		mAlarmBehaviourItems->mSubjectTxt->Show();
		mAlarmBehaviourItems->mBodyTxt->Show();
		mAlarmBehaviourItems->mToTxt->Show();
		mAlarmBehaviourItems->mDescription->Show();
		mAlarmBehaviourItems->mMessage->Show();
		mAlarmBehaviourItems->mAttendees->Show();
		field_update = true;
		break;
	}
	
	// Update text field if empty
	if (field_update && mAlarmBehaviourItems->mDescription->GetText().empty())
	{
		// Look for parent item
		// Get dialog that owns this one
		const CModelessDialog* dlg = dynamic_cast<const CModelessDialog*>(GetWindow()->GetDirector());
		cdstring summary;
		if (dynamic_cast<const CNewEventDialog*>(dlg))
			summary = static_cast<const CNewEventDialog*>(dlg)->GetCurrentSummary();
		else if (dynamic_cast<const CNewToDoDialog*>(dlg))
			summary = static_cast<const CNewToDoDialog*>(dlg)->GetCurrentSummary();
		mAlarmBehaviourItems->mDescription->SetText(summary);
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
		mUseAlert->SetState(false);
	}
	else
	{
		mUseAlert->SetState(true);
		
		// Status
		mStatus->SetValue(valarm->GetAlarmStatus() + eStatusActive);

		// Determine trigger type
		if (valarm->IsTriggerAbsolute())
		{
			mTriggerGroup->SelectItem(eTrigger_On);

			// Set start date-time (convert from UTC to current global timezone setting)
			iCal::CICalendarDateTime alarm_on = valarm->GetTriggerOn();
			iCal::CICalendarTimezone user_tzid;
			alarm_on.AdjustTimezone(user_tzid);
			mTriggerDateTimeZone->SetDateTimeZone(alarm_on, false);
		}
		else
		{
			mTriggerGroup->SelectItem(eTrigger_Duration);

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
					mAlarmBehaviourItems->mAction->SetValue(eActionAudioSound);
				else
				{
					mAlarmBehaviourItems->mAction->SetValue(eActionAudioSpeak);
					mAlarmBehaviourItems->mDescription->SetText(audio->GetSpeakText());
				}
			}
			else
				mAlarmBehaviourItems->mAction->SetValue(eActionAudioSound);
			break;
		}
		case iCal::eAction_VAlarm_Display:
		{
			mAlarmBehaviourItems->mAction->SetValue(eActionDisplay);
			const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay* display = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay*>(valarm->GetActionData());
			if (display)
				mAlarmBehaviourItems->mDescription->SetText(display->GetDescription());
			break;
		}
		case iCal::eAction_VAlarm_Email:
		{
			mAlarmBehaviourItems->mAction->SetValue(eActionEmail);
			const iCal::CICalendarVAlarm::CICalendarVAlarmEmail* email = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmEmail*>(valarm->GetActionData());
			if (email)
			{
				mAlarmBehaviourItems->mDescription->SetText(email->GetSummary());
				mAlarmBehaviourItems->mMessage->SetText(email->GetDescription());
				mAlarmBehaviourItems->mAttendees->SetTextList(email->GetAttendees());
			}
			break;
		}
		default:
			mAlarmBehaviourItems->mAction->SetValue(eActionNotSupported);
			break;
		}
		
		// Do repeat & interval
		mAlarmRepeatsItems->mRepeatCount->SetNumberValue(valarm->GetRepeats());
		mAlarmRepeatsItems->mInterval->SetDuration(valarm->GetInterval(), false);
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
	if (!mUseAlert->IsChecked())
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
		switch(mAlarmBehaviourItems->mAction->GetValue())
		{
		case eActionAudioSound:
			alarm->EditAction(iCal::eAction_VAlarm_Audio, new iCal::CICalendarVAlarm::CICalendarVAlarmAudio(cdstring::null_str));
			break;
		case eActionAudioSpeak:
		{
			cdstring speak_text = mAlarmBehaviourItems->mDescription->GetText();
			alarm->EditAction(iCal::eAction_VAlarm_Audio, new iCal::CICalendarVAlarm::CICalendarVAlarmAudio(speak_text));
			break;
		}
		case eActionDisplay:
		{
			cdstring description = mAlarmBehaviourItems->mDescription->GetText();
			alarm->EditAction(iCal::eAction_VAlarm_Display, new iCal::CICalendarVAlarm::CICalendarVAlarmDisplay(description));
			break;
		}
		case eActionEmail:
		{
			cdstring description = mAlarmBehaviourItems->mDescription->GetText();
			cdstring summary = mAlarmBehaviourItems->mMessage->GetText();
			cdstrvect attendees;
			mAlarmBehaviourItems->mAttendees->GetTextList(attendees);
			alarm->EditAction(iCal::eAction_VAlarm_Email, new iCal::CICalendarVAlarm::CICalendarVAlarmEmail(description, summary, attendees));
			break;
		}
		default:;	// Do not make any changes
		}
		
		// Trigger fields
		if (mTriggerGroup->GetSelectedItem() == eTrigger_On)
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
			unsigned long count = mAlarmRepeatsItems->mRepeatCount->GetNumberValue();
			
			iCal::CICalendarDuration interval;
			mAlarmRepeatsItems->mInterval->GetDuration(interval, false);
			alarm->EditRepeats(count, interval);
		}
	}
}

void CNewComponentAlarm::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	mUseAlert->SetActive(!read_only);

	mStatus->SetActive(!read_only && mUseAlert->IsChecked());
	DoStatus(mStatus->GetValue());
}

#pragma mark ______________________________CNewComponentAlarmBehaviour

#pragma mark -

void CNewComponentAlarmBehaviour::OnCreate()
{
// begin JXLayout2

    JXStaticText* obj1 =
        new JXStaticText("Action:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,5, 60,20);
    assert( obj1 != NULL );

    mAction =
        new HPopupMenu("",this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 75,5, 120,20);
    assert( mAction != NULL );

    CDivider* obj2 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 200,15, 270,2);
    assert( obj2 != NULL );

    mSpeakTxt =
        new JXStaticText("Speak:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,35, 62,21);
    assert( mSpeakTxt != NULL );

    mMessageTxt =
        new JXStaticText("Message:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,35, 62,21);
    assert( mMessageTxt != NULL );

    mSubjectTxt =
        new JXStaticText("Subject:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,35, 62,21);
    assert( mSubjectTxt != NULL );

    mDescription =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,35, 400,20);
    assert( mDescription != NULL );

    mBodyTxt =
        new JXStaticText("Message:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,60, 62,21);
    assert( mBodyTxt != NULL );

    mMessage =
        new CTextInputField(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,60, 400,20);
    assert( mMessage != NULL );

    mToTxt =
        new JXStaticText("Email to:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,85, 62,21);
    assert( mToTxt != NULL );

    mAttendees =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,85, 400,20);
    assert( mAttendees != NULL );

// end JXLayout2

	mAction->SetMenuItems("Play Sound %r | Speak Text %r | Display Alert %r | Email %r %l | Not Support %r %d");
	mAction->SetValue(1);
}

#pragma mark ______________________________CNewComponentAlarmRepeats

void CNewComponentAlarmRepeats::OnCreate()
{
// begin JXLayout3

    JXStaticText* obj1 =
        new JXStaticText("Repeat:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 57,20);
    assert( obj1 != NULL );

    mRepeatCount =
        new CNumberEdit(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,5, 70,20);
    assert( mRepeatCount != NULL );

    JXStaticText* obj2 =
        new JXStaticText("time(s)", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 145,5, 47,20);
    assert( obj2 != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Interval:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,50, 57,20);
    assert( obj3 != NULL );

    mInterval =
        new CDurationSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,45, 400,30);
    assert( mInterval != NULL );

// end JXLayout3

	mRepeatCount->OnCreate(0, 0x7FFFFFFF);
	mInterval->OnCreate();
}
