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

#include "CNewEventTiming.h"
#include "CNewToDoTiming.h"
#include "CTextDisplay.h"
#include "CTextField.h"

#include <JXFlatRect.h>
#include <JXStaticText.h>

#include <cassert>

#pragma mark -

void CNewComponentDetails::OnCreate()
{
	// Get UI items
// begin JXLayout1

    mTimingView =
        new JXFlatRect(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,5, 500,130);
    assert( mTimingView != NULL );

    JXStaticText* obj1 =
        new JXStaticText("Location:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,145, 72,20);
    assert( obj1 != NULL );

    mLocation =
        new CTextInputField(this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 90,145, 400,20);
    assert( mLocation != NULL );

    JXStaticText* obj2 =
        new JXStaticText("Description:", this,
                    JXWidget::kFixedLeft, JXWidget::kFixedTop, 10,175, 77,20);
    assert( obj2 != NULL );

    mDescription =
        new CTextInputDisplay(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 90,175, 400,110);
    assert( mDescription != NULL );

// end JXLayout1
}

void CNewComponentDetails::SetEvent(const iCal::CICalendarVEvent& vevent, const iCal::CICalendarComponentExpanded* expanded)
{
	if (mTimingPanel == NULL)
	{
		// Create the event timing panel
		mTimingPanel = new CNewEventTiming(mTimingView, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 130);
		mTimingPanel->OnCreate();
		mTimingPanel->FitToEnclosure();
	}

	// Set the relevant fields
	
	// Do timing panel
	mTimingPanel->SetEvent(vevent, expanded);
	
	mLocation->SetText(vevent.GetLocation());
	mDescription->SetText(vevent.GetDescription());
}

void CNewComponentDetails::SetToDo(const iCal::CICalendarVToDo& vtodo, const iCal::CICalendarComponentExpanded* expanded)
{
	if (mTimingPanel == NULL)
	{
		// Create the event timing panel
		mTimingPanel = new CNewToDoTiming(mTimingView, JXWidget::kFixedLeft, JXWidget::kFixedTop, 0, 0, 500, 130);
		mTimingPanel->OnCreate();
		mTimingPanel->FitToEnclosure();
	}

	// Set the relevant fields
	
	// Do timing panel
	mTimingPanel->SetToDo(vtodo, expanded);
	
	mLocation->SetText(vtodo.GetLocation());
	mDescription->SetText(vtodo.GetDescription());
}

void CNewComponentDetails::GetEvent(iCal::CICalendarVEvent& vevent)
{
	// Do timing panel
	mTimingPanel->GetEvent(vevent);
	
	// Do descriptive items
	{
		cdstring location = mLocation->GetText();
		cdstring description = mDescription->GetText();

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
		cdstring description = mDescription->GetText();

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
