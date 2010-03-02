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


// Source for CNewAttendeeDialog class

#include "CNewAttendeeDialog.h"

#include "CAdminLock.h"
#include "CCalendarAddress.h"
#include "CMulberryApp.h"
#include "CMulberryCommon.h"
#include "CPreferences.h"
#include "CSDIFrame.h"
#include "CUnicodeUtils.h"

#include "CICalendarCalAddressValue.h"

#include <string.h>

// __________________________________________________________________________________________________
// C L A S S __ C C R E A T E M A I L B O X D I A L O G
// __________________________________________________________________________________________________

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Constructor from stream
CNewAttendeeDialog::CNewAttendeeDialog(CWnd* pParent /*=NULL*/)
	: CHelpDialog(CNewAttendeeDialog::IDD, pParent),
	  mRolePopup(true), mStatusPopup(true)
{
	mPropData = NULL;
	mPropListData = NULL;
	mITIPSent = false;
}

// Default destructor
CNewAttendeeDialog::~CNewAttendeeDialog()
{
}

void CNewAttendeeDialog::DoDataExchange(CDataExchange* pDX)
{
	CHelpDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewAttendeeDialog)
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate)
	{
		if (mPropData)
			GetDetails(*mPropData);
		else if (mPropListData)
			GetDetails(*mPropListData);
	}
	else
	{
		InitControls();
		if (mPropData)
			SetDetails(*mPropData);
	}
}


BEGIN_MESSAGE_MAP(CNewAttendeeDialog, CHelpDialog)
	//{{AFX_MSG_MAP(CNewAttendeeDialog)
	ON_COMMAND(IDC_CALENDAR_NEWATTENDEE_STATUS, OnPartStat)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// O T H E R  M E T H O D S ____________________________________________________________________________

void CNewAttendeeDialog::InitControls()
{
	// Get controls
	mNames.SubclassDlgItem(IDC_CALENDAR_NEWATTENDEE_NAMES, this);
	mRolePopup.SubclassDlgItem(IDC_CALENDAR_NEWATTENDEE_ROLE, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mRolePopup.SetMenu(IDR_POPUP_ATTENDEE_ROLE);
	mRolePopup.SetValue(IDM_ROLE_REQUIRED);
	mStatusPopup.SubclassDlgItem(IDC_CALENDAR_NEWATTENDEE_STATUS, this, IDI_POPUPBTN, 0, 0, 0, true, false);
	mStatusPopup.SetMenu(IDR_POPUP_ATTENDEE_STATUS);
	mStatusPopup.SetValue(IDM_STATUS_NEEDS_ACTION);
	mRSVP.SubclassDlgItem(IDC_CALENDAR_NEWATTENDEE_RSVP, this);
}

// Set the details
void CNewAttendeeDialog::SetDetails(const iCal::CICalendarProperty& prop)
{
	// Determine name
	const iCal::CICalendarCalAddressValue* value = prop.GetCalAddressValue();
	if (value != NULL)
	{
		// Get email address
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
		
		mNames.SetText(temp);
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
	mRolePopup.SetValue(role + IDM_ROLE_CHAIR);

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
	mStatusPopup.SetValue(stat + IDM_STATUS_NEEDS_ACTION);

	// Determine RSVP and force check to enable/disable it
	if (prop.HasAttribute(iCal::cICalAttribute_RSVP))
	{
		const cdstring& rsvp = prop.GetAttributeValue(iCal::cICalAttribute_RSVP);
		mRSVP.SetCheck(rsvp == iCal::cICalAttribute_RSVP_TRUE);
		
		// Look for X-Mulberry-Needs-ITIP
		if (prop.HasAttribute(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP))
		{
			const cdstring& itip = prop.GetAttributeValue(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP);
			mITIPSent = (itip == iCal::cICalAttribute_RSVP_FALSE);
		}
	}
	else
	{
		mRSVP.SetCheck(0);
		mITIPSent = false;
	}
	OnPartStat();
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
	std::auto_ptr<CCalendarAddressList> addrs(mNames.GetAddresses());

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
			switch(mRolePopup.GetValue() - IDM_ROLE_CHAIR)
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
			switch(mStatusPopup.GetValue() - IDM_STATUS_NEEDS_ACTION)
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
			if (mRSVP.GetCheck() == 1)
			{
				iCal::CICalendarAttribute attr(iCal::cICalAttribute_RSVP, iCal::cICalAttribute_RSVP_TRUE);
				newprop.AddAttribute(attr);
			}
			
			// Handle iTIP request
			if (mNewItem)
			{
				// New attendee with RSVP => iTIP request needed
				if (mRSVP.GetCheck() == 1)
				{
					iCal::CICalendarAttribute attr2(iCal::cICalAttribute_ATTENDEE_X_NEEDS_ITIP, iCal::cICalAttribute_RSVP_TRUE);
					newprop.AddAttribute(attr2);
				}
			}
			else
			{
				// If RSVP is on
				if (mRSVP.GetCheck() == 1)
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

void CNewAttendeeDialog::OnPartStat()
{
	// RSVP only needed when PARTSTAT=NEEDS-ACTION
	mRSVP.EnableWindow(mStatusPopup.GetValue() == IDM_STATUS_NEEDS_ACTION);
}

bool CNewAttendeeDialog::PoseDialog(iCal::CICalendarProperty& prop)
{
	bool result = false;

	// Create the dialog
	CNewAttendeeDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mPropData = &prop;
	dlog.mNewItem = false;

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		result = true;
	}

	return result;
}

bool CNewAttendeeDialog::PoseDialog(iCal::CICalendarPropertyList& proplist)
{
	bool result = false;

	// Create the dialog
	CNewAttendeeDialog dlog(CSDIFrame::GetAppTopWindow());
	dlog.mPropListData = &proplist;
	dlog.mNewItem = true;

	// Let DialogHandler process events
	if (dlog.DoModal() == IDOK)
	{
		result = true;
	}

	return result;
}
