/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarRecurrence.h"

// ---------------------------------------------------------------------------
//	CNewComponentAlarm														  [public]
/**
	Default constructor */

CNewComponentAlarm::CNewComponentAlarm() :
	CNewComponentPanel(IDD_CALENDAR_NEW_ALARM), mStatus(true), mRelated(true)
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

BEGIN_MESSAGE_MAP(CNewComponentAlarm, CNewComponentPanel)
	//{{AFX_MSG_MAP(CNewComponentAlarm)
	ON_COMMAND(IDC_CALENDAR_NEW_ALARM_USE, OnUseAlert)
	ON_COMMAND(IDC_CALENDAR_NEW_ALARM_STATUS, OnStatus)
	ON_COMMAND(IDC_CALENDAR_NEW_ALARM_ALERTON, OnTrigger)
	ON_COMMAND(IDC_CALENDAR_NEW_ALARM_INTERVALON, OnDuration)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CALENDAR_NEW_ALARM_TABS, OnSelChangeAlarmTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentAlarm::OnInitDialog()
{
	CNewComponentPanel::OnInitDialog();

	// Get UI items
	mUseAlert.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_USE, this);

	mStatus.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_STATUS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mStatus.SetMenu(IDR_ALARM_STATES_POPUP);
	mStatus.SetValue(IDM_ALARM_STATES_ACTIVE);

	mTriggerOn.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_ALERTON, this);
	mTriggerDuration.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_INTERVALON, this);
	mTriggerOn.SetCheck(1);

	mTriggerDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_ALERT, this);
	mTriggerDateTimeZone = new CDateTimeZoneSelect;
	mTriggerDateTimeZoneItem.AddPanel(mTriggerDateTimeZone);
	mTriggerDateTimeZoneItem.SetPanel(0);

	mDurationItem.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_INTERVAL, this);
	mDuration = new CDurationSelect;
	mDurationItem.AddPanel(mDuration);
	mDurationItem.SetPanel(0);

	mRelated.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_INTERVALPOPUP, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mRelated.SetMenu(IDR_ALARM_INTERVALS_POPUP);
	mRelated.SetValue(IDM_ALARM_INTERVALS_BEFORESTART);

	mAlarmTabs.SubclassDlgItem(IDC_CALENDAR_NEW_ALARM_TABS, this);
	
	// Create tab panels
	mAlarmBehaviourItems = new CNewComponentAlarmBehaviour(this);
	mAlarmTabs.AddPanel(mAlarmBehaviourItems);

	mAlarmRepeatsItems = new CNewComponentAlarmRepeats(this);
	mAlarmTabs.AddPanel(mAlarmRepeatsItems);

	mAlarmTabs.SetPanel(0);

	// Init controls
	DoUseAlert(false);
	DoTrigger(false);

	return true;
}

void CNewComponentAlarm::DoUseAlert(bool use_alert)
{
	mStatus.EnableWindow(use_alert);
	DoStatus(mStatus.GetValue());
}

void CNewComponentAlarm::DoStatus(UINT value)
{
	switch(value)
	{
	case eStatusActive:
		EnableStatusView(mStatus.IsWindowEnabled());
		break;
	case eStatusCompleted:
	case eStatusDisabled:
		EnableStatusView(false);
		break;
	}
}

void CNewComponentAlarm::EnableStatusView(bool enable)
{
	mTriggerOn.EnableWindow(enable);
	mTriggerDuration.EnableWindow(enable);
	mTriggerDateTimeZone->EnableWindow(enable && (mTriggerOn.GetCheck() == 1));
	mDuration->EnableWindow(enable && (mTriggerDuration.GetCheck() == 1));
	mRelated.EnableWindow(enable && (mTriggerDuration.GetCheck() == 1));
	mAlarmTabs.EnableWindow(enable);
	mAlarmBehaviourItems->EnableWindow(enable);
	mAlarmRepeatsItems->EnableWindow(enable);
}

void CNewComponentAlarm::DoTrigger(bool use_duration)
{
	mTriggerDateTimeZone->EnableWindow(!use_duration);
	mDuration->EnableWindow(use_duration);
	mRelated.EnableWindow(use_duration);
}

void CNewComponentAlarm::DoTabs(UINT value)
{
}

void CNewComponentAlarm::DoActionPopup(UINT value)
{
	bool field_update = false;
	switch(value)
	{
	case eActionAudioSound:
	case eActionNotSupported:
		// Hide all captions and fields
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_SPEAKTXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGETXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL1TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL2TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL3TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_DESCRIPTION)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGE)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_ATTENDEES)->ShowWindow(SW_HIDE);
		break;
	case eActionAudioSpeak:
		// Show audio caption and input field only
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_SPEAKTXT)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGETXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL1TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL2TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL3TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_DESCRIPTION)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGE)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_ATTENDEES)->ShowWindow(SW_HIDE);
		field_update = true;
		break;
	case eActionDisplay:
		// Show display caption and input field only
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_SPEAKTXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGETXT)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL1TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL2TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL3TXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_DESCRIPTION)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGE)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_ATTENDEES)->ShowWindow(SW_HIDE);
		field_update = true;
		break;
	case eActionEmail:
		// Show email captions and input fields only
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_SPEAKTXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGETXT)->ShowWindow(SW_HIDE);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL1TXT)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL2TXT)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_EMAIL3TXT)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_DESCRIPTION)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGE)->ShowWindow(SW_SHOW);
		mAlarmBehaviourItems->GetDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_ATTENDEES)->ShowWindow(SW_SHOW);
		field_update = true;
		break;
	}
	
	// Update text field if empty
	if (field_update && mAlarmBehaviourItems->mDescription.GetText().empty())
	{
		// Look for parent item
		// Get window that owns this one
		CWnd* super = GetParent();
		while(super && !dynamic_cast<CModelessDialog*>(super))
			super = super->GetParent();
		CModelessDialog* dlg = dynamic_cast<CModelessDialog*>(super);
		cdstring summary;
		if (dynamic_cast<CNewEventDialog*>(dlg))
			summary = static_cast<CNewEventDialog*>(dlg)->GetCurrentSummary();
		else if (dynamic_cast<CNewToDoDialog*>(dlg))
			summary = static_cast<CNewToDoDialog*>(dlg)->GetCurrentSummary();
		mAlarmBehaviourItems->mDescription.SetText(summary);
	}
}

void CNewComponentAlarm::OnUseAlert()
{
	DoUseAlert(mUseAlert.GetCheck());
}

void CNewComponentAlarm::OnStatus()
{
	DoStatus(mStatus.GetValue());
}

void CNewComponentAlarm::OnTrigger()
{
	DoTrigger(false);
}

void CNewComponentAlarm::OnDuration()
{
	DoTrigger(true);
}

void CNewComponentAlarm::OnSelChangeAlarmTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	mAlarmTabs.SetPanel(mAlarmTabs.GetCurSel());
	DoTabs(mAlarmTabs.GetCurSel());

	*pResult = 0;
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
		mUseAlert.SetCheck(0);
		OnUseAlert();
	}
	else
	{
		mUseAlert.SetCheck(1);
		OnUseAlert();
		
		// Status
		mStatus.SetValue(valarm->GetAlarmStatus() + eStatusActive);
		OnStatus();

		// Determine trigger type
		if (valarm->IsTriggerAbsolute())
		{
			mTriggerOn.SetCheck(1);
			mTriggerDuration.SetCheck(0);
			DoTrigger(false);

			// Set start date-time (convert from UTC to current global timezone setting)
			iCal::CICalendarDateTime alarm_on = valarm->GetTriggerOn();
			iCal::CICalendarTimezone user_tzid;
			alarm_on.AdjustTimezone(user_tzid);
			mTriggerDateTimeZone->SetDateTimeZone(alarm_on, false);
		}
		else
		{
			mTriggerDuration.SetCheck(1);
			mTriggerOn.SetCheck(0);
			DoTrigger(true);

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
				mRelated.SetValue(after ? eRelatedAfterStart : eRelatedBeforeStart);
			else
				mRelated.SetValue(after ? eRelatedAfterEnd : eRelatedBeforeEnd);
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
					mAlarmBehaviourItems->mAction.SetValue(eActionAudioSound);
				else
				{
					mAlarmBehaviourItems->mAction.SetValue(eActionAudioSpeak);
					mAlarmBehaviourItems->mDescription.SetText(audio->GetSpeakText());
				}
			}
			else
				mAlarmBehaviourItems->mAction.SetValue(eActionAudioSound);
			break;
		}
		case iCal::eAction_VAlarm_Display:
		{
			mAlarmBehaviourItems->mAction.SetValue(eActionDisplay);
			const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay* display = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmDisplay*>(valarm->GetActionData());
			if (display)
				mAlarmBehaviourItems->mDescription.SetText(display->GetDescription());
			break;
		}
		case iCal::eAction_VAlarm_Email:
		{
			mAlarmBehaviourItems->mAction.SetValue(eActionEmail);
			const iCal::CICalendarVAlarm::CICalendarVAlarmEmail* email = dynamic_cast<const iCal::CICalendarVAlarm::CICalendarVAlarmEmail*>(valarm->GetActionData());
			if (email)
			{
				mAlarmBehaviourItems->mDescription.SetText(email->GetSummary());
				mAlarmBehaviourItems->mMessage.SetText(email->GetDescription());
				//mAlarmBehaviourItems->mAttendees.SetText(email->GetAttendees());
			}
			break;
		}
		default:
			mAlarmBehaviourItems->mAction.SetValue(eActionNotSupported);
			break;
		}
		DoActionPopup(mAlarmBehaviourItems->mAction.GetValue());

		// Do repeat & interval
		mAlarmRepeatsItems->mRepeatCount.SetValue(valarm->GetRepeats());
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
	if (mUseAlert.GetCheck() == 0)
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
		alarm->EditStatus(static_cast<iCal::EAlarm_Status>(mStatus.GetValue() - eStatusActive));

		// Action
		switch(mAlarmBehaviourItems->mAction.GetValue())
		{
		case eActionAudioSound:
			alarm->EditAction(iCal::eAction_VAlarm_Audio, new iCal::CICalendarVAlarm::CICalendarVAlarmAudio(cdstring::null_str));
			break;
		case eActionAudioSpeak:
		{
			cdstring speak_text = mAlarmBehaviourItems->mDescription.GetText();
			alarm->EditAction(iCal::eAction_VAlarm_Audio, new iCal::CICalendarVAlarm::CICalendarVAlarmAudio(speak_text));
			break;
		}
		case eActionDisplay:
		{
			cdstring description = mAlarmBehaviourItems->mDescription.GetText();
			alarm->EditAction(iCal::eAction_VAlarm_Display, new iCal::CICalendarVAlarm::CICalendarVAlarmDisplay(description));
			break;
		}
		case eActionEmail:
		{
			cdstring description = mAlarmBehaviourItems->mDescription.GetText();
			cdstring summary = mAlarmBehaviourItems->mMessage.GetText();
			cdstrvect attendees;
			//mAlarmBehaviourItems->mAttendees.GetText(attendees);
			alarm->EditAction(iCal::eAction_VAlarm_Email, new iCal::CICalendarVAlarm::CICalendarVAlarmEmail(description, summary, attendees));
			break;
		}
		default:;	// Do not make any changes
		}
		
		// Trigger fields
		if (mTriggerOn.GetCheck() == 1)
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

			bool after = (mRelated.GetValue() == eRelatedAfterStart) || (mRelated.GetValue() == eRelatedAfterEnd);
			bool start = (mRelated.GetValue() == eRelatedAfterStart) || (mRelated.GetValue() == eRelatedBeforeStart);

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
			unsigned long count = mAlarmRepeatsItems->mRepeatCount.GetValue();
			
			iCal::CICalendarDuration interval;
			mAlarmRepeatsItems->mInterval->GetDuration(interval, false);
			alarm->EditRepeats(count, interval);
		}
	}
}

void CNewComponentAlarm::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	mUseAlert.EnableWindow(!read_only);

	mStatus.EnableWindow(!read_only && (mUseAlert.GetCheck() == 1));
	DoStatus(mStatus.GetValue());
}

#pragma mark ______________________________CNewComponentAlarmBehaviour

// ---------------------------------------------------------------------------
//	CNewComponentAlarmBehaviour														  [public]
/**
	Default constructor */

CNewComponentAlarmBehaviour::CNewComponentAlarmBehaviour(CNewComponentAlarm* alarm) :
	CTabPanel(IDD_CALENDAR_ALARM_BEHAVIOUR),
	mAction(true)
{
	mAlarmPanel = alarm;
}


// ---------------------------------------------------------------------------
//	~CNewComponentAlarmBehaviour														  [public]
/**
	Destructor */

CNewComponentAlarmBehaviour::~CNewComponentAlarmBehaviour()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentAlarmBehaviour, CTabPanel)
	//{{AFX_MSG_MAP(CNewComponentAlarmBehaviour)
	ON_WM_ENABLE()

	ON_COMMAND(IDC_CALENDAR_ALARM_BEHAVIOUR_ACTION, OnAction)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentAlarmBehaviour::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Get UI items
	mAction.SubclassDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_ACTION, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mAction.SetMenu(IDR_ALARM_ACTIONS_POPUP);
	mAction.SetValue(IDM_ALARM_ACTIONS_SOUND);

	mDescription.SubclassDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_DESCRIPTION, this);
	mMessage.SubclassDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_MESSAGE, this);
	mAttendees.SubclassDlgItem(IDC_CALENDAR_ALARM_BEHAVIOUR_ATTENDEES, this);
	
	// Init control state
	OnAction();

	return true;
}

void CNewComponentAlarmBehaviour::OnEnable(BOOL bEnable)
{
	mAction.EnableWindow(bEnable);

	mDescription.EnableWindow(bEnable);
	mMessage.EnableWindow(bEnable);
	mAttendees.EnableWindow(bEnable);
}

void CNewComponentAlarmBehaviour::OnAction()
{
	mAlarmPanel->DoActionPopup(mAction.GetValue());
}

#pragma mark ______________________________CNewComponentAlarmRepeats

// ---------------------------------------------------------------------------
//	CNewComponentAlarmRepeats														  [public]
/**
	Default constructor */

CNewComponentAlarmRepeats::CNewComponentAlarmRepeats(CNewComponentAlarm* alarm) :
	CTabPanel(IDD_CALENDAR_ALARM_REPEATS)
{
	mAlarmPanel = alarm;
}


// ---------------------------------------------------------------------------
//	~CNewComponentAlarmRepeats														  [public]
/**
	Destructor */

CNewComponentAlarmRepeats::~CNewComponentAlarmRepeats()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentAlarmRepeats, CTabPanel)
	//{{AFX_MSG_MAP(CNewComponentAlarmRepeats)
	ON_WM_ENABLE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentAlarmRepeats::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Get UI items
	mRepeatCount.SubclassDlgItem(IDC_CALENDAR_ALARM_REPEATS_COUNT, this);
	mRepeatCount.SetValue(1);
	mRepeatCountSpin.SubclassDlgItem(IDC_CALENDAR_ALARM_REPEATS_COUNTSPIN, this);
	mRepeatCountSpin.SetRange(1, 1000);

	mIntervalItem.SubclassDlgItem(IDC_CALENDAR_ALARM_REPEATS_INTERVAL, this);
	mInterval = new CDurationSelect;
	mIntervalItem.AddPanel(mInterval);
	mIntervalItem.SetPanel(0);
	
	return true;
}

void CNewComponentAlarmRepeats::OnEnable(BOOL bEnable)
{
	mRepeatCount.EnableWindow(bEnable);
	mRepeatCountSpin.EnableWindow(bEnable);
	mInterval->EnableWindow(bEnable);
}
