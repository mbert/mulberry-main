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

#include "CToDoItem.h"

#include "CNewToDoDialog.h"
#include "CCalendarTableBase.h"
#include "CCalendarViewBase.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarComponentExpanded.h"

#include <strstream>

void CToDoItem::SelectToDo(bool select, bool silent)
{
	// Only if real to do
	if (mType != eToDo)
		return;

	if (mIsSelected != select)
	{
		mIsSelected = select;

		// Force redraw
		FRAMEWORK_REFRESH_WINDOW(this);
		
		Broadcast_Message(eBroadcast_SelectToDo, mIsSelected ? this : NULL);
	}
}

void CToDoItem::OnEditToDo()
{
	// Only if real to do
	if (mType != eToDo)
		return;

#if 0
	// IMPORTANT the VTODO object may get deleted if its is a recurrence item,
	// so we must cache the master VTODO here and use this
	iCal::CICalendarVToDo* master = mVToDo->GetTrueMaster<iCal::CICalendarVToDo>();

	CNewToDoDialog::StartEdit(*master);
#else
	CNewToDoDialog::StartEdit(*mVToDo->GetMaster<iCal::CICalendarVToDo>(), mVToDo.get());
#endif
}

void CToDoItem::OnDuplicateToDo()
{
	// Only if real to do
	if (mType != eToDo)
		return;

	// Duplicate it
	CNewToDoDialog::StartDuplicate(*mVToDo->GetTrueMaster<iCal::CICalendarVToDo>());
}

void CToDoItem::OnDeleteToDo()
{
	// Only if real to do
	if (mType != eToDo)
		return;

	iCal::CICalendar* calendar = iCal::CICalendar::GetICalendar(mVToDo->GetTrueMaster<iCal::CICalendarVToDo>()->GetCalendar());

	// Add to calendar and release auto-ptr
	if (calendar)
	{
		calendar->RemoveVToDo(mVToDo->GetTrueMaster<iCal::CICalendarVToDo>());
		mVToDo.reset();
	}

	// Do direct call to ListenTo_Message as we cannot broadcast when this object could be deleted
	dynamic_cast<CListener*>(mTable->GetCalendarView())->ListenTo_Message(eBroadcast_EditToDo, NULL);
}

void CToDoItem::SetupTagText()
{
	// Setup a help tag;
	std::ostrstream ostr;
	ostr << rsrc::GetString("ToDoTip::Summary") << mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetSummary();

	if (!mVToDo->GetMaster<iCal::CICalendarVToDo>()->HasEnd())
	{
		ostr << std::endl << rsrc::GetString("ToDoTip::No due date");
	}
	else
	{
		ostr << std::endl << rsrc::GetString("ToDoTip::Due on") << mVToDo->GetInstanceEnd().GetAdjustedTime().GetLocaleDate(iCal::CICalendarDateTime::eAbbrevDate);
	}

	// Always add calendar name
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar( mVToDo->GetMaster<iCal::CICalendarVToDo>()->GetCalendar());
	if (cal != NULL)
		ostr << std::endl << std::endl << rsrc::GetString("ToDoTip::Calendar") << cal->GetName();

	ostr << std::ends;

#if __dest_os == __mac_os || __dest_os == __mac_os_x

	SetTagText(ostr.str());

#elif __dest_os == __win32_os

	mTooltipText = ostr.str();

#elif __dest_os == __linux_os

	SetHint(ostr.str());

#endif

	ostr.freeze(false);
}
