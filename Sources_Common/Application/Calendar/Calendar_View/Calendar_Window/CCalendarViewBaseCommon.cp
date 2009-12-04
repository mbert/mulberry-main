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

#include "CCalendarViewBase.h"

#include "CCalendarView.h"
#include "CChooseDateDialog.h"
#include "CEventPreview.h"

void CCalendarViewBase::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case iCal::CICalendar::eBroadcast_Changed:
		CalendarChanged(static_cast<iCal::CICalendar*>(param));
		break;
	case iCal::CICalendar::eBroadcast_Closed:
		CalendarClosed(static_cast<iCal::CICalendar*>(param));
		break;
	default:;
	}
}

void CCalendarViewBase::PreviewComponent(const iCal::CICalendarComponentExpandedShared& comp)
{
	if (mCalendarView->GetUsePreview() && (mCalendarView->GetPreview() != NULL))
		mCalendarView->GetPreview()->SetComponent(comp);
}

bool CCalendarViewBase::IsSingleCalendar() const
{
	return mCalendarView->IsSingleCalendar();
}

iCal::CICalendar* CCalendarViewBase::GetCalendar() const
{
	return mCalendarView->GetCalendar();
}
	
void CCalendarViewBase::CalendarChanged(iCal::CICalendar* cal)
{
	// Just reset the current view
	ResetDate();
}

void CCalendarViewBase::CalendarClosed(iCal::CICalendar* cal)
{
	if (IsSingleCalendar())
	{
		// Clear it out
		mCalendarView->SetCalendar(NULL);
	}
	else
		// Just reset the current view
		ResetDate();
}

void CCalendarViewBase::OnPickDate()
{
	iCal::CICalendarDateTime dt(mDate);
	if (CChooseDateDialog::PoseDialog(dt))
		SetDate(dt);
}
