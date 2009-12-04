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

#ifndef H_CYearView
#define H_CYearView
#pragma once

#include "CCalendarViewBase.h"

#include "CIconButton.h"
#include "CPopupButton.h"
#include "CYearTable.h"

// ===========================================================================
//	CYearView

class	CYearView : public CCalendarViewBase
{
public:
						CYearView(CCommander* cmdr);
	virtual				~CYearView();

	virtual void ListenTo_Message(long msg, void* param);

	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual void DoPrint();

	NCalendarView::EYearLayout GetLayout() const
	{
		return mLayoutValue;
	}
	void SetLayout(NCalendarView::EYearLayout layout);

	virtual CCalendarTableBase* GetTable() const;

protected:
	NCalendarView::EYearLayout	mLayoutValue;

	// UI Objects
	CIconButton			mYearMinus;
	CIconButton			mYearPlus;

	CStatic				mYear;
	
	CPopupButton		mLayout;
	
	CYearTable			mTable;	

	virtual void		ResetDate();
			void		SetCaptions();

	afx_msg void		OnUpdateNewEvent(CCmdUI* pCmdUI);

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnPreviousYear();
	afx_msg void		OnNextYear();
	afx_msg void		OnThisYear();
	afx_msg void		OnNewEvent();
	afx_msg void		OnLayout();

	DECLARE_MESSAGE_MAP()
};

#endif
