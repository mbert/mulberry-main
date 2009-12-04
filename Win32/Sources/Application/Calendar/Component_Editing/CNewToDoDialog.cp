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
#include "CNewComponentDescription.h"
#include "CNewComponentTiming.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"
#include "CXStringResources.h"

#include "CCalendarStoreManager.h"
#include "CCalendarStoreNode.h"
#include "CITIPProcessor.h"

#include "CICalendarLocale.h"

uint32_t CNewToDoDialog::sTitleCounter = 0;
set<CNewToDoDialog*> CNewToDoDialog::sDialogs;

// ---------------------------------------------------------------------------
//	CNewToDoDialog														  [public]
/**
	Default constructor */

CNewToDoDialog::CNewToDoDialog()
	: CModelessDialog()
{
	sDialogs.insert(this);
}


// ---------------------------------------------------------------------------
//	~CNewToDoDialog														  [public]
/**
	Destructor */

CNewToDoDialog::~CNewToDoDialog()
{
	sDialogs.erase(this);
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewToDoDialog, CModelessDialog)
	//{{AFX_MSG_MAP(CNewToDoDialog)
	ON_COMMAND(IDC_CALENDAR_NEWTODO_SUMMARY, OnChangeSummary)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_CALENDARPOPUP, OnChangeCalendar)
	ON_COMMAND(IDC_CALENDAR_NEWTODO_COMPLETED, OnCompleted)
	ON_COMMAND(IDC_CALENDAR_NEWTODO_COMPLETEDNOW, OnCompletedNow)
	ON_COMMAND(IDC_CALENDAR_NEWTODO_ORGANISEREDIT, OnOrganiserEdit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CALENDAR_NEWTODO_TABS, OnSelChangeTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewToDoDialog::OnInitDialog()
{
	CModelessDialog::OnInitDialog();

	// Get UI items
	mSummary.SubclassDlgItem(IDC_CALENDAR_NEWTODO_SUMMARY, this);
	mSummary.SetAnyCmd(true);

	mCalendar.SubclassDlgItem(IDC_CALENDAR_NEWTODO_CALENDARPOPUP, this);
	mCompleted.SubclassDlgItem(IDC_CALENDAR_NEWTODO_COMPLETED, this);
	mCompletedDateTimeZoneItem.SubclassDlgItem(IDC_CALENDAR_NEWTODO_COMPLETEDON, this);
	mCompletedDateTimeZone = new CDateTimeZoneSelect;
	mCompletedDateTimeZoneItem.AddPanel(mCompletedDateTimeZone);
	mCompletedDateTimeZoneItem.SetPanel(0);
	mCompletedDateTimeZone->EnableWindow(false);
	mCompletedDateTimeZone->ShowWindow(SW_HIDE);
	mCompletedNow.SubclassDlgItem(IDC_CALENDAR_NEWTODO_COMPLETEDNOW, this);
	mCompletedNow.ShowWindow(SW_HIDE);

	mTabs.SubclassDlgItem(IDC_CALENDAR_NEWTODO_TABS, this);
	
	mOrganiserEdit.SubclassDlgItem(IDC_CALENDAR_NEWTODO_ORGANISEREDIT, this);

	// Init controls
	InitPanels();
	mTabs.SetPanel(0);
	
	return true;
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

void CNewToDoDialog::InitPanels()
{
	// Load each panel for the tabs
	mPanels.push_back(new CNewComponentTiming);
	mTabs.AddPanel(mPanels.back());

	mPanels.push_back(new CNewComponentDescription);
	mTabs.AddPanel(mPanels.back());

	mPanels.push_back(new CNewComponentAlarm);
	mTabs.AddPanel(mPanels.back());

	mPanels.push_back(new CNewComponentAttendees);
	mTabs.AddPanel(mPanels.back());
}

void CNewToDoDialog::OnChangeSummary()
{
	ChangedSummary();
}

void CNewToDoDialog::OnChangeCalendar()
{
	ChangedCalendar();
}

void CNewToDoDialog::OnCompleted()
{
	DoCompleted(mCompleted.GetCheck() == 1);
}

void CNewToDoDialog::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
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

bool CNewToDoDialog::ContainsToDo(const iCal::CICalendarVToDo& vtodo) const
{
	// Does this dialog contain a copy of this event
	return vtodo.GetMapKey() == mVToDo->GetMapKey();
}

void CNewToDoDialog::SetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Listen to original calendar as we need to cancel if calendar goes away
	iCal::CICalendarRef calref = vtodo.GetCalendar();
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
	if (cal)
		cal->Add_Listener(this);
	
	mVToDo = &vtodo;

	// Set the relevant fields
	
	mSummary.SetText(vtodo.GetSummary());

	mCalendar.SetCalendar(vtodo.GetCalendar());
	ChangedCalendar();

	mCompleted.SetCheck(vtodo.GetStatus() == iCal::eStatus_VToDo_Completed);

	mCompletedExists = vtodo.HasCompleted();
	if (mCompletedExists)
	{
		// COMPLETED is in UTC but we adjust to local timezone
		mActualCompleted = vtodo.GetCompleted();
		mActualCompleted.AdjustTimezone(iCal::CICalendarManager::sICalendarManager->GetDefaultTimezone());
	
		mCompletedDateTimeZone->SetDateTimeZone(mActualCompleted, false);
	}
	DoCompleted(vtodo.GetStatus() == iCal::eStatus_VToDo_Completed);

	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetToDo(vtodo);
	}
	
	// Set title to summary if not empty
	if (!vtodo.GetSummary().empty())
	{
		CUnicodeUtils::SetWindowTextUTF8(*this, vtodo.GetSummary());
	}

	// Determine read-only status based on organiser
	SetReadOnly((vtodo.CountProperty(iCal::cICalProperty_ORGANIZER) != 0) && !iCal::CITIPProcessor::OrganiserIsMe(vtodo));
	mOrganiserEdit.ShowWindow(mReadOnly ? SW_SHOW : SW_HIDE);
}

void CNewToDoDialog::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do descriptive items
	vtodo.EditSummary(mSummary.GetText());

	vtodo.EditStatus(mCompleted.GetCheck() == 1 ? iCal::eStatus_VToDo_Completed : iCal::eStatus_VToDo_NeedsAction);
	
	// Changed completed date if needed
	mCompletedDateTimeZone->GetDateTimeZone(mActualCompleted, false);
	if ((mCompleted.GetCheck() == 1) && (vtodo.GetCompleted() != mActualCompleted))
	{
		// Adjust to UTC and then change
		mActualCompleted.AdjustToUTC();
		vtodo.EditCompleted(mActualCompleted);
	}

	// Get from each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->GetToDo(vtodo);
	}
}

void CNewToDoDialog::ChangedSummary()
{
	CUnicodeUtils::SetWindowTextUTF8(this, mSummary.GetText());
}

void CNewToDoDialog::ChangedCalendar()
{
	iCal::CICalendarRef newcal;
	mCalendar.GetCalendar(newcal);
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(newcal);
	GetDlgItem(IDOK)->EnableWindow(!mReadOnly && (cal != NULL) && (!cal->IsReadOnly()));
}

void CNewToDoDialog::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	// This will reset state of OK button
	ChangedCalendar();

	mSummary.SetReadOnly(mReadOnly);
	mCalendar.EnableWindow(!mReadOnly);
	mCompleted.EnableWindow(!mReadOnly);
	mCompletedDateTimeZone->EnableWindow(!mReadOnly);
	mCompletedNow.EnableWindow(!mReadOnly);

	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetReadOnly(mReadOnly);
	}
}

cdstring CNewToDoDialog::GetCurrentSummary() const
{
	return mSummary.GetText();
}

void CNewToDoDialog::OnOK()
{
	bool result = true;
	switch(mAction)
	{
	case eNew:
	case eDuplicate:
		result = DoNewOK();
		break;
	case eEdit:
		result = DoEditOK();
		break;
	default:;
	}
	
	// Now do inherited if result was OK
	if (result)
		CModelessDialog::OnOK();
}

void CNewToDoDialog::OnCancel()
{
	DoCancel();
	
	// Now do inherited
	CModelessDialog::OnCancel();
}

void CNewToDoDialog::OnOrganiserEdit()
{
	SetReadOnly(mOrganiserEdit.GetCheck() == 0);
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
	GetToDo(*mVToDo);
	
	// Look for change to calendar
	if (newcal != mVToDo->GetCalendar())
	{
		// Use new calendar
		mVToDo->SetCalendar(newcal);
		
		// Set the default calendar for next time
		const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(new_cal);
		if (node != NULL)
			CPreferences::sPrefs->mDefaultCalendar.SetValue(node->GetAccountName());
	}

	// Add to calendar (this will do the display update)
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(mVToDo->GetCalendar());
	new_cal->AddNewVToDo(mVToDo);
	CCalendarView::ToDosChangedAll();
	
	return true;
}

bool CNewToDoDialog::DoEditOK()
{
	// Find the original calendar if it still exists
	iCal::CICalendarRef oldcal = mVToDo->GetCalendar();
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
	iCal::CICalendarVToDo*	original = static_cast<iCal::CICalendarVToDo*>(old_cal->FindComponent(mVToDo));
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
	GetToDo(*original);
	
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
	delete mVToDo;
	mVToDo = NULL;
}

bool CNewToDoDialog::GetCalendar(iCal::CICalendarRef oldcal, iCal::CICalendarRef& newcal, iCal::CICalendar*& new_cal)
{
	mCalendar.GetCalendar(newcal);
	if ((oldcal == 0) || (newcal != oldcal))
	{
		new_cal = iCal::CICalendar::GetICalendar(newcal);
		if (new_cal == NULL)
		{
			// Inform user of missing calendar
			CErrorDialog::StopAlert(rsrc::GetString("CNewToDoDialog::MissingNewCalendar"));
			
			// Force calendar popup reset and return to dialog
			mCalendar.Reset();
			mCalendar.SetCalendar(oldcal);
			return false;
		}
	}
	
	return true;
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

	StartModeless(*vtodo, CNewToDoDialog::eNew);
}

void CNewToDoDialog::StartEdit(const iCal::CICalendarVToDo& original)
{
	// Look for an existinf dialog for this event
	for(set<CNewToDoDialog*>::const_iterator iter = sDialogs.begin(); iter != sDialogs.end(); iter++)
	{
		if ((*iter)->ContainsToDo(original))
		{
			FRAMEWORK_WINDOW_TO_TOP(*iter)
			return;
		}
	}

	// Use a copy of the event
	iCal::CICalendarVToDo* vtodo = new iCal::CICalendarVToDo(original);
	
	StartModeless(*vtodo, CNewToDoDialog::eEdit);
}

void CNewToDoDialog::StartDuplicate(const iCal::CICalendarVToDo& original)
{
	// Start with an empty new event
	iCal::CICalendarVToDo* vtodo = new iCal::CICalendarVToDo(original);
	vtodo->Duplicated();
	
	StartModeless(*vtodo, CNewToDoDialog::eDuplicate);
}

void CNewToDoDialog::StartModeless(iCal::CICalendarVToDo& vtodo, EModelessAction action)
{
	CNewToDoDialog* dlog = new CNewToDoDialog;
	dlog->Create(IDD_CALENDAR_NEWTODO, CSDIFrame::GetAppTopWindow());
	dlog->SetAction(action);
	dlog->SetToDo(vtodo);
	dlog->ShowWindow(SW_SHOW);
}
