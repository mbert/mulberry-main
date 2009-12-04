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

#ifndef H_CMONTHVIEW
#define H_CMONTHVIEW
#pragma once

#include "CCalendarViewBase.h"
#include <LListener.h>

class CCalendarEventBase;
class CMonthTable;
class CTimezonePopup;

class LBevelButton;
class CStaticText;

namespace calstore
{
	class CCalendarStoreNode;
}


// ===========================================================================
//	CMonthView

class	CMonthView : public CCalendarViewBase,
						public LListener
{
public:
	enum { class_ID = 'CalM', pane_ID = 1802 };

						CMonthView(LStream *inStream);
	virtual				~CMonthView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void ListenToMessage(MessageT inMessage, void *ioParam);		// Respond to clicks in the icon buttons

	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = NULL);
	virtual void		FindCommandStatus(CommandT inCommand,
											Boolean &outEnabled,
											Boolean &outUsesMark,
											UInt16 &outMark,
											Str255 outName);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	virtual CCalendarTableBase* GetTable() const;

protected:
	enum
	{
		eMonthMinus_ID = 'LMNT',
		eMonthPlus_ID = 'PMNT',
		eYearMinus_ID = 'LYEA',
		eYearPlus_ID = 'PYEA',
		eTimezone_ID = 'TZID',
		eMonth_ID = 'MNTH',
		eYear_ID = 'YEAR',
		eTable_ID = 'TABL'
	};

	// UI Objects
	LBevelButton*		mMonthMinus;
	LBevelButton*		mMonthPlus;
	LBevelButton*		mYearMinus;
	LBevelButton*		mYearPlus;
	CTimezonePopup*		mTimezonePopup;

	CStaticText*		mMonth;
	CStaticText*		mYear;
	
	CMonthTable*		mTable;

	virtual void		FinishCreateSelf();
	
	virtual	void		ResetDate();
			void		SetCaptions();

			void		OnPreviousMonth();
			void		OnNextMonth();
			void		OnPreviousYear();
			void		OnNextYear();
			void		OnToday();
			void		OnNewEvent();
			void		OnTimezone();
};

#endif
