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

#include "CDateTimeZoneSelect.h"
#include "CDivider.h"
#include "CNumberEdit.h"

#include "CICalendarRecurrence.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXFlatRect.h>
#include <JXStaticText.h>
#include <JXTextButton.h>
#include "JXTextPushButton.h"
#include <JXTextRadioButton.h>
#include <JXRadioGroup.h>
#include <JXTextCheckbox.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <string.h>

#include <cassert>

// ---------------------------------------------------------------------------
//	CRecurrenceDialog														  [public]
/**
	Default constructor */

CRecurrenceDialog::CRecurrenceDialog(JXDirector* supervisor)
	: CDialogDirector(supervisor)
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

void CRecurrenceDialog::OnCreate()
{
	// Get UI items
// begin JXLayout

    JXWindow* window = new JXWindow(this, 540,430, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 540,430);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Every:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 47,20);
    assert( obj2 != NULL );

    mOccursInterval =
        new CNumberEdit(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 60,10, 70,20);
    assert( mOccursInterval != NULL );

    mOccursFreq =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 140,10, 110,20);
    assert( mOccursFreq != NULL );

    mOccursGroup =
        new JXRadioGroup(obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 10,35, 100,80);
    assert( mOccursGroup != NULL );
    mOccursGroup->SetBorderWidth(0);

    mOccursForEver =
        new JXTextRadioButton(1, "For Ever", mOccursGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 100,20);
    assert( mOccursForEver != NULL );

    mOccursCount =
        new JXTextRadioButton(2, "For:", mOccursGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,30, 90,20);
    assert( mOccursCount != NULL );

    mOccursUntil =
        new JXTextRadioButton(3, "Until:", mOccursGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,60, 90,20);
    assert( mOccursUntil != NULL );

    mOccursCounter =
        new CNumberEdit(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 115,65, 70,20);
    assert( mOccursCounter != NULL );

    JXStaticText* obj3 =
        new JXStaticText("time(s)", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,65, 47,20);
    assert( obj3 != NULL );

    mOccursDateTimeZone =
        new CDateTimeZoneSelect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,90, 400,30);
    assert( mOccursDateTimeZone != NULL );

    JXStaticText* obj4 =
        new JXStaticText("By Month", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,130, 65,20);
    assert( obj4 != NULL );

    CDivider* obj5 =
        new CDivider(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 70,140, 460,2);
    assert( obj5 != NULL );

    mByMonth[0] =
        new JXTextPushButton("Jan", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 30,160, 40,20);
    assert( mByMonth[0] != NULL );

    mByMonth[1] =
        new JXTextPushButton("Feb", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,160, 40,20);
    assert( mByMonth[1] != NULL );

    mByMonth[2] =
        new JXTextPushButton("Mar", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 110,160, 40,20);
    assert( mByMonth[2] != NULL );

    mByMonth[3] =
        new JXTextPushButton("Apr", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,160, 40,20);
    assert( mByMonth[3] != NULL );

    mByMonth[4] =
        new JXTextPushButton("May", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,160, 40,20);
    assert( mByMonth[4] != NULL );

    mByMonth[5] =
        new JXTextPushButton("Jun", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 230,160, 40,20);
    assert( mByMonth[5] != NULL );

    mByMonth[6] =
        new JXTextPushButton("Jul", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 270,160, 40,20);
    assert( mByMonth[6] != NULL );

    mByMonth[7] =
        new JXTextPushButton("Aug", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 310,160, 40,20);
    assert( mByMonth[7] != NULL );

    mByMonth[8] =
        new JXTextPushButton("Sep", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 350,160, 40,20);
    assert( mByMonth[8] != NULL );

    mByMonth[9] =
        new JXTextPushButton("Oct", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 390,160, 40,20);
    assert( mByMonth[9] != NULL );

    mByMonth[10] =
        new JXTextPushButton("Nov", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 430,160, 40,20);
    assert( mByMonth[10] != NULL );

    mByMonth[11] =
        new JXTextPushButton("Dec", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 470,160, 40,20);
    assert( mByMonth[11] != NULL );

    JXStaticText* obj6 =
        new JXStaticText("By Day of Month", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,195, 105,20);
    assert( obj6 != NULL );

    CDivider* obj7 =
        new CDivider(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 113,205, 110,2);
    assert( obj7 != NULL );

    mByMonthDay[0] =
        new JXTextPushButton("1", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,225, 30,20);
    assert( mByMonthDay[0] != NULL );

    mByMonthDay[1] =
        new JXTextPushButton("2", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,225, 30,20);
    assert( mByMonthDay[1] != NULL );

    mByMonthDay[2] =
        new JXTextPushButton("3", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,225, 30,20);
    assert( mByMonthDay[2] != NULL );

    mByMonthDay[3] =
        new JXTextPushButton("4", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,225, 30,20);
    assert( mByMonthDay[3] != NULL );

    mByMonthDay[4] =
        new JXTextPushButton("5", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,225, 30,20);
    assert( mByMonthDay[4] != NULL );

    mByMonthDay[5] =
        new JXTextPushButton("6", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,225, 30,20);
    assert( mByMonthDay[5] != NULL );

    mByMonthDay[6] =
        new JXTextPushButton("7", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,225, 30,20);
    assert( mByMonthDay[6] != NULL );

    mByMonthDay[7] =
        new JXTextPushButton("8", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,245, 30,20);
    assert( mByMonthDay[7] != NULL );

    mByMonthDay[8] =
        new JXTextPushButton("9", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,245, 30,20);
    assert( mByMonthDay[8] != NULL );

    mByMonthDay[9] =
        new JXTextPushButton("10", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,245, 30,20);
    assert( mByMonthDay[9] != NULL );

    mByMonthDay[10]  =
        new JXTextPushButton("11", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,245, 30,20);
    assert( mByMonthDay[10]  != NULL );

    mByMonthDay[11]  =
        new JXTextPushButton("12", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,245, 30,20);
    assert( mByMonthDay[11]  != NULL );

    mByMonthDay[12]  =
        new JXTextPushButton("13", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,245, 30,20);
    assert( mByMonthDay[12]  != NULL );

    mByMonthDay[13]  =
        new JXTextPushButton("14", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,245, 30,20);
    assert( mByMonthDay[13]  != NULL );

    mByMonthDay[14]  =
        new JXTextPushButton("15", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,265, 30,20);
    assert( mByMonthDay[14]  != NULL );

    mByMonthDay[15]  =
        new JXTextPushButton("16", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,265, 30,20);
    assert( mByMonthDay[15]  != NULL );

    mByMonthDay[16]  =
        new JXTextPushButton("17", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,265, 30,20);
    assert( mByMonthDay[16]  != NULL );

    mByMonthDay[17]  =
        new JXTextPushButton("18", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,265, 30,20);
    assert( mByMonthDay[17]  != NULL );

    mByMonthDay[18]  =
        new JXTextPushButton("19", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,265, 30,20);
    assert( mByMonthDay[18]  != NULL );

    mByMonthDay[19]  =
        new JXTextPushButton("20", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,265, 30,20);
    assert( mByMonthDay[19]  != NULL );

    mByMonthDay[20]  =
        new JXTextPushButton("21", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,265, 30,20);
    assert( mByMonthDay[20]  != NULL );

    mByMonthDay[21]  =
        new JXTextPushButton("22", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,285, 30,20);
    assert( mByMonthDay[21]  != NULL );

    mByMonthDay[22]  =
        new JXTextPushButton("23", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,285, 30,20);
    assert( mByMonthDay[22]  != NULL );

    mByMonthDay[23]  =
        new JXTextPushButton("24", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,285, 30,20);
    assert( mByMonthDay[23]  != NULL );

    mByMonthDay[24]  =
        new JXTextPushButton("25", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,285, 30,20);
    assert( mByMonthDay[24]  != NULL );

    mByMonthDay[25]  =
        new JXTextPushButton("26", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,285, 30,20);
    assert( mByMonthDay[25]  != NULL );

    mByMonthDay[26]  =
        new JXTextPushButton("27", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,285, 30,20);
    assert( mByMonthDay[26]  != NULL );

    mByMonthDay[27]  =
        new JXTextPushButton("28", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,285, 30,20);
    assert( mByMonthDay[27]  != NULL );

    mByMonthDay[28]  =
        new JXTextPushButton("29", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,305, 30,20);
    assert( mByMonthDay[28]  != NULL );

    mByMonthDay[29]  =
        new JXTextPushButton("30", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,305, 30,20);
    assert( mByMonthDay[29]  != NULL );

    mByMonthDay[30]  =
        new JXTextPushButton("31", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,305, 30,20);
    assert( mByMonthDay[30]  != NULL );

    mByMonthDayLast[0]  =
        new JXTextPushButton("-1", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,340, 30,20);
    assert( mByMonthDayLast[0]  != NULL );

    mByMonthDayLast[1]  =
        new JXTextPushButton("-2", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 40,340, 30,20);
    assert( mByMonthDayLast[1]  != NULL );

    mByMonthDayLast[2]  =
        new JXTextPushButton("-3", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 70,340, 30,20);
    assert( mByMonthDayLast[2]  != NULL );

    mByMonthDayLast[3]  =
        new JXTextPushButton("-4", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 100,340, 30,20);
    assert( mByMonthDayLast[3]  != NULL );

    mByMonthDayLast[4]  =
        new JXTextPushButton("-5", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 130,340, 30,20);
    assert( mByMonthDayLast[4]  != NULL );

    mByMonthDayLast[5]  =
        new JXTextPushButton("-6", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 160,340, 30,20);
    assert( mByMonthDayLast[5]  != NULL );

    mByMonthDayLast[6]  =
        new JXTextPushButton("-7", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 190,340, 30,20);
    assert( mByMonthDayLast[6]  != NULL );

    JXStaticText* obj8 =
        new JXStaticText("By Day", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 245,195, 65,20);
    assert( obj8 != NULL );

    CDivider* obj9 =
        new CDivider(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 300,205, 230,2);
    assert( obj9 != NULL );

    mByDay[0] =
        new JXTextPushButton("Sun", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,225, 40,20);
    assert( mByDay[0] != NULL );

    mByDay[1] =
        new JXTextPushButton("Mon", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 290,225, 40,20);
    assert( mByDay[1] != NULL );

    mByDay[2] =
        new JXTextPushButton("Tue", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 330,225, 40,20);
    assert( mByDay[2] != NULL );

    mByDay[3] =
        new JXTextPushButton("Wed", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 370,225, 40,20);
    assert( mByDay[3] != NULL );

    mByDay[4] =
        new JXTextPushButton("Thu", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 410,225, 40,20);
    assert( mByDay[4] != NULL );

    mByDay[5] =
        new JXTextPushButton("Fri", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 450,225, 40,20);
    assert( mByDay[5] != NULL );

    mByDay[6] =
        new JXTextPushButton("Sat", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 490,225, 40,20);
    assert( mByDay[6] != NULL );

    mByDayView =
        new JXFlatRect(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,265, 280,20);
    assert( mByDayView != NULL );

    mByDayPopup =
        new HPopupMenu("",mByDayView,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 150,20);
    assert( mByDayPopup != NULL );

    JXStaticText* obj10 =
        new JXStaticText("it occurs in the Month", mByDayView,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 150,0, 130,20);
    assert( obj10 != NULL );
    obj10->SetFontSize(10);

    JXStaticText* obj11 =
        new JXStaticText("By Position it Occurs in the Current Set", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 245,305, 235,20);
    assert( obj11 != NULL );

    CDivider* obj12 =
        new CDivider(obj1,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 480,315, 50,2);
    assert( obj12 != NULL );

    mBySetPosPopup =
        new HPopupMenu("",obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 250,340, 250,20);
    assert( mBySetPosPopup != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedLeft, JXWidget::kFixedBottom, 310,390, 85,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 425,390, 85,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout
	window->SetTitle("Recurrence");
	SetButtons(mOKBtn, mCancelBtn);


	mOccursInterval->OnCreate(1, 0x7FFFFFFF);
	mOccursFreq->SetMenuItems("Year(s) %r | Month(s) %r | Week(s) %r | Day(s) %r | Hour(s) %r | Minute(s) %r | Second(s) %r");
	mOccursFreq->SetValue(1);
	mOccursCounter->OnCreate(1, 0x7FFFFFFF);
	mOccursDateTimeZone->OnCreate();

	mByDayPopup->SetMenuItems("Any Time %r | The 1st Time %r | The 2nd Time %r | The 3rd Time %r | The 4th Time %r | The 2nd Last Time %r | The Last Time %r");
	mByDayPopup->SetValue(1);

	mBySetPosPopup->SetMenuItems("All Items %r | The 1st One Only %r | The Last One Only %r");
	mBySetPosPopup->SetValue(1);

	// Listen to some controls
	ListenTo(mOccursFreq);
	ListenTo(mOccursGroup);
	
	// Init controls
	DoOccursFreq(eOccurs_Yearly);
	DoOccursGroup(eOccurs_ForEver);

	// Focus on summary
}

// Handle controls
void CRecurrenceDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mOccursGroup)
			DoOccursGroup(index);
		return;
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mOccursFreq)
		{
			DoOccursFreq(is->GetIndex());
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

void CRecurrenceDialog::DoOccursFreq(JIndex value)
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

void CRecurrenceDialog::DoOccursGroup(JIndex value)
{
	mOccursCounter->SetActive(value == eOccurs_Count);
	mOccursDateTimeZone->SetActive(value == eOccurs_Until);
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
		mOccursGroup->SelectItem(eOccurs_Count);
		mOccursCounter->SetNumberValue(recur.GetCount());
	}
	else if (recur.GetUseUntil())
	{
		mOccursGroup->SelectItem(eOccurs_Until);
		
		// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time
		
		// Adjust UNTIL to new timezone
		iCal::CICalendarDateTime until(recur.GetUntil());
		until.AdjustTimezone(tzid);

		mOccursDateTimeZone->SetDateTimeZone(until, all_day);
	}
	else
		mOccursGroup->SelectItem(eOccurs_ForEver);
	
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
		mByMonth[i]->SetState(false);

	const std::vector<int32_t>& by_month = recur.GetByMonth();
	for(std::vector<int32_t>::const_iterator iter = by_month.begin(); iter != by_month.end(); iter++)
	{
		if ((*iter >= 1) && (*iter <= 12))
			mByMonth[*iter - 1]->SetState(true);
	}
}

void CRecurrenceDialog::SetByMonthDay(const iCal::CICalendarRecurrence& recur)
{
	// Unset all buttons first
	for(uint32_t i = 0; i < 31; i++)
		mByMonthDay[i]->SetState(false);
	for(uint32_t i = 0; i < 7; i++)
		mByMonthDayLast[i]->SetState(false);

	const std::vector<int32_t>& by_month_day = recur.GetByMonthDay();
	for(std::vector<int32_t>::const_iterator iter = by_month_day.begin(); iter != by_month_day.end(); iter++)
	{
		// NB Cannot handle all negative values, or zero
		if ((*iter >= 1) && (*iter <= 31))
			mByMonthDay[*iter - 1]->SetState(true);
		else if ((*iter >= -7) && (*iter <= -1))
			mByMonthDayLast[-(*iter) - 1]->SetState(true);
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
		mByDay[i]->SetState(false);

	// Set the day buttons
	const std::vector<iCal::CICalendarRecurrence::CWeekDayNum>& by_day = recur.GetByDay();
	for(std::vector<iCal::CICalendarRecurrence::CWeekDayNum>::const_iterator iter = by_day.begin(); iter != by_day.end(); iter++)
	{
		if (((*iter).second >= 0) && ((*iter).second <= 6))
			mByDay[(*iter).second]->SetState(true);
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
	if (mOccursGroup->GetSelectedItem() == eOccurs_ForEver)
	{
		// Nothing to add
	}
	else if (mOccursGroup->GetSelectedItem() == eOccurs_Count)
	{
		temp.SetUseCount(true);
		temp.SetCount(mOccursCounter->GetNumberValue());
	}
	else if (mOccursGroup->GetSelectedItem() == eOccurs_Until)
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
		if (mByMonth[i]->IsChecked())
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
		if (mByMonthDay[i]->IsChecked())
			by_month_day.push_back(i + 1);
	}
	for(uint32_t i = 0; i < 7; i++)
	{
		if (mByMonthDayLast[i]->IsChecked())
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
		if (mByDay[i]->IsChecked())
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
	bool result = false;

	// Create the dialog
	CRecurrenceDialog* dlog = new CRecurrenceDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetRecurrence(recur, tzid, all_day);

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetRecurrence(recur, tzid, all_day);
		dlog->Close();
		result = true;
	}

	return result;
}
