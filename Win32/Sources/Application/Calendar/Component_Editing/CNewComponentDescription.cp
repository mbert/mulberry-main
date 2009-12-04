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

#include "CNewComponentDescription.h"

// ---------------------------------------------------------------------------
//	CNewComponentDescription														  [public]
/**
	Default constructor */

CNewComponentDescription::CNewComponentDescription() :
	CNewComponentPanel(IDD_CALENDAR_NEW_DESCRIPTION)
{
}


// ---------------------------------------------------------------------------
//	~CNewComponentDescription														  [public]
/**
	Destructor */

CNewComponentDescription::~CNewComponentDescription()
{
}

#pragma mark -

BEGIN_MESSAGE_MAP(CNewComponentDescription, CNewComponentPanel)
	//{{AFX_MSG_MAP(CNewComponentDescription)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CNewComponentDescription::OnInitDialog()
{
	CNewComponentPanel::OnInitDialog();

	// Get UI items
	mLocation.SubclassDlgItem(IDC_CALENDAR_NEW_DESCRIPTION_LOCATION, this);
	mDescription.SubclassDlgItem(IDC_CALENDAR_NEW_DESCRIPTION_DESCRIPTION, this);

	return true;
}

void CNewComponentDescription::SetEvent(const iCal::CICalendarVEvent& vevent)
{
	// Set the relevant fields
	
	mLocation.SetText(vevent.GetLocation());
	mDescription.SetText(vevent.GetDescription());
}

void CNewComponentDescription::SetToDo(const iCal::CICalendarVToDo& vtodo)
{
	// Set the relevant fields
	
	mLocation.SetText(vtodo.GetLocation());
	mDescription.SetText(vtodo.GetDescription());
}

void CNewComponentDescription::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do descriptive items
	{
		cdstring location = mLocation.GetText();
		cdstring description = mDescription.GetText();
		
		vevent.EditDetails(description, location);
	}
}

void CNewComponentDescription::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do descriptive items
	{
		cdstring location = mLocation.GetText();
		cdstring description = mDescription.GetText();
		
		vtodo.EditDetails(description, location);
	}
}

void CNewComponentDescription::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;

	mLocation.SetReadOnly(read_only);
	mDescription.SetReadOnly(read_only);
}
