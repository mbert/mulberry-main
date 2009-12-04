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

#ifndef H_CToDoTable
#define H_CToDoTable

#include "CCalendarTableBase.h"

#include "CICalendarUtils.h"
#include "CICalendarVToDo.h"
#include "CICalendarComponentExpanded.h"

class CToDoItem;
typedef std::vector<CToDoItem*> CToDoItemList;

// ===========================================================================
//	CToDoTable

class	CToDoTable : public CCalendarTableBase
{
public:

			CToDoTable(JXScrollbarSet* scrollbarSet,
					 JXContainer* enclosure,
					 const HSizingOption hSizing,
					 const VSizingOption vSizing,
					 const JCoordinate x, const JCoordinate y,
					 const JCoordinate w, const JCoordinate h);
	virtual	~CToDoTable();
	
	virtual void		OnCreate();

	virtual void		ListenTo_Message(long msg, void* param);

	void				ResetTable();
	virtual void		ApertureResized(const JCoordinate dw, const JCoordinate dh);

	void				AddToDos(iCal::CICalendarExpandedComponents& vtodos);
	void				AddToDo(iCal::CICalendarComponentExpandedShared& vtodo, TableIndexT row);
	void				AddToDo(iCal::CICalendarVToDo::ECompletedStatus& vtodo_state, TableIndexT row);
	void				AddEmptyToDo();

protected:
	CToDoItemList		mToDos;

			void		SelectToDo(const CToDoItem* todo);


private:
};

#endif
