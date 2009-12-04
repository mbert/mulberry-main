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

#ifndef H_CToDoView
#define H_CToDoView
#pragma once

#include "CCalendarViewBase.h"
#include <LListener.h>

class CToDoTable;

class LPopupButton;
class LScrollerView;

// ===========================================================================
//	CToDoView

class	CToDoView : public CCalendarViewBase,
					public LListener
{
	friend class CCalendarView;

public:
	enum { class_ID = 'CalT', pane_ID = 1805 };

						CToDoView(LStream *inStream);
	virtual				~CToDoView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual CCalendarTableBase* GetTable() const;

protected:
	enum
	{
		eShowPopup_ID = 'SHOW',
		eScroller_ID = 'SCRL',
		eTable_ID = 'TABL'
	};

	enum
	{
		eShowAllDue = 1,
		eShowAll,
		eShowDueToday,
		eShowDueThisWeek,
		eShowOverdue
	};

	uint32_t			mShowValue;

	// UI Objects
	LPopupButton*		mShowPopup;
	
	LScrollerView*		mScroller;
	CToDoTable*			mTable;
	

	virtual void		FinishCreateSelf();
	
	virtual void		ResetDate();

			void		OnNewToDo();
			void		OnShowPopup();
};

#endif
