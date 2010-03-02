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

#include "CNewComponentDialog.h"

#include "CCalendarPopup.h"
#include "CCalendarView.h"
#include "CErrorDialog.h"
#include "CICalendar.h"
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

uint32_t CNewComponentDialog::sTitleCounter = 0;
std::set<CNewComponentDialog*> CNewComponentDialog::sDialogs;

// ---------------------------------------------------------------------------
//	CNewComponentDialog														  [public]
/**
	Default constructor */

CNewComponentDialog::CNewComponentDialog() :
	CModelessDialog()
{
	mReadOnly = false;
	sDialogs.insert(this);
}


// ---------------------------------------------------------------------------
//	~CNewComponentDialog														  [public]
/**
	Destructor */

CNewComponentDialog::~CNewComponentDialog()
{
	sDialogs.erase(this);
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentDialog, CModelessDialog)
	//{{AFX_MSG_MAP(CNewComponentDialog)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_SUMMARY, OnChangeSummary)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_CALENDARPOPUP, OnChangeCalendar)
	ON_COMMAND(IDC_CALENDAR_NEWEVENT_ORGANISEREDIT, OnOrganiserEdit)
	ON_NOTIFY(TCN_SELCHANGE, IDC_CALENDAR_NEWEVENT_TABS, OnSelChangeTabs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentDialog::OnInitDialog()
{
	CModelessDialog::OnInitDialog();

	// Get UI items
	mSummary.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_SUMMARY, this);
	mSummary.SetAnyCmd(true);

	mCalendar.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_CALENDARPOPUP, this);

	mTabs.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_TABS, this);
	
	mOrganiserEdit.SubclassDlgItem(IDC_CALENDAR_NEWEVENT_ORGANISEREDIT, this);

	// Init controls
	InitPanels();
	mTabs.SetPanel(0);

	return true;
}

void CNewComponentDialog::ListenTo_Message(long msg, void* param)
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

void CNewComponentDialog::OnChangeSummary()
{
	ChangedSummary();
}

void CNewComponentDialog::OnChangeCalendar()
{
	ChangedCalendar();
}

void CNewComponentDialog::OnSelChangeTabs(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: Add your control notification handler code here
	mTabs.SetPanel(mTabs.GetCurSel());

	*pResult = 0;
}

bool CNewComponentDialog::ContainsComponent(const iCal::CICalendarComponentRecur& vcomponent) const
{
	// Does this dialog contain a copy of this component
	return vcomponent.GetMapKey() == mComponent->GetMapKey();
}

void CNewComponentDialog::SetComponent(iCal::CICalendarComponentRecur& vcomponent, const iCal::CICalendarComponentExpanded* expanded)
{
	// Listen to original calendar as we need to cancel if calendar goes away
	iCal::CICalendarRef calref = vcomponent.GetCalendar();
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(calref);
	if (cal)
		cal->Add_Listener(this);
	
	mComponent = &vcomponent;
	mExpanded = expanded;
	mRecurring = expanded ? expanded->Recurring() : false;
	mIsOverride = expanded ? expanded->GetTrueMaster<iCal::CICalendarComponentRecur>() != expanded->GetMaster<iCal::CICalendarComponentRecur>() : false;

	const iCal::CICalendarProperty* attendee = iCal::CITIPProcessor::AttendeeMe(vcomponent);
	mAttended = attendee != NULL;

	// Set the relevant fields
	
	mSummary.SetText(vcomponent.GetSummary());

	mCalendar.SetCalendar(vcomponent.GetCalendar());
	ChangedCalendar();

	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetComponent(vcomponent, expanded);
	}
	
	// Set title to summary if not empty
	if (!vcomponent.GetSummary().empty())
	{
		CUnicodeUtils::SetWindowTextUTF8(this, vcomponent.GetSummary());
	}

	// Determine read-only status based on organiser
	SetReadOnly((vcomponent.CountProperty(iCal::cICalProperty_ORGANIZER) != 0) && !iCal::CITIPProcessor::OrganiserIsMe(vcomponent));
	mOrganiserEdit.ShowWindow(mReadOnly ? SW_SHOW : SW_HIDE);
}

void CNewComponentDialog::GetComponent(iCal::CICalendarComponentRecur& vcomponent)
{
	// Do descriptive items
	vcomponent.EditSummary(mSummary.GetText());
	
	// Get from each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->GetComponent(vcomponent);
	}
}

void CNewComponentDialog::ChangedSummary()
{
	CUnicodeUtils::SetWindowTextUTF8(*this, mSummary.GetText());
}

void CNewComponentDialog::ChangedCalendar()
{
	iCal::CICalendarRef newcal;
	mCalendar.GetCalendar(newcal);
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(newcal);
	GetDlgItem(IDOK)->EnableWindow(!mReadOnly && (cal != NULL) && (!cal->IsReadOnly()));
}

void CNewComponentDialog::ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status)
{
}

void CNewComponentDialog::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	// This will reset state of OK button
	ChangedCalendar();

	mSummary.SetReadOnly(mReadOnly);
	mCalendar.EnableWindow(!mReadOnly);

	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetReadOnly(mReadOnly);
	}
}

cdstring CNewComponentDialog::GetCurrentSummary() const
{
	return mSummary.GetText();
}

iCal::CICalendarRef CNewComponentDialog::GetCurrentCalendar() const
{
	iCal::CICalendarRef newcal;
	mCalendar.GetCalendar(newcal);
	return newcal;
}

void CNewComponentDialog::GetCurrentPeriod(iCal::CICalendarPeriod& period) const
{
	CNewComponentDetails* timing = static_cast<CNewComponentDetails*>(mPanels.front());
	timing->GetPeriod(period);
}

const CNewTimingPanel* CNewComponentDialog::GetTimingPanel() const
{
	const CNewComponentDetails* timing = static_cast<const CNewComponentDetails*>(mPanels.front());
	return timing->GetTimingPanel();
}

void CNewComponentDialog::OnOK()
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

void CNewComponentDialog::OnCancel()
{
	DoCancel();
	
	// Now do inherited
	CModelessDialog::OnCancel();
}

void CNewComponentDialog::OnOrganiserEdit()
{
	SetReadOnly(mOrganiserEdit.GetCheck() == 0);
}

bool CNewComponentDialog::GetCalendar(iCal::CICalendarRef oldcal, iCal::CICalendarRef& newcal, iCal::CICalendar*& new_cal)
{
	mCalendar.GetCalendar(newcal);
	if ((oldcal == 0) || (newcal != oldcal))
	{
		new_cal = iCal::CICalendar::GetICalendar(newcal);
		if (new_cal == NULL)
		{
			// Inform user of missing calendar
			CErrorDialog::StopAlert(rsrc::GetString("CNewComponentDialog::MissingNewCalendar"));
			
			// Force calendar popup reset and return to dialog
			mCalendar.Reset();
			mCalendar.SetCalendar(oldcal);
			return false;
		}
	}
	
	return true;
}
