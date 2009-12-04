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

#include "CPopupButton.h"
#include "CToDoTable.h"

class CToDoTable;

// ===========================================================================
//	CToDoView

class CToDoView : public CCalendarViewBase
{
	friend class CCalendarView;

public:
						CToDoView(CCommander* cmdr);
	virtual				~CToDoView();

	virtual void ListenTo_Message(long msg, void* param);

	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual CCalendarTableBase* GetTable() const;

protected:
	enum
	{
		eShowAllDue = IDM_TODOSHOW_ALLDUE,
		eShowAll,
		eShowDueToday,
		eShowDueThisWeek,
		eShowOverdue
	};

	uint32_t			mShowValue;

	// UI Objects
	CStatic				mShowPopupTitle;
	CPopupButton		mShowPopup;
	
	CToDoTable			mTable;
	
	virtual void		ResetDate();

	afx_msg void		OnUpdateNewToDo(CCmdUI* pCmdUI);

	afx_msg int 		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void		OnNewToDo();
	afx_msg void		OnShowPopup();

	DECLARE_MESSAGE_MAP()
};

#endif
