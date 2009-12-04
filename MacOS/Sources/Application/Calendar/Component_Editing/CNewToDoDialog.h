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

#ifndef H_CNewToDoDialog
#define H_CNewToDoDialog
#pragma once

#include "CNewComponentDialog.h"

#include "CICalendarVToDo.h"

class CDateTimeZoneSelect;
class LBevelButton;

// ===========================================================================
//	CNewToDoDialog

class CNewToDoDialog : public CNewComponentDialog
{
public:
	enum { class_ID = 'EdTD', pane_ID = 1813 };
	
	static void StartNew(const iCal::CICalendar* calin = NULL);
	static void StartEdit(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	static void StartDuplicate(const iCal::CICalendarVToDo& vtodo);
	
	CNewToDoDialog(LStream *inStream);
	virtual				~CNewToDoDialog();
	
	virtual void		ListenToMessage(MessageT inMessage, void *ioParam);
	
protected:
	enum
	{
		eCompleted_ID = 'COMP',
		eCompletedDateTimeZone_ID = 'CDTS',
		eCompletedNow_ID = 'TNOW',
	};
	
	// UI Objects
	LCheckBox*				mCompleted;
	CDateTimeZoneSelect*	mCompletedDateTimeZone;
	LBevelButton*			mCompletedNow;
	
	iCal::CICalendarDateTime	mActualCompleted;
	bool						mCompletedExists;
	
	static void StartModeless(iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded, EModelessAction action);
	
	
	virtual void		FinishCreateSelf();
	
	virtual void		InitPanels();
	
	virtual void	SetComponent(iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded);
	virtual void	GetComponent(iCal::CICalendarComponentRecur& vcomponent);
	
	void	DoCompleted(bool completed);
	void	DoNow();
	
	virtual void	SetReadOnly(bool read_only);
	
	virtual void	ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status);

	virtual bool	DoNewOK();
	virtual bool	DoEditOK();
	virtual void	DoCancel();
};

#endif
