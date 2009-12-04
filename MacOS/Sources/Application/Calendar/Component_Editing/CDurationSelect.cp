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

#include "CDurationSelect.h"

#include "CNumberEdit.h"

#include <LLittleArrows.h>
#include <LPopupButton.h>

// ---------------------------------------------------------------------------
//	CDurationSelect														  [public]
/**
	Default constructor */

CDurationSelect::CDurationSelect(LStream *inStream) :
	LView(inStream)
{
	mAllDay = false;
}


// ---------------------------------------------------------------------------
//	~CDurationSelect														  [public]
/**
	Destructor */

CDurationSelect::~CDurationSelect()
{
}

#pragma mark -

CDurationSelect* CDurationSelect::CreateInside(LView* parent)
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

	CDurationSelect* result = static_cast<CDurationSelect*>(UReanimator::CreateView(CDurationSelect::pane_ID, parent, defCommander));
	parent->ExpandSubPane(result, true, true);
	result->SetPaneID(parent->GetPaneID());
	result->SetVisible(true);
	
	return result;
}

void CDurationSelect::FinishCreateSelf()
{
	// Get UI items
	mDurationNumber = dynamic_cast<CNumberEdit*>(FindPaneByID(eDurationNumber_ID));
	mDurationNumberSpin = dynamic_cast<LLittleArrows*>(FindPaneByID(eDurationNumberSpin_ID));
	mDurationNumber->SetArrows(mDurationNumberSpin, 0, 1000, 0);
	mDurationPeriod = dynamic_cast<LPopupButton*>(FindPaneByID(eDurationPeriod_ID));

	mDurationTime = dynamic_cast<LView*>(FindPaneByID(eDurationTime_ID));
	mDurationHours = dynamic_cast<CNumberEdit*>(FindPaneByID(eDurationHours_ID));
	mDurationMinutes = dynamic_cast<CNumberEdit*>(FindPaneByID(eDurationMinutes_ID));
	mDurationSeconds = dynamic_cast<CNumberEdit*>(FindPaneByID(eDurationSeconds_ID));

	mDurationPeriod->AddListener(this);
}

// Respond to clicks in the icon buttons
void CDurationSelect::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
	case eDurationPeriod_ID:
		OnDurationPopup(mDurationPeriod->GetValue());
		break;
	}
}

void CDurationSelect::OnDurationPopup(UInt32 value)
{
	switch(value)
	{
	case eDurationPeriod_Days:
		mDurationTime->SetVisible(!mAllDay);
		break;
	case eDurationPeriod_Weeks:
	default:
		mDurationTime->SetVisible(false);
		break;
	}
}

void CDurationSelect::SetAllDay(bool all_day)
{
	mAllDay = all_day;
	mDurationTime->SetVisible(!all_day);
	
	// When going to all day, make sure duration is not zero
	if (all_day)
	{
		SInt32 number = mDurationNumber->GetNumberValue();
		if (number == 0)
			mDurationNumber->SetNumberValue(1);
	}
}

void CDurationSelect::SetDuration(const iCal::CICalendarDuration& du, bool all_day)
{
	SetAllDay(all_day);
	
	if (du.GetWeeks() > 0)
	{
		mDurationPeriod->SetValue(eDurationPeriod_Weeks);
		mDurationNumber->SetNumberValue(du.GetWeeks());
	}
	else
	{
		mDurationPeriod->SetValue(eDurationPeriod_Days);
		mDurationNumber->SetNumberValue(du.GetDays());
		mDurationHours->SetNumberValue(du.GetHours());
		mDurationMinutes->SetNumberValue(du.GetMinutes());
		mDurationSeconds->SetNumberValue(du.GetSeconds());
	}
}

void CDurationSelect::GetDuration(iCal::CICalendarDuration& du, bool all_day)
{
	int64_t seconds = 0;
	if (mDurationPeriod->GetValue() == eDurationPeriod_Weeks)
	{
		seconds = mDurationNumber->GetNumberValue() * 7LL * 24LL * 60LL * 60LL;
		du.SetDuration(seconds);
	}
	else
	{
		seconds = mDurationNumber->GetNumberValue() * 24LL * 60LL * 60LL;
		if (!all_day)
		{
			seconds += mDurationHours->GetNumberValue() * 60LL * 60LL;
			seconds += mDurationMinutes->GetNumberValue() * 60LL;
			seconds += mDurationSeconds->GetNumberValue();
		}
	}
	du.SetDuration(seconds);
}

void CDurationSelect::SetReadOnly(bool read_only)
{
	mDurationNumber->SetReadOnly(read_only);
	mDurationNumberSpin->SetEnabled(!read_only);
	mDurationPeriod->SetEnabled(!read_only);
	mDurationHours->SetReadOnly(read_only);
	mDurationMinutes->SetReadOnly(read_only);
	mDurationSeconds->SetReadOnly(read_only);
}
