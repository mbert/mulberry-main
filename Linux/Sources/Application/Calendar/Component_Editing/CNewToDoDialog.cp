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
#include "CTabController.h"
#include "CTextField.h"
#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarLocale.h"
#include "CITIPProcessor.h"

#include "TPopupMenu.h"

#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

// ---------------------------------------------------------------------------
//	CNewToDoDialog														  [public]
/**
	Default constructor */

CNewToDoDialog::CNewToDoDialog(JXDirector* supervisor)
		: CNewComponentDialog(supervisor)
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

void CNewToDoDialog::OnCreate()
{
	// Get UI items
	CNewComponentDialog::OnCreate();

	// begin JXLayout1

    mCompleted =
        new JXTextCheckbox("Completed", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 5,40, 90,20);
    assert( mCompleted != NULL );

    mCompletedDateTimeZone =
        new CDateTimeZoneSelect(mContainer,
                    JXWidget::kHElastic, JXWidget::kVElastic, 94,35, 400,30);
    assert( mCompletedDateTimeZone != NULL );

    mCompletedNow =
        new JXTextButton("Now", mContainer,
                    JXWidget::kHElastic, JXWidget::kVElastic, 495,40, 35,20);
    assert( mCompletedNow != NULL );
    mCompletedNow->SetFontSize(10);

// end JXLayout1

    ListenTo(mCompleted);
    ListenTo(mCompletedNow);
}

void CNewToDoDialog::InitPanels()
{
	// Load each panel for the tabs
	mPanels.push_back(new CNewComponentDetails(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 500,300));
	mTabs->AppendCard(mPanels.back(), "Details");

	mPanels.push_back(new CNewComponentRepeat(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 500,300));
	mTabs->AppendCard(mPanels.back(), "Repeat");

	mPanels.push_back(new CNewComponentAlarm(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 500,300));
	mTabs->AppendCard(mPanels.back(), "Alarms");

	mPanels.push_back(new CNewComponentAttendees(mTabs->GetCardEnclosure(), JXWidget::kFixedLeft, JXWidget::kFixedTop, 0,0, 500,300));
	mTabs->AppendCard(mPanels.back(), "Attendees");
}

void CNewToDoDialog::ListenTo_Message(long msg, void* param)
{
	switch(msg)
	{
	case iCal::CICalendar::eBroadcast_Closed:
		// Force dialog to close immediately as event is about to be deleted.
		// Any changes so far will be lost.
		OnCancel();
		break;
	default:;
	}
}

// Handle controls
void CNewToDoDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXTextButton::kPushed))
	{
		if (sender == mCompletedNow)
		{
			DoNow();
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mCompleted)
		{
			DoCompleted(mCompleted->IsChecked());
			return;
		}
	}

	CNewComponentDialog::Receive(sender, message);
}

void CNewToDoDialog::DoCompleted(bool set)
{
	if (set && !mCompletedExists)
	{
		mActualCompleted.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
		mActualCompleted.SetNow();
		mCompletedExists = true;
		mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
	}

	mCompletedDateTimeZone->SetVisible(set);
	mCompletedNow->SetVisible(set);
}

void CNewToDoDialog::DoNow()
{
	mActualCompleted.SetTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	mActualCompleted.SetNow();
	mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
}

void CNewToDoDialog::SetComponent(iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded)
{
	CNewComponentDialog::SetComponent(vcomponent, expanded);

	mCompleted->SetState(static_cast<iCal::CICalendarVToDo&>(vcomponent).GetStatus() == iCal::eStatus_VToDo_Completed);

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

	static_cast<iCal::CICalendarVToDo&>(vcomponent).EditStatus(mCompleted->IsChecked() ? iCal::eStatus_VToDo_Completed : iCal::eStatus_VToDo_NeedsAction);
	
	// Changed completed date if needed
	mCompletedDateTimeZone->GetDateTimeZone(mActualCompleted, false);
	if (mCompleted->IsChecked() && (static_cast<iCal::CICalendarVToDo&>(vcomponent).GetCompleted() != mActualCompleted))
	{
		// Adjust to UTC and then change
		mActualCompleted.AdjustToUTC();
		static_cast<iCal::CICalendarVToDo&>(vcomponent).EditCompleted(mActualCompleted);
	}
}

void CNewToDoDialog::SetReadOnly(bool read_only)
{
	CNewComponentDialog::SetReadOnly(read_only);

	mCompleted->SetActive(!mReadOnly);
	mCompletedDateTimeZone->SetActive(!mReadOnly);
	mCompletedNow->SetActive(!mReadOnly);
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
		mOKBtn->SetActive(false);
		
		return false;
	}
	
	// Find the original to do if it still exists
	iCal::CICalendarVToDo*	original = static_cast<iCal::CICalendarVToDo*>(old_cal->FindComponent(mComponent));
	if (original == NULL)
	{
		// Inform user of missing calendar
		CErrorDialog::StopAlert(rsrc::GetString("CNewToDoDialog::MissingOriginal"));
		
		// Disable OK button and return to dialog
		mOKBtn->SetActive(false);
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
	// Look for an existinf dialog for this event
	for(std::set<CNewComponentDialog*>::const_iterator iter = sDialogs.begin(); iter != sDialogs.end(); iter++)
	{
		if ((*iter)->ContainsComponent(original))
		{
			(*iter)->GetWindow()->Raise();
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
	CNewToDoDialog* dlog = new CNewToDoDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetAction(action);
	dlog->SetComponent(vtodo, expanded);
	dlog->Activate();
}
