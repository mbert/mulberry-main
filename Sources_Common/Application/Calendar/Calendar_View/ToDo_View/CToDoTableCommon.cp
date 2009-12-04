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

void CToDoTable::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case CToDoItem::eBroadcast_SelectToDo:
		// Select to do
		SelectToDo(reinterpret_cast<CToDoItem*>(param));
		break;

	default:
		CCalendarTableBase::ListenTo_Message(msg, param);
		break;
	}
}

void CToDoTable::ResetTable()
{
}

void CToDoTable::SelectToDo(const CToDoItem* todo)
{
	// Preview this one
	if (GetCalendarView() != NULL)
		GetCalendarView()->PreviewComponent((todo != NULL) ? todo->GetVToDo() : iCal::CICalendarComponentExpandedShared());
}

void CToDoTable::AddToDos(iCal::CICalendarExpandedComponents& vtodos)
{
	// Delete all to dos
	for(CToDoItemList::iterator iter = mToDos.begin(); iter != mToDos.end(); iter++)
	{
		delete *iter;
	}
	mToDos.clear();
	
	// Determine extra rows
	uint32_t extra = 0;
	if (vtodos.size() > 0)
	{
		extra++;
		iCal::CICalendarVToDo::ECompletedStatus previous_status = vtodos.front()->GetMaster<iCal::CICalendarVToDo>()->GetCompletionState();
		for(iCal::CICalendarExpandedComponents::iterator iter = vtodos.begin() + 1; iter != vtodos.end(); iter++)
		{
			if (previous_status != (*iter)->GetMaster<iCal::CICalendarVToDo>()->GetCompletionState())
			{
				previous_status = (*iter)->GetMaster<iCal::CICalendarVToDo>()->GetCompletionState();
				extra++;
			}
		}
	}
	else
		// One for empty row
		extra = 1;

	// Adjust existing rows
	if (mRows > vtodos.size() + extra)
		RemoveRows(mRows - vtodos.size() - extra, 1, false);
	else if (mRows < vtodos.size() + extra)
		InsertRows(vtodos.size() + extra - mRows, mRows, NULL);
	
	// Add each one
	if (vtodos.size() > 0)
	{
		TableIndexT row = 1;
		iCal::CICalendarVToDo::ECompletedStatus current_status = vtodos.front()->GetMaster<iCal::CICalendarVToDo>()->GetCompletionState();
		AddToDo(current_status, row++);
		for(iCal::CICalendarExpandedComponents::iterator iter = vtodos.begin(); iter != vtodos.end(); iter++, row++)
		{
			if (current_status != (*iter)->GetMaster<iCal::CICalendarVToDo>()->GetCompletionState())
			{
				current_status = (*iter)->GetMaster<iCal::CICalendarVToDo>()->GetCompletionState();
				AddToDo(current_status, row++);
			}
			AddToDo(*iter, row);
		}
	}
	else
	{
		AddEmptyToDo();
		
	}
}
