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

#ifndef H_CCalendarEventTableBase
#define H_CCalendarEventTableBase
#pragma once

#include "CCalendarTableBase.h"

#include "CCalendarEventBase.h"

// ===========================================================================
//	CCalendarEventTableBase

class CCalendarEventTableBase : public CCalendarTableBase
{
public:
	enum
	{
		eBroadcast_CalendarChanged = 'CalC'		// param = NULL
	};

	CCalendarEventTableBase(LStream *inStream);
	virtual ~CCalendarEventTableBase();
	
	virtual void ListenTo_Message(long msg, void* param);

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);
	virtual Boolean		HandleKeyPress(const EventRecord &inKeyEvent);

protected:
	CCalendarEventBaseList	mSelectedEvents;

	virtual void	ClickSelf(const SMouseDownEvent &inMouseDown);	// Click
	
			void	ClickEvent(CCalendarEventBase* item, bool shift);

			void	SelectEvent(CCalendarEventBase* item);
			void	UnselectEvent(CCalendarEventBase* item);
			void	UnselectAllEvents();
			void	ClearSelectedEvents();
	virtual void	SelectEvent(bool next);
			
	virtual void	SelectDay();

	virtual void	OnNewDraft(bool option);
	virtual void	OnEditEvent();
	virtual void	OnEditOneEvent(CCalendarEventBase* event);
	virtual void	OnDuplicateEvent();
	virtual void	OnDeleteEvent();
	virtual void	OnInviteAttendees();
	virtual void	OnProcessInvitation();
	
			void	GetSelectedEvents(iCal::CICalendarExpandedComponents& items);
			void	GetSelectedMasterEvents(iCal::CICalendarComponentRecurs& vevents);
};

#endif
