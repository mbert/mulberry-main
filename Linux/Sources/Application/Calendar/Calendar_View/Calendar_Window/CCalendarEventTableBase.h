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

	CCalendarEventTableBase(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
	virtual ~CCalendarEventTableBase();
	
	virtual void		ListenTo_Message(long msg, void* param);

	virtual bool 		ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void 		UpdateCommand(unsigned long cmd, CCmdUI* cmdui);
	virtual bool		HandleChar(const int key, const JXKeyModifiers& modifiers);

protected:
	CCalendarEventBaseList	mSelectedEvents;

	virtual void HandleMouseDown(const JPoint& pt,
									const JXMouseButton button,
									const JSize clickCount, 
									const JXButtonStates& buttonStates,
									const JXKeyModifiers& modifiers);
	
			void	ClickEvent(CCalendarEventBase* item, bool shift);

	virtual void	DrawAll(JXWindowPainter& p, const JRect& frameG);

			void	SelectEvent(CCalendarEventBase* item);
			void	UnselectEvent(CCalendarEventBase* item);
			void	UnselectAllEvents();
			void	ClearSelectedEvents();
	virtual void	SelectEvent(bool next);
			
	virtual void	SelectDay();

	// Common updaters
			void	OnUpdateSelectedEvent(CCmdUI* pCmdUI);
			void	OnUpdateDeleteEvent(CCmdUI* pCmdUI);
			void	OnUpdateInviteAttendees(CCmdUI* pCmdUI);

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
