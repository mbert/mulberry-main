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

#ifndef H_CNewComponentAlarm
#define H_CNewComponentAlarm
#pragma once

#include "CNewComponentPanel.h"

#include "CICalendarVAlarm.h"

#include "CCmdEdit.h"
#include "CNumberEdit.h"
#include "CPopupButton.h"
#include "CSubPanelController.h"
#include "CTabController.h"

// ===========================================================================
//	CNewComponentAlarm

class CDateTimeZoneSelect;
class CDurationSelect;
class CNewComponentAlarmBehaviour;
class CNewComponentAlarmRepeats;

class CNewComponentAlarm : public CNewComponentPanel
{
	friend class CNewComponentAlarmBehaviour;
	friend class CNewComponentAlarmRepeats;

public:
						CNewComponentAlarm();
	virtual				~CNewComponentAlarm();

	virtual void	SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetEvent(iCal::CICalendarVEvent& vevent);

	virtual void	SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetToDo(iCal::CICalendarVToDo& vtodo);

	virtual void	SetReadOnly(bool read_only);

protected:
	enum
	{
		eStatusActive = IDM_ALARM_STATES_ACTIVE,
		eStatusCompleted,
		eStatusDisabled
	};

	enum
	{
		eRelatedBeforeStart = IDM_ALARM_INTERVALS_BEFORESTART,
		eRelatedBeforeEnd,
		eRelatedAfterStart,
		eRelatedAfterEnd
	};

	enum
	{
		eTab_Action = 0,
		eTab_Repeat
	};

	enum
	{
		eActionAudioSound = IDM_ALARM_ACTIONS_SOUND,
		eActionAudioSpeak,
		eActionDisplay,
		eActionEmail,
		//eActionSeparator,
		eActionNotSupported
	};

// Dialog Data
	//{{AFX_DATA(CNewComponentAlarm)
	enum { IDD = IDD_CALENDAR_NEW_ALARM };
	CButton					mUseAlert;
	CPopupButton			mStatus;

	CButton					mTriggerOn;
	CButton					mTriggerDuration;

	CSubPanelController		mTriggerDateTimeZoneItem;
	CDateTimeZoneSelect*	mTriggerDateTimeZone;

	CSubPanelController		mDurationItem;
	CDurationSelect*		mDuration;

	CPopupButton			mRelated;

	CTabController			mAlarmTabs;

	CNewComponentAlarmBehaviour*	mAlarmBehaviourItems;
	CNewComponentAlarmRepeats*		mAlarmRepeatsItems;
	//}}AFX_DATA

			void	DoUseAlert(bool use_alert);
			void	DoStatus(UINT value);
			void	EnableStatusView(bool enable);
			void	DoTrigger(bool use_duration);
			void	DoTabs(UINT value);
			void	DoActionPopup(UINT value);

			void	SetAlarm(const iCal::CICalendarVAlarm* valarm);
			void	GetAlarm(iCal::CICalendarComponent& owner);

	// Generated message map functions
	//{{AFX_MSG(CNewComponentAlarm)
	virtual BOOL	OnInitDialog();
	afx_msg void	OnUseAlert();
	afx_msg void	OnStatus();
	afx_msg void	OnTrigger();
	afx_msg void	OnDuration();
	afx_msg void	OnSelChangeAlarmTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

class CNewComponentAlarmBehaviour : public CTabPanel
{
public:
						CNewComponentAlarmBehaviour(CNewComponentAlarm* alarm);
	virtual				~CNewComponentAlarmBehaviour();

	virtual void	SetContent(void* data) {}			// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

	// UI Objects
	CPopupButton			mAction;
	CCmdEdit				mDescription;
	CCmdEdit				mMessage;
	CCmdEdit				mAttendees;

protected:
	CNewComponentAlarm*		mAlarmPanel;

	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnEnable(BOOL bEnable);
	afx_msg	void	OnAction();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

class CNewComponentAlarmRepeats : public CTabPanel
{
public:
						CNewComponentAlarmRepeats(CNewComponentAlarm* alarm);
	virtual				~CNewComponentAlarmRepeats();

	virtual void	SetContent(void* data) {}			// Set data
	virtual bool	UpdateContent(void* data) { return true; }		// Force update of data

	// UI Objects
	CNumberEdit				mRepeatCount;
	CSpinButtonCtrl			mRepeatCountSpin;
	CSubPanelController		mIntervalItem;
	CDurationSelect*		mInterval;

protected:
	CNewComponentAlarm*		mAlarmPanel;

	// Generated message map functions
	//{{AFX_MSG(CDurationSelect)
	virtual BOOL	OnInitDialog();
	afx_msg	void	OnEnable(BOOL bEnable);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#endif
