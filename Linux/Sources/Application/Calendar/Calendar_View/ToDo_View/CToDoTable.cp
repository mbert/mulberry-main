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

#include "CToDoTable.h"

#include "CToDoItem.h"

#include <UNX_LTableMonoGeometry.h>

// ---------------------------------------------------------------------------
//	CToDoTable														  [public]
/**
	Default constructor */

CToDoTable::CToDoTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h) :
	CCalendarTableBase(scrollbarSet, enclosure, hSizing, vSizing, x, y, w, h)
{
	mTableGeometry = new LTableMonoGeometry(this, 32, 32);
}


// ---------------------------------------------------------------------------
//	~CToDoTable														  [public]
/**
	Destructor */

CToDoTable::~CToDoTable()
{
	// Delete all to dos
	for(CToDoItemList::iterator iter = mToDos.begin(); iter != mToDos.end(); iter++)
	{
		delete *iter;
	}
	mToDos.clear();
}

#pragma mark -

void
CToDoTable::OnCreate()
{
	// Call inherited
	CCalendarTableBase::OnCreate();

	InsertCols(1, 0, NULL);
	
	ApertureResized(0, 0);
	
	// Reset row height after font resets it
	SetRowHeight(32, 1, 1);
}

void CToDoTable::ApertureResized(const JCoordinate dw, const JCoordinate dh)
{
	// Do inherited
	CCalendarTableBase::ApertureResized(dw, dh);

	// Single column always the width of the super frame
	SetColWidth(GetApertureWidth(), 1, 1);
}

void CToDoTable::AddToDo(iCal::CICalendarComponentExpandedShared& vtodo, TableIndexT row)
{
	// Get cell frame
	JRect cellFrame;
	STableCell inCell(row, 1);
	GetLocalCellRectAlways(inCell, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame);
	todo->Add_Listener(this);
	todo->SetDetails(vtodo, this);
	todo->SetVisible(kTrue);
	
	mToDos.push_back(todo);
}

void CToDoTable::AddToDo(iCal::CICalendarVToDo::ECompletedStatus& vtodo_state, TableIndexT row)
{
	// Get cell frame
	JRect cellFrame;
	STableCell inCell(row, 1);
	GetLocalCellRectAlways(inCell, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame);
	todo->Add_Listener(this);
	switch(vtodo_state)
	{
	case iCal::CICalendarVToDo::eOverdue:
		todo->SetDetails(CToDoItem::eOverdue);
		break;
	case iCal::CICalendarVToDo::eDueNow:
	default:
		todo->SetDetails(CToDoItem::eDueNow);
		break;
	case iCal::CICalendarVToDo::eDueLater:
		todo->SetDetails(CToDoItem::eDueLater);
		break;
	case iCal::CICalendarVToDo::eDone:
		todo->SetDetails(CToDoItem::eDone);
		break;
	case iCal::CICalendarVToDo::eCancelled:
		todo->SetDetails(CToDoItem::eCancelled);
		break;
	}
	todo->SetVisible(kTrue);
	
	mToDos.push_back(todo);
}

void CToDoTable::AddEmptyToDo()
{
	// Get cell frame
	JRect cellFrame;
	STableCell inCell(1, 1);
	GetLocalCellRectAlways(inCell, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame);
	todo->Add_Listener(this);
	todo->SetDetails(CToDoItem::eEmpty);
	todo->SetVisible(kTrue);
	
	mToDos.push_back(todo);
}
