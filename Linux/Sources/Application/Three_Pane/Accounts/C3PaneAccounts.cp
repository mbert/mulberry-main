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


// Source for C3PaneAccounts class

#include "C3PaneAccounts.h"

#include "CAdbkManagerView.h"
#include "CAdminLock.h"
#include "CCalendarStoreView.h"
#include "CCommands.h"
#include "CServerView.h"
#include "CToolbarButton.h"
#include "C3PaneCalendarAccount.h"
#include "C3PaneContactsAccount.h"
#include "C3PaneMailAccount.h"
#include "C3PaneWindow.h"

#include "JXTabs.h"

#include <cassert>

// Static

// Static members

// C O N S T R U C T I O N / D E S T R U C T I O N  M E T H O D S

// Default constructor
C3PaneAccounts::C3PaneAccounts(JXContainer* enclosure,
						 const HSizingOption hSizing, 
						 const VSizingOption vSizing,
						 const JCoordinate x, const JCoordinate y,
						 const JCoordinate w, const JCoordinate h) 
	: C3PaneParentPanel(enclosure, hSizing, vSizing, x, y, w, h)
{
	mMailAccounts = NULL;
	mContactsAccounts = NULL;
	mCalendarAccounts = NULL;
}

// Default destructor
C3PaneAccounts::~C3PaneAccounts()
{
}

// O T H E R  M E T H O D S ____________________________________________________________________________

// Do various bits
void C3PaneAccounts::OnCreate()
{
	// Do inherited
	C3PaneParentPanel::OnCreate();

// begin JXLayout1

    mTabs =
        new JXTabs(this,
                    JXWidget::kHElastic, JXWidget::kFixedTop, 0,0, 275,35);
    assert( mTabs != NULL );

    mViewPane =
        new JXWidgetSet(this,
                    JXWidget::kHElastic, JXWidget::kVElastic, 0,35, 300,165);
    assert( mViewPane != NULL );

    mZoomBtn =
        new CToolbarButton("",this,
                    JXWidget::kFixedRight, JXWidget::kFixedTop, 280,7, 20,20);
    assert( mZoomBtn != NULL );

// end JXLayout1

	// Set tabs
	cdstring s;
	s.FromResource(IDS_3PANEACCOUNTS_TAB_MAIL);
	mTabs->AppendCard(NULL, s);
	s.FromResource(IDS_3PANEACCOUNTS_TAB_CONTACTS);
	mTabs->AppendCard(NULL, s);
	// Remove calendar if disabled by admin
	if (!CAdminLock::sAdminLock.mPreventCalendars)
	{
		s.FromResource(IDS_3PANEACCOUNTS_TAB_CALENDAR);
		mTabs->AppendCard(NULL, s);
	}

	mView = mViewPane;

	mZoomBtn->SetImage(IDI_3PANE_ZOOM, 0);
	mZoomBtn->SetSmallIcon(true);
	mZoomBtn->SetShowIcon(true);
	mZoomBtn->SetShowCaption(false);
	
	ListenTo(mTabs);
	ListenTo(mZoomBtn);

	// Always need these
	MakeMail();
	MakeContacts();
	MakeCalendar();
}

void C3PaneAccounts::MakeMail()
{
	// Read the mail account view resource
	mMailAccounts = new C3PaneMailAccount(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mMailAccounts->OnCreate();
	mMailAccounts->FitToEnclosure(kTrue, kTrue);
	mMailAccounts->Hide();
	
	// Create a toolbar for it
	MakeToolbars(GetServerView());
	
	// Listen to it to get focus info
	GetServerView()->Add_Listener(this);
}

void C3PaneAccounts::MakeContacts()
{
	// Read the contacts account view resource
	mContactsAccounts = new C3PaneContactsAccount(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mContactsAccounts->OnCreate();
	mContactsAccounts->FitToEnclosure(kTrue, kTrue);
	mContactsAccounts->Hide();

	// Create a toolbar for it
	MakeToolbars(GetContactsView());
	
	// Listen to it to get focus info
	GetContactsView()->Add_Listener(this);
}

void C3PaneAccounts::MakeCalendar()
{
	// Don't do if admin locks it out
	if (CAdminLock::sAdminLock.mPreventCalendars)
		return;

	// Read the mail account view resource
	mCalendarAccounts = new C3PaneCalendarAccount(mView, JXWidget::kHElastic, JXWidget::kVElastic, 0, 0, 100, 100);
	mCalendarAccounts->OnCreate();
	mCalendarAccounts->FitToEnclosure(kTrue, kTrue);
	mCalendarAccounts->Hide();
	
	// Create a toolbar for it
	MakeToolbars(GetCalendarStoreView());
	
	// Listen to it to get focus info
	GetCalendarStoreView()->Add_Listener(this);
}

// Respond to clicks in the icon buttons
void C3PaneAccounts::Receive(JBroadcaster* sender, const Message& message)
{
	if (message.Is(JXRadioGroup::kSelectionChanged) && (sender == mTabs))
	{
		JIndex index = dynamic_cast<const JXRadioGroup::SelectionChanged*>(&message)->GetID();
		OnTabs(index);
		return;
	}
	else if (message.Is(JXButton::kPushed))
	{
		if (sender == mZoomBtn)
		{
			OnZoom();
			return;
		}
	}
}

void C3PaneAccounts::OnTabs(JIndex index)
{
	N3Pane::EViewType view;
	switch(index)
	{
	case 1:
	default:
		view = N3Pane::eView_Mailbox;
		break;
	case 2:
		view = N3Pane::eView_Contacts;
		break;
	case 3:
		view = N3Pane::eView_Calendar;
		break;
	}

	// If we are a child of a 3-pane window then tell the 3-pane we are changing
	if (m3PaneWindow)
		// The 3 pane window will call this->SetViewType
		m3PaneWindow->SetViewType(view);
	else
		// Call this->SetViewType explicitly
		SetViewType(view);
}

void C3PaneAccounts::OnZoom()
{
	m3PaneWindow->ObeyCommand(CCommand::eToolbarZoomList, NULL);
}

bool C3PaneAccounts::TestClose()
{
	// Test each pane that exists
	if (mMailAccounts && !mMailAccounts->TestClose())
		return false;
	if (mContactsAccounts && !mContactsAccounts->TestClose())
		return false;
	if (mCalendarAccounts && !mCalendarAccounts->TestClose())
		return false;
	
	return true;
}

void C3PaneAccounts::DoClose()
{
	// Close each pane that exists
	if (mMailAccounts)
		mMailAccounts->DoClose();
	if (mContactsAccounts)
		mContactsAccounts->DoClose();
	if (mCalendarAccounts)
		mCalendarAccounts->DoClose();
}

void C3PaneAccounts::SetViewType(N3Pane::EViewType view)
{
	// Only bother if different
	if (mViewType == view)
		return;

	// Hide current view
	if (mCurrent)
		mCurrent->Hide();

	// Set the pane contents
	mViewType = view;
	JIndex tab_index = 0;
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		// Show the mail account
		if (!mMailAccounts)
			MakeMail();
		mCurrent = mMailAccounts;
		tab_index = 1;
		break;

	case N3Pane::eView_Contacts:
		// Show the contacts account
		if (!mContactsAccounts)
			MakeContacts();
		mCurrent = mContactsAccounts;
		tab_index = 2;
		break;

	case N3Pane::eView_Calendar:
		// Show the calendar account
		if (!mCalendarAccounts)
			MakeCalendar();
		mCurrent = mCalendarAccounts;
		tab_index = 3;
		break;

	case N3Pane::eView_Empty:
	//case N3Pane::eView_IM:
	//case N3Pane::eView_Bookmarks:
	default:;
		// Delete the content of all views
		mCurrent = NULL;
		break;
	}

	// Make sure tabs are in sync
	if (mTabs->GetSelectedItem() != tab_index)
	{
		SetListening(kFalse);
		mTabs->SelectItem(tab_index);
		SetListening(kTrue);
	}

	// Show the new one and always focus on it
	if (mCurrent)
	{
		mCurrent->Show();
		mCurrent->Focus();
	}
}

// Get server view from sub-pane
CServerView* C3PaneAccounts::GetServerView() const
{
	return mMailAccounts ? mMailAccounts->GetServerView() : NULL;
}

// Get server view from sub-pane
CAdbkManagerView* C3PaneAccounts::GetContactsView() const
{
	return mContactsAccounts ? mContactsAccounts->GetAdbkManagerView() : NULL;
}

// Get view from sub-pane
CCalendarStoreView* C3PaneAccounts::GetCalendarStoreView() const
{
	return mCalendarAccounts ? mCalendarAccounts->GetCalendarStoreView() : NULL;
}

// Reset state from prefs
void C3PaneAccounts::ResetState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetServerView())
			GetServerView()->ResetState(true);
		break;

	case N3Pane::eView_Contacts:
		if (GetContactsView())
			GetContactsView()->ResetState(true);
		break;

	case N3Pane::eView_Calendar:
		if (GetCalendarStoreView())
			GetCalendarStoreView()->ResetState(true);
		break;

	default:;
	}
}

// Save state in prefs
void C3PaneAccounts::SaveDefaultState()
{
	switch(mViewType)
	{
	case N3Pane::eView_Mailbox:
		if (GetServerView())
			GetServerView()->SaveDefaultState();
		break;

	case N3Pane::eView_Contacts:
		if (GetContactsView())
			GetContactsView()->SaveDefaultState();
		break;

	case N3Pane::eView_Calendar:
		if (GetCalendarStoreView())
			GetCalendarStoreView()->SaveDefaultState();
		break;

	default:;
	}
}
