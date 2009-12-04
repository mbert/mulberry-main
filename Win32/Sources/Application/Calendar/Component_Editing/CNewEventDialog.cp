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

#include "CNewEventDialog.h"

#include "CCalendarPopup.h"
#include "CCalendarView.h"
#include "CErrorDialog.h"
#include "CICalendar.h"
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

uint32_t CNewEventDialog::sTitleCounter = 0;
set<CNewEventDialog*> CNewEventDialog::sDialogs;

// ---------------------------------------------------------------------------
//	CNewEventDialog														  [public]
/**
	Default constructor */

CNewEventDialog::CNewEventDialog() :
	CModelessDialog(),
	mStatus(true)	
{
	mReadOnly = false;
	sDialogs.insert(this);
}


// ---------------------------------------------------------------------------
//	~CNewEventDialog														  [public]
/**
	Destructor */

CNewEventDialog::~CNewEventDialog()
{
	sDialogs.erase(this);
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewEventDialog, CModelessDialog)
	//{{AFX_MSG_MAP(CNewEventDialog)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_SUMMARY, OnChangeSummary)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_CALENDARPOPUP, OnChangeCalendar)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_ORGANISEREDIT, OnOrganiserEdit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CALENDAR_NEWEVENT_TABS, OnSelChangeTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewEventDialog::OnInitDialog()
{
	CModelessDialog::OnInitDialog();

	// Get UI items
	mSummary.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_SUMMARY, this);
	mSummary.SetAnyCmd(true);

	mCalendar.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_CALENDARPOPUP, this);
	mStatus.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_STATUS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mStatus.SetMenu(IDR_NEWEVENT_STATUS_POPUP);

	mTabs.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_TABS, this);
	
	mOrganiserEdit.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_ORGANISEREDIT, this);
	
	// Init controls
	InitPanels();
	mTabs.SetPanel(0);

	return true;
}

void CNewEventDialog::ListenTo_Message(long msg, void* param)
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

void CNewEventDialog::InitPanels()
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

void CNewEventDialog::OnChangeSummary()
{
	ChangedSummary();
}

void CNewEventDialog::OnChangeCalendar()
{
	ChangedCalendar();
}

void CNewEventDialog::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}

bool CNewEventDialog::ContainsEvent(const iCal::CICalendarVEvent& vevent) const
{
	// Does this dialog contain a copy of this event
	return vevent.GetMapKey() == mVEvent->GetMapKey();
}

void CNewEventDialog::SetEvent(iCal::CICalendarVEvent& vevent)
{
	// Listen to original calendar as we need to cancel if calendar goes away
	iCal::CICalendarRef calref = vevent.GetCalendar();
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
	if (cal)
		cal->Add_Listener(this);
	
	mVEvent = &vevent;

	// Set the relevant fields
	
	mSummary.SetText(vevent.GetSummary());

	mCalendar.SetCalendar(vevent.GetCalendar());
	ChangedCalendar();

	mStatus.SetValue(vevent.GetStatus() + IDM_NEWEVENT_STATUS_NONE);

	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetEvent(vevent);
	}
	
	// Set title to summary if not empty
	if (!vevent.GetSummary().empty())
	{
		CUnicodeUtils::SetWindowTextUTF8(this, vevent.GetSummary());
	}

	// Determine read-only status based on organiser
	SetReadOnly((vevent.CountProperty(iCal::cICalProperty_ORGANIZER) != 0) && !iCal::CITIPProcessor::OrganiserIsMe(vevent));
	mOrganiserEdit.ShowWindow(mReadOnly ? SW_SHOW : SW_HIDE);
}

void CNewEventDialog::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do descriptive items
	vevent.EditSummary(mSummary.GetText());
	
	vevent.EditStatus(static_cast<iCal::EStatus_VEvent>(mStatus.GetValue() - IDM_NEWEVENT_STATUS_NONE));

	// Get from each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->GetEvent(vevent);
	}
}

void CNewEventDialog::ChangedSummary()
{
	CUnicodeUtils::SetWindowTextUTF8(*this, mSummary.GetText());
}

void CNewEventDialog::ChangedCalendar()
{
	iCal::CICalendarRef newcal;
	mCalendar.GetCalendar(newcal);
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(newcal);
	GetDlgItem(IDOK)->EnableWindow(!mReadOnly && (cal != NULL) && (!cal->IsReadOnly()));
}

void CNewEventDialog::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	// This will reset state of OK button
	ChangedCalendar();

	mSummary.SetReadOnly(mReadOnly);
	mCalendar.EnableWindow(!mReadOnly);
	mStatus.EnableWindow(!mReadOnly);

	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetReadOnly(mReadOnly);
	}
}

cdstring CNewEventDialog::GetCurrentSummary() const
{
	return mSummary.GetText();
}

void CNewEventDialog::OnOK()
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

void CNewEventDialog::OnCancel()
{
	DoCancel();
	
	// Now do inherited
	CModelessDialog::OnCancel();
}

void CNewEventDialog::OnOrganiserEdit()
{
	SetReadOnly(mOrganiserEdit.GetCheck() == 0);
}

bool CNewEventDialog::DoNewOK()
{
	// Check and get new calendar if different
	iCal::CICalendarRef newcal;
	iCal::CICalendar* new_cal = NULL;
	if (!GetCalendar(0, newcal, new_cal))
		// Return to dialog
		return false;

	// Get updated info
	GetEvent(*mVEvent);
	
	// Look for change to calendar
	if (newcal != mVEvent->GetCalendar())
	{
		// Use new calendar
		mVEvent->SetCalendar(newcal);
		
		// Set the default calendar for next time
		const calstore::CCalendarStoreNode* node = calstore::CCalendarStoreManager::sCalendarStoreManager->GetNode(new_cal);
		if (node != NULL)
			CPreferences::sPrefs->mDefaultCalendar.SetValue(node->GetAccountName());
	}

	// Add to calendar (this will do the display update)
	new_cal->AddNewVEvent(mVEvent);
	CCalendarView::EventChangedAll(mVEvent);
	
	return true;
}

bool CNewEventDialog::DoEditOK()
{
	// Find the original calendar if it still exists
	iCal::CICalendarRef oldcal = mVEvent->GetCalendar();
	iCal::CICalendar* old_cal = iCal::CICalendar::GetICalendar(oldcal);
	if (old_cal == NULL)
	{
		// Inform user of missing calendar
		CErrorDialog::StopAlert(rsrc::GetString("CNewEventDialog::MissingOriginal"));
		
		// Disable OK button
		GetDlgItem(IDOK)->EnableWindow(false);
		
		return false;
	}
	
	// Find the original event if it still exists
	iCal::CICalendarVEvent*	original = static_cast<iCal::CICalendarVEvent*>(old_cal->FindComponent(mVEvent));
	if (original == NULL)
	{
		// Inform user of missing calendar
		CErrorDialog::StopAlert(rsrc::GetString("CNewEventDialog::MissingOriginal"));
		
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

	// Get updated info into original event
	GetEvent(*original);
	
	// Do calendar change
	if (new_cal != NULL)
	{
		// Remove from old calendar (without deleting)
		old_cal->RemoveVEvent(original, false);
		
		// Add to new calendar (without initialising)
		original->SetCalendar(newcal);
		new_cal->AddNewVEvent(original, true);
	}
	
	// Tell it it has changed (i.e. bump sequence, dirty calendar)
	original->Changed();

	CCalendarView::EventChangedAll(mVEvent);
	
	return true;
}

void CNewEventDialog::DoCancel()
{
	// Delete the event which we own and is not going to be used
	delete mVEvent;
	mVEvent = NULL;
}

bool CNewEventDialog::GetCalendar(iCal::CICalendarRef oldcal, iCal::CICalendarRef& newcal, iCal::CICalendar*& new_cal)
{
	mCalendar.GetCalendar(newcal);
	if ((oldcal == 0) || (newcal != oldcal))
	{
		new_cal = iCal::CICalendar::GetICalendar(newcal);
		if (new_cal == NULL)
		{
			// Inform user of missing calendar
			CErrorDialog::StopAlert(rsrc::GetString("CNewEventDialog::MissingNewCalendar"));
			
			// Force calendar popup reset and return to dialog
			mCalendar.Reset();
			mCalendar.SetCalendar(oldcal);
			return false;
		}
	}
	
	return true;
}

void CNewEventDialog::StartNew(const iCal::CICalendarDateTime& dtstart, const iCal::CICalendar* calin)
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

	// Start with an empty new event
	auto_ptr<iCal::CICalendarVEvent> vevent(static_cast<iCal::CICalendarVEvent*>(iCal::CICalendarVEvent::Create(cal->GetRef())));
	
	// Duration is one hour
	iCal::CICalendarDuration duration(60 * 60);
	
	// Set event with initial timing
	vevent->EditTiming(dtstart, duration);

	StartModeless(*vevent.release(), CNewEventDialog::eNew);
}

void CNewEventDialog::StartEdit(const iCal::CICalendarVEvent& original)
{
	// Look for an existinf dialog for this event
	for(set<CNewEventDialog*>::const_iterator iter = sDialogs.begin(); iter != sDialogs.end(); iter++)
	{
		if ((*iter)->ContainsEvent(original))
		{
			FRAMEWORK_WINDOW_TO_TOP(*iter)
			return;
		}
	}

	// Use a copy of the event
	iCal::CICalendarVEvent* vevent = new iCal::CICalendarVEvent(original);
	
	StartModeless(*vevent, CNewEventDialog::eEdit);
}

void CNewEventDialog::StartDuplicate(const iCal::CICalendarVEvent& original)
{
	// Start with an empty new event
	iCal::CICalendarVEvent* vevent = new iCal::CICalendarVEvent(original);
	vevent->Duplicated();
	
	StartModeless(*vevent, CNewEventDialog::eDuplicate);
}

void CNewEventDialog::StartModeless(iCal::CICalendarVEvent& vevent, EModelessAction action)
{
	CNewEventDialog* dlog = new CNewEventDialog;
	dlog->Create(IDD_CALENDAR_NEWEVENT, CSDIFrame::GetAppTopWindow());
	dlog->SetAction(action);
	dlog->SetEvent(vevent);
	dlog->ShowWindow(SW_SHOW);
}
