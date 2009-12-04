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

#ifndef H_CNewComponentDialog
#define H_CNewComponentDialog
#pragma once

#include "CModelessDialog.h"
#include "CListener.h"

#include "CICalendarComponentRecur.h"
#include "CICalendarComponentExpanded.h"

class LCheckBox;
class LPopupButton;
class LTabsControl;
class CStaticText;
class CTextFieldX;

class CCalendarPopup;
class CNewComponentPanel;
typedef std::vector<CNewComponentPanel*> CNewComponentPanelList;
class CNewTimingPanel;

// ===========================================================================
//	CNewComponentDialog

class CNewComponentDialog : public CModelessDialog, public CListener
{
public:
	enum EModelessAction
	{
		eNew,
		eEdit,
		eDuplicate
	};
	
	CNewComponentDialog(LStream *inStream);
	virtual				~CNewComponentDialog();
	
	virtual void		ListenTo_Message(long msg, void* param);
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);
	
	iCal::CICalendarRef GetCurrentCalendar() const;
	cdstring	GetCurrentSummary() const;
	void		GetCurrentPeriod(iCal::CICalendarPeriod& period) const;
	const CNewTimingPanel* GetTimingPanel() const;
	
	bool ContainsComponent(const iCal::CICalendarComponentRecur& vcomponent) const;
	
protected:
	enum
	{
		eSummary_ID = 'SUMM',
		eCalendar_ID = 'CALP',
		
		eTabs_ID = 'TABS',
		eTabView_ID = 'VIEW',
		
		eChangeOrganised_ID = 'ORGE',
		eAttendText_ID = 'ATTT',
		eAttend_ID = 'ATTN',
		eOK_ID = 'OKBT'
	};
	
	// UI Objects
	CTextFieldX*			mSummary;
	CCalendarPopup*			mCalendar;
	
	LTabsControl*			mTabs;
	LView*					mTabView;
	
	LCheckBox*				mOrganiserEdit;
	CStaticText*			mAttendText;
	LPopupButton*			mAttendPopup;
	
	EModelessAction			mAction;
	CNewComponentPanelList	mPanels;
	iCal::CICalendarComponentRecur*	mComponent;
	const iCal::CICalendarComponentExpanded* mExpanded;
	bool					mRecurring;
	bool					mIsOverride;
	uint32_t				mCurrentPanel;
	bool					mReadOnly;
	bool					mAttended;

	static uint32_t					sTitleCounter;
	static std::set<CNewComponentDialog*>	sDialogs;

	virtual void		FinishCreateSelf();

	virtual void		InitPanels() = 0;

	void	SetAction(EModelessAction action)
	{
		mAction = action;
	}
	virtual void	SetComponent(iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded = NULL);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);
	void	ChangedSummary();
	void	ChangedCalendar();
	virtual void ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status);
	
	virtual void	SetReadOnly(bool read_only);
	
	void	DoTab(UInt32 value);
	
	virtual void	OnOK();
	virtual void	OnCancel();
	void	OnOrganiserEdit();
	void    OnAttendEdit();
	
	virtual uint32_t&	TitleCounter()
	{
		return sTitleCounter;
	}
	
	virtual bool	DoNewOK() = 0;
	virtual bool	DoEditOK() = 0;
	virtual void	DoCancel()  = 0;
	
	bool	GetCalendar(iCal::CICalendarRef oldcal, iCal::CICalendarRef& newcal, iCal::CICalendar*& new_cal);
};

#endif
