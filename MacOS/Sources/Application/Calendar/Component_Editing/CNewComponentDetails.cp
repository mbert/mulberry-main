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

#include "CNewComponentDetails.h"

//#include "CDateTimeZoneSelect.h"
#include "CNewEventTiming.h"
#include "CNewToDoTiming.h"
#include "CPreferences.h"

//#include "CICalendarDuration.h"
//#include "CICalendar.h"

#include "CTextDisplay.h"
#include "CTextFieldX.h"
//#include "MyCFString.h"

// ---------------------------------------------------------------------------
//	CNewComponentDetails														  [public]
/**
 Default constructor */

CNewComponentDetails::CNewComponentDetails(LStream *inStream) :
CNewComponentPanel(inStream)
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

void CNewComponentDetails::FinishCreateSelf()
{
	// Get UI items
	mTimingView = dynamic_cast<LView*>(FindPaneByID(eTimingView_ID));
	
	mLocation = dynamic_cast<CTextFieldX*>(FindPaneByID(eLocation_ID));
	mDescriptionFocus = FindPaneByID(eDescriptionFocus_ID);
	mDescription = dynamic_cast<CTextDisplay*>(FindPaneByID(eDescription_ID));
	mUID = dynamic_cast<CTextFieldX*>(FindPaneByID(eUID_ID));
    if (CPreferences::sPrefs->mShowUID.Value())
    {
        mUID->SetVisible(true);
        mUID->SetReadOnly(true);
        mDescriptionFocus->ResizeFrameBy(0, -30, false);
    }
}

void CNewComponentDetails::SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded)
{
	if (mTimingPanel == NULL)
	{
		// Find super commander in view chain
		LView* parent = GetSuperView();
		while((parent != NULL) && (dynamic_cast<LDialogBox*>(parent) == NULL))
			parent = parent->GetSuperView();
		LDialogBox* dlog = dynamic_cast<LDialogBox*>(parent);
		
		// Get the tab group
		LCommander* defCommander = NULL;
		if (dlog)
			dlog->GetSubCommanders().FetchItemAt(1, defCommander);
		
		// Load each panel for the tabs
		mTimingPanel = static_cast<CNewEventTiming*>(UReanimator::CreateView(CNewEventTiming::pane_ID, mTimingView, defCommander));
		mTimingView->ExpandSubPane(mTimingPanel, true, true);
		mTimingPanel->SetVisible(true);
	}
	
	// Set the relevant fields
	
	// Do timing panel
	mTimingPanel->SetEvent(vevent, expanded);
	
	// Set details
	mLocation->SetText(vevent.GetLocation());
	mDescription->SetText(vevent.GetDescription());
    mUID->SetText(vevent.GetUID());
}

void CNewComponentDetails::SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded)
{
	if (mTimingPanel == NULL)
	{
		// Find super commander in view chain
		LView* parent = GetSuperView();
		while((parent != NULL) && (dynamic_cast<LDialogBox*>(parent) == NULL))
			parent = parent->GetSuperView();
		LDialogBox* dlog = dynamic_cast<LDialogBox*>(parent);
		
		// Get the tab group
		LCommander* defCommander = NULL;
		if (dlog)
			dlog->GetSubCommanders().FetchItemAt(1, defCommander);
		
		// Load each panel for the tabs
		mTimingPanel = static_cast<CNewToDoTiming*>(UReanimator::CreateView(CNewToDoTiming::pane_ID, mTimingView, defCommander));
		mTimingView->ExpandSubPane(mTimingPanel, true, true);
		mTimingPanel->SetVisible(true);
	}
	
	// Set the relevant fields
	
	// Do timing panel
	mTimingPanel->SetToDo(vtodo, expanded);
	
	// Set recurrence
	mLocation->SetText(vtodo.GetLocation());
	mDescription->SetText(vtodo.GetDescription());
    mUID->SetText(vtodo.GetUID());
}

void CNewComponentDetails::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do timing panel
	mTimingPanel->GetEvent(vevent);
	
	// Do descriptive items
	{
		cdstring location = mLocation->GetText();
		cdstring description;
		mDescription->GetText(description);
		
		vevent.EditDetails(description, location);
	}
}

void CNewComponentDetails::GetPeriod(iCal::CICalendarPeriod& period)
{
	// Do timing panel
	static_cast<CNewEventTiming*>(mTimingPanel)->GetPeriod(period);
}

void CNewComponentDetails::GetToDo(iCal::CICalendarVToDo& vtodo)
{
	// Do timing panel
	mTimingPanel->GetToDo(vtodo);
	
	// Do descriptive items
	{
		cdstring location = mLocation->GetText();
		cdstring description;
		mDescription->GetText(description);
		
		vtodo.EditDetails(description, location);
	}
}

void CNewComponentDetails::SetReadOnly(bool read_only)
{
	mReadOnly = read_only;
	
	mTimingPanel->SetReadOnly(read_only);
	
	mLocation->SetReadOnly(read_only);
	mDescription->SetReadOnly(read_only);
}
