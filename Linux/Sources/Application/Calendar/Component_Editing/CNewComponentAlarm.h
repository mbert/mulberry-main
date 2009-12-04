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

#include "CNewComponentPanel.h"
#include "CTabPanel.h"

#include "CICalendarVAlarm.h"

#include "HPopupMenu.h"

class CDateTimeZoneSelect;
class CDurationSelect;
class CNumberEdit;
class CTextInputField;
class CTextInputDisplay;

class CNewComponentAlarmBehaviour;
class CNewComponentAlarmRepeats;

class CTabController;
class JXFlatRect;
class JXRadioGroup;
class JXStaticText;
class JXTextCheckbox;
class JXTextRadioButton;

// ===========================================================================
//	CNewComponentAlarm

class CNewComponentAlarm : public CNewComponentPanel
{
public:
	CNewComponentAlarm(JXContainer* enclosure,
						const HSizingOption hSizing, const VSizingOption vSizing,
						const JCoordinate x, const JCoordinate y,
						const JCoordinate w, const JCoordinate h)
		: CNewComponentPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual			~CNewComponentAlarm() {}

	virtual void	OnCreate();

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual void	SetReadOnly(bool read_only);

protected:

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
		//eActionSeparator,
		eActionNotSupported
	};

	// UI Objects
// begin JXLayout1

    JXTextCheckbox*      mUseAlert;
    JXFlatRect*          mUseAlertView;
    CTabController*      mTabs;
    HPopupMenu*          mStatus;
    JXRadioGroup*        mTriggerGroup;
    JXTextRadioButton*   mTriggerOn;
    CDateTimeZoneSelect* mTriggerDateTimeZone;
    JXTextRadioButton*   mTriggerDuration;
    CDurationSelect*     mDuration;
    HPopupMenu*          mRelated;

// end JXLayout1

	CNewComponentAlarmBehaviour*	mAlarmBehaviourItems;
	CNewComponentAlarmRepeats*		mAlarmRepeatsItems;

	virtual void Receive(JBroadcaster* sender, const Message& message);
	
			void	DoUseAlert(bool use_alert);
			void	DoStatus(JIndex value);
			void	DoTrigger(bool use_duration);
			void	DoActionPopup(JIndex value);

			void	SetAlarm(const iCal::CICalendarVAlarm* valarm);
			void	GetAlarm(iCal::CICalendarComponent& owner);
};

class CNewComponentAlarmBehaviour : public CTabPanel
{
public:
	CNewComponentAlarmBehaviour(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual	~CNewComponentAlarmBehaviour() {}

	virtual void	OnCreate();
	virtual void	SetData(void* data) {}			// Set data
	virtual bool	UpdateData(void* data) { return true; }		// Force update of data

	// UI Objects
// begin JXLayout2

    HPopupMenu*        mAction;
    JXStaticText*      mSpeakTxt;
    JXStaticText*      mMessageTxt;
    JXStaticText*      mSubjectTxt;
    CTextInputField*   mDescription;
    JXStaticText*      mBodyTxt;
    CTextInputField*   mMessage;
    JXStaticText*      mToTxt;
    CTextInputDisplay* mAttendees;

// end JXLayout2
};

class CNewComponentAlarmRepeats : public CTabPanel
{
public:
	CNewComponentAlarmRepeats(JXContainer* enclosure,
				const HSizingOption hSizing, const VSizingOption vSizing,
				const JCoordinate x, const JCoordinate y,
				const JCoordinate w, const JCoordinate h)
		: CTabPanel(enclosure, hSizing, vSizing, x, y, w, h) {}
	virtual	~CNewComponentAlarmRepeats() {}

	virtual void	OnCreate();
	virtual void	SetData(void* data) {}			// Set data
	virtual bool	UpdateData(void* data) { return true; }		// Force update of data

	// UI Objects
// begin JXLayout3

    CNumberEdit*     mRepeatCount;
    CDurationSelect* mInterval;

// end JXLayout3
};

#endif
