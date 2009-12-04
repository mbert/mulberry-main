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

#include "CTableDrag.h"

#include "CICalendarComponentRecur.h"

// Classes

class CAttendeeTable : public CTableDrag,
						public LBroadcaster
{

public:
	enum { class_ID = 'CAtb' };
	
						CAttendeeTable(LStream *inStream);
	virtual				~CAttendeeTable();

	virtual void		FinishCreateSelf(void);				// Get details of sub-panes

	virtual void		DrawCell(const STableCell &inCell,
									const Rect &inLocalRect);

	virtual void		ClickCell(
								const STableCell		&inCell,
								const SMouseDownEvent	&inMouseDown);

	virtual	void		GetTooltipText(cdstring& txt, const STableCell &inCell);				// Get text for current tooltip cell

	virtual void		SelectionChanged();

	void		ResetTable(const iCal::CICalendarPropertyList* items, const iCal::CICalendarProperty* organizer);

protected:
	const iCal::CICalendarPropertyList*	mAttendees;
	const iCal::CICalendarProperty*	mOrganizer;

	virtual void		AdaptToNewSurroundings(void);						// Adjust column widths

			void		InitTable();
};

#endif
