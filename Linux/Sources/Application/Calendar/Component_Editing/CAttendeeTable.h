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


//	CAttendeeTable.h

#ifndef __CAttendeeTable__MULBERRY__
#define __CAttendeeTable__MULBERRY__

#include "CTableDragAndDrop.h"

#include "CICalendarComponentRecur.h"

// Classes

class CAttendeeTable : public CTableDragAndDrop
{

public:
	enum
	{
		eBroadcast_SelectionChange,
		eBroadcast_DblClk
	};

			CAttendeeTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
	virtual	~CAttendeeTable();

	virtual void		OnCreate();
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

	virtual	void		GetTooltipText(cdstring& txt, const STableCell &inCell);				// Get text for current tooltip cell

	virtual void		SelectionChanged();

			void		ResetTable(const iCal::CICalendarPropertyList* items, const iCal::CICalendarProperty* organizer);

protected:
	const iCal::CICalendarPropertyList*	mAttendees;
	const iCal::CICalendarProperty*	mOrganizer;

	virtual void		LDblClickCell(const STableCell& inCell, const JXKeyModifiers& mods);
	virtual void		DrawCell(JPainter* pDC, const STableCell& inCell, const JRect& inLocalRect);

			void		InitTable();
};

#endif
