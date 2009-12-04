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

#ifndef H_CNewComponentAlarm
#define H_CNewComponentAlarm
#pragma once

#include "CNewComponentPanel.h"
#include <LListener.h>

#include "CICalendarVAlarm.h"

class CDateTimeZoneSelect;
class CDurationSelect;
class CNumberEdit;
class CTextFieldX;
class CTextDisplay;

class LCheckBox;
class LLittleArrows;
class LPopupButton;
class LPushButton;
class LRadioButton;
class LTabsControl;

// ===========================================================================
//	CNewComponentAlarm

class CNewComponentAlarm : public CNewComponentPanel,
							public LListener
{
public:
	enum { class_ID = 'Ncal', pane_ID = 1818 };

						CNewComponentAlarm(LStream *inStream);
	virtual				~CNewComponentAlarm();

	virtual void	ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eUseAlert_ID = 'ALRT',
		eUseAlertView_ID = 'UALR',

		eStatus_ID = 'STAT',

		eTriggerOn_ID = 'ENDS',
		eTriggerDuration_ID = 'DURA',

		eTriggerDateTimeZone_ID = 'DTDS',

		eDuration_ID = 'DURS',
		
		eRelated_ID = 'INTE',

		eTabs_ID = 'TABS',

		eActionView_ID = 'ACTN',

		eAction_ID = 'ACTI',
		
		eAction_CaptionAudio_ID = 'CAP1',
		eAction_CaptionDisplay_ID = 'CAP2',
		eAction_CaptionEmail1_ID = 'CAP3',
		eAction_CaptionEmail2_ID = 'CAP4',
		eAction_CaptionEmail3_ID = 'CAP5',

		eAction_CaptionDescription_ID = 'DESC',
		eAction_CaptionMessage_ID = 'BODY',
		eAction_CaptionAttendeesFocus_ID = 'ATTD',
		eAction_CaptionAttendees_ID = 'ADDR',

		eRepeatView_ID = 'REPE',
		eRepeat_Count_ID = 'RNUM',
		eRepeat_Count_Spin_ID = 'RNUm',
		eRepeat_Interval_ID = 'INTV'
	};

	enum
	{
		eStatusActive = 1,
		eStatusCompleted,
		eStatusDisabled
	};

	enum
	{
		eTrigger_On = 1,
		eTrigger_Duration
	};

	enum
	{
		eRelatedBeforeStart = 1,
		eRelatedBeforeEnd,
		eRelatedAfterStart,
		eRelatedAfterEnd
	};

	enum
	{
		eTab_Action = 1,
		eTab_Repeat
	};

	enum
	{
		eActionAudioSound = 1,
		eActionAudioSpeak,
		eActionDisplay,
		eActionEmail,
		eActionSeparator,
		eActionNotSupported
	};

	// UI Objects
	LCheckBox*				mUseAlert;
	LView*					mUseAlertView;

	LPopupButton*			mStatus;

	LRadioButton*			mTriggerOn;
	LRadioButton*			mTriggerDuration;

	CDateTimeZoneSelect*	mTriggerDateTimeZone;

	CDurationSelect*		mDuration;

	LPopupButton*			mRelated;

	LTabsControl*			mTabs;

	LView*					mActionView;
	LPopupButton*			mAction;
	CTextFieldX*			mDescription;
	CTextFieldX*			mMessage;
	CTextDisplay*			mAttendees;

	LView*					mRepeatView;
	CNumberEdit*			mRepeatCount;
	LLittleArrows*			mRepeatCountSpin;
	CDurationSelect*		mInterval;

	virtual void		FinishCreateSelf();
	
			void	DoUseAlert(bool use_alert);
			void	DoStatus(UInt32 value);
			void	DoTrigger(bool use_duration);
			void	DoTabs(UInt32 value);
			void	DoActionPopup(UInt32 value);

			void	SetAlarm(const iCal::CICalendarVAlarm* valarm);
			void	GetAlarm(iCal::CICalendarComponent& owner);
};

#endif
