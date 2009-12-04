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

#include "CDateTimeZoneSelect.h"

#include "CChooseDateDialog.h"
#include "CTimezonePopup.h"

#include "CICalendarManager.h"

#include <LBevelButton.h>
#include <LClock.h>

// ---------------------------------------------------------------------------
//	CDateTimeZoneSelect														  [public]
/**
	Default constructor */

CDateTimeZoneSelect::CDateTimeZoneSelect(LStream *inStream) :
	LView(inStream)
{
	mAllDay = false;
}


// ---------------------------------------------------------------------------
//	~CDateTimeZoneSelect														  [public]
/**
	Destructor */

CDateTimeZoneSelect::~CDateTimeZoneSelect()
{
}

#pragma mark -

CDateTimeZoneSelect* CDateTimeZoneSelect::CreateInside(LView* parent)
{
	// Find super commander in view chain
	LView* cparent = parent;
	while((cparent != NULL) && (dynamic_cast<LDialogBox*>(cparent) == NULL))
		cparent = cparent->GetSuperView();
	LDialogBox* dlog = dynamic_cast<LDialogBox*>(cparent);

	// Get the tab group
	LCommander* defCommander = NULL;
	if (dlog)
		dlog->GetSubCommanders().FetchItemAt(1, defCommander);

	CDateTimeZoneSelect* result = static_cast<CDateTimeZoneSelect*>(UReanimator::CreateView(CDateTimeZoneSelect::pane_ID, parent, defCommander));
	parent->ExpandSubPane(result, true, true);
	result->SetPaneID(parent->GetPaneID());
	result->SetVisible(true);
	
	return result;
}

void CDateTimeZoneSelect::FinishCreateSelf()
{
	// Get UI items
	mDate = dynamic_cast<LClock*>(FindPaneByID(eDate_ID));
	mDateBtn = dynamic_cast<LBevelButton*>(FindPaneByID(eDateBtn_ID));
	mTime = dynamic_cast<LClock*>(FindPaneByID(eTime_ID));
	mZone = dynamic_cast<CTimezonePopup*>(FindPaneByID(eZone_ID));
	
	// Find super commander in view chain
	LView* cparent = GetSuperView();
	while((cparent != NULL) && (dynamic_cast<LDialogBox*>(cparent) == NULL))
		cparent = cparent->GetSuperView();
	LDialogBox* dlog = dynamic_cast<LDialogBox*>(cparent);

	mClockHandler.InstallMLTEEventHandler(GetWindowEventTarget(dlog->GetMacWindow()));

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

// Respond to clicks in the icon buttons
void CDateTimeZoneSelect::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eDateBtn_ID:
		OnDateBtn();
		break;
	}
}

void CDateTimeZoneSelect::SetAllDay(bool all_day)
{
	mAllDay = all_day;
	mTime->SetVisible(!all_day);
	mZone->SetVisible(!all_day);
}

void CDateTimeZoneSelect::SetDateTimeZone(const iCal::CICalendarDateTime& dt, bool all_day)
{
	SetAllDay(all_day);
	
	LongDateRec dtr;
	::memset(&dtr, 0, sizeof(LongDateRec));
	dtr.ld.year = dt.GetYear();
	dtr.ld.month = dt.GetMonth();
	dtr.ld.day = dt.GetDay();
	dtr.ld.hour = dt.GetHours();
	dtr.ld.minute = dt.GetMinutes();
	dtr.ld.second = dt.GetSeconds();
	
	mDate->SetLongDate(dtr);
	mTime->SetLongDate(dtr);
	
	if (all_day)
		mZone->SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	else
		mZone->SetTimezone(dt.GetTimezone());
}

void CDateTimeZoneSelect::GetDateTimeZone(iCal::CICalendarDateTime& dt, bool all_day)
{
	dt.SetDateOnly(all_day);

	// Get date
	{
		LongDateRec dtr;
		mDate->GetLongDate(dtr);
		
		dt.SetYear(dtr.ld.year);
		dt.SetMonth(dtr.ld.month);
		dt.SetDay(dtr.ld.day);
	}
	
	// Get time & zone if required
	if (!all_day)
	{
		LongDateRec dtr;
		mTime->GetLongDate(dtr);
		
		dt.SetHours(dtr.ld.hour);
		dt.SetMinutes(dtr.ld.minute);
		dt.SetSeconds(dtr.ld.second);
		
		// Get timezone
		mZone->GetTimezone(dt.GetTimezone());
	}
}

void CDateTimeZoneSelect::SetReadOnly(bool read_only)
{
	mDate->SetEnabled(!read_only);
	mDateBtn->SetEnabled(!read_only);
	mTime->SetEnabled(!read_only);
	mZone->SetEnabled(!read_only);
}

void CDateTimeZoneSelect::OnDateBtn()
{
	// Get current date from controls
	iCal::CICalendarDateTime dt;
	GetDateTimeZone(dt, mAllDay);
	
	// Now do date chooser dialog
	if (CChooseDateDialog::PoseDialog(dt))
	{
		// Set the new date/time
		SetDateTimeZone(dt, mAllDay);
	}
}