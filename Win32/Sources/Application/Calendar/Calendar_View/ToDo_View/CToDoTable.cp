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
#include "CToDoItem.h"

#include <WIN_LTableMonoGeometry.h>

BEGIN_MESSAGE_MAP(CToDoTable, CCalendarTableBase)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

// ---------------------------------------------------------------------------
//	CToDoTable														  [public]
/**
	Default constructor */

CToDoTable::CToDoTable()
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

int CToDoTable::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CCalendarTableBase::OnCreate(lpCreateStruct) == -1)
		return -1;

	// Set initial size for control repositioner
	InitResize(lpCreateStruct->cx, lpCreateStruct->cy);

	int width = lpCreateStruct->cx;
	int height = lpCreateStruct->cy;

	// Create columns and adjust flag rect
	InsertCols(1, 0, NULL);
	SetColWidth(width - 16, 1, 1);

	return 0;
}

// Resize columns
void CToDoTable::OnSize(UINT nType, int cx, int cy)
{
	CCalendarTableBase::OnSize(nType, cx, cy);

	// Resize child windows
	SizeChanged(cx, cy);

	// Adjust for vert scroll bar
	if (!(GetStyle() & WS_VSCROLL))
		cx -= 16;

	// Set width/height of cells
	SetColWidth(cx, 1, 1);
}

void CToDoTable::AddToDo(iCal::CICalendarComponentExpandedShared& vtodo, TableIndexT row)
{
	// Get cell frame
	CRect cellFrame;
	STableCell inCell(row, 1);
	GetLocalCellRectAlways(inCell, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame, this);
	todo->Add_Listener(this);
	todo->SetDetails(vtodo, this);
	todo->ShowWindow(SW_SHOW);
	
	mToDos.push_back(todo);
}

void CToDoTable::AddToDo(iCal::CICalendarVToDo::ECompletedStatus& vtodo_state, TableIndexT row)
{
	// Get cell frame
	CRect cellFrame;
	STableCell inCell(row, 1);
	GetLocalCellRectAlways(inCell, cellFrame);

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
	todo->ShowWindow(SW_SHOW);
	
	mToDos.push_back(todo);
}

void CToDoTable::AddEmptyToDo()
{
	// Get cell frame
	CRect cellFrame;
	STableCell inCell(1, 1);
	GetLocalCellRectAlways(inCell, cellFrame);

	// Create new month event
	CToDoItem* todo = CToDoItem::Create(this, cellFrame, this);
	todo->Add_Listener(this);
	todo->SetDetails(CToDoItem::eEmpty);
	todo->ShowWindow(SW_SHOW);
	
	mToDos.push_back(todo);
}
