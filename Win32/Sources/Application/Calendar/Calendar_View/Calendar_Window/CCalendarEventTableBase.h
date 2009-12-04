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

	CCalendarEventTableBase();
	virtual ~CCalendarEventTableBase();
	
	virtual void ListenTo_Message(long msg, void* param);

protected:
	CCalendarEventBaseList	mSelectedEvents;

	virtual	bool	HandleKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);		// Handle key down
	afx_msg	void	OnLButtonDown(UINT nFlags, CPoint point);				// Clicked somewhere
	
			void	ClickEvent(CCalendarEventBase* item, bool shift);

			void	SelectEvent(CCalendarEventBase* item);
			void	UnselectEvent(CCalendarEventBase* item);
			void	UnselectAllEvents();
			void	ClearSelectedEvents();
	virtual void	SelectEvent(bool next);
			
	virtual void	SelectDay();
			
	// Common updaters
	afx_msg void	OnUpdateSelectedEvent(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateDeleteEvent(CCmdUI* pCmdUI);
	afx_msg void	OnUpdateInviteAttendees(CCmdUI* pCmdUI);

	// message handlers
	afx_msg void	OnNewDraft();
	afx_msg void	OnNewDraftOption();
	virtual void	OnNewDraft(bool option);
	afx_msg void	OnEditEvent();
	virtual void	OnEditOneEvent(CCalendarEventBase* event);
	afx_msg void	OnDuplicateEvent();
	afx_msg void	OnDeleteEvent();
	afx_msg void	OnInviteAttendees();
	
			void	GetSelectedEvents(iCal::CICalendarExpandedComponents& items);
			void	GetSelectedMasterEvents(iCal::CICalendarComponentRecurs& vevents);

	DECLARE_MESSAGE_MAP()
};

#endif
