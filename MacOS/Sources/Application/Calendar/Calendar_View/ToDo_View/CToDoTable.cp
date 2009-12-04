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

#include "CCalendarViewBase.h"
#include "CGUtils.h"
#include "CToDoItem.h"

#include <LTableMonoGeometry.h>

// ---------------------------------------------------------------------------
//	CToDoTable														  [public]
/**
	Default constructor */

CToDoTable::CToDoTable(LStream *inStream) :
	CCalendarTableBase(inStream)
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
CToDoTable::FinishCreateSelf()
{
	// Call inherited
	CCalendarTableBase::FinishCreateSelf();

	InsertCols(1, 0, NULL);
	
	AdaptToNewSurroundings();
}

void CToDoTable::AdaptToNewSurroundings()
{
	// Do inherited
	CCalendarTableBase::AdaptToNewSurroundings();

	// Single column always the width of the super frame
	SDimension16 frame;
	GetFrameSize(frame);
	SetColWidth(frame.width, 1, 1);
}

void CToDoTable::AddToDo(iCal::CICalendarComponentExpandedShared& vtodo, TableIndexT row)
{
	// Get cell frame
	STableCell cell(row, 1);
	Rect cellFrameQD;
	HIRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrameQD);
	CGUtils::QDToHIRect(cellFrameQD, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame, this);
	todo->Add_Listener(this);
	todo->SetDetails(vtodo, this);
	todo->SetVisible(true);
	
	mToDos.push_back(todo);
}

void CToDoTable::AddToDo(iCal::CICalendarVToDo::ECompletedStatus& vtodo_state, TableIndexT row)
{
	// Get cell frame
	STableCell cell(row, 1);
	Rect cellFrameQD;
	HIRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrameQD);
	CGUtils::QDToHIRect(cellFrameQD, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame, this);
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
	todo->SetVisible(true);
	
	mToDos.push_back(todo);
}

void CToDoTable::AddEmptyToDo()
{
	// Get cell frame
	STableCell cell(1, 1);
	Rect cellFrameQD;
	HIRect cellFrame;
	GetLocalCellRectAlways(cell, cellFrameQD);
	CGUtils::QDToHIRect(cellFrameQD, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame, this);
	todo->Add_Listener(this);
	todo->SetDetails(CToDoItem::eEmpty);
	todo->SetVisible(true);
	
	mToDos.push_back(todo);
}
