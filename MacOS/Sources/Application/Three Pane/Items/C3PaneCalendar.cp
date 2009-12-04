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


// Source for C3PaneCalendar class

#include "C3PaneCalendar.h"

#include "CCalendarView.h"
#include "CResources.h"

// __________________________________________________________________________________________________
// C L A S S __ C M A I L B O X L I S T W I N D O W
// __________________________________________________________________________________________________

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneCalendar::C3PaneCalendar()
{
	mDoneInit = false;
	mUseSubstitute = false;
}

// Constructor from stream
C3PaneCalendar::C3PaneCalendar(LStream *inStream)
		: C3PaneViewPanel(inStream)
{
	mDoneInit = false;
	mUseSubstitute = false;
}

// Default destructor
C3PaneCalendar::~C3PaneCalendar()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneCalendar::FinishCreateSelf()
{
	// Do inherited
	C3PaneViewPanel::FinishCreateSelf();

	// Get sub-panes
	mCalendarView = (CCalendarView*) FindPaneByID(paneid_3PaneView);
	
	// Hide it until a mailbox is set
	Hide();
}

CBaseView* C3PaneCalendar::GetBaseView() const
{
	return mCalendarView;
}

void C3PaneCalendar::Init()
{
	// Set the list of mailboxes currently open
	if (!mDoneInit)
	{
		//mCalendarView->SetSubstituteItems(CPreferences::sPrefs->m3PaneOpenMailboxes.GetValue());
		mCalendarView->InitView();
	}
	mDoneInit = true;
}

void C3PaneCalendar::GetOpenItems(cdstrvect& items) const
{
}

void C3PaneCalendar::CloseOpenItems()
{
}

void C3PaneCalendar::SetOpenItems(const cdstrvect& items)
{
}

void C3PaneCalendar::DoClose()
{
	// Get the list of mailboxes currently open
	//cdstrvect opened;
	//mCalendarView->GetSubstituteItems(opened);
	//CPreferences::sPrefs->m3PaneOpenMailboxes.SetValue(opened);

	// Close the server view
	mCalendarView->DoClose();
}

bool C3PaneCalendar::TestCloseAll()
{
	// Get the server view to do the test
	//return mCalendarView->TestCloseAll();
	return false;
}

void C3PaneCalendar::DoCloseAll()
{
	// Close the server view
	//mCalendarView->DoCloseAll();
}

bool C3PaneCalendar::TestCloseOne(unsigned long index)
{
	// Get the server view to do the test
	//return mCalendarView->TestCloseOne(index);
	return false;
}

void C3PaneCalendar::DoCloseOne(unsigned long index)
{
	// Close the server view
	//mCalendarView->DoCloseOne(index);
}

bool C3PaneCalendar::TestCloseOthers(unsigned long index)
{
	// Get the server view to do the test
	//return mCalendarView->TestCloseOthers(index);
	return false;
}

void C3PaneCalendar::DoCloseOthers(unsigned long index)
{
	// Close the server view
	//mCalendarView->DoCloseOthers(index);
}

void C3PaneCalendar::SetUseSubstitute(bool subs)
{
	//mUseSubstitute = subs;
	//mCalendarView->SetUseSubstitute(subs);
}

bool C3PaneCalendar::IsSubstituteLocked(unsigned long index) const
{
	// Test the view
	//return mCalendarView->IsSubstituteLocked(index);
	return false;
}

bool C3PaneCalendar::IsSubstituteDynamic(unsigned long index) const
{
	// Test the view
	//return mCalendarView->IsSubstituteDynamic(index);
	return false;
}

bool C3PaneCalendar::IsSpecified() const
{
	return true;
}

cdstring C3PaneCalendar::GetTitle() const
{
	return mCalendarView->GetTitle();
}

ResIDT C3PaneCalendar::GetIconID() const
{
	return ICNx_3PanePreviewCalendar;
}
