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
#include "CMulberryApp.h"
#include "CNewComponentDetails.h"
#include "CNewComponentPanel.h"
#include "CPreferences.h"
#include "CTabController.h"
#include "CTextField.h"
#include "CWindowsMenu.h"
#include "CXStringResources.h"

#include "CITIPProcessor.h"

#include "TPopupMenu.h"

#include <JXStaticText.h>
#include <JXTextButton.h>
#include <JXTextCheckbox.h>
#include <JXUpRect.h>
#include <JXWindow.h>

#include <jXGlobals.h>

#include <cassert>

uint32_t CNewComponentDialog::sTitleCounter = 0;
std::set<CNewComponentDialog*> CNewComponentDialog::sDialogs;

// ---------------------------------------------------------------------------
//	CNewComponentDialog														  [public]
/**
	Default constructor */

CNewComponentDialog::CNewComponentDialog(JXDirector* supervisor)
		: CModelessDialog(supervisor)
{
	mCurrentPanel = 0;
	mReadOnly = false;
	sDialogs.insert(this);

	CWindowsMenu::AddWindow(this, false);
}


// ---------------------------------------------------------------------------
//	~CNewComponentDialog														  [public]
/**
	Destructor */

CNewComponentDialog::~CNewComponentDialog()
{
	sDialogs.erase(this);
	CWindowsMenu::RemoveWindow(this);
}

#pragma mark -

void CNewComponentDialog::OnCreate()
{
	// Get UI items
// begin JXLayout

    JXWindow* window = new JXWindow(this, 520,475, "");
    assert( window != NULL );
    SetWindow(window);

    mContainer =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 520,475);
    assert( mContainer != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Summary:", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,10, 70,20);
    assert( obj1 != NULL );

    mSummary =
        new CTextInputField(mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 80,10, 200,20);
    assert( mSummary != NULL );

    mCalendar =
        new CCalendarPopup("", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 300,10, 200,20);
    assert( mCalendar != NULL );

    mTabs =
        new CTabController(mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,70, 500,330);
    assert( mTabs != NULL );

    mOrganiserEdit =
        new JXTextCheckbox("Allow Changes to Organised Event", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 10,410, 235,20);
    assert( mOrganiserEdit != NULL );

    mAttendTxt =
        new JXStaticText("Attend:", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,435, 70,20);
    assert( mAttendTxt != NULL );

    mAttend =
        new HPopupMenu("", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 80,435, 150,20);
    assert( mAttend != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", mContainer,
                    JXWidget::kFixedLeft, JXWidget::kVElastic, 290,435, 85,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("]");

    mOKBtn =
        new JXTextButton("OK", mContainer,
                    JXWidget::kHElastic, JXWidget::kVElastic, 405,435, 85,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout

	window->SetTitle("New Event");
	SetButtons(mOKBtn, mCancelBtn);

	CModelessDialog::OnCreate();
	
	mCalendar->OnCreate();

	mAttend->SetMenuItems("Needs Action %r | Accepted %r | Declined %r | Tentative %r | Delegated %r");
	mAttend->SetValue(1);

	ListenTo(mSummary);
	ListenTo(mCalendar);
	ListenTo(mTabs);
	ListenTo(mOrganiserEdit);
	ListenTo(mAttend);

	// Init controls
	InitPanels();
	DoTab(1);

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
void CNewComponentDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JTextEditor16::kTextChanged))
	{
		if (sender == mSummary)
		{
			ChangedSummary();
			return;
		}
	}
	else if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mCalendar)
		{
			ChangedCalendar();
			return;
		}
		else if (sender == mAttend)
		{
			OnAttendEdit();
			return;
		}
	}
	else if (message.Is(JXRadioGroup::kSelectionChanged))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		if (sender == mTabs)
		{
			DoTab(mTabs->GetSelectedItem());
			return;
		}
	}
	else if (message.Is(JXCheckbox::kPushed))
	{
		if (sender == mOrganiserEdit)
		{
			OnOrganiserEdit();
			return;
		}
	}

	CModelessDialog::Receive(sender, message);
}

void CNewComponentDialog::DoTab(JIndex value)
{
	// Only if different
	if (mCurrentPanel != value - 1)
	{
		mCurrentPanel = value - 1;
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
		GetWindow()->SetTitle(vcomponent.GetSummary());
	}

	// Determine read-only status based on organiser
	SetReadOnly((vcomponent.CountProperty(iCal::cICalProperty_ORGANIZER) != 0) && !iCal::CITIPProcessor::OrganiserIsMe(vcomponent));
	mOrganiserEdit->SetVisible(mReadOnly);

	mAttendTxt->SetVisible(mAttended);
	mAttend->SetVisible(mAttended);
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
		mAttend->SetValue(stat + 1);
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
	GetWindow()->SetTitle(mSummary->GetText());
	CWindowsMenu::RenamedWindow();
}

void CNewComponentDialog::ChangedCalendar()
{
	iCal::CICalendarRef newcal;
	mCalendar->GetCalendar(newcal);
	iCal::CICalendar* cal = iCal::CICalendar::GetICalendar(newcal);
	mOKBtn->SetActive(!mReadOnly && (cal != NULL) && (!cal->IsReadOnly()));
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
	mCalendar->SetActive(!mReadOnly);

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
	SetReadOnly(!mOrganiserEdit->IsChecked());
}

void CNewComponentDialog::OnAttendEdit()
{
	const iCal::CICalendarProperty* attendee = iCal::CITIPProcessor::AttendeeMe(*mComponent);

	cdstring stattxt;
	switch(mAttend->GetValue() - 1)
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

