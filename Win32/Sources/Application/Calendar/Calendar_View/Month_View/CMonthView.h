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

#include "CIconButton.h"
#include "CMonthTable.h"
#include "CMonthTitleTable.h"
#include "CTimezonePopup.h"

class CCalendarEventBase;

namespace calstore
{
	class CCalendarStoreNode;
}


// ===========================================================================
//	CMonthView

class	CMonthView : public CCalendarViewBase
{
public:
						CMonthView(CCommander* cmdr);
	virtual				~CMonthView();

	virtual void ListenTo_Message(long msg, void* param);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	virtual CCalendarTableBase* GetTable() const;

protected:
	// UI Objects
	CIconButton			mMonthMinus;
	CIconButton			mMonthPlus;
	CIconButton			mYearMinus;
	CIconButton			mYearPlus;
	CTimezonePopup		mTimezonePopup;

	CStatic				mMonth;
	CStatic				mYear;
	
	CMonthTitleTable	mTitles;
	CMonthTable			mTable;

	virtual	void		ResetDate();
			void		SetCaptions();

	afx_msg void		OnUpdateNewEvent(CCmdUI* pCmdUI);

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnPreviousMonth();
	afx_msg void		OnNextMonth();
	afx_msg void		OnPreviousYear();
	afx_msg void		OnNextYear();
	afx_msg void		OnToday();
	afx_msg void		OnNewEvent();
	afx_msg void		OnTimezone();

	DECLARE_MESSAGE_MAP()
};

#endif
