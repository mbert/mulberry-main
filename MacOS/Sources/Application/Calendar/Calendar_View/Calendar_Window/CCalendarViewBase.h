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

#ifndef H_CCalendarViewBase
#define H_CCalendarViewBase
#pragma once

#include <LView.h>
#include <LCommander.h>
#include "CBroadcaster.h"
#include "CListener.h"

#include "CICalendar.h"
#include "CICalendarDateTime.h"
#include "CICalendarComponentExpanded.h"

#include <UPrinting.h>

class CCalendarTableBase;
class CCalendarView;

// ===========================================================================
//	CCalendarViewBase

class CCalendarViewBase : public LView,
							public LCommander,
							public CBroadcaster,
							public CListener
{
public:
	enum
	{
		eBroadcast_ViewChanged = 'CVch',		// param = CCalendarViewBase*
		eBroadcast_DblClkToMonth = 'DblM',		// param = iCal::CICalendarDateTime*
		eBroadcast_DblClkToDay = 'DblD'			// param = iCal::CICalendarDateTime*
	};

	CCalendarViewBase(LStream *inStream) :
		LView(inStream)
		{ mCalendarView = NULL; }

	virtual	~CCalendarViewBase() {}

	virtual void ListenTo_Message(long msg, void* param);

	void SetCalendarView(CCalendarView* view)
		{ mCalendarView = view; }

	bool IsSingleCalendar() const;
	iCal::CICalendar* GetCalendar() const;

	void PreviewComponent(const iCal::CICalendarComponentExpandedShared& comp);

	virtual void SetDate(const iCal::CICalendarDateTime& date)
		{ mDate = date; ResetDate(); }
	virtual void		ResetDate() = 0;

	const iCal::CICalendarDateTime& GetDate() const
		{ return mDate; }
	virtual iCal::CICalendarDateTime GetSelectedDate() const = 0;

	virtual cdstring GetTitle() const = 0;

	virtual void DoPrint() {}

	virtual CCalendarTableBase* GetTable() const = 0;

protected:
	CCalendarView*				mCalendarView;
	iCal::CICalendarDateTime	mDate;
	
	void	CalendarChanged(iCal::CICalendar* cal);
	void	CalendarClosed(iCal::CICalendar* cal);

	void	OnPickDate();

	LPrintSpec					mMyPrintSpec;

	LPrintSpec*		GetPrintSpec()
		{ return &mMyPrintSpec; }
};

#endif
