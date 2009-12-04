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

#include "CNewComponentDialog.h"

#include "CCalendarPopup.h"
#include "CCalendarView.h"
#include "CErrorDialog.h"
#include "CICalendar.h"
#include "CNewComponentDetails.h"
#include "CNewComponentPanel.h"
#include "CStaticText.h"
#include "CTextFieldX.h"
#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include "CITIPProcessor.h"

#include <LCheckbox.h>
#include <LPopupButton.h>
#include <LTabsControl.h>
#include "MyCFString.h"

uint32_t CNewComponentDialog::sTitleCounter = 0;
std::set<CNewComponentDialog*> CNewComponentDialog::sDialogs;

// ---------------------------------------------------------------------------
//	CNewComponentDialog														  [public]
/**
 Default constructor */

CNewComponentDialog::CNewComponentDialog(LStream *inStream)
: CModelessDialog(inStream)
{
	mCurrentPanel = 0;
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

void CNewComponentDialog::FinishCreateSelf()
{
	CModelessDialog::FinishCreateSelf();
	
	// Get UI items
	mSummary = dynamic_cast<CTextFieldX*>(FindPaneByID(eSummary_ID));
	mSummary->AddListener(this);
	mCalendar = dynamic_cast<CCalendarPopup*>(FindPaneByID(eCalendar_ID));
	
	mTabs = dynamic_cast<LTabsControl*>(FindPaneByID(eTabs_ID));
	mTabView = dynamic_cast<LView*>(FindPaneByID(eTabView_ID));
	
	mOrganiserEdit = dynamic_cast<LCheckBox*>(FindPaneByID(eChangeOrganised_ID));
	mAttendText = dynamic_cast<CStaticText*>(FindPaneByID(eAttendText_ID));
	mAttendPopup = dynamic_cast<LPopupButton*>(FindPaneByID(eAttend_ID));
	
	// Init controls
	InitPanels();
	DoTab(1);
	mPanels[mCurrentPanel]->SetVisible(true);
	
	// Focus on summary
}

bool CNewComponentDialog::ContainsComponent(const iCal::CICalendarComponentRecur& vcomponent) const
{
	// Does this dialog contain a copy of this component
	return vcomponent.GetMapKey() == mComponent->GetMapKey();
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

// Handle controls
void CNewComponentDialog::ListenToMessage(
										  MessageT	inMessage,
										  void		*ioParam)
{
	switch (inMessage)
	{
		case eTabs_ID:
			DoTab(mTabs->GetValue());
			break;
			
		case eSummary_ID:
			ChangedSummary();
			break;
			
		case eCalendar_ID:
			ChangedCalendar();
			break;
			
		case eChangeOrganised_ID:
			OnOrganiserEdit();
			break;
			
		case eAttend_ID:
			OnAttendEdit();
			break;
			
		default:
			CModelessDialog::ListenToMessage(inMessage, ioParam);
			break;
	}
}

void CNewComponentDialog::DoTab(UInt32 value)
{
	// Only if different
	if (mCurrentPanel != value - 1)
	{
		mPanels[mCurrentPanel]->SetVisible(false);
		mCurrentPanel = value - 1;
		mPanels[mCurrentPanel]->SetVisible(true);
	}
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
	
	mSummary->SetText(vcomponent.GetSummary());
	
	mCalendar->SetCalendar(vcomponent.GetCalendar());
	ChangedCalendar();
	
	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetComponent(vcomponent, expanded);
	}
	
	// Set title to summary if not empty
	if (!vcomponent.GetSummary().empty())
	{
		MyCFString cftitle(vcomponent.GetSummary(), kCFStringEncodingUTF8);
		SetCFDescriptor(cftitle);
	}
	
	// Determine read-only status based on organiser
	SetReadOnly((vcomponent.CountProperty(iCal::cICalProperty_ORGANIZER) != 0) && !iCal::CITIPProcessor::OrganiserIsMe(vcomponent));
	mOrganiserEdit->SetVisible(mReadOnly);
	
	mAttendText->SetVisible(mAttended);
	mAttendPopup->SetVisible(mAttended);
	if (mAttended)
	{
		unsigned long stat = 0;
		if (attendee->HasAttribute(iCal::cICalAttribute_PARTSTAT))
		{
			const cdstring& partstat = attendee->GetAttributeValue(iCal::cICalAttribute_PARTSTAT);
			if (partstat.compare(iCal::cICalAttribute_PARTSTAT_NEEDSACTION, true) == 0)
				stat = iCal::ePartStat_NeedsAction;
			else if (partstat.compare(iCal::cICalAttribute_PARTSTAT_ACCEPTED, true) == 0)
				stat = iCal::ePartStat_Accepted;
			else if (partstat.compare(iCal::cICalAttribute_PARTSTAT_DECLINED, true) == 0)
				stat = iCal::ePartStat_Declined;
			else if (partstat.compare(iCal::cICalAttribute_PARTSTAT_TENTATIVE, true) == 0)
				stat = iCal::ePartStat_Tentative;
			else if (partstat.compare(iCal::cICalAttribute_PARTSTAT_DELEGATED, true) == 0)
				stat = iCal::ePartStat_Delegated;
			else if (partstat.compare(iCal::cICalAttribute_PARTSTAT_COMPLETED, true) == 0)
				stat = iCal::ePartStat_Completed;
			else if (partstat.compare(iCal::cICalAttribute_PARTSTAT_INPROCESS, true) == 0)
				stat = iCal::ePartStat_InProcess;
			else
				stat = iCal::ePartStat_NeedsAction;
		}
		else
			// 2445 default
			stat = iCal::ePartStat_NeedsAction;
		mAttendPopup->SetValue(stat + 1);
	}
}

void CNewComponentDialog::GetComponent(iCal::CICalendarComponentRecur& vcomponent)
{
	// Do descriptive items
	vcomponent.EditSummary(mSummary->GetText());
	
	// Get from each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->GetComponent(vcomponent);
	}
}

void CNewComponentDialog::ChangedSummary()
{
	MyCFString cftitle(mSummary->GetText(), kCFStringEncodingUTF8);
	SetCFDescriptor(cftitle);
	CWindowsMenu::RenamedWindow();
}

void CNewComponentDialog::ChangedCalendar()
{
	iCal::CICalendarRef newcal;
	mCalendar->GetCalendar(newcal);
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(newcal);
	FindPaneByID(eOK_ID)->SetEnabled((!mReadOnly || mAttended) && (cal != NULL) && (!cal->IsReadOnly()));
}

void CNewComponentDialog::ChangedMyStatus(const iCal::CICalendarProperty& attendee, const cdstring& new_status)
{
}

void CNewComponentDialog::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;
	
	// This will reset state of OK button
	ChangedCalendar();
	
	mSummary->SetReadOnly(mReadOnly);
	mCalendar->SetEnabled(!mReadOnly);
	
	// Set in each panel
	for(CNewComponentPanelList::iterator iter = mPanels.begin(); iter != mPanels.end(); iter++)
	{
		(*iter)->SetReadOnly(mReadOnly);
	}
}

cdstring CNewComponentDialog::GetCurrentSummary() const
{
	return mSummary->GetText();
}

iCal::CICalendarRef CNewComponentDialog::GetCurrentCalendar() const
{
	iCal::CICalendarRef newcal;
	mCalendar->GetCalendar(newcal);
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
	SetReadOnly(mOrganiserEdit->GetValue() == 0);
}

void CNewComponentDialog::OnAttendEdit()
{
	const iCal::CICalendarProperty* attendee = iCal::CITIPProcessor::AttendeeMe(*mComponent);

	cdstring stattxt;
	switch(mAttendPopup->GetValue() - 1)
	{
		case iCal::ePartStat_NeedsAction:
			stattxt = iCal::cICalAttribute_PARTSTAT_NEEDSACTION;
			break;
		case iCal::ePartStat_Accepted:
			stattxt = iCal::cICalAttribute_PARTSTAT_ACCEPTED;
			break;
		case iCal::ePartStat_Declined:
			stattxt = iCal::cICalAttribute_PARTSTAT_DECLINED;
			break;
		case iCal::ePartStat_Tentative:
			stattxt = iCal::cICalAttribute_PARTSTAT_TENTATIVE;
			break;
		case iCal::ePartStat_Delegated:
			stattxt = iCal::cICalAttribute_PARTSTAT_DELEGATED;
			break;
		case iCal::ePartStat_Completed:
			stattxt = iCal::cICalAttribute_PARTSTAT_COMPLETED;
			break;
		case iCal::ePartStat_InProcess:
			stattxt = iCal::cICalAttribute_PARTSTAT_INPROCESS;
			break;
		default:;				
	}
	
	ChangedMyStatus(*attendee, stattxt);
}

bool CNewComponentDialog::GetCalendar(iCal::CICalendarRef oldcal, iCal::CICalendarRef& newcal, iCal::CICalendar*& new_cal)
{
	mCalendar->GetCalendar(newcal);
	if ((oldcal == 0) || (newcal != oldcal))
	{
		new_cal = iCal::CICalendar::GetICalendar(newcal);
		if (new_cal == NULL)
		{
			// Inform user of missing calendar
			CErrorDialog::StopAlert(rsrc::GetString("CNewComponentDialog::MissingNewCalendar"));
			
			// Force calendar popup reset and return to dialog
			mCalendar->Reset();
			mCalendar->SetCalendar(oldcal);
			return false;
		}
	}
	
	return true;
}
