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
class CSimpleTitleTable;

class CAttendeeTable : public CTableDragAndDrop
{

public:
						CAttendeeTable();
	virtual				~CAttendeeTable();

	virtual BOOL 		SubclassDlgItem(UINT nID, CWnd* pParent);

	virtual void		SelectionChanged();

			void		ResetTable(const iCal::CICalendarPropertyList* items);

			void		SetTitles(CSimpleTitleTable* titles)
			{
				mTitles = titles;
			}

protected:
	const iCal::CICalendarPropertyList*	mAttendees;
	CSimpleTitleTable*					mTitles;

			void		InitTable();

	virtual void		LDblClickCell(
								const STableCell&		inCell,
								UINT nFlags);			// Click in the cell
	virtual void		DrawCell(CDC* pDC,
								const STableCell&	inCell,
								const CRect&		inLocalRect);					// Draw the string

	// message handlers
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()
};

#endif
