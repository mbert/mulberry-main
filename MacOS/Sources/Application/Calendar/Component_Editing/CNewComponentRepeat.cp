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
#include "CNewEventDialog.h"
#include "CNewTimingPanel.h"
#include "CNewToDoDialog.h"
#include "CNumberEdit.h"
#include "CRecurrenceDialog.h"
#include "CStaticText.h"

#include "CICalendar.h"
#include "CICalendarRecurrence.h"
#include "CICalendarRecurrenceSet.h"

#include <LCheckBox.h>
#include <LLittleArrows.h>
#include <LPopupButton.h>
#include <LPushButton.h>
#include <LRadioButton.h>
#include <LTabsControl.h>
#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CNewComponentRepeat														  [public]
/**
 Default constructor */

CNewComponentRepeat::CNewComponentRepeat(LStream *inStream) :
CNewComponentPanel(inStream)
{
}


// ---------------------------------------------------------------------------
//	~CNewComponentRepeat														  [public]
/**
 Destructor */

CNewComponentRepeat::~CNewComponentRepeat()
{
}

#pragma mark -

void CNewComponentRepeat::FinishCreateSelf()
{
	// Get UI items
	mRepeats = dynamic_cast<LCheckBox*>(FindPaneByID(eRepeats_ID));
	mRepeatsTabs = dynamic_cast<LTabsControl*>(FindPaneByID(eRepeatsTabs_ID));
	
	mOccursSimpleItems = dynamic_cast<LView*>(FindPaneByID(eOccursSimpleItems_ID));
	
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
	
	mOccursAdvancedItems = dynamic_cast<LView*>(FindPaneByID(eOccursAdvancedItems_ID));
	
	mOccursDescription = dynamic_cast<CStaticText*>(FindPaneByID(eOccursDescription_ID));
	mOccursEdit = dynamic_cast<LPushButton*>(FindPaneByID(eOccursEdit_ID));
	
	// Listen to some controls
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
	
	// Init controls
	DoRepeat(false);
	DoRepeatTab(eOccurs_Simple);
	DoOccursGroup(eOccurs_ForEver);
}

// Respond to clicks in the icon buttons
void CNewComponentRepeat::ListenToMessage(MessageT inMessage,void *ioParam)
{
	switch (inMessage)
	{
		case eOccursEdit_ID:
			DoOccursEdit();
			break;
		case eRepeats_ID:
			DoRepeat(mRepeats->GetValue() == 1);
			break;
		case eRepeatsTabs_ID:
			DoRepeatTab(mRepeatsTabs->GetValue());
			break;
		case eOccursForEver_ID:
			DoOccursGroup(eOccurs_ForEver);
			break;
		case eOccursCount_ID:
			DoOccursGroup(eOccurs_Count);
			break;
		case eOccursUntil_ID:
			DoOccursGroup(eOccurs_Until);
			break;
	}
}

const CNewTimingPanel* CNewComponentRepeat::GetTimingPanel() const
{
	// Look for parent item
	LView* super = GetSuperView();
	while(super && !dynamic_cast<CModelessDialog*>(super))
		super = super->GetSuperView();
	CModelessDialog* dlg = dynamic_cast<CModelessDialog*>(super);
	
	if (dynamic_cast<CNewEventDialog*>(dlg))
		return static_cast<CNewEventDialog*>(dlg)->GetTimingPanel();
	else if (dynamic_cast<CNewToDoDialog*>(dlg))
		return static_cast<CNewToDoDialog*>(dlg)->GetTimingPanel();
	else
		return NULL;
}

void CNewComponentRepeat::DoRepeat(bool repeat)
{
	mRepeatsTabs->SetEnabled(repeat);
}

void CNewComponentRepeat::DoRepeatTab(UInt32 value)
{
	switch(value)
	{
		case eOccurs_Simple:
			mOccursSimpleItems->SetVisible(true);
			mOccursAdvancedItems->SetVisible(false);
			break;
		case eOccurs_Advanced:
			mOccursSimpleItems->SetVisible(false);
			mOccursAdvancedItems->SetVisible(true);
			mOccursEdit->SetVisible(true);
			
			// Set description to advanced item
		{
			MyCFString txt(mAdvancedRecur.GetUIDescription(), kCFStringEncodingUTF8);
			mOccursDescription->SetCFDescriptor(txt);
		}
			break;
		case eOccurs_Complex:
			mOccursSimpleItems->SetVisible(false);
			mOccursAdvancedItems->SetVisible(true);
			mOccursEdit->SetVisible(false);
			
			// Set description to complex item
		{
			MyCFString txt(mComplexDescription, kCFStringEncodingUTF8);
			mOccursDescription->SetCFDescriptor(txt);
		}
			break;
	}
}

void CNewComponentRepeat::DoOccursGroup(UInt32 value)
{
	mOccursCounter->SetEnabled(value == eOccurs_Count);
	mOccursCounterSpin->SetEnabled(value == eOccurs_Count);
	
	mOccursDateTimeZone->SetEnabled(value == eOccurs_Until);
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
		MyCFString txt(mAdvancedRecur.GetUIDescription(), kCFStringEncodingUTF8);
		mOccursDescription->SetCFDescriptor(txt);
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
			mRepeats->SetValue(1);
			mRepeatsTabs->SetValue(eOccurs_Simple);
			
			// Set frequency
			mOccursFreq->SetValue(cFreqValueToPopup[recur->GetFreq()]);
			
			// Set interval
			mOccursInterval->SetNumberValue(recur->GetInterval());
			
			// Set count
			if (recur->GetUseCount())
			{
				mOccursCount->SetValue(1);
				mOccursCounter->SetNumberValue(recur->GetCount());
			}
			else if (recur->GetUseUntil())
			{
				mOccursUntil->SetValue(1);
				
				// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time
				
				// Get tzid set in the start
				iCal::CICalendarTimezone tzid;
				GetTimingPanel()->GetTimezone(tzid);
				
				// Adjust UNTIL to new timezone
				iCal::CICalendarDateTime until(recur->GetUntil());
				until.AdjustTimezone(tzid);
				
				mOccursDateTimeZone->SetDateTimeZone(until, GetTimingPanel()->GetAllDay());
			}
			else
				mOccursForEver->SetValue(1);
			
			// Always remove the complex tab as user cannot create a complex item
			mRepeatsTabs->SetMaxValue(2);
			return;
		}
		else if (recurs->IsAdvancedUI())
		{
			const iCal::CICalendarRecurrence* recur = recurs->GetUIRecurrence();
			
			// Cache the value we will be editing
			mAdvancedRecur = *recur;
			
			// Is repeating
			mRepeats->SetValue(1);
			mRepeatsTabs->SetValue(eOccurs_Advanced);
			
			// Always remove the complex tab as user cannot create a complex item
			mRepeatsTabs->SetMaxValue(2);
			
			return;
		}
		
		// Fall through to here => complex rule
		mComplexDescription = recurs->GetUIDescription();
		
		// Is repeating
		mRepeats->SetValue(1);
		mRepeatsTabs->SetValue(eOccurs_Complex);
		
	}
	else
	{
		// Is not repeating
		mRepeats->SetValue(0);
		mRepeatsTabs->SetValue(eOccurs_Simple);
		
		// Always remove the complex tab as user cannot create a complex item
		mRepeatsTabs->SetMaxValue(2);
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
	if (mRepeats->GetValue() == 0)
		return true;
	
	// Do not do anything to existing recurrence if complex mode
	if (mRepeatsTabs->GetValue() == eOccurs_Complex)
		return false;
	
	// Get simple/advanced data
	if (mRepeatsTabs->GetValue() == eOccurs_Simple)
	{
		// Simple mode means we only do a single RRULE
		iCal::CICalendarRecurrence recur;
		
		// Set frequency
		recur.SetFreq(cFreqPopupToValue[mOccursFreq->GetValue() - 1]);
		
		// Set interval
		recur.SetInterval(mOccursInterval->GetNumberValue());
		
		// Determine end
		if (mOccursForEver->GetValue() == 1)
		{
			// Nothing to add
		}
		else if (mOccursCount->GetValue() == 1)
		{
			recur.SetUseCount(true);
			recur.SetCount(mOccursCounter->GetNumberValue());
		}
		else if (mOccursUntil->GetValue() == 1)
		{
			// NB the UNTIL value is always UTC, but we want to display it to the user relative to their start time,
			// so we must convert from dialog tzid to UTC
			
			// Get value from dialog
			iCal::CICalendarDateTime until;
			mOccursDateTimeZone->GetDateTimeZone(until, GetTimingPanel()->GetAllDay());
			
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
	
	mRepeats->SetEnabled(!read_only);
	mRepeatsTabs->SetEnabled(!read_only && (mRepeats->GetValue() == 1));
}
