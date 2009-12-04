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

#include "CNewComponentRepeat.h"

#include "CDateTimeZoneSelect.h"
#include "CDivider.h"
#include "CNewEventDialog.h"
#include "CNewToDoDialog.h"
#include "CNewEventTiming.h"
#include "CNewToDoTiming.h"
#include "CNumberEdit.h"
#include "CRecurrenceDialog.h"
#include "CTabController.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarRecurrence.h"
#include "CICalendarRecurrenceSet.h"

#include "TPopupMenu.h"

#include <JXFlatRect.h>
#include <JXRadioGroup.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextRadioButton.h>
#include <JXTextButton.h>
#include <JXWindow.h>

#include <cassert>

#pragma mark -

void CNewComponentRepeat::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mRepeats =
        new JXTextCheckbox("Repeats", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 80,20);
    assert( mRepeats != NULL );

    CDivider* obj1 =
        new CDivider(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 88,20, 402,2);
    assert( obj1 != NULL );

    mRepeatsTabs =
        new CTabController(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 480,140);
    assert( mRepeatsTabs != NULL );

// end JXLayout1

	mRepeatSimpleItems = new CNewComponentRepeatSimple(mRepeatsTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 120);
	mRepeatsTabs->AppendCard(mRepeatSimpleItems, "Simple");
	mRepeatAdavancedItems = new CNewComponentRepeatAdvanced(mRepeatsTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 120);
	mRepeatsTabs->AppendCard(mRepeatAdavancedItems, "Advanced");
	mRepeatComplexItems = new CNewComponentRepeatComplex(mRepeatsTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 120);
	mRepeatsTabs->AppendCard(mRepeatComplexItems, "Complex");

	// Listen to some controls
	ListenTo(mRepeats);
	ListenTo(mRepeatsTabs);
	ListenTo(mRepeatSimpleItems->mOccursGroup);
	ListenTo(mRepeatAdavancedItems->mOccursEdit);
	
	// Init controls
	DoRepeat(false);
	DoRepeatTab(eOccurs_Simple);
	DoOccursGroup(eOccurs_ForEver);
}

// Respond to clicks in the icon buttons
void CNewComponentRepeat::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mRepeats)
		{
			DoRepeat(mRepeats->IsChecked());
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mRepeatsTabs)
			DoRepeatTab(index);
		else if (sender == mRepeatSimpleItems->mOccursGroup)
			DoOccursGroup(index);
		return;
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mRepeatAdavancedItems->mOccursEdit)
		{
			DoOccursEdit();
			return;
		}
	}
}

const CNewTimingPanel* CNewComponentRepeat::GetTimingPanel() const
{
	// Look for parent item
	const CModelessDialog* dlg = dynamic_cast<const CModelessDialog*>(GetWindow()->GetDirector());

	if (dynamic_cast<const CNewEventDialog*>(dlg))
		return static_cast<const CNewEventDialog*>(dlg)->GetTimingPanel();
	else if (dynamic_cast<const CNewToDoDialog*>(dlg))
		return static_cast<const CNewToDoDialog*>(dlg)->GetTimingPanel();
	else
		return NULL;
}

void CNewComponentRepeat::DoRepeat(bool repeat)
{
	mRepeatsTabs->SetActive(repeat);
}

void CNewComponentRepeat::DoRepeatTab(JIndex value)
{
	switch(value)
	{
	case eOccurs_Simple:
		break;
	case eOccurs_Advanced:
		// Set description to advanced item
		mRepeatAdavancedItems->mOccursDescription->SetText(mAdvancedRecur.GetUIDescription());
		break;
	case eOccurs_Complex:
		// Set description to complex item
		mRepeatComplexItems->mOccursDescription->SetText(mComplexDescription);
		break;
	}
}

void CNewComponentRepeat::DoOccursGroup(JIndex value)
{
	mRepeatSimpleItems->mOccursCounter->SetActive(value == eOccurs_Count);
	mRepeatSimpleItems->mOccursDateTimeZone->SetActive(value == eOccurs_Until);
}

void CNewComponentRepeat::DoOccursEdit()
{
	// Get tzid set in the start
	iCal::CICalendarTimezone tzid;
	GetTimingPanel()->GetTimezone(tzid);

	bool all_day = GetTimingPanel()->GetAllDay();

	// Edit the stored recurrence item
	iCal::CICalendarRecurrence	temp(mAdvancedRecur);
	
	if (CRecurrenceDialog::PoseDialog(temp, tzid, all_day))
	{
		mAdvancedRecur = temp;

		// Update description
		mRepeatAdavancedItems->mOccursDescription->SetText(mAdvancedRecur.GetUIDescription());
	}
}

void CNewComponentRepeat::SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded)
{
	// Set recurrence
	SetRecurrence(vevent.GetRecurrenceSet());
}

void CNewComponentRepeat::SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded)
{
	// Set recurrence
	//SetRecurrence(vtodo.GetRecurrenceSet());
}

void CNewComponentRepeat::SetRecurrence(const iCal::CICalendarRecurrenceSet* recurs)
{
	static const int cFreqValueToPopup[] =
	{
		CNewComponentRepeat::eOccurs_Secondly, CNewComponentRepeat::eOccurs_Minutely, CNewComponentRepeat::eOccurs_Hourly,
		CNewComponentRepeat::eOccurs_Daily, CNewComponentRepeat::eOccurs_Weekly, CNewComponentRepeat::eOccurs_Monthly, CNewComponentRepeat::eOccurs_Yearly
	};

	// See whether it is simple enough that we can handle it
	if ((recurs != NULL) && recurs->HasRecurrence())
	{
		if (recurs->IsSimpleUI())
		{
			const iCal::CICalendarRecurrence* recur = recurs->GetUIRecurrence();

			// Is repeating
			mRepeats->SetState(true);
			mRepeatsTabs->ShowCard(eOccurs_Simple);

			// Set frequency
			mRepeatSimpleItems->mOccursFreq->SetValue(cFreqValueToPopup[recur->GetFreq()]);
			
			// Set interval
			mRepeatSimpleItems->mOccursInterval->SetNumberValue(recur->GetInterval());
			
			// Set count
			if (recur->GetUseCount())
			{
				mRepeatSimpleItems->mOccursGroup->SelectItem(eOccurs_Count);
				mRepeatSimpleItems->mOccursCounter->SetNumberValue(recur->GetCount());
			}
			else if (recur->GetUseUntil())
			{
				mRepeatSimpleItems->mOccursGroup->SelectItem(eOccurs_Until);
				
				// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time
				
				// Get tzid set in the start
				iCal::CICalendarTimezone tzid;
				GetTimingPanel()->GetTimezone(tzid);

				// Adjust UNTIL to new timezone
				iCal::CICalendarDateTime until(recur->GetUntil());
				until.AdjustTimezone(tzid);

				mRepeatSimpleItems->mOccursDateTimeZone->SetDateTimeZone(until, GetTimingPanel()->GetAllDay());
			}
			else
				mRepeatSimpleItems->mOccursGroup->SelectItem(eOccurs_ForEver);
			
			// Always remove the complex tab as user cannot create a complex item
			mRepeatsTabs->DeactivateCard(eOccurs_Complex);
			return;
		}
		else if (recurs->IsAdvancedUI())
		{
			const iCal::CICalendarRecurrence* recur = recurs->GetUIRecurrence();

			// Cache the value we will be editing
			mAdvancedRecur = *recur;
			
			// Is repeating
			mRepeats->SetState(true);
			mRepeatsTabs->ShowCard(eOccurs_Advanced);
			
			// Always remove the complex tab as user cannot create a complex item
			mRepeatsTabs->DeactivateCard(eOccurs_Complex);
			
			return;
		}
		
		// Fall through to here => complex rule
		mComplexDescription = recurs->GetUIDescription();

		// Is repeating
		mRepeats->SetState(true);
		mRepeatsTabs->ShowCard(eOccurs_Complex);
				
	}
	else
	{
		// Is not repeating
		mRepeats->SetState(false);
		mRepeatsTabs->ShowCard(eOccurs_Simple);
		
		// Always remove the complex tab as user cannot create a complex item
		mRepeatsTabs->DeactivateCard(eOccurs_Complex);
	}
}

void CNewComponentRepeat::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do recurrence items
	// NB in complex mode we do not change the existing set
	iCal::CICalendarRecurrenceSet recurs;
	if (GetRecurrence(recurs))
		vevent.EditRecurrenceSet(recurs);
}

void CNewComponentRepeat::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do recurrence items
	// NB in complex mode we do not change the existing set
	//iCal::CICalendarRecurrenceSet recurs;
	//if (GetRecurrence(recurs))
	//	vtodo.EditRecurrenceSet(recurs);
}

static const iCal::ERecurrence_FREQ cFreqPopupToValue[] = 
{
	iCal::eRecurrence_YEARLY, iCal::eRecurrence_MONTHLY, iCal::eRecurrence_WEEKLY, iCal::eRecurrence_DAILY,
	iCal::eRecurrence_HOURLY, iCal::eRecurrence_MINUTELY, iCal::eRecurrence_SECONDLY
};

bool CNewComponentRepeat::GetRecurrence(iCal::CICalendarRecurrenceSet& recurs)
{
	// Only if repeating enabled
	if (!mRepeats->IsChecked())
		return true;
	
	// Do not do anything to existing recurrence if complex mode
	if (mRepeatsTabs->GetSelectedItem() == eOccurs_Complex)
		return false;
	
	// Get simple/advanced data
	if (mRepeatsTabs->GetSelectedItem() == eOccurs_Simple)
	{
		// Simple mode means we only do a single RRULE
		iCal::CICalendarRecurrence recur;
		
		// Set frequency
		recur.SetFreq(cFreqPopupToValue[mRepeatSimpleItems->mOccursGroup->GetSelectedItem() - 1]);
		
		// Set interval
		recur.SetInterval(mRepeatSimpleItems->mOccursInterval->GetNumberValue());
		
		// Determine end
		if (mRepeatSimpleItems->mOccursGroup->GetSelectedItem() == eOccurs_ForEver)
		{
			// Nothing to add
		}
		else if (mRepeatSimpleItems->mOccursGroup->GetSelectedItem() == eOccurs_Count)
		{
			recur.SetUseCount(true);
			recur.SetCount(mRepeatSimpleItems->mOccursCounter->GetNumberValue());
		}
		else if (mRepeatSimpleItems->mOccursGroup->GetSelectedItem() == eOccurs_Until)
		{
			// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time,
			// so we must convert from dialog tzid to UTC
			
			// Get value from dialog
			iCal::CICalendarDateTime until;
			mRepeatSimpleItems->mOccursDateTimeZone->GetDateTimeZone(until, GetTimingPanel()->GetAllDay());
			
			// Adjust to UTC
			until.AdjustToUTC();

			recur.SetUseUntil(true);
			recur.SetUntil(until);
		}
		
		// Now add recurrence
		recurs.Add(recur);
	}
	else
		// Just add advanced item
		recurs.Add(mAdvancedRecur);
	
	return true;
}

void CNewComponentRepeat::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	mRepeats->SetActive(!read_only);
	mRepeatsTabs->SetActive(!read_only && mRepeats->IsChecked());
}

#pragma mark ______________________________CNewComponentRepeatSimple

void CNewComponentRepeatSimple::OnCreate()
{
// begin JXLayout2

    JXStaticText* obj1 =
        new JXStaticText("Every:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 5,5, 47,20);
    assert( obj1 != NULL );

    mOccursInterval =
        new CNumberEdit(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 55,5, 70,20);
    assert( mOccursInterval != NULL );

    mOccursFreq =
        new HPopupMenu("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 135,5, 110,20);
    assert( mOccursFreq != NULL );

    mOccursGroup =
        new JXRadioGroup(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 5,30, 80,85);
    assert( mOccursGroup != NULL );
    mOccursGroup->SetBorderWidth(0);

    mOccursForEver =
        new JXTextRadioButton(1, "For Ever", mOccursGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 80,20);
    assert( mOccursForEver != NULL );

    mOccursCount =
        new JXTextRadioButton(2, "For:", mOccursGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,30, 70,20);
    assert( mOccursCount != NULL );

    mOccursUntil =
        new JXTextRadioButton(3, "Until:", mOccursGroup,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,60, 70,20);
    assert( mOccursUntil != NULL );

    mOccursCounter =
        new CNumberEdit(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,60, 70,20);
    assert( mOccursCounter != NULL );

    JXStaticText* obj2 =
        new JXStaticText("time(s)", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 150,65, 47,20);
    assert( obj2 != NULL );

    mOccursDateTimeZone =
        new CDateTimeZoneSelect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,85, 400,30);
    assert( mOccursDateTimeZone != NULL );

// end JXLayout2

	mOccursInterval->OnCreate(1, 0x7FFFFFFF);
	mOccursFreq->SetMenuItems("Year(s) %r | Month(s) %r | Week(s) %r | Day(s) %r | Hour(s) %r | Minute(s) %r | Second(s) %r");
	mOccursFreq->SetValue(1);
	mOccursCounter->OnCreate(1, 0x7FFFFFFF);
	mOccursDateTimeZone->OnCreate();
}

#pragma mark ______________________________CNewComponentRepeatAdvanced

void CNewComponentRepeatAdvanced::OnCreate()
{
// begin JXLayout3

    JXStaticText* obj1 =
        new JXStaticText("Description:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 80,20);
    assert( obj1 != NULL );

    mOccursDescription =
        new JXStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,5, 380,40);
    assert( mOccursDescription != NULL );

    mOccursEdit =
        new JXTextButton("Edit...", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 368,56, 80,25);
    assert( mOccursEdit != NULL );
    mOccursEdit->SetFontSize(10);

// end JXLayout3
}

#pragma mark ______________________________CNewComponentRepeatComplex

void CNewComponentRepeatComplex::OnCreate()
{
// begin JXLayout4

    JXStaticText* obj1 =
        new JXStaticText("Description:", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,5, 80,20);
    assert( obj1 != NULL );

    mOccursDescription =
        new JXStaticText("", this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,5, 380,40);
    assert( mOccursDescription != NULL );

// end JXLayout4
}
