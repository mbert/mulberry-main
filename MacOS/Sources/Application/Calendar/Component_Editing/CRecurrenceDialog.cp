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

#include "CRecurrenceDialog.h"

#include "CBalloonDialog.h"
#include "CDateTimeZoneSelect.h"
#include "CMulberryApp.h"
#include "CNumberEdit.h"

#include "CICalendarRecurrence.h"

#include <LBevelButton.h>
#include <LLittleArrows.h>
#include <LPopupButton.h>
#include <LRadioButton.h>

// ---------------------------------------------------------------------------
//	CRecurrenceDialog														  [public]
/**
	Default constructor */

CRecurrenceDialog::CRecurrenceDialog(LStream *inStream)
		: LDialogBox(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CRecurrenceDialog														  [public]
/**
	Destructor */

CRecurrenceDialog::~CRecurrenceDialog()
{
}

#pragma mark -

void CRecurrenceDialog::FinishCreateSelf()
{
	LDialogBox::FinishCreateSelf();

	// Get UI items
	mOccursInterval = dynamic_cast<CNumberEdit*>(FindPaneByID(eOccursInterval_ID));
	mOccursIntervalSpin = dynamic_cast<LLittleArrows*>(FindPaneByID(eOccursIntervalSpin_ID));
	mOccursInterval->SetArrows(mOccursIntervalSpin, 1, 1000, 0);
	mOccursFreq = dynamic_cast<LPopupButton*>(FindPaneByID(eOccursFreq_ID));

	mOccursForEver = dynamic_cast<LRadioButton*>(FindPaneByID(eOccursForEver_ID));
	mOccursCount = dynamic_cast<LRadioButton*>(FindPaneByID(eOccursCount_ID));
	mOccursUntil = dynamic_cast<LRadioButton*>(FindPaneByID(eOccursUntil_ID));

	mOccursCounter = dynamic_cast<CNumberEdit*>(FindPaneByID(eOccursCounter_ID));
	mOccursCounterSpin = dynamic_cast<LLittleArrows*>(FindPaneByID(eOccursCounterSpin_ID));
	mOccursCounter->SetArrows(mOccursCounterSpin, 1, 1000, 0);

	mOccursDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eOccursDateTimeZone_ID)));

	for(uint32_t i = 0; i < 12; i++)
		mByMonth[i] = dynamic_cast<LBevelButton*>(FindPaneByID(eByMonth01_ID + i));

	for(uint32_t i = 0; i < 31; i++)
		mByMonthDay[i] = dynamic_cast<LBevelButton*>(FindPaneByID(eByMonthDay01_ID + i));

	for(uint32_t i = 0; i < 7; i++)
		mByMonthDayLast[i] = dynamic_cast<LBevelButton*>(FindPaneByID(eByMonthDayLast01_ID + i));

	for(uint32_t i = 0; i < 7; i++)
		mByDay[i] = dynamic_cast<LBevelButton*>(FindPaneByID(eByDay01_ID + i));
	mByDayView = dynamic_cast<LView*>(FindPaneByID(eByDayView_ID));
	mByDayPopup = dynamic_cast<LPopupButton*>(FindPaneByID(eByDayPopup));

	mBySetPosPopup = dynamic_cast<LPopupButton*>(FindPaneByID(eBySetPosPopup_ID));

	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
	
	// Init controls
	DoOccursFreq(eOccurs_Yearly);
	DoOccursGroup(eOccurs_ForEver);

	// Focus on summary
}

// Handle controls
void CRecurrenceDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case eOccursFreq_ID:
		// Terminate dialog with the date
		DoOccursFreq(mOccursFreq->GetValue());
		break;

	case eOccursForEver_ID:
		// Terminate dialog with the date
		DoOccursGroup(eOccurs_ForEver);
		break;
	case eOccursCount_ID:
		// Terminate dialog with the date
		DoOccursGroup(eOccurs_Count);
		break;
	case eOccursUntil_ID:
		// Terminate dialog with the date
		DoOccursGroup(eOccurs_Until);
		break;

	default:
		LDialogBox::ListenToMessage(inMessage, ioParam);
		break;
	}
}

void CRecurrenceDialog::DoOccursFreq(UInt32 value)
{
	// Hide the relative day in month controls if not Yearly or Monthly
	switch(value)
	{
	case eOccurs_Yearly:
	case eOccurs_Monthly:
		mByDayView->SetVisible(true);
		break;
	default:
		mByDayView->SetVisible(false);
		break;
	}
}

void CRecurrenceDialog::DoOccursGroup(UInt32 value)
{
	mOccursCounter->SetEnabled(value == eOccurs_Count);
	mOccursCounterSpin->SetEnabled(value == eOccurs_Count);

	mOccursDateTimeZone->SetEnabled(value == eOccurs_Until);
}

void CRecurrenceDialog::SetRecurrence(const iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day)
{
	static const int cFreqValueToPopup[] =
	{
		CRecurrenceDialog::eOccurs_Secondly, CRecurrenceDialog::eOccurs_Minutely, CRecurrenceDialog::eOccurs_Hourly,
		CRecurrenceDialog::eOccurs_Daily, CRecurrenceDialog::eOccurs_Weekly, CRecurrenceDialog::eOccurs_Monthly, CRecurrenceDialog::eOccurs_Yearly
	};

	// Set frequency
	mOccursFreq->SetValue(cFreqValueToPopup[recur.GetFreq()]);
	
	// Set interval
	mOccursInterval->SetNumberValue(recur.GetInterval());
	
	// Set count
	if (recur.GetUseCount())
	{
		mOccursCount->SetValue(1);
		mOccursCounter->SetNumberValue(recur.GetCount());
	}
	else if (recur.GetUseUntil())
	{
		mOccursUntil->SetValue(1);
		
		// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time
		
		// Adjust UNTIL to new timezone
		iCal::CICalendarDateTime until(recur.GetUntil());
		until.AdjustTimezone(tzid);

		mOccursDateTimeZone->SetDateTimeZone(until, all_day);
	}
	else
		mOccursForEver->SetValue(1);
	
	// Set BYxxx items
	SetByMonth(recur);
	SetByMonthDay(recur);
	SetByDay(recur);
	SetBySetPos(recur);
}

void CRecurrenceDialog::SetByMonth(const iCal::CICalendarRecurrence& recur)
{
	// Unset all buttons first
	for(uint32_t i = 0; i < 12; i++)
		mByMonth[i]->SetValue(0);

	const std::vector<int32_t>& by_month = recur.GetByMonth();
	for(std::vector<int32_t>::const_iterator iter = by_month.begin(); iter != by_month.end(); iter++)
	{
		if ((*iter >= 1) && (*iter <= 12))
			mByMonth[*iter - 1]->SetValue(1);
	}
}

void CRecurrenceDialog::SetByMonthDay(const iCal::CICalendarRecurrence& recur)
{
	// Unset all buttons first
	for(uint32_t i = 0; i < 31; i++)
		mByMonthDay[i]->SetValue(0);
	for(uint32_t i = 0; i < 7; i++)
		mByMonthDayLast[i]->SetValue(0);

	const std::vector<int32_t>& by_month_day = recur.GetByMonthDay();
	for(std::vector<int32_t>::const_iterator iter = by_month_day.begin(); iter != by_month_day.end(); iter++)
	{
		// NB Cannot handle all negative values, or zero
		if ((*iter >= 1) && (*iter <= 31))
			mByMonthDay[*iter - 1]->SetValue(1);
		else if ((*iter >= -7) && (*iter <= -1))
			mByMonthDayLast[-(*iter) - 1]->SetValue(1);
	}
}

void CRecurrenceDialog::SetByDay(const iCal::CICalendarRecurrence& recur)
{
	static const int32_t cByDayNumToPopup[] =
	{
		eByDay_2ndLast, eByDay_1stLast, eByDay_Any, eByDay_1st, eByDay_2nd, eByDay_3rd, eByDay_4th
	};

	// Unset all buttons first
	for(uint32_t i = 0; i < 7; i++)
		mByDay[i]->SetValue(0);

	// Set the day buttons
	const std::vector<iCal::CICalendarRecurrence::CWeekDayNum>& by_day = recur.GetByDay();
	for(std::vector<iCal::CICalendarRecurrence::CWeekDayNum>::const_iterator iter = by_day.begin(); iter != by_day.end(); iter++)
	{
		if (((*iter).second >= 0) && ((*iter).second <= 6))
			mByDay[(*iter).second]->SetValue(1);
	}
	
	// Set the popup
	if (by_day.size())
	{
		// Validate range before setting
		int32_t number = by_day.front().first;
		if ((number >= -2) && (number <= 4))
		{
			mByDayPopup->SetValue(cByDayNumToPopup[number + 2]);
		}
		else
			mByDayPopup->SetValue(eByDay_Any);
	}
}

void CRecurrenceDialog::SetBySetPos(const iCal::CICalendarRecurrence& recur)
{
	// Can only have empty BYSETPOS or single +1 or -1
	mBySetPosPopup->SetValue(eBySetPos_All);
	
	// Set the day buttons
	const std::vector<int32_t>& by_setpos = recur.GetBySetPos();
	if ((by_setpos.size() == 1) && (by_setpos[0] == 1))
		mBySetPosPopup->SetValue(eBySetPos_1st);
	else if ((by_setpos.size() == 1) && (by_setpos[0] == -1))
		mBySetPosPopup->SetValue(eBySetPos_Last);
}

static const iCal::ERecurrence_FREQ cFreqPopupToValue[] = 
{
	iCal::eRecurrence_YEARLY, iCal::eRecurrence_MONTHLY, iCal::eRecurrence_WEEKLY, iCal::eRecurrence_DAILY,
	iCal::eRecurrence_HOURLY, iCal::eRecurrence_MINUTELY, iCal::eRecurrence_SECONDLY
};

void CRecurrenceDialog::GetRecurrence(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day)
{
	// Create a temp item first (this will have all fields cleared)
	iCal::CICalendarRecurrence temp;
	
	// Set frequency
	temp.SetFreq(cFreqPopupToValue[mOccursFreq->GetValue() - 1]);
	
	// Set interval
	temp.SetInterval(mOccursInterval->GetNumberValue());
	
	// Determine end
	if (mOccursForEver->GetValue() == 1)
	{
		// Nothing to add
	}
	else if (mOccursCount->GetValue() == 1)
	{
		temp.SetUseCount(true);
		temp.SetCount(mOccursCounter->GetNumberValue());
	}
	else if (mOccursUntil->GetValue() == 1)
	{
		// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time,
		// so we must convert from dialog tzid to UTC
		
		// Get value from dialog
		iCal::CICalendarDateTime until;
		mOccursDateTimeZone->GetDateTimeZone(until, all_day);
		
		// Adjust to UTC
		until.AdjustToUTC();

		temp.SetUseUntil(true);
		temp.SetUntil(until);
	}
	
	// Get BYxxx items
	GetByMonth(temp);
	GetByMonthDay(temp);
	GetByDay(temp);
	GetBySetPos(temp);

	// Now set the actual recurrence
	recur = temp;
}

void CRecurrenceDialog::GetByMonth(iCal::CICalendarRecurrence& recur)
{
	// Get each button
	std::vector<int32_t> by_month;
	for(uint32_t i = 0; i < 12; i++)
	{
		if (mByMonth[i]->GetValue() == 1)
			by_month.push_back(i + 1);
	}
	recur.SetByMonth(by_month);
}

void CRecurrenceDialog::GetByMonthDay(iCal::CICalendarRecurrence& recur)
{
	// Get each button
	std::vector<int32_t> by_month_day;
	for(uint32_t i = 0; i < 31; i++)
	{
		if (mByMonthDay[i]->GetValue() == 1)
			by_month_day.push_back(i + 1);
	}
	for(uint32_t i = 0; i < 7; i++)
	{
		if (mByMonthDayLast[i]->GetValue() == 1)
			by_month_day.push_back(-(i + 1));
	}
	recur.SetByMonthDay(by_month_day);
}

static const int32_t cPopupToByDayNum[] =
{
	0, 0, 1, 2, 3, 4, -2, -1
};

void CRecurrenceDialog::GetByDay(iCal::CICalendarRecurrence& recur)
{
	// Get the popup number
	int32_t number = cPopupToByDayNum[mByDayPopup->GetValue()];
	
	// Disable number for some frequencies
	switch(mOccursFreq->GetValue())
	{
	case eOccurs_Yearly:
	case eOccurs_Monthly:
		break;
	default:
		number = 0;
		break;
	}

	// Get each button
	std::vector<iCal::CICalendarRecurrence::CWeekDayNum> by_day;
	for(uint32_t i = 0; i < 7; i++)
	{
		if (mByDay[i]->GetValue() == 1)
			by_day.push_back(iCal::CICalendarRecurrence::CWeekDayNum(number, static_cast<iCal::CICalendarDateTime::EDayOfWeek>(i)));
	}
	recur.SetByDay(by_day);

}

void CRecurrenceDialog::GetBySetPos(iCal::CICalendarRecurrence& recur)
{
	// Can only have empty BYSETPOS or single +1 or -1
	std::vector<int32_t> by_setpos;
	switch(mBySetPosPopup->GetValue())
	{
	case eBySetPos_All:
	default:
		// Leave it empty
		break;
	case eBySetPos_1st:
		by_setpos.push_back(1);
		break;
	case eBySetPos_Last:
		by_setpos.push_back(-1);
		break;
	}
	recur.SetBySetPos(by_setpos);
}

bool CRecurrenceDialog::PoseDialog(iCal::CICalendarRecurrence& recur, const iCal::CICalendarTimezone& tzid, bool all_day)
{
	// Create the dialog
	CBalloonDialog theHandler(pane_ID, CMulberryApp::sApp);

	CRecurrenceDialog* dlog = static_cast<CRecurrenceDialog*>(theHandler.GetDialog());
	dlog->SetRecurrence(recur, tzid, all_day);

	// Let DialogHandler process events
	while (true)
	{
		MessageT hitMessage = theHandler.DoDialog();

		if (hitMessage == msg_OK)
		{
			dlog->GetRecurrence(recur, tzid, all_day);
			return true;
		}
		else if (hitMessage == msg_Cancel)
		{
			return false;
		}
	}
}
