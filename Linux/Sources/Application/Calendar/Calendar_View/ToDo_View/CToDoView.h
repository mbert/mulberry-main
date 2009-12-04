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

#include "CCalendarViewBase.h"

class CToDoTable;

#include "HPopupMenu.h"

// ===========================================================================
//	CToDoView

class	CToDoView : public CCalendarViewBase
{
	friend class CCalendarView;

public:

			CToDoView(JXContainer* enclosure,
					const HSizingOption hSizing, const VSizingOption vSizing,
					const JCoordinate x, const JCoordinate y,
					const JCoordinate w, const JCoordinate h);
	virtual	~CToDoView();

	virtual void ListenTo_Message(long msg, void* param);
	virtual void Receive(JBroadcaster* sender, const Message& message);

	virtual bool ObeyCommand(unsigned long cmd, SMenuCommandChoice* menu = NULL);
	virtual void UpdateCommand(unsigned long cmd, CCmdUI* cmdui);

	virtual iCal::CICalendarDateTime GetSelectedDate() const;

	virtual cdstring GetTitle() const;

	virtual CCalendarTableBase* GetTable() const;

	virtual void ResetFont(const SFontInfo& finfo);			// Reset text traits

protected:

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
// begin JXLayout1

    HPopupMenu* mShowPopup;

// end JXLayout1
	
	CToDoTable*			mTable;
	

	virtual void		OnCreate();
	
	virtual void		ResetDate();

			void		OnUpdateNewToDo(CCmdUI* pCmdUI);

			void		OnNewToDo();
			void		OnShowPopup();
};

#endif
