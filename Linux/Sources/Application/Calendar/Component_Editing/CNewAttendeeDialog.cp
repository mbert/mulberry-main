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


// Source for CNewAttendeeDialog class

#include "CNewAttendeeDialog.h"

#include "CCalendarAddressDisplay.h"
#include "CAddressList.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CURL.h"

#include "CICalendarCalAddressValue.h"

#include <JXWindow.h>
#include <JXUpRect.h>
#include <JXStaticText.h>
#include <JXTextCheckbox.h>
#include <JXTextButton.h>
#include "TPopupMenu.h"

#include <jXGlobals.h>

#include <cassert>
#include <memory>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CNewAttendeeDialog::CNewAttendeeDialog(JXDirector* supervisor)
		: CDialogDirector(supervisor)
{
	mITIPSent = false;
}

// Default destructor
CNewAttendeeDialog::~CNewAttendeeDialog()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Get details of sub-panes
void CNewAttendeeDialog::OnCreate()
{
	// Get UI items
// begin JXLayout

    JXWindow* window = new JXWindow(this, 440,320, "");
    assert( window != NULL );
    SetWindow(window);

    JXUpRect* obj1 =
        new JXUpRect(window,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,0, 440,320);
    assert( obj1 != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Attendees:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,10, 75,20);
    assert( obj2 != NULL );

    mNames =
        new CCalendarAddressDisplay(obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 10,30, 420,150);
    assert( mNames != NULL );

    JXStaticText* obj3 =
        new JXStaticText("Role:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,195, 55,20);
    assert( obj3 != NULL );

    mRolePopup =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,190, 180,25);
    assert( mRolePopup != NULL );

    JXStaticText* obj4 =
        new JXStaticText("Status:", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,230, 55,20);
    assert( obj4 != NULL );

    mStatusPopup =
        new HPopupMenu("", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 75,225, 180,25);
    assert( mStatusPopup != NULL );

    mRSVP =
        new JXTextCheckbox("RSVP Required", obj1,
                    JXWidget::kHElastic, JXWidget::kVElastic, 15,260, 155,20);
    assert( mRSVP != NULL );

    mCancelBtn =
        new JXTextButton("Cancel", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 225,285, 85,25);
    assert( mCancelBtn != NULL );
    mCancelBtn->SetShortcuts("^[");

    mOKBtn =
        new JXTextButton("OK", obj1,
                    JXWidget::kFixedRight, JXWidget::kFixedBottom, 330,285, 85,25);
    assert( mOKBtn != NULL );
    mOKBtn->SetShortcuts("^M");

// end JXLayout
	window->SetTitle("Attendees");
	SetButtons(mOKBtn, mCancelBtn);

	mRolePopup->SetMenuItems("Chair %r | Participation Required %r | Participation Optional %r | Not Participating %r");
	mRolePopup->SetValue(2);

	mStatusPopup->SetMenuItems("Needs Action %r | Accepted %r | Declined %r | Tentative %r | Delegated %r");
	mStatusPopup->SetValue(1);

	// Link controls to this window
	ListenTo(mStatusPopup);

}

// Handle controls
void CNewAttendeeDialog::Receive(JBroadcaster* sender, const Message& message)
{
	if(message.Is(JXMenu::kItemSelected))
	{
		const JXMenu::ItemSelected* is = dynamic_cast<const JXMenu::ItemSelected*>(&message);
		if (sender == mStatusPopup)
		{
			mRSVP->SetActive(is->GetIndex() - 1 == iCal::ePartStat_NeedsAction);
			return;
		}
	}
	
	CDialogDirector::Receive(sender, message);
}

// Set the details
void CNewAttendeeDialog::SetDetails(const iCal::CICalendarProperty& prop)
{
	// Determine name
	const iCal::CICalendarCalAddressValue* value = prop.GetCalAddressValue();
	if (value != NULL)
	{
		// Get email address
		cdstring temp = "<";
		temp += value->GetValue();
		temp += ">";

		// Get CN
		if (prop.HasAttribute(iCal::cICalAttribute_CN))
		{
			cdstring cntxt = prop.GetAttributeValue(iCal::cICalAttribute_CN);
			if (cntxt.length() > 0)
			{
				cntxt += " ";
				cntxt += temp;
				temp = cntxt;
			}

		}
		
		mNames->SetText(temp);
	}
	
	// Determine role
	unsigned long role = 0;
	if (prop.HasAttribute(iCal::cICalAttribute_ROLE))
	{
		const cdstring& roletxt = prop.GetAttributeValue(iCal::cICalAttribute_ROLE);
		if (roletxt.compare(iCal::cICalAttribute_ROLE_CHAIR, true) == 0)
			role = iCal::ePartRole_Chair;
		else if (roletxt.compare(iCal::cICalAttribute_ROLE_REQ_PART, true) == 0)
			role = iCal::ePartRole_Required;
		else if (roletxt.compare(iCal::cICalAttribute_ROLE_OPT_PART, true) == 0)
			role = iCal::ePartRole_Optional;
		else if (roletxt.compare(iCal::cICalAttribute_ROLE_NON_PART, true) == 0)
			role = iCal::ePartRole_Non;
		else
			role = iCal::ePartRole_Required;
	}
	else
		// 2445 default
		role = iCal::ePartRole_Required;
	mRolePopup->SetValue(role + 1);

	// Determine status
	unsigned long stat = 0;
	if (prop.HasAttribute(iCal::cICalAttribute_PARTSTAT))
	{
		const cdstring& partstat = prop.GetAttributeValue(iCal::cICalAttribute_PARTSTAT);
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
	mStatusPopup->SetValue(stat + 1);
	mRSVP->SetActive(stat == iCal::ePartStat_NeedsAction);


	// Determine RSVP
	if (prop.HasAttribute(iCal::cICalAttribute_RSVP))
	{
		const cdstring& rsvp = prop.GetAttributeValue(iCal::cICalAttribute_RSVP);
		mRSVP->SetState(rsvp == iCal::cICalAttribute_RSVP_TRUE);
		
		// Look for X-Mulberry-Needs-ITIP
		if (prop.HasAttribute(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP))
		{
			const cdstring& itip = prop.GetAttributeValue(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP);
			mITIPSent = (itip == iCal::cICalAttribute_RSVP_FALSE);
		}
	}
	else
	{
		mRSVP->SetState(false);
		mITIPSent = false;
	}
}

// Get the details
void CNewAttendeeDialog::GetDetails(iCal::CICalendarProperty& prop)
{
	iCal::CICalendarPropertyList temp;
	GetDetails(temp);

	if (temp.size() > 0)
		prop = temp.front();
}

// Get the details
void CNewAttendeeDialog::GetDetails(iCal::CICalendarPropertyList& proplist)
{
	std::auto_ptr<CCalendarAddressList> addrs(mNames->GetAddresses());

	for(CCalendarAddressList::const_iterator iter = addrs->begin(); iter != addrs->end(); iter++)
	{
		// Get calendar user address
		cdstring addr = (*iter)->GetCalendarAddress();

		// Create a new property
		iCal::CICalendarProperty newprop(iCal::cICalProperty_ATTENDEE, addr, iCal::CICalendarValue::eValueType_CalAddress);

		// Add attributes
		{
			// CN
			cdstring cntxt = (*iter)->GetName();
			if (cntxt.length() != 0)
			{
				iCal::CICalendarAttribute attr(iCal::cICalAttribute_CN, cntxt);
				newprop.AddAttribute(attr);
			}
		}
		{
			// ROLE
			cdstring roletxt;
			switch(mRolePopup->GetValue() - 1)
			{
			case iCal::ePartRole_Chair:
				roletxt = iCal::cICalAttribute_ROLE_CHAIR;
				break;
			case iCal::ePartRole_Required:
				roletxt = iCal::cICalAttribute_ROLE_REQ_PART;
				break;
			case iCal::ePartRole_Optional:
				roletxt = iCal::cICalAttribute_ROLE_OPT_PART;
				break;
			case iCal::ePartRole_Non:
				roletxt = iCal::cICalAttribute_ROLE_NON_PART;
				break;
			default:;				
			}
			if (roletxt.length() != 0)
			{
				iCal::CICalendarAttribute attr(iCal::cICalAttribute_ROLE, roletxt);
				newprop.AddAttribute(attr);
			}
		}
		bool needs_action = false;
		{
			// PARTSTAT
			cdstring stattxt;
			switch(mStatusPopup->GetValue() - 1)
			{
			case iCal::ePartStat_NeedsAction:
				stattxt = iCal::cICalAttribute_PARTSTAT_NEEDSACTION;
				needs_action = true;
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
			if (stattxt.length() != 0)
			{
				iCal::CICalendarAttribute attr(iCal::cICalAttribute_PARTSTAT, stattxt);
				newprop.AddAttribute(attr);
			}
		}
		
		// RSVP etc only needed if attendee has action to take
		if (needs_action)
		{
			// RSVP
			if (mRSVP->IsChecked())
			{
				iCal::CICalendarAttribute attr(iCal::cICalAttribute_RSVP, iCal::cICalAttribute_RSVP_TRUE);
				newprop.AddAttribute(attr);
			}
			
			// Handle iTIP request
			if (mNewItem)
			{
				// New attendee with RSVP => iTIP request needed
				if (mRSVP->IsChecked())
				{
					iCal::CICalendarAttribute attr2(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP, iCal::cICalAttribute_RSVP_TRUE);
					newprop.AddAttribute(attr2);
				}
			}
			else
			{
				// If RSVP is on
				if (mRSVP->IsChecked())
				{
					// Use iTIP attribute from existing item
					iCal::CICalendarAttribute attr2(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP, mITIPSent ? iCal::cICalAttribute_RSVP_FALSE : iCal::cICalAttribute_RSVP_TRUE);
					newprop.AddAttribute(attr2);
				}
			}
		}
		
		proplist.push_back(newprop);
	}
}

bool CNewAttendeeDialog::PoseDialog(iCal::CICalendarProperty& prop)
{
	bool result = false;

	// Create the dialog
	CNewAttendeeDialog* dlog = new CNewAttendeeDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->SetDetails(prop);
	dlog->mNewItem = false;

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(prop);
		dlog->Close();
		result = true;
	}

	return result;
}

bool CNewAttendeeDialog::PoseDialog(iCal::CICalendarPropertyList& proplist)
{
	bool result = false;

	// Create the dialog
	CNewAttendeeDialog* dlog = new CNewAttendeeDialog(JXGetApplication());
	dlog->OnCreate();
	dlog->mNewItem = true;

	// Test for OK
	if (dlog->DoModal(false) == kDialogClosed_OK)
	{
		dlog->GetDetails(proplist);
		dlog->Close();
		result = true;
	}

	return result;
}
