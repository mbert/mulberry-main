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

#include "CNewComponentDetails.h"

#include "CNewEventTiming.h"
#include "CNewToDoTiming.h"
#include "CUnicodeUtils.h"

#include "CICalendarDuration.h"
#include "CICalendar.h"

#pragma mark ______________________________CNewComponentDetails

// ---------------------------------------------------------------------------
//	CNewComponentDetails														  [public]
/**
	Default constructor */

CNewComponentDetails::CNewComponentDetails() :
	CNewComponentPanel(IDD_CALENDAR_NEW_DETAILS)
{
	mTimingPanel = NULL;
}


// ---------------------------------------------------------------------------
//	~CNewComponentDetails														  [public]
/**
	Destructor */

CNewComponentDetails::~CNewComponentDetails()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentDetails, CNewComponentPanel)
	//{{AFX_MSG_MAP(CNewComponentDetails)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentDetails::OnInitDialog()
{
	CNewComponentPanel::OnInitDialog();

	// Get UI items
	mTimingView.SubclassDlgItem(IDC_CALENDAR_NEW_DETAILS_PANEL, this);

	mLocation.SubclassDlgItem(IDC_CALENDAR_NEW_DETAILS_LOCATION, this);
	mDescription.SubclassDlgItem(IDC_CALENDAR_NEW_DETAILS_DESCRIPTION, this);
	
	return true;
}

void CNewComponentDetails::SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded)
{
	if (mTimingPanel == NULL)
	{
		// Create the event timing panel
		mTimingPanel = new CNewEventTiming;
		mTimingView.AddPanel(mTimingPanel);
		mTimingView.SetPanel(0);
	}

	// Set the relevant fields
	
	// Do timing panel
	mTimingPanel->SetEvent(vevent, expanded);
	
	mLocation.SetText(vevent.GetLocation());
	mDescription.SetText(vevent.GetDescription());
}

void CNewComponentDetails::SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded)
{
	if (mTimingPanel == NULL)
	{
		// Create the to do timing panel
		mTimingPanel = new CNewToDoTiming;
		mTimingView.AddPanel(mTimingPanel);
		mTimingView.SetPanel(0);
	}

	// Set the relevant fields
	
	// Do timing panel
	mTimingPanel->SetToDo(vtodo, expanded);
	
	mLocation.SetText(vtodo.GetLocation());
	mDescription.SetText(vtodo.GetDescription());
}

void CNewComponentDetails::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do timing panel
	mTimingPanel->GetEvent(vevent);
	
	// Do descriptive items
	{
		cdstring location = mLocation.GetText();
		cdstring description = mDescription.GetText();

		vevent.EditDetails(description, location);
	}
}

void CNewComponentDetails::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do timing panel
	mTimingPanel->GetToDo(vtodo);
	
	// Do descriptive items
	{
		cdstring location = mLocation.GetText();
		cdstring description = mDescription.GetText();

		vtodo.EditDetails(description, location);
	}
}

void CNewComponentDetails::GetPeriod(iCal::CICalendarPeriod& period)
{
	// Do timing panel
	static_cast<CNewEventTiming*>(mTimingPanel)->GetPeriod(period);
}

void CNewComponentDetails::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	mTimingPanel->SetReadOnly(read_only);

	mLocation.SetReadOnly(read_only);
	mDescription.SetReadOnly(read_only);
}
