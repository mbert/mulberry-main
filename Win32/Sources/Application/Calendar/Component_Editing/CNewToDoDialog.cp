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

#include "CNewToDoDialog.h"

#include "CICalendar.h"
#include "CICalendarManager.h"

#include "CCalendarPopup.h"
#include "CCalendarView.h"
#include "CDateTimeZoneSelect.h"
#include "CErrorDialog.h"
#include "CMulberryApp.h"
#include "CNewComponentAlarm.h"
#include "CNewComponentAttendees.h"
#include "CNewComponentDetails.h"
#include "CNewComponentRepeat.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CITIPProcessor.h"

#include "CICalendarLocale.h"

// ---------------------------------------------------------------------------
//	CNewToDoDialog														  [public]
/**
	Default constructor */

CNewToDoDialog::CNewToDoDialog()
	: CNewComponentDialog()
{
}


// ---------------------------------------------------------------------------
//	~CNewToDoDialog														  [public]
/**
	Destructor */

CNewToDoDialog::~CNewToDoDialog()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewToDoDialog, CModelessDialog)
	//{{AFX_MSG_MAP(CNewToDoDialog)
	ON_COMMAND(IDC_CALENDAR_NEWTODO_COMPLETED, OnCompleted)
	ON_COMMAND(IDC_CALENDAR_NEWTODO_COMPLETEDNOW, OnCompletedNow)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewToDoDialog::OnInitDialog()
{
	CNewComponentDialog::OnInitDialog();

	// Get UI items
	mCompleted.SubclassDlgItem(IDC_CALENDAR_NEWTODO_COMPLETED, this);
	mCompletedDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEWTODO_COMPLETEDON, this);
	mCompletedDateTimeZone = new CDateTimeZoneSelect;
	mCompletedDateTimeZoneItem.AddPanel(mCompletedDateTimeZone);
	mCompletedDateTimeZoneItem.SetPanel(0);
	mCompletedDateTimeZone->EnableWindow(false);
	mCompletedDateTimeZone->ShowWindow(SW_HIDE);
	mCompletedNow.SubclassDlgItem(IDC_CALENDAR_NEWTODO_COMPLETEDNOW, this);
	mCompletedNow.ShowWindow(SW_HIDE);

	// Init controls
	InitPanels();
	mTabs.SetPanel(0);
	
	return true;
}

void CNewToDoDialog::InitPanels()
{
	// Load each panel for the tabs
	mPanels.push_back(new CNewComponentDetails);
	mTabs.AddPanel(mPanels.back());

	mPanels.push_back(new CNewComponentRepeat);
	mTabs.AddPanel(mPanels.back());

	mPanels.push_back(new CNewComponentAlarm);
	mTabs.AddPanel(mPanels.back());

	mPanels.push_back(new CNewComponentAttendees);
	mTabs.AddPanel(mPanels.back());
}

void CNewToDoDialog::OnCompleted()
{
	DoCompleted(mCompleted.GetCheck() == 1);
}

void CNewToDoDialog::DoCompleted(bool set)
{
	// Force setting of completed state
	if (set && !mCompletedExists)
	{
		mActualCompleted.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
		mActualCompleted.SetNow();
		mCompletedExists = true;
		mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
	}

	mCompletedDateTimeZone->ShowWindow(set ? SW_SHOW : SW_HIDE);
	mCompletedNow.ShowWindow(set ? SW_SHOW : SW_HIDE);
}

void CNewToDoDialog::OnCompletedNow()
{
	mActualCompleted.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	mActualCompleted.SetNow();
	mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
}

void CNewToDoDialog::SetComponent(iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded)
{
	CNewComponentDialog::SetComponent(vcomponent, expanded);

	mCompleted.SetCheck(static_cast<iCal::CICalendarVToDo&>(vcomponent).GetStatus() == iCal::eStatus_VToDo_Completed);

	mCompletedExists = static_cast<iCal::CICalendarVToDo&>(vcomponent).HasCompleted();
	if (mCompletedExists)
	{
		// COMPLETED is in UTC but we adjust to local timezone
		mActualCompleted = static_cast<iCal::CICalendarVToDo&>(vcomponent).GetCompleted();
		mActualCompleted.AdjustTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	
		mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
	}
	DoCompleted(static_cast<iCal::CICalendarVToDo&>(vcomponent).GetStatus() == iCal::eStatus_VToDo_Completed);
}

void CNewToDoDialog::GetComponent(iCal::CICalendarComponentRecur& vcomponent)
{
	CNewComponentDialog::GetComponent(vcomponent);

	// Changed completed date if needed
	mCompletedDateTimeZone->GetDateTimeZone(mActualCompleted, false);
	if ((mCompleted.GetCheck() == 1) && (static_cast<iCal::CICalendarVToDo&>(vcomponent).GetCompleted() != mActualCompleted))
	{
		// Adjust to UTC and then change
		mActualCompleted.AdjustToUTC();
		static_cast<iCal::CICalendarVToDo&>(vcomponent).EditCompleted(mActualCompleted);
	}
}

void CNewToDoDialog::SetReadOnly(bool read_only)
{
	CNewComponentDialog::SetReadOnly(read_only);

	mCompleted.EnableWindow(!mReadOnly);
	mCompletedDateTimeZone->EnableWindow(!mReadOnly);
	mCompletedNow.EnableWindow(!mReadOnly);
}

void CNewToDoDialog::ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status)
{
	static_cast<CNewComponentAttendees*>(mPanels.back())->ChangedMyStatus(attendee, new_status);
}

bool CNewToDoDialog::DoNewOK()
{
	// Check and get new calendar if different
	iCal::CICalendarRef newcal;
	iCal::CICalendar* new_cal = NULL;
	if (!GetCalendar(0, newcal, new_cal))
		// Return to dialog
		return false;

	// Get updated info
	GetComponent(*mComponent);
	
	// Look for change to calendar
	if (newcal != mComponent->GetCalendar())
	{
		// Use new calendar
		mComponent->SetCalendar(newcal);
		
		// Set the default calendar for next time
		const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(new_cal);
		if (node != NULL)
			CPreferences::sPrefs->mDefaultCalendar.SetValue(node->GetAccountName());
	}

	// Add to calendar (this will do the display update)
	//iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVToDo->GetCalendar());
	new_cal->AddNewVToDo(static_cast<iCal::CICalendarVToDo*>(mComponent));
	CCalendarView::ToDosChangedAll();
	
	return true;
}

bool CNewToDoDialog::DoEditOK()
{
	// Find the original calendar if it still exists
	iCal::CICalendarRef oldcal = mComponent->GetCalendar();
	iCal::CICalendar* old_cal = iCal::CICalendar::GetICalendar(oldcal);
	if (old_cal == NULL)
	{
		// Inform user of missing calendar
		CErrorDialog::StopAlert(rsrc::GetString("CNewToDoDialog::MissingOriginal"));
		
		// Disable OK button
		GetDlgItem(IDOK)->EnableWindow(false);
		
		return false;
	}
	
	// Find the original to do if it still exists
	iCal::CICalendarVToDo*	original = static_cast<iCal::CICalendarVToDo*>(old_cal->FindComponent(mComponent));
	if (original == NULL)
	{
		// Inform user of missing calendar
		CErrorDialog::StopAlert(rsrc::GetString("CNewToDoDialog::MissingOriginal"));
		
		// Disable OK button and return to dialog
		GetDlgItem(IDOK)->EnableWindow(false);
		return false;
	}

	// Check and get new calendar if different
	iCal::CICalendarRef newcal;
	iCal::CICalendar* new_cal = NULL;
	if (!GetCalendar(oldcal, newcal, new_cal))
		// Return to dialog
		return false;

	// Get updated info
	GetComponent(*original);
	
	// Look for change to calendar
	if (new_cal != NULL)
	{
		// Remove from old calendar (without deleting)
		old_cal->RemoveVToDo(original, false);
		
		// Add to new calendar (without initialising)
		original->SetCalendar(newcal);
		new_cal->AddNewVToDo(original, true);
	}
	
	// Tell it it has changed (i.e. bump sequence, dirty calendar)
	original->Changed();

	CCalendarView::ToDosChangedAll();
	
	return true;
}

void CNewToDoDialog::DoCancel()
{
	// Delete the to do which we own and is not going to be used
	delete mComponent;
	mComponent = NULL;
}

void CNewToDoDialog::StartNew(const iCal::CICalendar* calin)
{
	const iCal::CICalendarList& cals = calstore::CCalendarStoreManager::sCalendarStoreManager->GetActiveCalendars();
	if (cals.size() == 0)
		return;

	const iCal::CICalendar* cal = calin;
	if (cal == NULL)
	{
		// Default calendar is the first one
		cal = cals.front();

		// Check prefs for default calendar
		const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(CPreferences::sPrefs->mDefaultCalendar.GetValue());
		if ((node != NULL) && (node->GetCalendar() != NULL))
			cal = node->GetCalendar();
	}

	// Start with an empty to do
	iCal::CICalendarVToDo* vtodo = static_cast<iCal::CICalendarVToDo*>(iCal::CICalendarVToDo::Create(cal->GetRef()));
	
	// Set event with initial timing
	vtodo->EditTiming();

	StartModeless(*vtodo, NULL, CNewToDoDialog::eNew);
}

void CNewToDoDialog::StartEdit(const iCal::CICalendarVToDo& original, const iCal::CICalendarComponentExpanded* expanded)
{
	// Look for an existing dialog for this event
	for(std::set<CNewComponentDialog*>::const_iterator iter = sDialogs.begin(); iter != sDialogs.end(); iter++)
	{
		if ((*iter)->ContainsComponent(original))
		{
			FRAMEWORK_WINDOW_TO_TOP(*iter)
			return;
		}
	}

	// Use a copy of the event
	iCal::CICalendarVToDo* vtodo = new iCal::CICalendarVToDo(original);
	
	StartModeless(*vtodo, expanded, CNewToDoDialog::eEdit);
}

void CNewToDoDialog::StartDuplicate(const iCal::CICalendarVToDo& original)
{
	// Start with an empty new event
	iCal::CICalendarVToDo* vtodo = new iCal::CICalendarVToDo(original);
	vtodo->Duplicated();
	
	StartModeless(*vtodo, NULL, CNewToDoDialog::eDuplicate);
}

void CNewToDoDialog::StartModeless(iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded, EModelessAction action)
{
	CNewToDoDialog* dlog = new CNewToDoDialog;
	dlog->Create(IDD_CALENDAR_NEWTODO, CSDIFrame::GetAppTopWindow());
	dlog->SetAction(action);
	dlog->SetComponent(vtodo, expanded);
	dlog->ShowWindow(SW_SHOW);
}
