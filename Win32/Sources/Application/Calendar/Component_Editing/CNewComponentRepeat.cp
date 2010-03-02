/*
    Copyright (c) 2007-2009 Cyrus Daboo. All rights reserved.
    
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
#include "CModelessDialog.h"
#include "CNewEventDialog.h"
#include "CNewToDoDialog.h"
#include "CNewTimingPanel.h"
#include "CRecurrenceDialog.h"
#include "CUnicodeUtils.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"
#include "CICalendarRecurrence.h"
#include "CICalendarRecurrenceSet.h"

#pragma mark ______________________________CNewComponentRepeat

// ---------------------------------------------------------------------------
//	CNewComponentRepeat														  [public]
/**
	Default constructor */

CNewComponentRepeat::CNewComponentRepeat() :
	CNewComponentPanel(IDD_CALENDAR_NEW_REPEAT)
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

BEGIN_MESSAGE_MAP(CNewComponentRepeat, CNewComponentPanel)
	//{{AFX_MSG_MAP(CNewComponentRepeat)
	ON_COMMAND(IDC_CALENDAR_NEW_REPEAT_REPEATS, OnRepeat)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CALENDAR_NEW_REPEAT_TABS, OnSelChangeRepeatTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentRepeat::OnInitDialog()
{
	CNewComponentPanel::OnInitDialog();

	// Get UI items
	mRepeats.SubclassDlgItem(IDC_CALENDAR_NEW_REPEAT_REPEATS, this);
	mRepeatsTabs.SubclassDlgItem(IDC_CALENDAR_NEW_REPEAT_TABS, this);
	
	// Create tab panels
	mRepeatSimpleItems = new CNewComponentRepeatSimple(this);
	mRepeatsTabs.AddPanel(mRepeatSimpleItems);

	mRepeatAdavancedItems = new CNewComponentRepeatAdvanced(this);
	mRepeatsTabs.AddPanel(mRepeatAdavancedItems);

	mRepeatComplexItems = new CNewComponentRepeatComplex;
	mRepeatsTabs.AddPanel(mRepeatComplexItems);
	mRepeatsTabs.SetPanel(0);

	// Init controls
	DoRepeat(false);
	DoRepeatTab(eOccurs_Simple);
	DoOccursGroup(eOccurs_ForEver);
	
	return true;
}

void CNewComponentRepeat::OnRepeat()
{
	DoRepeat(mRepeats.GetCheck() == 1);
}

void CNewComponentRepeat::OnSelChangeRepeatTab(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	mRepeatsTabs.SetPanel(mRepeatsTabs.GetCurSel());
	DoRepeatTab(mRepeatsTabs.GetCurSel());

	*pResult = 0;
}

const CNewTimingPanel* CNewComponentRepeat::GetTimingPanel() const
{
	// Look for parent item
	CWnd* super = GetParent();
	while(super && !dynamic_cast<CModelessDialog*>(super))
		super = super->GetParent();
	CModelessDialog* dlg = dynamic_cast<CModelessDialog*>(super);

	if (dynamic_cast<const CNewEventDialog*>(dlg))
		return static_cast<const CNewEventDialog*>(dlg)->GetTimingPanel();
	else if (dynamic_cast<const CNewToDoDialog*>(dlg))
		return static_cast<const CNewToDoDialog*>(dlg)->GetTimingPanel();
	else
		return NULL;
}

void CNewComponentRepeat::DoRepeat(bool repeat)
{
	mRepeatsTabs.EnableWindow(repeat);
	mRepeatSimpleItems->EnableWindow(repeat);
	mRepeatAdavancedItems->EnableWindow(repeat);
	if (mRepeatComplexItems != NULL)
		mRepeatComplexItems->EnableWindow(repeat);
}

void CNewComponentRepeat::DoRepeatTab(UInt32 value)
{
	switch(value)
	{
	case eOccurs_Simple:
		break;
	case eOccurs_Advanced:
		// Set description to advanced item
		CUnicodeUtils::SetWindowTextUTF8(&mRepeatAdavancedItems->mOccursDescription, mAdvancedRecur.GetUIDescription());
		break;
	case eOccurs_Complex:
		// Set description to complex item
		CUnicodeUtils::SetWindowTextUTF8(&mRepeatComplexItems->mOccursDescription, mComplexDescription);
		break;
	}
}

void CNewComponentRepeat::DoOccursGroup(UInt32 value)
{
	mRepeatSimpleItems->mOccursCounter.EnableWindow(value == eOccurs_Count);
	mRepeatSimpleItems->mOccursCounterSpin.EnableWindow(value == eOccurs_Count);

	mRepeatSimpleItems->mOccursDateTimeZone->EnableWindow(value == eOccurs_Until);
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
		if (mRepeatComplexItems != NULL)
			CUnicodeUtils::SetWindowTextUTF8(&mRepeatComplexItems->mOccursDescription, mAdvancedRecur.GetUIDescription());
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
			mRepeats.SetCheck(1);
			DoRepeat(true);
			mRepeatsTabs.SetCurSel(eOccurs_Simple);
			mRepeatsTabs.SetPanel(eOccurs_Simple);
			DoRepeatTab(mRepeatsTabs.GetCurSel());

			// Set frequency
			mRepeatSimpleItems->mOccursFreq.SetValue(cFreqValueToPopup[recur->GetFreq()] + IDM_CALENDAR_REPEAT_FREQ_YEARS);
			
			// Set interval
			mRepeatSimpleItems->mOccursInterval.SetValue(recur->GetInterval());
			
			// Set count
			if (recur->GetUseCount())
			{
				mRepeatSimpleItems->mOccursForEver.SetCheck(0);
				mRepeatSimpleItems->mOccursCount.SetCheck(1);
				mRepeatSimpleItems->mOccursUntil.SetCheck(0);
				DoOccursGroup(eOccurs_Count);
				mRepeatSimpleItems->mOccursCounter.SetValue(recur->GetCount());
			}
			else if (recur->GetUseUntil())
			{
				mRepeatSimpleItems->mOccursForEver.SetCheck(0);
				mRepeatSimpleItems->mOccursCount.SetCheck(0);
				mRepeatSimpleItems->mOccursUntil.SetCheck(1);
				DoOccursGroup(eOccurs_Until);
				
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
			{
				mRepeatSimpleItems->mOccursForEver.SetCheck(1);
				mRepeatSimpleItems->mOccursCount.SetCheck(0);
				mRepeatSimpleItems->mOccursUntil.SetCheck(0);
				DoOccursGroup(eOccurs_ForEver);
			}
			
			// Always remove the complex tab as user cannot create a complex item
			mRepeatsTabs.RemovePanel(2);
			mRepeatComplexItems = NULL;
			return;
		}
		else if (recurs->IsAdvancedUI())
		{
			const iCal::CICalendarRecurrence* recur = recurs->GetUIRecurrence();

			// Cache the value we will be editing
			mAdvancedRecur = *recur;
			
			// Is repeating
			mRepeats.SetCheck(1);
			DoRepeat(true);
			mRepeatsTabs.SetCurSel(eOccurs_Advanced);
			mRepeatsTabs.SetPanel(eOccurs_Advanced);
			DoRepeatTab(mRepeatsTabs.GetCurSel());
			
			// Always remove the complex tab as user cannot create a complex item
			mRepeatsTabs.RemovePanel(2);
			mRepeatComplexItems = NULL;
			
			return;
		}
		
		// Fall through to here => complex rule
		mComplexDescription = recurs->GetUIDescription();

		// Is repeating
		mRepeats.SetCheck(1);
		DoRepeat(true);
		mRepeatsTabs.SetCurSel(eOccurs_Complex);
		mRepeatsTabs.SetPanel(eOccurs_Complex);
		DoRepeatTab(mRepeatsTabs.GetCurSel());
	}
	else
	{
		// Is not repeating
		mRepeats.SetCheck(0);
		mRepeatsTabs.SetCurSel(eOccurs_Simple);
		mRepeatsTabs.SetPanel(eOccurs_Simple);
		DoRepeatTab(mRepeatsTabs.GetCurSel());
		DoRepeat(false);
		
		// Always remove the complex tab as user cannot create a complex item
		mRepeatsTabs.RemovePanel(2);
		mRepeatComplexItems = NULL;
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
	if (mRepeats.GetCheck() == 0)
		return true;
	
	// Do not do anything to existing recurrence if complex mode
	if (mRepeatsTabs.GetCurSel() == eOccurs_Complex)
		return false;
	
	// Get simple/advanced data
	if (mRepeatsTabs.GetCurSel() == eOccurs_Simple)
	{
		// Simple mode means we only do a single RRULE
		iCal::CICalendarRecurrence recur;
		
		// Set frequency
		recur.SetFreq(cFreqPopupToValue[mRepeatSimpleItems->mOccursFreq.GetValue() - IDM_CALENDAR_REPEAT_FREQ_YEARS]);
		
		// Set interval
		recur.SetInterval(mRepeatSimpleItems->mOccursInterval.GetValue());
		
		// Determine end
		if (mRepeatSimpleItems->mOccursForEver.GetCheck() == 1)
		{
			// Nothing to add
		}
		else if (mRepeatSimpleItems->mOccursCount.GetCheck() == 1)
		{
			recur.SetUseCount(true);
			recur.SetCount(mRepeatSimpleItems->mOccursCounter.GetValue());
		}
		else if (mRepeatSimpleItems->mOccursUntil.GetCheck() == 1)
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

	mRepeats.EnableWindow(!read_only);
	mRepeatsTabs.EnableWindow(!read_only && (mRepeats.GetCheck() == 1));

	if (mRepeatSimpleItems != NULL)
		mRepeatSimpleItems->EnableWindow(!read_only && (mRepeats.GetCheck() == 1));
	if (mRepeatAdavancedItems != NULL)
		mRepeatAdavancedItems->EnableWindow(!read_only && (mRepeats.GetCheck() == 1));
	if (mRepeatComplexItems != NULL)
		mRepeatComplexItems->EnableWindow(!read_only && (mRepeats.GetCheck() == 1));
}

#pragma mark ______________________________CNewComponentRepeatSimple

// ---------------------------------------------------------------------------
//	CNewComponentRepeatSimple														  [public]
/**
	Default constructor */

CNewComponentRepeatSimple::CNewComponentRepeatSimple(CNewComponentRepeat* timing) :
	CTabPanel(IDD_CALENDAR_REPEAT_SIMPLE),
	mOccursFreq(true)
{
	mTimingPanel = timing;
	mOccursDateTimeZone = NULL;
}


// ---------------------------------------------------------------------------
//	~CNewComponentRepeatSimple														  [public]
/**
	Destructor */

CNewComponentRepeatSimple::~CNewComponentRepeatSimple()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentRepeatSimple, CTabPanel)
	//{{AFX_MSG_MAP(CNewComponentRepeatSimple)
	ON_WM_ENABLE()

	ON_COMMAND(IDC_CALENDAR_REPEAT_FOREVER, OnOccursForEver)
	ON_COMMAND(IDC_CALENDAR_REPEAT_COUNT, OnOccursCount)
	ON_COMMAND(IDC_CALENDAR_REPEAT_UNTIL, OnOccursUntil)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentRepeatSimple::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Get UI items
	mOccursInterval.SubclassDlgItem(IDC_CALENDAR_REPEAT_INTERVAL, this);
	mOccursInterval.SetValue(1);
	mOccursIntervalSpin.SubclassDlgItem(IDC_CALENDAR_REPEAT_INTERVALSPIN, this);
	mOccursIntervalSpin.SetRange(1, 1000);
	mOccursFreq.SubclassDlgItem(IDC_CALENDAR_REPEAT_FREQ, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mOccursFreq.SetMenu(IDR_CALENDAR_REPEAT_FREQ);
	mOccursFreq.SetValue(IDM_CALENDAR_REPEAT_FREQ_YEARS);

	mOccursForEver.SubclassDlgItem(IDC_CALENDAR_REPEAT_FOREVER, this);
	mOccursCount.SubclassDlgItem(IDC_CALENDAR_REPEAT_COUNT, this);
	mOccursUntil.SubclassDlgItem(IDC_CALENDAR_REPEAT_UNTIL, this);
	mOccursForEver.SetCheck(1);

	mOccursCounter.SubclassDlgItem(IDC_CALENDAR_REPEAT_COUNTER, this);
	mOccursCounter.SetValue(1);
	mOccursCounterSpin.SubclassDlgItem(IDC_CALENDAR_REPEAT_COUNTERSPIN, this);
	mOccursCounterSpin.SetRange(1, 1000);

	mOccursDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_REPEAT_DATETZ, this);
	mOccursDateTimeZone = new CDateTimeZoneSelect;
	mOccursDateTimeZoneItem.AddPanel(mOccursDateTimeZone);
	mOccursDateTimeZoneItem.SetPanel(0);
	
	return true;
}

void CNewComponentRepeatSimple::OnEnable(BOOL bEnable)
{
	mOccursInterval.EnableWindow(bEnable);
	mOccursIntervalSpin.EnableWindow(bEnable);
	mOccursFreq.EnableWindow(bEnable);

	mOccursForEver.EnableWindow(bEnable);
	mOccursCount.EnableWindow(bEnable);
	mOccursUntil.EnableWindow(bEnable);

	mOccursCounter.EnableWindow(bEnable && (mOccursCount.GetCheck() == 1));
	mOccursCounterSpin.EnableWindow(bEnable && (mOccursCount.GetCheck() == 1));

	mOccursDateTimeZone->EnableWindow(bEnable && (mOccursUntil.GetCheck() == 1));
}

void CNewComponentRepeatSimple::OnOccursForEver()
{
	mTimingPanel->DoOccursGroup(CNewComponentRepeat::eOccurs_ForEver);
}

void CNewComponentRepeatSimple::OnOccursCount()
{
	mTimingPanel->DoOccursGroup(CNewComponentRepeat::eOccurs_Count);
}

void CNewComponentRepeatSimple::OnOccursUntil()
{
	mTimingPanel->DoOccursGroup(CNewComponentRepeat::eOccurs_Until);
}

#pragma mark ______________________________CNewComponentRepeatAdvanced

// ---------------------------------------------------------------------------
//	CNewComponentRepeatAdvanced														  [public]
/**
	Default constructor */

CNewComponentRepeatAdvanced::CNewComponentRepeatAdvanced(CNewComponentRepeat* timing) :
	CTabPanel(IDD_CALENDAR_REPEAT_ADVANCED)
{
	mTimingPanel = timing;
}


// ---------------------------------------------------------------------------
//	~CNewComponentRepeatAdvanced														  [public]
/**
	Destructor */

CNewComponentRepeatAdvanced::~CNewComponentRepeatAdvanced()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentRepeatAdvanced, CTabPanel)
	//{{AFX_MSG_MAP(CNewComponentRepeatAdvanced)
	ON_WM_ENABLE()

	ON_COMMAND(IDC_CALENDAR_REPEAT_EDIT, OnOccursEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentRepeatAdvanced::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Get UI items
	mOccursDescription.SubclassDlgItem(IDC_CALENDAR_REPEAT_DESCRIPTION, this);
	mOccursEdit.SubclassDlgItem(IDC_CALENDAR_REPEAT_EDIT, this);
	
	return true;
}

void CNewComponentRepeatAdvanced::OnEnable(BOOL bEnable)
{
	mOccursEdit.EnableWindow(bEnable);
}

void CNewComponentRepeatAdvanced::OnOccursEdit()
{
	mTimingPanel->DoOccursEdit();
}

#pragma mark ______________________________CNewComponentRepeatComplex

// ---------------------------------------------------------------------------
//	CNewComponentRepeatComplex														  [public]
/**
	Default constructor */

CNewComponentRepeatComplex::CNewComponentRepeatComplex() :
	CTabPanel(IDD_CALENDAR_REPEAT_COMPLEX)
{
}


// ---------------------------------------------------------------------------
//	~CNewComponentRepeatComplex														  [public]
/**
	Destructor */

CNewComponentRepeatComplex::~CNewComponentRepeatComplex()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentRepeatComplex, CTabPanel)
	//{{AFX_MSG_MAP(CNewComponentRepeatComplex)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentRepeatComplex::OnInitDialog()
{
	CTabPanel::OnInitDialog();

	// Get UI items
	mOccursDescription.SubclassDlgItem(IDC_CALENDAR_REPEAT_DESCRIPTION, this);
	
	return true;
}
