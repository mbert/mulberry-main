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

#ifndef H_CSummaryView
#define H_CSummaryView
#pragma once

#include "CCalendarViewBase.h"

#include "CCalendarViewTypes.h"
#include "CPopupButton.h"
#include "CSummaryTable.h"
#include "CSummaryTitleTable.h"
#include "CTimezonePopup.h"
#include "CWindowStatesFwd.h"

// ===========================================================================
//	CSummaryView

class CSummaryView : public CCalendarViewBase
{
public:
						CSummaryView(CCommander* cmdr);
	virtual				~CSummaryView();

	virtual void ListenTo_Message(long msg, void* param);

	virtual void SetDate(const iCal::CICalendarDateTime& date);
	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	NCalendarView::ESummaryType GetType() const
	{
		return mSummaryType;
	}
	void SetType(NCalendarView::ESummaryType type);

	NCalendarView::ESummaryRanges GetRange() const
	{
		return mSummaryRange;
	}
	void SetRange(NCalendarView::ESummaryRanges range);

	virtual CCalendarTableBase* GetTable() const;

	virtual void	InitColumns();								// Init columns and text

	virtual void	ResetFont(CFont* font);							// Reset list font

protected:

	// UI Objects
	CTimezonePopup	mTimezonePopup;
	CPopupButton	mRangePopup;

	CSummaryTitleTable	mTitles;
	CSummaryTable		mTable;
	
	NCalendarView::ESummaryType		mSummaryType;
	NCalendarView::ESummaryRanges	mSummaryRange;
	cdstring						mRangeText;

	CColumnInfoArray		mColumnInfo;
	
	virtual	void		ResetDate();

	afx_msg void		OnUpdateNewEvent(CCmdUI* pCmdUI);

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnToday();
	afx_msg void		OnNewEvent();
	afx_msg void		OnTimezone();
	afx_msg void		OnRange();

	DECLARE_MESSAGE_MAP()
};

#endif
