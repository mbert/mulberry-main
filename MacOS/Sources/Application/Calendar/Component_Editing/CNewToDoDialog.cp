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

#include "CCalendarView.h"
#include "CDateTimeZoneSelect.h"
#include "CErrorDialog.h"
#include "CMulberryApp.h"
#include "CNewComponentAlarm.h"
#include "CNewComponentAttendees.h"
#include "CNewComponentDetails.h"
#include "CNewComponentRepeat.h"
#include "CNewComponentDetails.h"
#include "CPreferences.h"
//#include "CStaticText.h"
//#include "CTextFieldX.h"
//#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CICalendarLocale.h"
//#include "CITIPProcessor.h"

//#include "MyCFString.h"

#include <LBevelButton.h>
#include <LCheckBox.h>
#include <LTabsControl.h>

// ---------------------------------------------------------------------------
//	CNewToDoDialog														  [public]
/**
	Default constructor */

CNewToDoDialog::CNewToDoDialog(LStream *inStream)
 : CNewComponentDialog(inStream)
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

void CNewToDoDialog::FinishCreateSelf()
{
	CNewComponentDialog::FinishCreateSelf();

	// Get UI items
	mCompleted = dynamic_cast<LCheckBox*>(FindPaneByID(eCompleted_ID));
	mCompletedDateTimeZone = CDateTimeZoneSelect::CreateInside(dynamic_cast<LView*>(FindPaneByID(eCompletedDateTimeZone_ID)));
	mCompletedDateTimeZone->SetEnabled(false);
	mCompletedDateTimeZone->SetVisible(false);
	mCompletedNow = dynamic_cast<LBevelButton*>(FindPaneByID(eCompletedNow_ID));
	mCompletedNow->SetVisible(false);

	// Listen to some of them
	UReanimator::LinkListenerToBroadcasters(this, this, pane_ID);
}

void CNewToDoDialog::InitPanels()
{
	// Use tab group as default commander
	LCommander* defCommander;
	GetSubCommanders().FetchItemAt(1, defCommander);

	// Load each panel for the tabs
	mPanels.push_back(static_cast<CNewComponentDetails*>(UReanimator::CreateView(CNewComponentDetails::pane_ID, mTabView, defCommander)));
	mTabView->ExpandSubPane(mPanels.back(), true, true);
	mPanels.back()->SetVisible(false);

	mPanels.push_back(static_cast<CNewComponentRepeat*>(UReanimator::CreateView(CNewComponentRepeat::pane_ID, mTabView, defCommander)));
	mTabView->ExpandSubPane(mPanels.back(), true, true);
	mPanels.back()->SetVisible(false);

	mPanels.push_back(static_cast<CNewComponentAlarm*>(UReanimator::CreateView(CNewComponentAlarm::pane_ID, mTabView, defCommander)));
	mTabView->ExpandSubPane(mPanels.back(), true, true);
	mPanels.back()->SetVisible(false);

	mPanels.push_back(static_cast<CNewComponentAttendees*>(UReanimator::CreateView(CNewComponentAttendees::pane_ID, mTabView, defCommander)));
	mTabView->ExpandSubPane(mPanels.back(), true, true);
	mPanels.back()->SetVisible(false);
}

// Handle controls
void CNewToDoDialog::ListenToMessage(
	MessageT	inMessage,
	void		*ioParam)
{
	switch (inMessage)
	{
	case eCompleted_ID:
		DoCompleted(mCompleted->GetValue() == 1);
		break;

	case eCompletedNow_ID:
		DoNow();
		break;

	default:
		CNewComponentDialog::ListenToMessage(inMessage, ioParam);
		break;
	}
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

	mCompleted->SetValue(static_cast<iCal::CICalendarVToDo&>(vcomponent).GetStatus() == iCal::eStatus_VToDo_Completed);

	mCompletedExists = static_cast<iCal::CICalendarVToDo&>(vcomponent).HasCompleted();
	if (mCompletedExists)
	{
		// COMPLETED is in UTC but we adjust to local timezone
		mActualCompleted = static_cast<iCal::CICalendarVToDo&>(vcomponent).GetCompleted();
		mActualCompleted.AdjustTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	
		mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
	}
}

void CNewToDoDialog::GetComponent(iCal::CICalendarComponentRecur& vcomponent)
{
	CNewComponentDialog::GetComponent(vcomponent);

	// Do descriptive items
	static_cast<iCal::CICalendarVToDo&>(vcomponent).EditStatus(mCompleted->GetValue() == 1 ? iCal::eStatus_VToDo_Completed : iCal::eStatus_VToDo_NeedsAction);
	
	// Changed completed date if needed
	mCompletedDateTimeZone->GetDateTimeZone(mActualCompleted, false);
	if ((mCompleted->GetValue() == 1) && (static_cast<iCal::CICalendarVToDo&>(vcomponent).GetCompleted() != mActualCompleted))
	{
		// Adjust to UTC and then change
		mActualCompleted.AdjustToUTC();
		static_cast<iCal::CICalendarVToDo&>(vcomponent).EditCompleted(mActualCompleted);
	}
}

void CNewToDoDialog::SetReadOnly(bool read_only)
{
	CNewComponentDialog::SetReadOnly(read_only);
	
	mCompleted->SetEnabled(!mReadOnly);
	mCompletedDateTimeZone->SetEnabled(!mReadOnly);
	mCompletedNow->SetEnabled(!mReadOnly);
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
	//iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mComponent->GetCalendar());
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
		FindPaneByID(eOK_ID)->Disable();
		
		return false;
	}
	
	// Find the original to do if it still exists
	iCal::CICalendarVToDo*	original = static_cast<iCal::CICalendarVToDo*>(old_cal->FindComponent(mComponent));
	if (original == NULL)
	{
		// Inform user of missing calendar
		CErrorDialog::StopAlert(rsrc::GetString("CNewToDoDialog::MissingOriginal"));
		
		// Disable OK button and return to dialog
		FindPaneByID(eOK_ID)->Disable();
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
			(*iter)->Select();
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
	CNewToDoDialog* dlog = static_cast<CNewToDoDialog*>(LWindow::CreateWindow(pane_ID, CMulberryApp::sApp));
	dlog->SetAction(action);
	dlog->SetComponent(vtodo, expanded);
	dlog->Show();
}
