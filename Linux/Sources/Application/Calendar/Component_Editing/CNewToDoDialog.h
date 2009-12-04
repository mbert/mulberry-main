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

#include "CNewComponentDialog.h"
#include "CListener.h"

#include "CICalendarVToDo.h"

#include "HPopupMenu.h"

class JXTextCheckbox;
class JXTextButton;

class CDateTimeZoneSelect;

// ===========================================================================
//	CNewToDoDialog

class CNewToDoDialog : public CNewComponentDialog
{
public:
	static void StartNew(const iCal::CICalendar* calin = NULL);
	static void StartEdit(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded);
	static void StartDuplicate(const iCal::CICalendarVToDo& vtodo);

						CNewToDoDialog(JXDirector* supervisor);
	virtual				~CNewToDoDialog();

	virtual void		ListenTo_Message(long msg, void* param);

protected:

	// UI Objects
// begin JXLayout1

    JXTextCheckbox*      mCompleted;
    CDateTimeZoneSelect* mCompletedDateTimeZone;
    JXTextButton*        mCompletedNow;

// end JXLayout1

	iCal::CICalendarDateTime	mActualCompleted;
	bool						mCompletedExists;

	static void StartModeless(iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded, EModelessAction action);

	virtual void		OnCreate();

	virtual void		Receive(JBroadcaster* sender, const Message& message);

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
